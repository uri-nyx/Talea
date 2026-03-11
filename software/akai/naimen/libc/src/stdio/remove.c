#include "xstdio.h"

/* Akai system call */
int ak_unlink(const char *);

int(remove)(const char *fname)
{ /* remove a file */
    return (ak_unlink(fname));
}
