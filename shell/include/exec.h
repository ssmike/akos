#ifndef EXEC_H
#define EXEC_H

#include <sys/types.h>
#include <stdbool.h>

struct job;
extern int waitForJob(struct job * x);
extern bool debug;
extern void zombie_clr();
extern bool is_interactive;
extern struct job ** background_jobs;
extern int background_jobs_n;
extern pid_t * background;
extern struct job * foreground;
extern pid_t for_c_pid;
extern void execute(struct job*);
extern void init_shell(int argc, char ** argv);
extern void exit_shell();
extern void setvar(char * name, char * val);
extern char * findvar(char * name);
extern int status;
extern int tty_fd;
extern int findpid(pid_t p);
extern void delete_pid(pid_t p);

#endif
