#ifndef GETSS_H
#define GETSS_H

#define GETSS_DEFAULT_LIM 10000

#include <stdio.h>
#define ALLOCATION_FAILED (-1)
#define EOF_ERROR (-2)
#define LIMIT_EXCEEDED (-3)
#define PAGES_LIMIT 10
/* if normal exit returns length of string */

extern int getss(char ** res);
extern int getss_(char ** res, size_t lim);
extern int fgetss(FILE * fin, char ** res);
extern int fgetss_(FILE * fin, char ** res, size_t lim);

#endif
