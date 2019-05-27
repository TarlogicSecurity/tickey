#include "p_trace.h"

#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <errno.h>

long ptrace_attach(pid_t pid) {
    return ptrace(PTRACE_ATTACH, pid, NULL, NULL);
}

long ptrace_detach(pid_t pid){
    return ptrace(PTRACE_DETACH, pid, NULL, NULL);
}

long ptrace_peektext(pid_t pid, void* addr) {
    return ptrace(PTRACE_PEEKTEXT, pid, addr, NULL);
}

long ptrace_poketext(pid_t pid, void* addr, long word) {
    return ptrace(PTRACE_POKETEXT, pid, addr, word);
}

long ptrace_getregs(pid_t pid, struct user_regs_struct *regs) {
    return ptrace(PTRACE_GETREGS, pid, NULL, regs);
}

long ptrace_setregs(pid_t pid, struct user_regs_struct *regs) {
    return ptrace(PTRACE_SETREGS, pid, NULL, regs);
}

long ptrace_singlestep(pid_t pid) {
    return ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL);
}

long ptrace_cont(pid_t pid, long signal){
    return ptrace(PTRACE_CONT, pid, NULL, signal);
}

long ptrace_interrupt(pid_t pid){
    return ptrace(PTRACE_INTERRUPT, pid, NULL, NULL);
}


size_t ptrace_read_memory(pid_t pid, void *local_dest, void *remote_src, size_t size) {
    size_t i = 0;
    for(i = 0; i < size; i += sizeof(long)){
        errno = 0;
        long word = ptrace_peektext(pid, remote_src + i);
        if(word == -1 && errno != 0){
            return i;
        }
        
        size_t j = 0;
        for(j = 0; j < sizeof(long); j++){
            if((i + j) >= size){
                return i + j;
            }
            ((char*)local_dest)[i + j] = ((char*)&word)[j];
        }
    }
    
    return i;
}



long _write_half_word(int pid, void *remote_dest, void *local_src, size_t size){
    long word = ptrace_peektext(pid, remote_dest);
    if(word == -1 && errno != 0){
        return -1;
    }

    size_t j = 0;
    for(j = 0; j < size; j++){
        ((char*)&word)[j] = ((char*)local_src)[j];
    }
    if(-1 == ptrace_poketext(pid, remote_dest, word)){
        return -1;
    }
    return 0;
}

size_t ptrace_write_memory(int pid, void* remote_dest, void *local_src, size_t size) {
    size_t i = 0;
    for(i = 0; i < size; i += sizeof(long)){
        if((i + sizeof(long)) <= size){
            long word = *(long*)(local_src + i);
            if(-1 == ptrace_poketext(pid, remote_dest + i, word)){
                return i;
            }
        }else{
            if(-1 == _write_half_word(pid, remote_dest + i, local_src + i, size - i)) {
                return i;
            }
        }
    }
    return size;
}


