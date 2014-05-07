#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <semaphore.h>
int n;
int **matr1, **matr2;

struct pair {
    int first, second;  
};

struct pair * make_pair(int x, int y) {
    struct pair * pp = malloc(sizeof(struct pair));
    if (pp == NULL) {
        exit(2);
    }
    pp->first = x;
    pp->second = y;
    return pp;
}

#define THREADS 10
#define FAIL(u, s) if (u) {printf(s); printf("\n"); return 1;}
sem_t sem;
pthread_t * threads;

void * thread(void * arg) {
    struct pair * tr = (struct pair *)arg;
    int x = tr->first, y = tr->second, i, res = 0;
    for (i = 0; i < n; i++) {
        res += matr1[x][i] * matr2[i][y];
    }
    sem_post(&sem);
    pthread_exit((void*)res);
}

int main(int argc, char ** argv) {
    FILE * f1, * f2;
    int n1, n2, i, j;
    FAIL(argc != 3, "2 filenames needed");
    f1 = fopen(argv[1], "r");
    f2 = fopen(argv[2], "r");
    FAIL((f1 == NULL || f2 == NULL), "failed to open files");
    FAIL(fscanf(f1, "%d", &n1) != 1, "i/o error");
    FAIL(fscanf(f2, "%d", &n2) != 1, "i/o error");
    FAIL(n1 != n2, "sizes don't match");
    n = n1;
    FAIL((matr1 = (int**)malloc(n1 * sizeof(int*))) == NULL, "memory allocation error");
    for (i = 0; i < n1; i++) {
        FAIL((matr1[i] = (int*)malloc(n1 * sizeof(int))) == NULL, "memory allocation error");
        for (j = 0; j < n1; j++)
            FAIL(fscanf(f1, "%d", &matr1[i][j]) != 1, "i/o error");
    }
    fclose(f1);
    FAIL((matr2 = (int**)malloc(n2 * sizeof(int*))) == NULL, "memory allocation error");
    for (i = 0; i < n2; i++) {
        FAIL((matr2[i] = (int*)malloc(n1 * sizeof(int))) == NULL, "memory allocation error");
        for (j = 0; j < n2; j++)
            FAIL(fscanf(f2, "%d", &matr2[i][j]) != 1, "i/o error");
    }
    fclose(f2);
    FAIL(sem_init(&sem, 0, THREADS), "sem_init error");
    FAIL((threads = (pthread_t*)malloc(n * sizeof(pthread_t))) == NULL, "memory allocation error");
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            FAIL(pthread_create(&threads[j], 0, &thread, make_pair(i, j)), "pthread_create error");
            FAIL(sem_wait(&sem), "sem_wait error");
        }
        for (j = 0; j < n; j++) {
            void * ret;
            FAIL(pthread_join(threads[j], &ret), "pthread_join error");
            printf(" %d", (int)ret);
        }
        printf("\n");
    }
    
    FAIL(sem_destroy(&sem), "sem_destroy error");
}
