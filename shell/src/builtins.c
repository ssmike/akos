#include "builtins.h"
#include "exec.h"
#include <stdio.h>
#include "argparse.h"

int jobs(struct command * t);
int pwd(struct command * t);

char * builtin_names[builtins_n] = {"jobs", "pwd"};
builtin functions[builtins_n] = {&jobs};


int jobs(struct command * t) {
    int i;
    for (i = 0; i < background_jobs_n; i++) {
        printf("background job controller : PID - %d\n", background[i]);
        printf("id : %d\n", i);
        print_job_desc(background_jobs[i]);
    }
    return 0;
}

int pwd(struct command * t) {
    printf("%s", getenv("PWD"));
    return 0;
}
