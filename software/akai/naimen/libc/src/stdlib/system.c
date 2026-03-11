#include <stdlib.h>

#ifndef _SYSTEM_SHELL
#define _SYSTEM_SHELL "/a/bin/sh"
#endif

#ifndef _SYTEM_SHELL_C
#define _SYSTEM_SHELL_C "-c"
#endif

int ak_rfork(unsigned long flags, unsigned long heirloom);
int ak_exec(const char *path, int argc, char **argv, int flags);
int ak_wait(int pid, int *status, int options);

int system(const char *s)
{
    if (s) { /* not just a test */
        int   pid, status;
        char *args[3];

        /*RF_MEM_SHARE | RF_FIL_CLEAN | RF_LEASE_STDERR | RF_LEASE_STDIN | RF_LEASE_STDOUT |
            RF_PARENT_KEEP_RUNNING*/
        pid = ak_rfork(0x879, 0);

        if (pid < 0)
            return -1;
        else if (pid == 0) {
            args[0] = _SYSTEM_SHELL_C;
            args[1] = (char*)s;
            args[2] = NULL;
            ak_exec(_SYSTEM_SHELL, 2, args, 255 /*O_EXEC_GUESS*/);
            exit(EXIT_FAILURE);
        }

        ak_wait(pid, &status, 0 /*WAIT_HANG*/);
        return status;
    }
    return (-1);
}
