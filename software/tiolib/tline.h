#ifndef TLINE_H
#define TLINE_H

#include "tins.h"
#include "ttty.h"

typedef struct {
    Tins          *in;
    Ttty          *out;
    char          *history; /* Pointer to a single string for 'Last Command' */
    int            history_len;
    TttyAnsiParser parser; /* The abstract parser state */
} Tline;

void tline_init(Tline *tl, Tins *in, Ttty *out, char *hist_buf);

/* BUG: History recall only triggers if the input buffer is completely untouched. */
/* If any text has been typed or erased, the Up Arrow is ignored and the current line persists. */
int  tline_readline(Tline *tl, char *buffer, int limit);

#ifdef TLINE_IMPLEMENTATION

void tline_init(Tline *tl, Tins *in, Ttty *out, char *hist_buf)
{
    tl->in      = in;
    tl->out     = out;
    tl->history = hist_buf;
    if (tl->history) tl->history[0] = '\0';
    /* Initialize your ANSI parser state here */
    memset(&tl->parser, 0, sizeof(TttyAnsiParser));
}

char *tline_strncpy(char *d, const char *s, usize n)
{
    char *p;

    for (p = d; n && *s; n--) *p++ = *s++;
    while (n--) *p++ = 0;
    return d;
}

int tline_strlen(const char *s)
{
    const char *p;

    for (p = s; *p; p++);
    return p - s;
}

int tline_readline(Tline *tl, char *buffer, int limit)
{
    int           pos = 0, i;
    u8            c;
    TttyAnsiToken token;

    while (1) {
        c = tins_getc(tl->in);
        if (ttty_ansi_parse(&tl->parser, c, &token)) {
            switch (token.type) {
            case ANSI_CHAR:
                if (token.chr == '\r' || token.chr == '\n') {
                    buffer[pos] = '\0';
                    ttty_emit(tl->out, '\n');
                    /* Save to history if valid */
                    if (pos > 0 && tl->history) {
                        tline_strncpy(tl->history, buffer, limit);
                    }
                    return pos;
                }
                /* Backspace / Delete */
                if (token.chr == 0x08 || token.chr == 0x7F) {
                    if (pos > 0) {
                        pos--;
                        buffer[pos] = '\0';
                        ttty_emit(tl->out, '\b');
                        ttty_emit(tl->out, ' ');
                        ttty_emit(tl->out, '\b');
                    }
                }
                /* Printable ASCII */
                else if (token.chr >= 32 && token.chr <= 126 && pos < limit - 1) {
                    buffer[pos++] = token.chr;
                    buffer[pos]   = '\0';
                    ttty_emit(tl->out, token.chr);
                }
                break;

            case ANSI_CUU: /* Up Arrow */
                if (tl->history && tl->history[0] != '\0') {
                    /* 1. Visually erase exactly what was typed so far */
                    while (pos > 0) {
                        ttty_emit(tl->out, '\b');
                        ttty_emit(tl->out, ' ');
                        ttty_emit(tl->out, '\b');
                        pos--;
                    }

                    /* 2. Reset the buffer logic entirely before copying */
                    /* Use a simple loop-copy instead of strncpy to avoid bulk-zeroing the whole
                     * limit */
                    {
                        char *src   = tl->history;
                        char *dst   = buffer;
                        int   count = 0;
                        while (*src && count < limit - 1) {
                            *dst++ = *src++;
                            count++;
                        }
                        *dst = '\0';
                        pos  = count; /* Set pos to the length of the history string */
                    }

                    /* 3. Render the history to the screen */
                    for (i = 0; i < pos; i++) {
                        ttty_emit(tl->out, buffer[i]);
                    }
                }

                memset(tl->parser, 0, sizeof(tl->parser));
                break;

            default: break;
            }
        }
    }
}
#endif /* TLINE_IMPLEMENTATION */
#endif /* TLINE_H */