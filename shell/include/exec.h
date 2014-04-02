#ifndef EXEC_H
#define EXEC_H

struct job;

int replace_vars(char **);
void execute(struct job*);
void init_shell(int argc, char ** argv);

#endif
