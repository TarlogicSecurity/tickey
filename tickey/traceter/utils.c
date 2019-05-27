#define _GNU_SOURCE
#include "utils.h"

#include <stdlib.h>
#include <elf.h>
#include <stdio.h>

char * _proc_exe_path(pid_t pid) {
    char * exe_path = NULL;
    asprintf(&exe_path, "/proc/%d/exe", pid);
    return exe_path;
}

int _process_exe_e_ident(pid_t pid, uint8_t * e_ident){
    int ok = -1;
    FILE * fp = NULL;
    char * exe_path = _proc_exe_path(pid);
    if(NULL == exe_path){
        goto close;
    }
    
    fp = fopen(exe_path, "r");
    if(NULL == fp){
        goto close;
    }
    
    if(fread(e_ident, 1, EI_NIDENT, fp) != EI_NIDENT){
        goto close;
    }
    ok = 0;
    
close:
    if(exe_path){free(exe_path); exe_path = NULL;}
    if(fp){fclose(fp); fp = NULL;}
    return ok;
}

int process_arch(pid_t pid) {
    uint8_t e_ident[EI_NIDENT] = {0};
    if(-1 == _process_exe_e_ident(pid, e_ident)){
        return -1;
    }
    
    int arch = e_ident[EI_CLASS] * 32;
    return arch;
}

int current_process_arch(){
    if(sizeof(void*) == 8){
        return ARCH_64;
    }   
    return ARCH_32;
}


