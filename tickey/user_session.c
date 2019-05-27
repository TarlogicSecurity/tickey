#include "user_session.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <utmpx.h>
#include <paths.h>
#include <pwd.h>


static uid_t get_user_id(const char *name){
	struct passwd *user_info = getpwnam(name);
	if(user_info == NULL){
		return -1;
	}
	return user_info->pw_uid;
}

static user_session_t* new_logged_user(uid_t uid, const char *username, const char *terminal){
    
    user_session_t *user = calloc(sizeof(user_session_t), 1);
    if(user == NULL){
        return NULL;
    }
    
    user->uid = uid;
    memcpy(user->username, username, UT_NAMESIZE);
    memcpy(user->terminal, terminal, UT_LINESIZE);
    
    return user;
}

void release_users_sessions_list(users_sessions_list_t ** users){
    if(*users){
        if((*users)->users){
            size_t i = 0;
            for(i = 0; i < (*users)->length; i++){
                if(((*users)->users[i]) != NULL){
                    free((*users)->users[i]); (*users)->users[i] = NULL;
                }
            }
            free((*users)->users); (*users)->users = NULL;
        }
        free(*users); *users = NULL;
    }
}

static users_sessions_list_t* new_logged_users_list(){
    return (users_sessions_list_t*)calloc(sizeof(users_sessions_list_t),1);
}

static int add_logged_user(users_sessions_list_t* users, user_session_t* user){
    user_session_t ** aux_ptr = realloc(users->users, 
            (users->length + 1) * sizeof(user_session_t*));
    if(aux_ptr == NULL){
        return -1;
    }
    
    users->users = aux_ptr;
    users->users[users->length] = user;
    users->length++;
    
    return 0;
}


users_sessions_list_t * users_sessions(){
    users_sessions_list_t * users = new_logged_users_list();
    
	setutxent();
	struct utmpx *ut = NULL;
	while((ut = getutxent()) != NULL){
		if(ut->ut_type != USER_PROCESS){
            continue;
        }
        uid_t uid = get_user_id(ut->ut_user);
        if(uid == -1){
            continue;
        }
        user_session_t *user = new_logged_user(uid, ut->ut_user, ut->ut_line);
        if(user == NULL){
            continue;
        }
        
        if(add_logged_user(users, user) == -1){
            free(user); user = NULL;
        }
	}
	endutxent();

	return users;
}

