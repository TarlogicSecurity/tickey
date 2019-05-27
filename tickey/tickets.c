#include "tickets.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "keyctl.h"

ticket_t * get_ticket(const uint32_t ticket_id){

	size_t buffer_size = keyctl_read_size(ticket_id);
	if(buffer_size == 0){
		return NULL;
	}

	ticket_t * ticket = (ticket_t*)malloc(sizeof(ticket_t) + buffer_size);
	if(ticket == NULL){
		return NULL;
	}

	ticket->size = buffer_size;
	buffer_size = keyctl_read(ticket_id, ticket->data, ticket->size);
	if(buffer_size > ticket->size){
		free(ticket);
		ticket = NULL;
	}

	return ticket;	
}

char * get_ticket_name(const uint32_t ticket_id){
	key_info_t * keyring_info = get_key_info(ticket_id);
	if(keyring_info == NULL){
		return NULL;
	}
	
	char * ticket_name = strdup(keyring_info->description);

	free(keyring_info);
	return ticket_name;
}

tickets_t * get_tickets(const key_ids_t *tickets_ids){
	
	tickets_t * tickets = calloc(sizeof(tickets_t), 1);
	if(tickets == NULL){
		return NULL;
	}
	tickets->length = 0;

	size_t i = 0;
	for(i = 0; i < tickets_ids->length; i++){
		uint32_t ticket_id = tickets_ids->ids[i];
		
		char * ticket_name = get_ticket_name(ticket_id);
		if(ticket_name == NULL){
			continue;
		}

		if (strcmp(ticket_name, "__krb5_princ__") == 0){
			tickets->krb5_princ = get_ticket(ticket_id);
			if(tickets->krb5_princ == NULL){
				destroy_tickets(&tickets);
				return NULL;
			}
		}
		else{
			ticket_t * ticket = get_ticket(ticket_id);
			
			ticket_t ** aux_tickets = NULL;
			aux_tickets = (ticket_t**)realloc(tickets->tickets, (tickets->length + 1) * sizeof(ticket_t*));
			if(aux_tickets == NULL){
				free(ticket); ticket = NULL;
				destroy_tickets(&tickets);
				return NULL;
			}
			
			tickets->tickets = aux_tickets;
			tickets->tickets[tickets->length] = ticket;
			tickets->length++;
		}
		free(ticket_name); ticket_name = NULL;
	}

	return tickets;
}

key_info_t * get_ticket_key_info_from_persistent(uid_t uid){
	key_info_t * key_info = NULL;

	key_id_t persistent_key_id = keyctl_get_persistent(uid, KEY_SPEC_PROCESS_KEYRING);
	if(persistent_key_id == -1){
		return NULL;
	}

	key_id_t user_krb_key = keyctl_search(persistent_key_id, "keyring", "_krb", KEY_SPEC_PROCESS_KEYRING);
	if(user_krb_key == -1){
		return NULL;
	}	
	
	key_ids_t * sub_keys = get_subkeys(user_krb_key);
	if(sub_keys == NULL){
		return NULL;
	}

	size_t i = 0;
	for(i = 0; i < sub_keys->length; i++){
		key_info = get_key_info(sub_keys->ids[i]);
		
		if(strcmp(key_info->type, "keyring") == 0){
			break;
		}
		free(key_info); key_info = NULL;
	}

	free(sub_keys);
	
	return key_info;
}
tickets_t * _recolect_all_tickets_from_root_key(key_id_t root_key_id){
	key_ids_t * tickets_ids = get_subkeys(root_key_id);
	if(tickets_ids == NULL){
		return NULL;
	}

	tickets_t * tickets = get_tickets(tickets_ids);
	free(tickets_ids); tickets_ids = NULL;

	return tickets;
}

key_id_t _get_root_tickets_key_from_user_persistent_key(uid_t uid){
	key_info_t * key_info = get_ticket_key_info_from_persistent(uid);
	if(key_info == NULL){
		return -1;
	}	
	key_id_t keyring = keyctl_search(key_info->id, "keyring", key_info->description, KEY_SPEC_PROCESS_KEYRING);
 	free(key_info); key_info = NULL;
	
	return keyring;
}

tickets_t * get_tickets_from_user_persistent_key(uid_t uid){
	key_id_t keyring = _get_root_tickets_key_from_user_persistent_key(uid);
	if (keyring == -1){
		return NULL;
	}
	return _recolect_all_tickets_from_root_key(keyring);
}

tickets_t * get_tickets_from_named_key(const char * key_type, const char * key_name){

	long k_type = KEY_SPEC_SESSION_KEYRING;
	if(strcmp(key_type, "user") == 0){
		k_type = KEY_SPEC_USER_KEYRING;
	}

	key_id_t key_id = keyctl_search(k_type, "keyring", key_name, 0);
	if (key_id == -1){
		return NULL;
	}
	
	return _recolect_all_tickets_from_root_key(key_id);
}




int destroy_tickets(tickets_t **tickets_ref){
	tickets_t * tickets = *tickets_ref;	

	if(!*tickets_ref) {
		return 0;
	}

	if(tickets->krb5_princ){
		free(tickets->krb5_princ);
		tickets->krb5_princ = NULL;
	}

	if(tickets->tickets){
		size_t i = 0;
		for(i = 0; i < tickets->length; i++){
			if(tickets->tickets[i]){
				free(tickets->tickets[i]);
				tickets->tickets[i] = NULL;
			}
		}
		free(tickets->tickets);
		tickets->tickets = NULL;
	}
	
	free(*tickets_ref);
	*tickets_ref = NULL;
	return 0;
}


#define CCACHE_HEADER_LENGTH 16
char * generate_ccache_header(){
	static char header[CCACHE_HEADER_LENGTH];

	*(uint8_t*)header = 5;
	*(uint8_t*)(&header[1]) = 4;

	*(uint16_t*)(header + 2) = htobe16(12);
	*(uint16_t*)(header + 4) = htobe16(1);
	*(uint16_t*)(header + 6) = htobe16(8);
	*(uint32_t*)(header + 8) = htobe32(4);
	*(uint32_t*)(header + 12) = htobe32(32);
	
	return header;
}

int write_tickets_to_ccache(const tickets_t * tickets, const char * filename){
	size_t i = 0;
	FILE *fp = fopen(filename, "w");
	
	if(fp == NULL){
		return -1;
	}

	fwrite(generate_ccache_header(), CCACHE_HEADER_LENGTH, 1, fp); 
	fwrite(tickets->krb5_princ->data, tickets->krb5_princ->size, 1, fp);
	for(i = 0; i < tickets->length; i++){
		fwrite(tickets->tickets[i]->data, tickets->tickets[i]->size, 1, fp);
	}
	fclose(fp);
	
	return 0;
} 
