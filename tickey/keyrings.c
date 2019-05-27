#include "keyrings.h"
#include <stdlib.h>
#include <string.h>


#define DESCRIPTION_SEPARATOR ";"


key_ids_t * get_subkeys(key_id_t keyring_id){
	int32_t buffer_size = 0;
	key_ids_t * subkeyrings_ids = NULL;

	buffer_size = keyctl_read_size(keyring_id);
	if(buffer_size == -1){
		return NULL;
	}

	subkeyrings_ids = malloc(sizeof(key_ids_t) + buffer_size);
	if(subkeyrings_ids == NULL) {
		return NULL;
	}

	subkeyrings_ids->length = buffer_size / sizeof(uint32_t);
	keyctl_read(keyring_id, (char*)(subkeyrings_ids->ids), buffer_size);

	return subkeyrings_ids;
}

key_info_t * new_keyring_info(uint32_t id, const char * type, uint32_t uid, 
                                  uint32_t gid, uint32_t perms, const char * description) {

	key_info_t * keyring_info = NULL;
	keyring_info = (key_info_t*)malloc(sizeof(key_info_t) + strlen(type) + 1 + strlen(description) + 1);
	if(keyring_info == NULL){
		return NULL;
	}

	keyring_info->id = id;
	keyring_info->uid = uid;
	keyring_info->gid = gid;
	keyring_info->perms = perms;

	char * str_ptr = ((char*)keyring_info) + sizeof(key_info_t);
	strcpy(str_ptr, type);
	keyring_info->type = str_ptr;

	str_ptr += strlen(type) + 1;
	strcpy(str_ptr, description);
	keyring_info->description = str_ptr;

	return keyring_info;
}

key_info_t* get_key_info(key_id_t keyring_id){
	int32_t buffer_size = 0;
	char * description = NULL;
	key_info_t * keyring_info = NULL;
    char * type = NULL;
    char * desc = NULL;

	buffer_size = keyctl_describe_size(keyring_id);
	if(buffer_size == -1){
		return NULL;
	}

	description = (char*)calloc(buffer_size + 1, 1);
	if(description == NULL){
		return NULL;
	}
	
	size_t new_buffer_size = keyctl_describe(keyring_id, description, buffer_size);
	if(new_buffer_size > buffer_size){
		goto close;
	}

	char * tok = NULL;
	tok = strtok(description, DESCRIPTION_SEPARATOR);
	type = strdup(tok);
	tok = strtok(NULL, DESCRIPTION_SEPARATOR);
	uint32_t uid = strtoul(tok, NULL, 0);
	tok = strtok(NULL, DESCRIPTION_SEPARATOR);
	uint32_t gid = strtoul(tok, NULL, 0);
	tok = strtok(NULL, DESCRIPTION_SEPARATOR);
	uint32_t perms = strtoul(tok, NULL, 0);
	tok = strtok(NULL, DESCRIPTION_SEPARATOR);	
	desc = strdup(tok);
	
	keyring_info = new_keyring_info(keyring_id, type, uid, gid, perms, desc);

close:
	if(type){ free(type); type = NULL;}
	if(desc){ free(desc); desc = NULL;}
	free(description);
	return keyring_info;
}

