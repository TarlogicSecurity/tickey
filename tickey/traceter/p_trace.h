
#ifndef P_TRACE_H
#define P_TRACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/user.h>
#include <sys/types.h>

long ptrace_attach(pid_t pid);

long ptrace_detach(pid_t pid);

long ptrace_peektext(pid_t pid, void* addr);

long ptrace_poketext(pid_t pid, void* addr, long word);

long ptrace_getregs(pid_t pid, struct user_regs_struct *regs);

long ptrace_setregs(pid_t pid, struct user_regs_struct *regs);

long ptrace_singlestep(pid_t pid);

long ptrace_cont(pid_t pid, long signal);

// PTRACE_INTERRUPT  only  works  on  tracees  attached  by PTRACE_SEIZE.
long ptrace_interrupt(pid_t pid);

size_t ptrace_read_memory(pid_t pid, void *local_dest, void *remote_src, size_t size);

size_t ptrace_write_memory(int pid, void *remote_dest, void *local_src, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* P_TRACE_H */

