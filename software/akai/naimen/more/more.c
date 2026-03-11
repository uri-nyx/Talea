#include <akai.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: implement isatty and related helpers in a library for akai

static struct {
    u32 w, h;
} Term;

#define SCROLLBACK 500

#define PROMPT                                                                      \
    do {                                                                            \
        if (end) {                                                                  \
            printf("\x1b[7m\x1b[%d;1H\x1b[2K(%s) END\x1b[27m", Term.h, fname);      \
        } else {                                                                    \
            printf("\x1b[7m\x1b[%d;1H\x1b[2K(%s) --MORE--\x1b[27m", Term.h, fname); \
        }                                                                           \
        fflush(stdout);                                                             \
    } while (0)

struct line {
    long  fpos;
    usize len;
};

static struct {
    usize        size, count;
    struct line *data;
} Lines;

#define LINE(n)    (&Lines.data[(n)])
#define LOFFSET(n) (Lines.data[(n)].fpos)
#define LLEN(n)    (Lines.data[(n)].len)

static void lappend(long pos, usize len)
{
    if (Lines.count >= Lines.size) {
        Lines.size = Lines.size ? Lines.size * 2 : Term.h;
        Lines.data = realloc(Lines.data, Lines.size * sizeof(struct line));
        if (Lines.data == NULL) {
            fputs("lappend: realloc failed!\n", stderr);
            exit(EXIT_FAILURE);
        }
    }

    Lines.data[Lines.count].fpos = pos;
    Lines.data[Lines.count].len  = len;
    Lines.count++;
}

static void clear_prompt(int at_line)
{
    printf("\x1b[%d;1H\x1b[2K\r", at_line);
}

static int getlines(FILE *f, char *buf, usize until, bool show)
{
    while ((Lines.count <= until) && fgets(buf, Term.w, f)) {
        usize len = strlen(buf);

        if (show) fputs(buf, stdout);

        lappend(ftell(f) - len, len);
    }
    return feof(f);
}

static int reprint(FILE *f, char *buf, usize line)
{
    int end = false;
    if (line < Lines.count) {
        fseek(f, LOFFSET(line), SEEK_SET);
        fgets(buf, Term.w, f);
        end = feof(f);
    } else {
        end = getlines(f, buf, line, false);
    }

    if (buf && !end) fputs(buf, stdout);
    return end;
}

void page(FILE *f, char *fname)
{
    usize top_line = 0, bot_line = 0, last_line = 0;
    int   end = false;
    char  c, *buf;

    Lines.count = 0;
    Lines.size  = 0;
    Lines.data  = NULL;

    buf = malloc(Term.w);

    if (!buf) {
        fputs("page: malloc failed!\n", stderr);
        exit(EXIT_FAILURE);
    }

    fputs("\x1b[?25l\x1b[2J\x1b[H", stdout);
    end      = getlines(f, buf, Term.h - 2, true);
    bot_line = Lines.count - 1;

    PROMPT;

again:

    // line_to_pos(file, line);

    c = toupper(getchar());

    switch (c) {
    case '\b':
        // whole page up
        if (top_line > 0) {
            long len    = Term.h - 1; // 29 rows
            long target = (long)top_line - len;
            target      = target < 0 ? 0 : target;

            fputs("\x1b[2J\x1b[H", stdout);

            top_line = target;
            while (target < top_line + len) {
                if (end = reprint(f, buf, target)) break;
                target++;
            }

            bot_line = target - 1;
        }
        PROMPT;
        goto again;
        break;
    case 'K':
    case '-':
        if (top_line > 0) {
            clear_prompt(Term.h);
            fputs("\x1b[1T\x1b[;H", stdout);
            top_line--;
            bot_line--;
            end = reprint(f, buf, top_line);
        }
        PROMPT;
        goto again;
        break;
    case ' ':
        // whole page down
        if (!end) {
            usize target = bot_line + Term.h - 2;

            fputs("\x1b[2J\x1b[H", stdout);
            reprint(f, buf, bot_line);

            while (bot_line < target && !end) {
                top_line++;
                bot_line++;
                end = reprint(f, buf, bot_line);
            }
        }
        PROMPT;
        goto again;
        break;
    case 'J':
    case '.':
    case '\n':
    case '\r':
        if (!end) {
            clear_prompt(Term.h);
            fprintf(stdout, "\x1b[1S\x1b[%d;1H", Term.h - 1);
            top_line++;
            bot_line++;
            end = reprint(f, buf, bot_line);
        }
        PROMPT;
        goto again;
        break;
    case 'Q': break;
    default: goto again; break;
    }

    puts("\x1b[27m\x1b[?25h");
    free(Lines.data);
    free(buf);
    return;
}

int main(int argc, char **argv)
{
    int   res;
    u8    oldmode;
    u8    imode = IN_BLOCK;
    FILE *f;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 127;
    }

    f = fopen(argv[1], "r");
    if (!f) {
        fprintf(stderr, "Could not open file '%s'\n", argv[1]);
        return 1;
    }

    ak_set_preempt(NO_PREEMPT);

    res = ak_dev_ctl(PDEV_STDIN, PX_GETCANON, NULL, 0);
    if (res < A_OK) goto failure;

    oldmode = (u8)res;
    setvbuf(stdin, NULL, _IONBF, 0);

    res = ak_dev_ctl(PDEV_STDIN, PX_SETCANON, &imode, 1);
    if (res) goto failure;

    // use isatty and query the terminal
    Term.h = 30;
    Term.w = 80;

    page(f, argv[1]);

    setvbuf(stdin, NULL, _IOLBF, 0);
    res = ak_dev_ctl(PDEV_STDIN, PX_SETCANON, &oldmode, 1);
    if (res) goto failure;

    fclose(f);
    ak_set_preempt(PREEMPT_ROBIN);
    return 0;

failure:
    fclose(f);
    ak_set_preempt(PREEMPT_ROBIN);
    return 1;
}
