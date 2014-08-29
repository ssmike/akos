#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "logic.h"
#include "game.h"
#include <stdio.h>
#include "memmove.h"

static int sk;

void handler(int sg) {
    close(sk);
    _exit(0);
}

int fx, fy;
char ** field;

pthread_t * client_threads = NULL;
size_t ctrs = 0, ctcs = 0;

int main() {
    FILE * fin;
    int port, i, j;
    int clients;
    struct sockaddr_in inaddr;
    pagesz = sysconf(_SC_PAGESIZE);
    fin = (FILE*)ASSERT_NULL(fopen("ini", "r"), "file opening");
    fscanf(fin, "port=%d\n", &port);
    fscanf(fin, "clients=%d\n", &clients);
    fscanf(fin, "fx=%d\n", &fx);
    fscanf(fin, "fy=%d\n", &fy);
    field = (char**)ASSERT_NULL(malloc(fx * sizeof(char*)), "field allocation");
    for (i = 0; i < fx; i++) {
        field[i] = (char*)ASSERT_NULL(malloc(fy * sizeof(char)), "field row allocation");
        for (j = 0; j < fy; j++) {
            fscanf(fin, " %c", &field[i][j]);
        }
    }
    sk = socket(AF_INET, SOCK_STREAM, 0);
    inaddr.sin_family = AF_INET;
    inaddr.sin_port = htons(port);
    inaddr.sin_addr.s_addr = INADDR_ANY;
    bind(sk, (struct sockaddr *)&inaddr, sizeof inaddr);
    listen(sk, clients);
    signal(SIGTERM, handler);
    signal(SIGINT, handler);
    while (1) {
        struct sockaddr_in inc;
        socklen_t ssz = sizeof inc;
        int fd = accept(sk, (struct sockaddr *)&inc, &ssz);
        if (increase((void**)&client_threads, &ctcs, &ctrs, sizeof(pthread_t *)) != 0) {
            close(fd);
        } else {
            pthread_create(&client_threads[ctrs/sizeof(pthread_t) - 1], NULL, &join_client, (void*)fd);
        }
    }
}
