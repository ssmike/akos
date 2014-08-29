#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <pthread.h>

extern struct player {
    int x, y, hp, amm;
    char name[20];
    FILE * stream;
};

extern volatile int rooms_count;

extern volatile struct room {
    struct player * players;
    int players_count;
    size_t plcsz, plrsz;
    pthread_mutex_t room_m;
    char name[20];
} * rooms;

extern pthread_mutex_t rooms_m;

extern char ** field;
extern int fx, fy;

#endif
