#ifndef GETSS_H
#define GETSS_H
#define GETSS_DEFAULT_LIM 10000
#include <stdio.h>
/*
 * TODO: add windows support
 * all functions return negative values on errors
 */
#define ALLOCATION_FAILED (-1)
#define EOF_ERROR (-2)
#define LIMIT_EXCEEDED (-3)

int getss(char ** res);
int getss_(char ** res, int lim);
int fgetss(FILE * fin, char ** res);
int fgetss_(FILE * fin, char ** res, int lim);

#endif
