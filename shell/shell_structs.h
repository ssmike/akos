#ifndef SHELL_STRUCTS_H
#define SHELL_STRUCTS_H

#include <stdbool.h>

struct command{
    char * input, * output;
    bool out_append;
    char * name;
    char ** args;
    int argc;
};

struct job{
    struct command ** commands; 
    int commandsc;
    bool background;
};


#endif
