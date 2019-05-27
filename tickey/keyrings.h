
#ifndef KEYRINGS_C
#define KEYRINGS_C

#include <stdint.h>

#include "keyctl.h"

typedef int32_t key_id_t;

typedef struct{
	size_t length;
	key_id_t ids[];
} key_ids_t;

typedef struct{
	key_id_t id;
	char * type;
	uint32_t uid;
	uint32_t gid;
	uint32_t perms;
	char * description;
} key_info_t;


key_ids_t * get_subkeys(key_id_t key_id);
key_info_t* get_key_info(key_id_t key_id);

#endif
