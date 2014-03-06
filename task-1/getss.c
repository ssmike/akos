#include "getss.h"
#include <stdlib.h>

/*TODO
 * 
 * Можно было бы и макросами сделать
 * наверно.  
 */

int getss(char ** res) {
    return fgetss(stdin, res);
}

int getss_(char ** res, int lim) {
    return fgetss_(stdin, res, lim);
}

int fgetss(FILE * fin, char ** res) {
    return fgetss_(fin, res, GETSS_DEFAULT_LIM);
}

#ifdef __unix__
#define line_end '\n'
#else
#define line_end '\r'
#endif

int fgetss_(FILE * fin, char ** res, int lim) {
    *res = NULL;
    int cchar;
    int sz = 0;
    int allsz = 1;
    char * buf, * tmp;
    if (feof(stdin)) {
        return EOF_ERROR;
    }
    buf = malloc(sizeof(char));
    if (buf == NULL) return ALLOCATION_FAILED;
    while (sz < lim && (cchar = fgetc(fin)) != line_end && !feof(fin)) {
        while (sz + 1 >= allsz) {
            tmp = realloc(buf, allsz * 2 * sizeof(char));
            if (tmp == NULL) {
                *res = buf;
                return ALLOCATION_FAILED;
            } else buf = tmp;
            allsz *= 2;
        } 
        buf[sz++] = cchar;
    }
    if (sz >= lim) {
        *res = buf;
        return LIMIT_EXCEEDED;
    }
    tmp = realloc(buf, (sz + 1) * sizeof(char));
    if (tmp == NULL) {
        *res = buf;
        return ALLOCATION_FAILED;
    } else buf = tmp;
    buf[sz] = '\0';
    *res = buf;
#ifndef __unix__
    {
        char tmp_char; 
        while((tmp_char = fgetc(fin)) != '\n' && !feof(fin));
    }
#endif
    return sz;
}
