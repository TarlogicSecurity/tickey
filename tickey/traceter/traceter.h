#ifndef TRACETER_H
#define TRACETER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/user.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <signal.h>

#define NOT_STOPPED_TRACE_ERROR 1
#define WAIT_TRACE_ERROR 2
#define NO_SYSCALL_ADDR_TRACE_ERROR 3
#define INCOMPATIBLE_ARCH_TRACE_ERROR 4 
#define SIGNAL_FAILED_TRACE_ERROR 5 
    
#define PTRACE_ATTACH_TRACE_ERROR 101
#define PTRACE_GETREGS_TRACE_ERROR 102
#define PTRACE_CONT_TRACE_ERROR 103
#define PTRACE_SETREGS_TRACE_ERROR 104
#define PTRACE_SINGLESTEP_TRACE_ERROR 105
#define PTRACE_WRITE_MEM_TRACE_ERROR 106
#define PTRACE_READ_MEM_TRACE_ERROR 106
    
extern __thread int errno_trace;   

typedef struct {
    pid_t pid;
    void* syscall;
} trace_session_t;


trace_session_t * init_trace_session(pid_t pid);
trace_session_t * init_forked_trace_session(pid_t pid);
int terminate_session(trace_session_t ** session);


uid_t exec_getuid(trace_session_t * session);

void * exec_mmap(trace_session_t * session, 
        void *addr, size_t length, int prot, int flags, int fd, off_t offset);

int exec_munmap(trace_session_t * session, void *addr, size_t length);

pid_t exec_fork(trace_session_t * session);

int exec_execve(trace_session_t * session,
        const char *filename, char *const argv[], char *const envp[]);

int exec_execve_from_local(trace_session_t * session,
        const char *filename, char *const argv[], char *const envp[]);

int send_signal(trace_session_t * session, int signal);

trace_session_t * fork_session(trace_session_t * session);

unsigned long rax(trace_session_t * session);
unsigned long rip(trace_session_t * session);

#ifdef __cplusplus
}
#endif

#endif /* TRACETER_H */

