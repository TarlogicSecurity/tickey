#ifndef PROC_INFO_H
#define PROC_INFO_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <sys/types.h>

typedef struct {
    uid_t euid;
    pid_t pid;
} process_info_t;


process_info_t* process_using_tty(const char *terminal_name);

#ifdef __cplusplus
}
#endif

#endif /* PROC_INFO_H */

