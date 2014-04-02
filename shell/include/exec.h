#ifndef EXEC_H
#define EXEC_H

#include <sys/types.h>

struct job;

extern struct pid_t ** background;
extern struct job * foreground;
extern pid_t for_c_pid;
void execute(struct job*);
void init_shell(int argc, char ** argv);
void exit_shell();
void setvar(char * name, char * val);
char * findvar(char * name);

#endif
