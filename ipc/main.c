#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

/*
 * if arg1 arg2 arg3 | arg4 arg5 arg6 | wc ; then
 *     arg7 arg8 >> arg9
 * else
 *     echo "error"
 * fi
 */


int fd0[2], fd1[2];
int fd;
int st1, st2, st3;

void closeall() {
    close(fd0[0]);
    close(fd0[1]);
    close(fd1[0]);
    close(fd1[1]);
}

int main(int argc, char ** argv) {
    if (argc != 10) return 3;
    if (pipe(fd0) == -1 || pipe(fd1) == -1) {
        printf("error");
        return 3;
    }
    if (fork() == 0) {
        dup2(fd0[1], 1);
        closeall();
        execlp(argv[1], argv[1], argv[2], argv[3], NULL);
    } 
    if (fork() == 0) {
        dup2(fd0[0], 0);
        dup2(fd1[1], 1); 
        closeall();
        execlp(argv[4], argv[4], argv[5], argv[6], NULL);
    }
    if (fork() == 0) {
        dup2(fd1[0], 0);
        closeall();
        execlp("wc", "wc", NULL);
    }
    closeall();
    wait(&st1);
    wait(&st2);
    wait(&st3);
    if (st1 != 0 || st2 != 0 || st3 != 0) {
        fprintf(stderr, "error");
        return 0;
    }
    fd = open(argv[9], O_CREAT | O_APPEND | O_WRONLY , S_IRWXU);
    dup2(fd, 1);
    close(fd);
    execlp(argv[7], argv[7], argv[8], NULL);
}
