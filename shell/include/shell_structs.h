#ifndef SHELL_STRUCTS_H
#define SHELL_STRUCTS_H

#include <stdbool.h>
#include <sys/types.h>

struct command{
    char * input, * output;
    bool out_append;
    char * name;
    char ** args;
    int argc;
    pid_t pid;
};

struct job{
    struct command ** commands; 
    int commandsc;
    bool background;
    pid_t ctl_grp;
};


#endif
