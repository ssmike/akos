#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <sys/queue.h>
#include <stdbool.h>



struct command{
    LIST_ENTRY(command) cds;
    char * fin, * fout;
    bool out_append;
    char * c_s;   
};

struct job{
    LIST_HEAD(command_list, command) cds; 
    bool background;
};

struct job * parse_input();
void print_job_desc();

#endif
