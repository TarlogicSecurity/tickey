#define _GNU_SOURCE
#include "proc_info.h"
#include <linux/kdev_t.h>

#include <utmp.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>


typedef struct {
    pid_t pid;
    uid_t uid;
    int tty;
} proc_t;

bool is_numerical_string(const char* string) {
    const char* str_ptr = string;

    if (strlen(string) == 0) {
        return false;
    }

    for (; *str_ptr; str_ptr++) {
        if (!isdigit(*str_ptr)) {
            return false;
        }
    }

    return true;
}


uid_t get_process_uid(pid_t pid) {
    char *pid_path = NULL;
    struct stat pid_stat = {0};
    uid_t uid = -1;
    asprintf(&pid_path, "/proc/%d", pid);
    if (pid_path == NULL) {
        goto close;
    }
    
    if(stat(pid_path, &pid_stat) == -1){
        goto close;
    }
    
    uid = pid_stat.st_uid;
    
close:
    if(pid_path){free(pid_path);pid_path=NULL;}

    return uid;
}

int get_process_tty(pid_t pid) {
    char *stat_path = NULL;
    int tty = -1, n = 0;
    FILE * stat_file = NULL;
    
    asprintf(&stat_path, "/proc/%d/stat", pid);
    if (stat_path == NULL) {
        goto close;
    }
    
    stat_file = fopen(stat_path, "r");
    if(stat_file == NULL) {
        goto close;
    }
    
    char stat_content[2048] = {0};
    char * stat_numbers = NULL;
    
    fread(stat_content, sizeof(stat_content) - 1, 1, stat_file);
    stat_numbers = strrchr(stat_content, ')') + 1;
    
    n = sscanf(stat_numbers, " %*c %*d %*d %*d %d", &tty);
    if(n != 1){
        tty = -1;
    }
       
close:
    if(stat_path){free(stat_path);stat_path=NULL;}
    if(stat_file){fclose(stat_file);stat_file=NULL;}
    return tty;
}

proc_t* get_process_info(pid_t pid) {
    proc_t *proc = NULL;
    
    uid_t uid = get_process_uid(pid);
    if(uid == -1){
        goto close;
    }
    
    int tty = get_process_tty(pid);
    if(tty == -1){
        goto close;
    }

    proc = malloc(sizeof (proc_t));
    if(proc == NULL) {
        goto close;
    }
    proc->pid = pid;
    proc->tty = tty;
    proc->uid = uid;

close:

    return proc;
}


proc_t** get_processes_info() {
    DIR *proc_dir = NULL;
    struct dirent *dir = NULL;
    proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        return NULL;
    }

    proc_t **procs = calloc(sizeof (proc_t*), 1);
    size_t procs_size = 0;
    
    while ((dir = readdir(proc_dir)) != NULL) {
        if (!is_numerical_string(dir->d_name)) {
            continue;
        }

        proc_t *proc = get_process_info(atoi(dir->d_name));
        if(proc == NULL) {
            continue;
        }
        
        proc_t **tmp = realloc(procs, sizeof(proc_t*) * (procs_size + 2));
        if(tmp == NULL){
            break;
        }
        procs = tmp;
        procs[procs_size] = proc;
        procs[procs_size + 1] = NULL;
        procs_size++;
        
    }

    closedir(proc_dir);
    
    return procs;
}

void release_procs(proc_t ***procs) {
    if(procs){
        proc_t **procs_aux = *procs;
        while(*procs_aux){
            free(*procs_aux); *procs_aux = NULL;
            (procs_aux)++;
        }
        free(*procs); *procs = NULL;
    }
}

#define MAJOR_TTY 4
#define MAJOR_PTS 136


static int _parse_terminal_name(const char *terminal_name) {
    
    int minor = 0, tty = 0;
    
    
    if(NULL != strstr(terminal_name, "pts")){
        tty = MAJOR_PTS;
        if(1 != sscanf(terminal_name, "%*3s/%d", &minor)){
            return -1;
        }
        
    }
    else if(NULL != strstr(terminal_name,"tty")){
        tty = MAJOR_TTY;
        if(1 != sscanf(terminal_name, "%*3s%d", &minor)){
            return -1;
        }
    }
    else{
        return -1;
    }
    
    tty <<= 8;
    tty |= (minor & 0xff);
    return tty;
} 

static process_info_t* new_process_info(uid_t uid, pid_t pid){
    process_info_t* p_info = malloc(sizeof(process_info_t));
    if(NULL == p_info){
        return NULL;
    }
    
    p_info->euid = uid;
    p_info->pid = pid;
    
    return p_info;
}

process_info_t* process_using_tty(const char *terminal_name){
    proc_t** procs = NULL;
    int tty = _parse_terminal_name(terminal_name);
    if(-1 == tty){
        return NULL;
    }
    
    procs = get_processes_info();
    proc_t ** proc = procs;
    process_info_t *p_info = NULL;
    for(;*proc;proc++){
        if((*proc)->tty != tty ){
            continue;
        }
        
        p_info = new_process_info((*proc)->uid, (*proc)->pid);
        if(NULL != p_info){
            goto close;
        }
    }
    
close:
    release_procs(&procs);
    return p_info;
}
