#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
int ccol;



void action(int snum, siginfo_t * sinfo, void * context) {
    char ff[10];
    if (ccol == 20) _exit(0);
    sprintf(ff, "pid - %d\n", (int)sinfo->si_pid);
    write(1, ff, sizeof(char) * strlen(ff));
}

int main() {
    int i;
    ccol = 0;
    struct sigaction sa;
    sa.sa_sigaction = action;
    sa.sa_flags = SA_SIGINFO;
    for (i = 0; i < 40; i++)
        sigaction(i, &sa, NULL);
    while (1)
        sleep (10);
}
