#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#define MAXN 100000

int * arr;
int ** order;
int n = -1;

void init() {
    arr = malloc(n * sizeof(int));
    order = malloc(n * sizeof(int*));
}

void readArr() {
    int i;
    for (i = 0; i < n; i++) {
        if (scanf("%d", arr + i) != 1) {
            fprintf(stderr, "wrong number of elements\n");
            exit(-1);
        }
    }
}

void randomArr() {
    int i;
    for (i = 0; i < n; i++) {
        arr[i] = rand();
    }
}

void makeStepOrder(int step) {
    int i, pos = 0;
    order[0] = arr;
    for (i = 1; i < n; i++, pos = (pos + step) % n) {
        order[i] = arr + pos;
    }
}

void makeRandomOrder() {
    int i, pos = 0;
    order[0] = arr;
    for (i = 1; i < n; i++, pos = (abs(rand())) % n) {
        order[i] = arr + pos;
    }
}

int sum() {
    int ** orderit = order;
    int i, res = 0;
    for (i = 0; i < n; i++, orderit++) {
        res += **orderit;
    }
    return res;
}

int main(int argc, char ** argv) {
    int i, stp;
    bool rnd = false, bystep = false, fromstdin = false;
    clock_t start, finish;
    for (i = 1; i < argc; i++) {
        if (argc != 0 && strcmp("-n", argv[i - 1]) == 0) {
            sscanf(argv[i], "%d", &n);
        }
        if (strcmp("-r", argv[i - 1]) == 0) {
            rnd = true;
        }
        if (argc != 0 && strcmp("-step", argv[i - 1]) == 0) {
            bystep = true;
            sscanf(argv[i], "%d", &stp);
        }
        if (strcmp("-stdin", argv[i - 1]) == 0) {
            fromstdin = true;
        }
    }
    if (n == -1) {
        fprintf(stderr, "provide number of elements!!!\n");
        return -1;
    }
    init();
    if (fromstdin) {
        readArr();
    } else {
        randomArr();
    }
    if (bystep) {
        makeStepOrder(stp);
    } 
    if (rnd) {
        if (bystep) {
            fprintf(stderr, "provide valid comand line arguments\n");
            return -1;
        }
        makeRandomOrder();
    }
    start = clock();
    sum();
    finish = clock();
    printf("%.6fsec\n", ((double)(finish - start)) / CLOCKS_PER_SEC);
}
