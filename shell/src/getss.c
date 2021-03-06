#include "getss.h"
#include <stdlib.h>
#include <unistd.h>

int getss(char ** res)
{
    return fgetss(stdin, res);
}

int getss_(char ** res, size_t lim)
{
    return fgetss_(stdin, res, lim);
}

int fgetss(FILE * fin, char ** res)
{
    return fgetss_(fin, res, GETSS_DEFAULT_LIM);
}

int fgetss_(FILE * fin, char ** res, size_t lim)
{
    int cchar;
    size_t psize;
    size_t sz = 0;
    size_t allsz = sizeof(char);
    char * buf, * tmp;
    *res = NULL;
    psize = sysconf(_SC_PAGESIZE);
    if (feof(stdin))
    {
        return EOF_ERROR;
    }
    buf = malloc(sizeof(char));
    if (buf == NULL) return ALLOCATION_FAILED;

    /*
     * Такая конструкция затрудняет чтение и отладку.
     */
    while (sz < lim && (cchar = fgetc(fin)) != '\n' && !feof(fin))
    {
        while (sz + sizeof(char) >= allsz)
        {
            size_t newbsz = allsz * 2;
            if (allsz > PAGES_LIMIT * psize)
            {
                if (allsz % psize != 0)
                {
                    newbsz = allsz + psize - (allsz % psize);
                }
                else
                {
                    newbsz = allsz + psize;
                }
            }
            tmp = realloc(buf, newbsz);
            if (tmp == NULL)
            {
                *res = buf;
                return ALLOCATION_FAILED;
            }
            else buf = tmp;
            allsz = newbsz;
        }
        buf[sz/sizeof(char)] = cchar;
        sz += sizeof(char);
    }
    if (sz >= lim)
    {
        *res = buf;
        return LIMIT_EXCEEDED;
    }
    tmp = realloc(buf, sz + 2 * sizeof(char));
    if (tmp == NULL)
    {
        *res = buf;
        return ALLOCATION_FAILED;
    }
    else buf = tmp;
    buf[sz/sizeof(char) + 1] = '\0';
    buf[sz/sizeof(char)] = '\n';
    *res = buf;
    return sz + 1;
}
