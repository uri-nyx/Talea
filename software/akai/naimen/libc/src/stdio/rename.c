#include "xstdio.h"

/* Akai system call */
int ak_rename(const char*, const char*);

int(rename)(const char *oldnm, const char *newnm)
{ /* rename a file */
    return ak_rename(oldnm, newnm);
}
