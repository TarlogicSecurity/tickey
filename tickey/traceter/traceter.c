#include "traceter.h"

#include <stdlib.h>
#include <sys/wait.h>
#include <stdint.h>
#include <asm/unistd_64.h>
#include <string.h>

#include <stdio.h>
#include <bits/signum.h>

#include "p_trace.h"
#include "utils.h"
    
#define nullanize(ptr) free(ptr); ptr = NULL
#define release(ptr) if(ptr){nullanize(ptr);}
#define release_file(fp) if(fp){fclose(fp); fp = NULL;}

#define SYSCALL_SIZE 2
#define SYSCALL_64 0x050f
#define SYSCALL_32 0x80cd

#define USER_MEM_MAX_ADDR_64 0x7fffffffffff
#define USER_MEM_MAX_ADDR_32 0xbfffffff

#ifdef __x86_64__
#define SYSCALL	SYSCALL_64
#define USER_MEM_MAX_ADDR USER_MEM_MAX_ADDR_64
#else
#define SYSCALL	SYSCALL_32
#define USER_MEM_MAX_ADDR USER_MEM_MAX_ADDR_32
#endif

__thread int errno_trace = 0; 


static int _setregs(trace_session_t * session, struct user_regs_struct *regs){
    int ok = ptrace_setregs(session->pid, regs);
    if(-1 == ok){
        errno_trace = PTRACE_SETREGS_TRACE_ERROR;
    }
    return ok;
}

static int _getregs(trace_session_t * session, struct user_regs_struct *regs){
    int ok = ptrace_getregs(session->pid, regs);
    if(-1 == ok){
        errno_trace = PTRACE_GETREGS_TRACE_ERROR;
    }
    return ok;
}

unsigned long rax(trace_session_t * session) {
    struct user_regs_struct syscall_regs = {0};
    if(-1 == _getregs(session, &syscall_regs)){
        return -1;
    }
    
    return syscall_regs.rax;
}


unsigned long rip(trace_session_t * session) {
    struct user_regs_struct syscall_regs = {0};
    if(-1 == _getregs(session, &syscall_regs)){
        return -1;
    }
    
    return syscall_regs.rip;
}


static int _continue(trace_session_t * session){
    int ok = ptrace_cont(session->pid, 0);
    if(-1 == ok){
        errno_trace = PTRACE_CONT_TRACE_ERROR;
    }
    return ok;
}


static bool was_stopped_by_sigtrap(int status){
    return WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP;
}


static int _wait_for_singlestep(trace_session_t * session) {
    int status = 0;
    if(waitpid(session->pid, &status, WUNTRACED) != session->pid) {
        return -1;
    }
    
    if(!was_stopped_by_sigtrap(status)){
        return -1;
    }
    return 0;
}


static int _singlestep(trace_session_t * session){
    int ok = ptrace_singlestep(session->pid);
    if(-1 == ok){
        errno_trace = PTRACE_SINGLESTEP_TRACE_ERROR;
        return -1;
    }
    
    if(-1 == _wait_for_singlestep(session)) {
        return -1;
    }
    
    return ok;
}

static size_t _write_memory(trace_session_t * session, 
        void* addr, uint8_t *data, size_t data_size){
    size_t write_size = ptrace_write_memory(session->pid, 
            addr, data, data_size);
    if(data_size != write_size){
        errno_trace = PTRACE_WRITE_MEM_TRACE_ERROR;
        return -1;
    }
    return write_size;
    
}


static size_t _read_memory(trace_session_t * session, 
        uint8_t *data, void* addr, size_t data_size){
    size_t read_size = ptrace_read_memory(session->pid, 
            data, addr, data_size);
    if(data_size != read_size){
        errno_trace = PTRACE_READ_MEM_TRACE_ERROR;
        return -1;
    }
    return read_size;
    
}

int send_signal(trace_session_t * session, int signal){
    int ok = kill(session->pid, signal);
    if(ok == -1){
        errno_trace = SIGNAL_FAILED_TRACE_ERROR;
    }
    return ok;
}


static int attach_process(pid_t pid){
    if(current_process_arch() != process_arch(pid)){
        errno_trace = INCOMPATIBLE_ARCH_TRACE_ERROR;
        return -1;
    }
    
    if(ptrace_attach(pid) == -1){
        errno_trace = PTRACE_ATTACH_TRACE_ERROR;
        return -1;
    }
    
    int process_status = 0;
    if(waitpid(pid, &process_status, WUNTRACED) != pid){
        errno_trace = WAIT_TRACE_ERROR;
        return -1;
    }
    
    if(!WIFSTOPPED(process_status)){
        errno_trace = NOT_STOPPED_TRACE_ERROR;
        return -1;
    }
    
    return 0;
}

static int detach_process(pid_t pid){
    return (int) ptrace_detach(pid);
}

static trace_session_t * new_trace_session(pid_t pid){
    trace_session_t *trace_session = calloc(sizeof(trace_session_t), 1);
    if(NULL == trace_session){
        return NULL;
    }
    trace_session->pid = pid;
    
    return trace_session;
}

static void release_trace_session(trace_session_t ** session){
    release(*session);
}


static bool _is_syscall_address(trace_session_t * session, void * addr){
    uint16_t ins = 0;
    int ok = _read_memory(session, (uint8_t*)&ins, addr, SYSCALL_SIZE);
    return -1 != ok && SYSCALL == ins;
}

trace_session_t * init_trace_session(pid_t pid){
    if(-1 == attach_process(pid)){
        return NULL;
    }
    
    trace_session_t *session = new_trace_session(pid);
    if(NULL == session){
        goto close_error;
    }
    
    unsigned long pc = rip(session);
    if(-1 != pc) {
        void* syscall_addr = (void*)(pc - SYSCALL_SIZE);
        if(_is_syscall_address(session, syscall_addr)){
            session->syscall = syscall_addr;
        }
    }
    
    return session;
    
close_error:
    release_trace_session(&session);
    detach_process(pid);
    return NULL;
}

int resume_process(trace_session_t * session){
    if(-1 == _continue(session)){
        return -1;
    }
    return 0;
}

int stop_process(trace_session_t * session) {
    if(-1 == ptrace_interrupt(session->pid)){
        return -1;
    }
    return 0;
}

int terminate_session(trace_session_t ** session){
    detach_process((*session)->pid);
    release_trace_session(session);
    return 0;
}


static int _set_syscall_regs(trace_session_t * session, 
        unsigned long syscall_number,
		unsigned long rdi, unsigned long rsi, unsigned long rdx,
		unsigned long r10, unsigned long r8, unsigned long r9){
    
    struct user_regs_struct syscall_regs = {0};
    
    if(-1 == _getregs(session, &syscall_regs)){
        return -1;
    }
    
    syscall_regs.rax = syscall_number;
    syscall_regs.rdi = rdi;
    syscall_regs.rsi = rsi;
    syscall_regs.rdx = rdx;
    syscall_regs.r10 = r10;
    syscall_regs.r8 = r8;
    syscall_regs.r9 = r9;
    
    if(NULL == session->syscall){
        errno_trace = NO_SYSCALL_ADDR_TRACE_ERROR;
        return -1;
    }
    syscall_regs.rip = (unsigned long) session->syscall;
    
    
    return _setregs(session, &syscall_regs);
}


static unsigned long _execute_syscall_core(trace_session_t * session, 
        unsigned long syscall_number,
		unsigned long rdi, unsigned long rsi, unsigned long rdx,
		unsigned long r10, unsigned long r8, unsigned long r9) {
    
    if(-1 == _set_syscall_regs(session, syscall_number, 
            rdi, rsi, rdx, r10, r8, r9)){
        return -1;
    }
    
    if(-1 == _singlestep(session)){
        return -1;
    }
    
    return rax(session);
}


static unsigned long _execute_syscall(trace_session_t * session, 
        unsigned long syscall_number,
		unsigned long rdi, unsigned long rsi, unsigned long rdx,
		unsigned long r10, unsigned long r8, unsigned long r9) {
    
    struct user_regs_struct backup_regs = {0};
    
    
    if(-1 == _getregs(session, &backup_regs)) {
        return -1;
    }
    
    unsigned long result = _execute_syscall_core(session, syscall_number, 
            rdi, rsi, rdx, r10, r8, r9);
    
    if(-1 == _setregs(session, &backup_regs)){
        return -1;
    }
    
    return result;
}

uid_t exec_getuid(trace_session_t * session){
    return (uid_t) _execute_syscall(session, __NR_getuid,
            0,0,0,0,0,0
    );
}

void * exec_mmap(trace_session_t * session, 
        void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    unsigned long map_addr = _execute_syscall(session, __NR_mmap,
            (unsigned long) addr,
            (unsigned long) length,
            (unsigned long) prot,
            (unsigned long) flags,
            (unsigned long) fd,
            (unsigned long) offset
        );
    
    
    if(USER_MEM_MAX_ADDR < map_addr){
        map_addr = 0;
    }
    
    return (void*)map_addr;
}

int exec_munmap(trace_session_t * session, void *addr, size_t length) {
    return (int) _execute_syscall(session, __NR_munmap,
            (unsigned long) addr,
            (unsigned long) length,
            0,0,0,0
        );
}

pid_t exec_fork(trace_session_t * session){
    return (pid_t) _execute_syscall(session, __NR_fork,
            0,0,0,0,0,0
        );
}

int exec_execve(trace_session_t * session,
        const char *filename, char *const argv[], char *const envp[]) {
    if(-1 == _set_syscall_regs(session, __NR_execve, 
            (unsigned long) filename,
            (unsigned long) argv,
            (unsigned long) envp,
            0, 0, 0)){
        return -1;
    }
    unsigned long rip_prev = rip(session);
    if(-1 == _singlestep(session)){
        return -1;
    }
    unsigned long rip_next = rip(session);
    
    if ((rip_next - rip_prev) == SYSCALL_SIZE){
        return -1; // execve shouldn't continue the current execution
    }
    
    return 0;
}

static void _calc_argv_size_length(char *const argv[], 
        size_t *size, size_t *length) {
    size_t argv_size = sizeof(char*), argv_length = 1;
    char *const *current = NULL;
    for(current = argv; 
            *current != NULL; current++, argv_length++) {
        argv_size += strlen(*current) + 1 + sizeof(char*);
    }
    
    *size = argv_size;
    *length = argv_length;
}

static void _copy_argv_to_local_with_remote_pointers(
        char *const argv[], size_t argv_length,
        void * local_buffer, size_t addr_delta){
    
    char ** current_str_ptr = (char **)local_buffer;
    char * current_str = (char*)(local_buffer + (argv_length * sizeof(char*)));
    
    char *const *current = NULL;
    for(current = argv;
            *current != NULL; current++) {
        *current_str_ptr = (char*)(addr_delta + current_str);
        memcpy(current_str, *current, strlen(*current));
        current_str += strlen(current_str) + 1;
        current_str_ptr++;
    }
}


int exec_execve_from_local(trace_session_t * session,
        const char *filename, char *const argv[], char *const envp[]) {
    
    int result = -1;
    size_t filename_size = strlen(filename) + 1;
    size_t buffer_size = filename_size;
    void * remote_buffer = NULL;
    
    size_t argv_size = 0, argv_length = 0;
    _calc_argv_size_length(argv, &argv_size, &argv_length);
    buffer_size += argv_size;
    
    size_t envp_size = 0, envp_length = 0;
    _calc_argv_size_length(envp, &envp_size, &envp_length);
    buffer_size += envp_size;
    
    void* local_buffer = calloc(buffer_size,1);
    if(NULL == local_buffer){
        goto close;
    }
    
    remote_buffer = exec_mmap(session, NULL, buffer_size, 
            PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if(NULL == remote_buffer){
        goto close;
    }
    
    size_t addr_delta = remote_buffer - local_buffer;
    
    char * local_filename = (char*)local_buffer;
    char ** local_argv = (char**)(((void*)local_filename) + filename_size);
    char ** local_envp = (char**)(((void*)local_argv) + argv_size);
    
    memcpy(local_filename, filename, filename_size);
    
    _copy_argv_to_local_with_remote_pointers(argv, argv_length, 
            local_argv, addr_delta);
    
    _copy_argv_to_local_with_remote_pointers(envp, envp_length, 
            local_envp, addr_delta);
    
    if(-1 == _write_memory(session, 
            remote_buffer, (uint8_t*)local_buffer, buffer_size)){
        goto close;
    }
    
    char * remote_filename = local_filename + addr_delta;
    char ** remote_argv = (char**)(((void*)local_argv) + addr_delta);
    char ** remote_envp = (char**)(((void*)local_envp) + addr_delta);
    
    result = exec_execve(session, 
                     remote_filename, remote_argv, remote_envp);
    
close:
    release(local_buffer);
    if(remote_buffer){
        exec_munmap(session, remote_buffer, buffer_size);
    }
    return result;
}

trace_session_t * fork_session(trace_session_t * session) {
    
    pid_t pid = exec_fork(session);
    if(-1 == pid) {
        return NULL;
    }
    
    trace_session_t *forked_session = init_trace_session(pid);
    if(NULL == forked_session){
        goto close_error;
    }
    return forked_session;
    
close_error:
    kill(pid, SIGKILL);
    return NULL;
}

trace_session_t * init_forked_trace_session(pid_t pid){
    
    trace_session_t * session = init_trace_session(pid);
    if(NULL == session){
        return NULL;
    }
    
    trace_session_t *forked_session = fork_session(session);
    terminate_session(&session);
    
    return forked_session;
}

