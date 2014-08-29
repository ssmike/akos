#define _POSIX_SOURCE 1
#include "logic.h"
#include "game.h"
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "memmove.h"


volatile int rooms_count;

volatile struct room * rooms;
size_t roomsrsz = 0, roomscsz = 0;

pthread_mutex_t rooms_m = PTHREAD_MUTEX_INITIALIZER;

void spawn(struct player * p, char * name) {
    int sxy = 0, i, j;
    for (i = 0; i < fx; i++)
       for (j = 0; j < fy; j++)
           sxy += field[i][j] == '0';
    memset(p->name, 0, 20 * sizeof(char));
    for (i = 0; name[i] != '\0'; i++)
        p->name[i] = name[i];
    sxy = rand() % sxy;
    p->hp = 20;
    p->amm = 10;
    for (i = 0; i < fx; i++)
        for (j = 0; j < fy; j++) {
            if (field[i][j] == '1') sxy--;
            if (sxy == 0 && field[i][j] == '0') {
                p->x = i;
                p->y = j;
                return;
            }
        }
}

bool valid(int x, int y) {
    return x < fx && y < fy && x >= 0 && y >= 0 && field[x][y] != '1';
}

void scannsend(FILE* fs, volatile struct room * rm, int x, int y) {
    int i;
    pthread_mutex_lock((pthread_mutex_t*)&rm->room_m);
    fprintf(fs, "players:\n");
    for (i = 0; i < rm->players_count; i++) {
        if (rm->players[i].hp > 0)
            fprintf(fs, "%s %d %d\n", rm->players[i].name, rm->players[i].x, rm->players[i].y);
    }
    pthread_mutex_unlock((pthread_mutex_t*)&rm->room_m);
}

void shoot(volatile struct room * rm, int x, int y, int dx, int dy) {
    x += dx; y += dy;
    while (valid(x, y)) {
        int i;
        pthread_mutex_lock((pthread_mutex_t*)&rm->room_m);
        for (i = 0; i < rm->players_count; i++) {
            if (x == rm->players[i].x && y == rm->players[i].y) {
               rm->players[i].hp -= 11;
               if (rm->players[i].hp <= 0) {
                   rm->players[i].x = -100;
                   rm->players[i].y = -100;
               }
               break;
            }
        }
        pthread_mutex_unlock((pthread_mutex_t*)&rm->room_m);
        x += dx; y += dy;
    }
}

void dialogue(FILE * fs, volatile struct room * rm, int p_id) {
    int i, j;
    char command[20];
    struct player * p;
    pthread_mutex_lock((pthread_mutex_t*)&rm->room_m);
    p = &rm->players[p_id];
    pthread_mutex_unlock((pthread_mutex_t*)&rm->room_m);
    while (1) {
        if (p->hp <= 0) {
            fprintf(fs, "YOU ARE DEAD!!\n");
            return;
        }
        fscanf(fs, " %20s", command);
        if (strcmp(command, "MOVE") == 0) {
            int dx, dy;
            fscanf(fs, "%d %d", &dx, &dy);
            if (abs(dx) + abs(dy) != 1) continue;
            if (valid(p->x + dx, p->y + dy)) {
                p->x += dx;
                p->y += dy;
            }
        } else if (strcmp(command, "SHOOT") == 0) {
            int dx, dy;
            fscanf(fs, "%d %d", &dx, &dy);
            if (abs(dx) + abs(dy) != 1) continue;
            shoot(rm, p->x, p->y, dx, dy);
        } else if (strcmp(command, "LEAVE") == 0) {
            break;
        }
        if (p->hp <= 0) {
            fprintf(fs, "YOU ARE DEAD!!\n");
            return;
        }
        scannsend(fs, rm, p->x, p->y);
    }
}

void* join_client(void * arg) {
    char name[20], rname[20];
    int r_num = -1;
    volatile struct room * r;
    int i, j, fd = (int) arg, p_id;
    FILE * fr = fdopen(fd, "a+");
    setbuf(fr, NULL);
    fprintf(fr, "Welcome to surin server!!\n");
    fprintf(fr, "your name: ");
    fscanf(fr, "%15s", name);
    for (i = 0; i < fx; i++) {
        for (j = 0; j < fy; j++) {
            fprintf(fr, "%c", field[i][j]);
        }
        fprintf(fr, "\n");
    }
    fprintf(fr, "%d rooms availible:\n", rooms_count);
    pthread_mutex_lock(&rooms_m);
    for (i = 0; i < rooms_count; i++) {
        fprintf(fr, "%s\n", rooms[i].name);
    }
    pthread_mutex_unlock(&rooms_m);
    fprintf(fr, "\nroom name:\n");
    fscanf(fr, "%20s", rname);
    
    pthread_mutex_lock(&rooms_m);
    for (i = 0; i < rooms_count; i++) {
        if (strcmp((char*)(rooms[i].name), rname) == 0) {
            r_num = i;
            break;
        }
    }
    if (r_num == -1) {
        if (increase((void*)&rooms, &roomscsz, &roomsrsz, sizeof(struct room)) != 0) return; 
        rooms[rooms_count].players = NULL;
        rooms[rooms_count].players_count = 0;
        rooms[rooms_count].plcsz = 0;
        rooms[rooms_count].plrsz = 0;
        memcpy((char*)(rooms[rooms_count].name), rname, sizeof rname);
        pthread_mutex_init((pthread_mutex_t*)(&rooms[rooms_count].room_m), NULL);
        r_num = rooms_count;
        rooms_count++;
    }     
    r = &rooms[r_num];
    if (increase((void**)&(r->players), (size_t*)&(r->plcsz), (size_t*)&(r->plrsz), sizeof(struct player *)) != 0) return;
    p_id = r->players_count;
    spawn(&(r->players[r->players_count++]), name);
    pthread_mutex_unlock(&rooms_m);
    
    dialogue(fr, &rooms[r_num], p_id);

    fclose(fr);
}
