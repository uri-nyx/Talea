#include <akai.h>
#include <assert.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

jmp_buff env;

#define MAX_ARGV 16

struct AkshBuiltin {
    const char *name;
    int (*func)(int argc, char **argv);
};

static void emit(char c)
{
    _trace(0x0d0d0d, c);
    ak_dev_out(PDEV_STDOUT, 0, c);
}

static void miniprint(const char *fmt, ...)
{
    static const char *null_str = "(NULL)";
    char               c;
    char               specifier = 0;

    va_list ap;
    va_start(ap, fmt);

    while ((c = *fmt++)) {
        if (c == '%') {
            u8   width    = 0;
            bool pad_zero = false;

            specifier = *fmt++;
            if (!specifier) return; /* end printing if no specifier found */

            if (specifier == '0') {
                pad_zero  = true;
                specifier = *fmt++;
            }

            if (!specifier) return; /* end printing if no specifier found */

            if (specifier >= '0' && specifier <= '9') {
                width     = (width * 10) + (specifier - '0');
                specifier = *fmt++;
            }

            if (!specifier) return; /* end printing if no specifier found */

            switch (specifier) {
            case 's': {
                char *s = va_arg(ap, char *);
                if (!s) s = (char *)null_str; /* print (NULL) if s is NULL */
                while (*s) emit(*s++);
                break;
            }
            case 'c': {
                char chr = va_arg(ap, int);
                emit(chr);
                break;
            }
            case 'x': {
                char hex_chars[] = "0123456789ABCDEF";
                /* a 32 bit int fits in 8 characters */
                char buf[16];
                int  i = 0;
                u32  x = va_arg(ap, int);
                u8   content_len;

                if (x == 0) {
                    buf[i++] = '0';
                    goto emit_hex;
                }

                /* Extract digits from least significant to most significant */
                while (x > 0) {
                    buf[i++] = hex_chars[x & 0xF];
                    x >>= 4;
                }
emit_hex:
                content_len = i;
                while (content_len < width) {
                    emit(pad_zero ? '0' : ' ');
                    content_len++;
                }

                /* Print the buffer in reverse */
                while (i > 0) emit(buf[--i]);
                break;
            }
            case 'd': {
                char buf[16]; /* Enough for -2,147,483,648 plus null (no commas) */
                i32  i = 0;
                u32  num;
                int  n           = va_arg(ap, int);
                int  content_len = 0;
                bool is_neg      = n < 0;

                /* Handle Negative Numbers */
                if (is_neg) {
                    /* Use unsigned to avoid overflow issues with INT_MIN */
                    num = (u32)(-n);
                } else {
                    num = (u32)n;
                }

                /* Extract digits (Right to Left) */
                if (num == 0) {
                    buf[i++] = '0';
                    goto emit_dec;
                }

                while (num > 0) {
                    buf[i++] = (num % 10) + '0';
                    num /= 10;
                }

                if (pad_zero && is_neg) emit('-');

emit_dec:
                content_len = i + (is_neg ? 1 : 0);

                while (content_len < width) {
                    emit(pad_zero ? '0' : ' ');
                    content_len++;
                }

                if (!pad_zero && is_neg) emit('-');

                /* Print buffer in reverse (Left to Right) */
                while (i > 0) emit(buf[--i]);
                break;
            }
            case '%': {
                /* escaped % */
                emit('%');
                break;
            }
            default: break;
            }
        } else {
            emit(c);
        }
    }

    va_end(ap);
}

static int tokenize(char *line, usize sz, char **out_argv, usize max_arg)
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

static int cmd_clear(int argc, char **argv)
{
    char cls[] = "\x1b[2J";
    ak_dev_ctl(PDEV_STDOUT, PX_WRITE, cls, 4);
    return 0;
}

static int getcwd(char *buff, usize len)
{
    int res = ak_getcwd(buff, len);
    if (res != A_OK) return res;
    buff[len - 1] = 0;
    return 0;
}

static void prompt(const char *p)
{
    static char buff[32];
    int         res = 0;
    if ((res = getcwd(buff, 32)) != 0) return;
    ak_dev_ctl(PDEV_STDOUT, PX_WRITE, buff, strlen(buff));
    ak_dev_ctl(PDEV_STDOUT, PX_WRITE, (void *)p, strlen(p));
}

static int cmd_pwd(int argc, char **argv)
{
    static char buff[32];
    int         res = 0;
    if ((res = getcwd(buff, 32)) != 0) return res;
    ak_dev_ctl(PDEV_STDOUT, PX_WRITE, buff, strlen(buff));
    emit('\n');
}

static int cmd_ls(int argc, char **argv)
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
            res   = builtins[i].func(argc, argv);
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

static void print_tm(const char *label, const struct tm *t)
{
    printf("%s: %04d-%02d-%02d %02d:%02d:%02d wday=%d yday=%d isdst=%d\n", label, t->tm_year + 1900,
           t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, t->tm_wday, t->tm_yday,
           t->tm_isdst);
}

static void test_roundtrip(time_t ts)
{
    struct tm  tm1, tm2;
    time_t     ts2;
    char       buf[64]; /* Convert to broken-down time */
    struct tm *p = gmtime(&ts);

    tm1 = *p;           /* Convert back */
    ts2 = mktime(&tm1); /* Convert again */
    p   = gmtime(&ts2);
    tm2 = *p;

    printf("timestamp: %lu\n", (unsigned long)ts);
    print_tm("gmtime", &tm1);
    printf("mktime -> %lu\n", (unsigned long)ts2);
    print_tm("gmtime2", &tm2); /* asctime / ctime */
    printf("asctime: %s", asctime(&tm1));
    printf("ctime: %s", ctime(&ts)); /* strftime */
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S (%a)", &tm1);
    printf("strftime: %s\n", buf);
    printf("----\n");
}

void main()
{
    char   line[256];
    char  *argv[MAX_ARGV];
    u8     imode[1] = { IN_CANON | IN_ECHO | IN_CRNL };
    u8     omode[1] = { OUT_NLCR };
    int    res;
    u8    *test, *test2, *p1, *p2;
    time_t tim;

    res = ak_dev_ctl(PDEV_STDIN, PX_SETCANON, imode, 1);
    if (res) ak_exit(1);

    res = ak_dev_ctl(PDEV_STDOUT, PX_SETCANON, omode, 1);
    if (res) ak_exit(2);
    _trace(0xDFAAD110);

    cmd_clear(0, NULL);
    printf("Aksh v%d.%d -- A shell for the Akai Operating System\n", 0, 1);
    tim = time(NULL);
    printf("%s", ctime(&tim));

    {
        int pid;
        int        status = -1;
        puts("====== RFORK TEST ======\n");
        pid = ak_rfork(RF_MEM_SHARE | RF_FIL_CLEAN | RF_LEASE_STDOUT | RF_PARENT_KEEP_RUNNING,
                       RF_INHERIT_TEXTBUFFER);

        if (pid == 0) {
            // child
            puts("I am the Child!\n");
            ak_exit(12);
        } else if (pid < 0) {
            printf("Rfork error %x\n", (unsigned)pid); // FIXME: printf("%x", int) causes a kernel panic!
        } else {
            ak_wait(pid, &status, WAIT_HANG);
            printf("Child %d exited with code %d\n", pid, status);
        }
    }

    while (1) {
        prompt("> ");
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
