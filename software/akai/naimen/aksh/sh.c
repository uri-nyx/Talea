#include <akai.h>
#include <ctype.h>
#include <string.h>

#define MAX_ARGV 16

struct AkshBuiltin {
    const char *name;
    int (*func)(int argc, char **argv);
};

int tokenize(char *line, usize sz, char **out_argv, usize max_arg)
{
    int   argc = 0;
    char *r, *w;
    char *end = line + sz;

    if (!line || !out_argv) return 0;

    /* clang-format off */
    for (r = line, w = line; r < end;) {
        char quoted = 0;

        if (argc >= max_arg) break;
        while (r < end && *r && isspace(*r)) r++;

        if (r >= end || !*r) break;

        quoted = (*r == '\'' || *r == '"') ? *r++ : 0;
        out_argv[argc++] = w;
        while (r < end && *r && (quoted ? *r != quoted : !isspace(*r)) ) {
            if (*r == '\\' && r + 1 < end)
            {
                char next = *(r+1);  
                if ( next == '"' || next == '\'' || next == ' ') {
                    r++;
                    *w++ = *r++;
                    continue;
                }
            }

            *w++ = *r++;
        }

        *w++ = '\0'; 
        if (r < end) r++; 
    }
    /* clang-format on */

    return argc;
}

int cmd_clear(int argc, char **argv)
{
    char cls[] = "\x1b[2J";
    ak_dev_ctl(PDEV_STDOUT, PX_WRITE, cls, 4);
    return 0;
}

int cmd_pwd(int argc, char **argv)
{
    char buff[32];
    int  res = ak_getcwd(buff, 32);
    if (res != A_OK) return res;
    buff[31] = 0;
    ak_dev_ctl(PDEV_STDOUT, PX_WRITE, buff, strlen(buff));
    return 0;
}

int cmd_ls(int argc, char **argv)
{
    static struct AkaiDir      d;
    static struct AkaiDirEntry e;

    const char *path = (argc < 2) ? "." : argv[1];
    int         res;
    usize       i;

    res = ak_opendir(&d, path, 0);
    if (res != A_OK) return res;

    do {
        res = ak_readdir(&d, &e, 0);
        if (res != A_OK) return res;

        ak_dev_ctl(PDEV_STDOUT, PX_WRITE, ADIR_FNAME(e.data), strlen(ADIR_FNAME(e.data)));
        ak_dev_ctl(PDEV_STDOUT, PX_WRITE, "\n", 1);
    } while (*ADIR_FNAME(e.data));

    res = ak_closedir(&d, 0);
    return res;
}

static const struct AkshBuiltin builtins[] = { { "clear", cmd_clear },
                                               { "pwd", cmd_pwd },
                                               { "ls", cmd_ls },
                                               { NULL, NULL } };

static int aksh_exec(int argc, char **argv)
{
    int i     = 0;
    int found = 0;
    int res;

    while (builtins[i].name) {
        if (strcmp(argv[0], builtins[i].name) == 0) {
            res = builtins[i].func(argc, argv);
            found = 1;
            break;
        }
        i++;
    }

    if (!found) {
        ak_dev_ctl(PDEV_STDOUT, PX_WRITE, "Unknown command\n", 16);
    }

    return res;
}

void main()
{
    char  line[256];
    char *argv[MAX_ARGV];
    u8    imode[1] = { IN_CANON | IN_ECHO | IN_CRNL };
    u8    omode[1] = { OUT_NLCR };
    int   res;
    u8   *test, *test2, *p1, *p2;

    res = ak_dev_ctl(PDEV_STDIN, PX_SETCANON, imode, 1);
    if (res) ak_exit(1);

    res = ak_dev_ctl(PDEV_STDOUT, PX_SETCANON, omode, 1);
    if (res) ak_exit(2);

    cmd_clear(0, NULL);

    while (1) {
        cmd_pwd(0, NULL);
        ak_dev_ctl(PDEV_STDOUT, PX_WRITE, "> ", 2);
        res = ak_dev_ctl(PDEV_STDIN, PX_READ, line, 256);
        if (res < 0) {
            _trace(0xc00da, res);
            ak_dev_ctl(PDEV_STDOUT, PX_WRITE, "Error reading line!\n", 20);
        } else {
            usize i;

            res = tokenize(line, res, argv, MAX_ARGV); // tokenize uses the same line buffer

            if (res > 0) {
                aksh_exec(res, argv);
            }
        }
    }

    return;
}
