#include "xstdio.h"

int ak_open(const char *, unsigned int);

int _Fopen(const char *path, unsigned int smode, const char *mods)
{ /* open a file */
    unsigned int              acc;
    static const unsigned int rwmap[] = { 1, 2, 1 | 2, 1 };

    acc = rwmap[smode & 0x03];
    if (smode & (_MOPENA | _MTRUNC)) acc |= 0x08;
    if (smode & _MCREAT) acc |= 0x10;
    if (smode & _MOPENA) acc |= 0x30;

    return (ak_open(path, acc));
}
