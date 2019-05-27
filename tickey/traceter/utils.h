#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <sys/types.h>
    
#define ARCH_32 32
#define ARCH_64 64

int process_arch(pid_t pid);
int current_process_arch();


#ifdef __cplusplus
}
#endif

#endif /* UTILS_H */

