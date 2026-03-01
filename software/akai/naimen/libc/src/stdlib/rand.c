#include <stdlib.h>

int rand(void)
{
    _Randseed = _Randseed * 1103515245UL + 12345;
    return (unsigned int)(_Randseed >> 16) & RAND_MAX;
}
