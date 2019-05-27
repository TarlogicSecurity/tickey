#include "users.h"
#include <stdint.h>
#include <stdlib.h>
#include <utmpx.h>
#include <paths.h>
#include <pwd.h>


int get_user_id(const char *name){
	struct passwd *user_info = getpwnam(name);
	if(user_info == NULL){
		return -1;
	}
	return user_info->pw_uid;
}


user_ids_t * new_user_ids(uid_t * uids, size_t uids_length){
	
	user_ids_t * user_ids = malloc(sizeof(user_ids_t) + sizeof(uid_t) * uids_length);
	if(user_ids == NULL){
		return NULL;
	}
	user_ids->length = uids_length;
	
	size_t i = 0;
	for(i = 0; i < uids_length; i++){
		user_ids->ids[i] = uids[i];
	}

	return user_ids;
}

user_ids_t * logged_users(){
	
	uid_t * uids = NULL;
	size_t uids_length = 0;

	setutxent();
	struct utmpx *ut = NULL;
	while((ut = getutxent()) != NULL){
		if(ut->ut_type == USER_PROCESS){
			uids = realloc(uids, (uids_length + 1) * sizeof(uid_t));
			if(uids == NULL){
				return NULL;
			}

			uids[uids_length] = get_user_id(ut->ut_user);
			uids_length++; 
		}
	}
	endutxent();

	user_ids_t * user_ids = new_user_ids(uids, uids_length);
	free(uids); uids = NULL;

	return user_ids;
}

