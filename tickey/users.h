#ifndef USERS_C
#define USERS_C

#include <sys/types.h>

typedef struct{
	size_t length;
	uid_t ids[];
} user_ids_t;

user_ids_t * logged_users();

#endif
