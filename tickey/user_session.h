#ifndef LOGGED_USERS_H
#define LOGGED_USERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <utmp.h>
#include <sys/types.h>

typedef struct {
    uid_t uid;
    char username[UT_NAMESIZE + 1];
    char terminal[UT_LINESIZE + 1];
} user_session_t;
    
typedef struct {
    size_t length;
    user_session_t ** users;
} users_sessions_list_t;
    
users_sessions_list_t * users_sessions();
void release_users_sessions_list(users_sessions_list_t ** users);

#ifdef __cplusplus
}
#endif

#endif /* LOGGED_USERS_H */

