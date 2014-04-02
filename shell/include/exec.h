#ifndef EXEC_H
#define EXEC_H

struct job;

void execute(struct job*);
void init_shell(int argc, char ** argv);
void setvar(char * name, char * val);
char * findvar(char * name);

#endif
