#ifndef TICKETS_C
#define TICKETS_C

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#include "keyrings.h"


typedef struct{
	size_t size;
	char data[];
} ticket_t;

typedef struct{
	ticket_t * krb5_princ;
	size_t length;
	ticket_t ** tickets;
} tickets_t;

int destroy_tickets(tickets_t **tickets_ref);
int write_tickets_to_ccache(const tickets_t * tickets, const char * filename);
tickets_t * get_tickets_from_user_persistent_key(uid_t uid);
tickets_t * get_tickets_from_named_key(const char * key_type, const char * key_name);
#endif
