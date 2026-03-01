#include <stdlib.h>

extern void ak_exit(int);
void          exit(int status){
    ak_exit(status);
}
