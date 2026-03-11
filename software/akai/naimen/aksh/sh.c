#include <akai.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define VERSION "0.3"
#define DATE    __DATE__

#define AKSH_MAX_ARGV 16
#define AKSH_MAX_LINE 256
#define AKSH_MAX_PATH 256

#define ISFILE(entry) (!(ADIR_FATTRIB(entry) == AK_ATTR_DIR))

static int isatty(unsigned int proxy)
{
    int res = ak_dev_ctl(proxy, PX_GET_DEV, NULL, 0);
    if (res < A_OK) return 0;

    return (res == (PDEV_TYPE_HW | DEV_TEXTBUFFER)) || (res == (PDEV_TYPE_HW | DEV_SERIAL));
}

static bool aksh_exit        = false;
static bool aksh_prompt_path = false;
static char aksh_prompt[10]  = "> ";

static int aksh_last_exit_code = 0;

static int aksh_exec(int argc, char **argv);

struct AkshBuiltin {
    const char *name;
    int (*func)(int argc, char **argv);
};

static char *Aksh_Path[] = {
    "/A/BIN",
    "/B/BIN",
    // TODO: format disk H "/H/BIN",
    NULL,
};

struct AkshRedir {
    const char *infile;
    const char *outfile;
    const char *heretoken;
    char        herepath[AKSH_MAX_PATH];
    bool        append;
    bool        heredoc;
};

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
    char cls[] = "\x1b[2J\x1b[H";
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

static void prompt(bool with_path, const char *p)
{
    static char buff[32];
    if (with_path) {
        int res = 0;
        if ((res = getcwd(buff, 32)) != 0) return;
        printf("%s%s", buff, p);
    } else {
        printf("%s", p);
    }

    fflush(stdout);
}

static int cmd_pwd(int argc, char **argv)
{
    static char buff[32];
    int         res = 0;
    if ((res = getcwd(buff, 32)) != 0) return res;
    buff[31] = 0;
    printf("%s\n", buff);
    return 0;
}

static int cmd_cd(int argc, char **argv)
{
    int res;

    if (argc != 2) {
        printf("Usage: %s <path>\n", argv[0]);
        return -1;
    }

    if ((res = ak_chdir(argv[1])) != A_OK) {
        fprintf(stderr, "Error changing directory to '%s'\n", argv[1]);
        return res;
    }

    return 0;
}

static int cmd_prompt(int argc, char **argv)
{
    usize len;
    if (argc >= 2) {
        aksh_prompt_path = argv[1][0] != '0';
    }

    if (argc > 2) strncpy(aksh_prompt, argv[2], 10);

    return 0;
}

static int cmd_ls(int argc, char **argv)
{
    static struct AkaiDir      d;
    static struct AkaiDirEntry e;

    const char *path;
    int         res;
    bool        option_l = false;
    usize       i;

    if (argc < 2) {
        path = ".";
    } else if (strcmp(argv[1], "-l") == 0) {
        path     = argc == 2 ? "." : argv[2];
        option_l = true;
    } else {
        path = argv[1];
    }

    res = ak_opendir(&d, path, 0);
    if (res != A_OK) {
        fprintf(stderr, "Not a directory '%s' (%d)\n", path, res);
        return res;
    }

    while (1) {
        char *name;
        bool  dir;
        res = ak_readdir(&d, &e, 0);
        if (res != A_OK) break;

        name = ADIR_FNAME(e.data);
        if (!*name) break; // end

        dir = !!(ADIR_FATTRIB(e.data) & AK_ATTR_DIR);

        if (option_l) {
            printf("%c %8u %s\n", dir ? 'd' : '-', ADIR_FSIZE(e.data), name);
        } else {
            printf("%s%c ", name, dir ? '/' : ' ');
        }
    }

    if (!option_l) putchar('\n');
    fflush(stdout);

    res = ak_closedir(&d, 0);
    return res;
}

static int cmd_echo(int argc, char **argv)
{
    usize i;
    for (i = 1; i < argc; i++) {
        fputs(argv[i], stdout);
        if (i + 1 < argc) putchar(' ');
    }
    putchar('\n');
    return 0;
}

static int cmd_cat(int argc, char **argv)
{
    bool  donl;
    usize i;
    if (argc < 2) {
        // read from stdin
        char  buf[256];
        usize n;
        while ((n = fread(buf, 1, sizeof(buf), stdin)) > 0) {
            fwrite(buf, 1, n, stdout);
        }
        return 0;
    }

    for (i = 1; i < argc; i++) {
        char  buf[256];
        usize n;

        FILE *f = fopen(argv[i], "r");
        if (!f) {
            fprintf(stderr, "cat: cannot open %s\n", argv[i]);
            continue;
        }

        while ((n = fread(buf, 1, sizeof(buf), f)) > 0) fwrite(buf, 1, n, stdout);

        fclose(f);
        if (buf[n - 1] != '\n' && isatty(PDEV_STDOUT)) donl = true;
    }

    if (donl) putchar('\n');

    fflush(stdout);
    return 0;
}

static int cmd_exit(int argc, char **argv)
{
    usize i;
    int   code;

    if (ak_getpid() == 1) {
        fputs("Cannot exit from original shell\n", stderr);
        return 127;
    }

    aksh_exit = true;
    if (argc < 2) {
        return 0;
    }

    errno = 0;
    code  = strtoul(argv[1], NULL, 0);
    if (errno != 0) return 127;
    return code;
}

int run_batch(int argc, char **argv)
{
    char  line[AKSH_MAX_LINE];
    char *sargv[AKSH_MAX_ARGV];
    int   lineno = 0;

    FILE *f;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s <script | or stdin>\n", argv[0]);
        return 127;
    }

    if (argc == 2) {
        f = fopen(argv[1], "r");
    } else if (argc == 1) {
        f = stdin;
    }

    if (!f) {
        fprintf(stderr, "Could not open batch file %s\n", argv[1]);
        return 1;
    }

    while (fgets(line, sizeof(line), f)) {
        // strip newline
        int sargc;
        lineno++;
        line[strcspn(line, "\r\n")] = 0;

        // tokenize
        sargc = tokenize(line, strlen(line), sargv, AKSH_MAX_ARGV);
        if (sargc > 0) {
            usize i;
            int   ret;
            ret = aksh_exec(sargc, sargv);
            if (ret != 0) {
                fprintf(stderr, "aksh: error at %s:%d\n", argv[1], lineno + 1);
            }
        }
    }

    fclose(f);
    return 0;
}

static int cmd_touch(int argc, char **argv)
{
    usize i;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <new-file> [more-files...]\n", argv[0]);
        return 127;
    };

    for (i = 1; i < argc; i++) {
        int fd = ak_open(argv[i], O_WRITE | O_OPEN_ALWAYS);
        if (fd >= 0)
            ak_close(fd);
        else
            fprintf(stderr, "Error creating %s (%d)\n", argv[i], fd);
    }
    return 0;
}

static int cmd_mkdir(int argc, char **argv)
{
    usize i;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <new-dir> [more-dirs...]\n", argv[0]);
        return 127;
    };

    for (i = 1; i < argc; i++) {
        int res = ak_mkdir(argv[i]);
        if (res != A_OK) fprintf(stderr, "Error creating %s, (%d)\n", argv[i], res);
    }
    return 0;
}

static int cmd_rmdir(int argc, char **argv)
{
    usize i;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <dir> [more-dirs...]\n", argv[0]);
        return 127;
    };

    for (i = 1; i < argc; i++) {
        int res = ak_unlink(argv[i]);
        if (res != A_OK) fprintf(stderr, "Error deleting %s (%d)\n", argv[i], res);
    }
    return 0;
}

static int cmd_rm(int argc, char **argv)
{
    usize i;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file> [more-files...]\n", argv[0]);
        return 127;
    };

    for (i = 1; i < argc; i++) {
        int res = ak_unlink(argv[i]);
        if (res != A_OK) fprintf(stderr, "Error deleting %s (%d)\n", argv[i], res);
    }
    return 0;
}

static int cmd_buildinfo(int argc, char **argv)
{
    if (argc > 1 && (strcmp(argv[1], "--license") == 0)) {
        puts("\x1b[1mIn the name of the Prince of Talandel, its Seas, and its Colonies\x1b[22m");

        puts("\t\x1b[3mand by His order, The Patron of the House of Taleä\x1b[23m\n\n");
        puts("Let it be known now all those who read,  that whomever that uses this  computing"
             "system shall be subject to the  Courts of  Justice of the City of  Talandel, and"
             "abide by the Laws of Commerce under the authority of His Highness The Prince.\n");
    } else if (argc > 1 && (strcmp(argv[1], "--reallicense") == 0)) {
        puts("Copyright (c) 2026 Uri Nyx <rserranof03@gmail.com>");

        puts(
            "Permission is hereby granted, free of charge, to any person obtaining a copy of this "
            "software and associated documentation files (the \"Software\"), to deal in the Software "
            "without restriction, including without limitation the rights to use, copy, modify, merge,"
            " publish, distribute, sublicense, and/or sell copies of the Software, and to permit "
            "persons to whom the Software is furnished to do so, subject to the following conditions:");

        puts(
            "The above copyright notice and this permission notice shall be included in all copies or "
            "substantial portions of the Software.");
        puts("");
        puts(
            "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, "
            "INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR "
            "PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE "
            "FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR "
            "OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER "
            "DEALINGS IN THE SOFTWARE.");
    }

    printf("Aksh shell. Version v%s (build %s)\n", VERSION, DATE);
    return 0;
}

static int cmd_lastexit(int argc, char **argv)
{
    printf("%d\n", aksh_last_exit_code);
}

static int cmd_help(int argc, char **argv);

static const struct AkshBuiltin builtins[] = { { "clear", cmd_clear },
                                               { "pwd", cmd_pwd },
                                               { "ls", cmd_ls },
                                               { "cd", cmd_cd },
                                               { "cat", cmd_cat },
                                               { "aksh", run_batch },
                                               { "echo", cmd_echo },
                                               { "touch", cmd_touch },
                                               { "rm", cmd_rm },
                                               { "help", cmd_help },
                                               { "mkdir", cmd_mkdir },
                                               { "rmdir", cmd_rmdir },
                                               { "prompt", cmd_prompt },
                                               { "lastexit", cmd_lastexit },
                                               { "exit", cmd_exit },
                                               { "buildinfo", cmd_buildinfo },
                                               { NULL, NULL } };

static int cmd_help(int argc, char **argv)
{
    usize i;
    for (i = 0; builtins[i].name; i++) fprintf(stdout, "%s ", builtins[i].name);
    putchar('\n');
    return 0;
}

static bool find_in_path(const char *cmd, char *out, usize outsz)
{
    static struct AkaiDirEntry st;
    int                        res, i;

    if (strchr(cmd, '/')) {
        if (ak_stat(cmd, &st) == A_OK && ISFILE(st.data)) {
            strncpy(out, cmd, outsz);
            return true;
        }
        return false;
    }

    for (i = 0; Aksh_Path[i]; i++) {
        sprintf(out, "%s/%s", Aksh_Path[i], cmd);
        if (ak_stat(out, &st) == A_OK && ISFILE(st.data)) return true;
    }

    return false;
}

static bool parse_redirection(int *argc, char **argv, struct AkshRedir *r)
{
    usize i;
    bool  out_found = false;
    r->infile       = NULL;
    r->outfile      = NULL;
    r->heretoken    = NULL;
    r->append       = false;
    r->heredoc      = false;

    memset(r->herepath, 0, sizeof(r->herepath));

    for (i = 0; i < *argc; i++) {
        if (!out_found && strcmp(argv[i], ">") == 0 && i + 1 < *argc) {
            r->outfile = argv[i + 1];
            memmove(&argv[i], &argv[i + 2], (*argc - i - 2) * sizeof(char *));
            *argc -= 2;
            i--;
            out_found = true;
        } else if (!out_found && strcmp(argv[i], ">>") == 0 && i + 1 < *argc) {
            r->outfile = argv[i + 1];
            r->append  = true;
            memmove(&argv[i], &argv[i + 2], (*argc - i - 2) * sizeof(char *));
            *argc -= 2;
            i--;
            out_found = true;
        } else if (strcmp(argv[i], "<") == 0 && i + 1 < *argc) {
            r->heredoc = false;
            r->infile  = argv[i + 1];
            memmove(&argv[i], &argv[i + 2], (*argc - i - 2) * sizeof(char *));
            *argc -= 2;
            i--;
        } else if (strcmp(argv[i], "<<") == 0 && i + 1 < *argc) {
            // heredoc
            r->heredoc   = true;
            r->heretoken = argv[i + 1];
            tmpnam(r->herepath);
            r->infile = r->herepath;
            memmove(&argv[i], &argv[i + 2], (*argc - i - 2) * sizeof(char *));
            *argc -= 2;
            i--;
        }
    }

    if (r->heredoc) {
        char  docline[AKSH_MAX_LINE];
        char *readline;
        FILE *doc = fopen(r->infile, "w");

        while ((readline = fgets(docline, sizeof(docline), stdin))) {
            usize len = strlen(readline);
            if (len && readline[len - 1] == '\n') readline[len - 1] = '\0';

            if (strcmp(readline, r->heretoken) == 0) break;

            fputs(readline, doc);
            fputc('\n', doc);
        }

        fclose(doc);
    }

    return true;
}

static int spawn_and_wait(const char *path, int argc, char **argv, struct AkshRedir *r)
{
    int status, res, stdin_fd = -1, stdout_fd = -1;
    int pid;

    ak_set_preempt(PREEMPT_ROBIN);

    pid = ak_rfork(RF_MEM_SHARE | RF_FIL_CLEAN | RF_PARENT_KEEP_RUNNING | RF_LEASE_STDIN |
                       RF_LEASE_STDOUT | RF_LEASE_STDERR,
                   0);
    if (pid < 0) {
        fprintf(stderr, "rfork returned err %d\n", pid);
        return pid;
    }

    if (pid == 0) {
        // child

        if (r->infile) {
            u32 buf[2] = { RES_FILE, -1 };
            stdin_fd   = ak_open(r->infile, O_READ | O_OPEN_EXISTING);
            if (stdin_fd < A_OK) {
                fprintf(stderr, "Could not open %s\n", r->infile);
                ak_exit(127);
            }
            buf[1] = stdin_fd;
            ak_dev_ctl(PDEV_STDIN, PX_ATTACH, buf, sizeof(buf));
        }

        if (r->outfile) {
            u32 buf[2] = { RES_FILE, -1 };
            int mode   = r->append ? O_APPEND : O_CREATE_ALWAYS;
            stdout_fd  = ak_open(r->outfile, O_WRITE | mode);
            if (stdout_fd < A_OK) {
                fprintf(stderr, "Could not open %s\n", r->outfile);
                if (stdin_fd >= 0) ak_close(stdin_fd);
                ak_exit(127);
            }
            buf[1] = stdout_fd;
            ak_dev_ctl(PDEV_STDOUT, PX_ATTACH, buf, sizeof(buf));
        }

        res = ak_exec(path, argc, argv, O_EXEC_GUESS);
        fprintf(stderr, "Error executing %s (%d) [%x]\n", path, res, ak_error(0));
        if (stdin_fd >= 0) ak_close(stdin_fd);
        if (stdout_fd >= 0) ak_close(stdout_fd);
        ak_exit(127);
    }

    // parent
    res = ak_wait(pid, &status, WAIT_HANG);
    ak_set_preempt(NO_PREEMPT);
    if (r->heredoc) ak_unlink(r->herepath);
    if (res < 0) {
        fprintf(stderr, "Shell error: wait %d\n", res);
        return res;
    }

    return status;
}

static int run_builtin_with_redir(int (*func)(int, char **), int argc, char **argv,
                                  struct AkshRedir *r)
{
    u32 saved_in  = ak_dev_ctl(PDEV_STDIN, PX_GET_DEV, NULL, 0);
    u32 saved_out = ak_dev_ctl(PDEV_STDOUT, PX_GET_DEV, NULL, 0);
    int res, stdin_fd = -1, stdout_fd = -1;

    if (r->infile) {
        u32 buf[2] = { RES_FILE, -1 };
        stdin_fd   = ak_open(r->infile, O_READ | O_OPEN_EXISTING);
        if (stdin_fd < A_OK) {
            fprintf(stderr, "Could not open %s\n", r->infile);
            return (127);
        }
        buf[1] = stdin_fd;
        ak_dev_ctl(PDEV_STDIN, PX_ATTACH, buf, sizeof(buf));
    }

    if (r->outfile) {
        int res;
        u32 buf[2] = { RES_FILE, -1 };
        int mode   = r->append ? O_APPEND : O_CREATE_ALWAYS;
        stdout_fd  = ak_open(r->outfile, O_WRITE | mode);
        if (stdout_fd < A_OK) {
            if (stdin_fd < 0) ak_close(stdin_fd);
            fprintf(stderr, "Could not open %s\n", r->outfile);
            return (127);
        }
        buf[1] = stdout_fd;
        res    = ak_dev_ctl(PDEV_STDOUT, PX_ATTACH, buf, sizeof(buf));
    }

    res = func(argc, argv);

    if (r->infile) {
        u32 buf[2];
        buf[0] = PDEV_GET_RES_TYPE(saved_in);
        buf[1] = PDEV_GET_RES(saved_in);

        fflush(stdin);
        clearerr(stdin);
        ak_dev_ctl(PDEV_STDIN, PX_ATTACH, buf, sizeof(buf));
        ak_close(stdin_fd);
        if (r->heredoc) ak_unlink(r->herepath);
    }

    if (r->outfile) {
        u32 buf[2];
        buf[0] = PDEV_GET_RES_TYPE(saved_out);
        buf[1] = PDEV_GET_RES(saved_out);

        fflush(stdout);
        clearerr(stdout);
        ak_dev_ctl(PDEV_STDOUT, PX_ATTACH, buf, sizeof(buf));
        ak_seek(stdout_fd, 0, SEEK_SET);
        ak_close(stdout_fd);
    }

    return res;
}

static int check_shebang(const char *path, char *out, usize outsz, char **out_argv, usize max_arg)
{
    usize len;
    char *p = out;
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    if (fgets(out, outsz, f) == NULL) goto fail;

    len = strlen(out);
    if (len < 3) goto fail;

    if (*p++ == '#' && *p++ == '!') {
        out = p;
        return tokenize(p, strlen(p), out_argv, max_arg);
    }

fail:
    fclose(f);
    return 0;
}

static int aksh_exec(int argc, char **argv)
{
    int  i     = 0;
    int  found = 0;
    int  res;
    char fullpath[AKSH_MAX_PATH];

    struct AkshRedir r;

    if (!(argc > 0 && argv && argv[0])) {
        fprintf(stderr, "Invalid input to aksh_exec\n");
        res = 127;
        goto setcode;
    }

    if (argv[0][0] == '#') {
        res = aksh_last_exit_code;
        goto setcode;
    }

    parse_redirection(&argc, argv, &r);

    while (builtins[i].name) {
        if (strcmp(argv[0], builtins[i].name) == 0) {
            res = run_builtin_with_redir(builtins[i].func, argc, argv, &r);
            goto setcode;
        }
        i++;
    }

    if (find_in_path(argv[0], fullpath, sizeof(fullpath))) {
        char  bangs[AKSH_MAX_PATH];
        char *sargv[AKSH_MAX_ARGV * 2];
        int   sargc;

        if ((sargc = check_shebang(fullpath, bangs, sizeof(bangs), sargv, AKSH_MAX_ARGV))) {
            usize i;
            for (i = 0; i < argc; i++) sargv[sargc + i] = argv[i];
            res = spawn_and_wait(sargv[0], sargc + argc - 1, &sargv[1], &r);
            goto setcode;
        } else {
            res = spawn_and_wait(fullpath, argc - 1, &argv[1], &r);
            goto setcode;
        }
    } else {
        fprintf(stderr, "Command not found: %s\n", argv[0]);
    }

    res = 127;
setcode:
    aksh_last_exit_code = res;
    return res;
}

int repl(const char *autoexec)
{
    char  line[AKSH_MAX_LINE];
    char *argv[AKSH_MAX_ARGV];
    u8    imode[1] = { IN_CANON | IN_ECHO | IN_CRNL };
    u8    omode[1] = { OUT_NLCR };
    int   res;
    bool  docold = true;

reset:
    ak_set_preempt(NO_PREEMPT);

    res = ak_dev_ctl(PDEV_STDIN, PX_SETCANON, imode, 1);
    if (res) ak_exit(1);

    res = ak_dev_ctl(PDEV_STDOUT, PX_SETCANON, omode, 1);
    if (res) ak_exit(2);

    if (autoexec && docold) {
        char *args[2];
        docold  = false;
        args[0] = "aksh";
        args[1] = (char *)autoexec;
        run_batch(2, args);
    }

    for (;;) {
        prompt(aksh_prompt_path, aksh_prompt);
        if (!fgets(line, sizeof(line), stdin)) {
            fputs("Unexpected EOF!\n", stderr);
            clearerr(stdin);
            goto reset;
        } else {
            // tokenize uses the same line  buffer
            int argc = tokenize(line, strlen(line), argv, AKSH_MAX_ARGV);
            if (argc > 0) res = aksh_exec(argc, argv);

            if (aksh_exit) break;
        }
    }

    return res;
}

int main(int argc, char **argv)
{
    int res;
    if (argc < 3) {
        return repl(NULL);
    } else if (argc >= 3 && strcmp(argv[1], "-c") == 0) {
        char *cargv[AKSH_MAX_ARGV];
        int   cargc = tokenize(argv[2], strlen(argv[2]), cargv, AKSH_MAX_ARGV);
        if (cargc > 0) res = aksh_exec(cargc, cargv);
        ak_set_preempt(PREEMPT_ROBIN);
        return res;
    } else if (argc >= 3 && strcmp(argv[1], "-f") == 0) {
        char *args[2];
        args[0] = "aksh";
        args[1] = argv[2];
        return run_batch(2, args);
    } else if (argc >= 3 && strcmp(argv[1], "-x") == 0) {
        return repl(argv[2]);
    }

    return 0;
}
