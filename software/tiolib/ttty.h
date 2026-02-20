#ifndef TTTY_h
#define TTTY_h

/***
 *      ttty.h is a header-only library that provides device-agnostic terminal /
 * console / text engine functionality without using the standard library or any
 * hardware.
 *
 *      To use: simply include this header in your program, and, in ONLY ONE
 * COMPILATION UNIT, #define TTTY_IMPLEMENTATION
 *      Then, write a function with signature:
 *          void ttty_outc(struct Ttty, char);
 *      That outputs a character to your desired location (buffer, serial port,
 * socket ...). You may use the ttty.priv pointer for any needs you have to
 * implement this function.
 *      Then just instantiate, by using Ttty_init or building a Ttty struct
 * manually. Remember it is C89.
 */

#ifndef HAS_LIBSIRIUS_TYPES
typedef unsigned char u8;
typedef unsigned int  u32;
typedef i8 bool;
#define true  1
#define false 0
#endif

#ifndef TTTY_MUX_MAX_TARGETS
#define TTTY_MUX_MAX_TARGETS 4
#endif

#ifndef TTTY_TAB_W
#define TTTY_TAB_W 8
#endif

#ifndef TTTY_MAX_BPC
#define TTTY_MAX_BPC 8
#endif

#ifdef TTTY_USE_PRINTF
/* Sorry, we need stdarg.h for printf */
#include <stdarg.h>
/* May as well get ctype.h */
#include <ctype.h>
#endif

enum TttyAnsiCmd {
    ANSI_NONE = 0,
    ANSI_CHAR = 1,

    /* Standard CSI */
    ANSI_CUU = 'A',
    ANSI_CUD = 'B',
    ANSI_CUF = 'C',
    ANSI_CUB = 'D',
    ANSI_CNL = 'E',
    ANSI_CPL = 'F',
    ANSI_CHA = 'G',
    ANSI_CUP = 'H',
    ANSI_ED  = 'J',
    ANSI_EL  = 'K',
    ANSI_SGR = 'm',

    /* Private modes use a bitmask (e.g., 0x100) to stay unique */
    ANSI_PRIV_H = 'h' | 0x100,
    ANSI_PRIV_L = 'l' | 0x100
};

#define TTTY_MAX_ANSI_PARAMS 4
enum TttyAnsiState { ANSI_STATE_GROUND, ANSI_STATE_ESC, ANSI_STATE_CSI };

typedef struct {
    enum TttyAnsiState state; /* 0:GROUND, 1:ESC, 2:CSI */
    int                params[TTTY_MAX_ANSI_PARAMS];
    int                p_idx;
    int                current_num;
    bool               is_private;
} TttyAnsiParser;

typedef struct {
    enum TttyAnsiCmd type;
    int              params[4];
    int              param_count;
    u8               chr; /* The actual character if type == ANSI_CHAR */
} TttyAnsiToken;

enum TttyMode {
    TTTY_STREAM,     /* The tty writes to a stream of bytes */
    TTTY_XY,         /* The tty writes to a 2d screen of chars */
    TTTY_XYLF = 1,   /* Same as TTTY_XY, LF is a newline */
    TTTY_XYCRLF,     /* CRLF needed for newline */
    TTTY_ANSI = 0x80 /*Flag for ansi*/
};

typedef struct Ttty {
    const char *name; /* The tty name */

    u8 w, h; /* Viewport dimensions */
    u8 x, y; /* Logical cursor position */
    u8 bpc;  /* bytes per character */

    bool backspace;

    enum TttyMode mode;
    void         *priv; /* Any data needed for outchar (e.g. a serial port, a file) */

    /* ANSI parser state */
    bool           has_ansi;
    TttyAnsiParser ansi;

    u8 default_att[TTTY_MAX_BPC]; /* up to MAX_BPC attributes */
    u8 tabsize;

    /* Implementation defined function. Outputs a character. */
    void (*outchar)(struct Ttty *tty, char c);

    /* Implementation defined functions for XY modes. Sets the logical cursor position*/
    void (*setpos)(struct Ttty *tty, u8 x, u8 y);
} Ttty;

enum TttyMuxPriority {
    TTTY_DEBUG = 0,  // Verbose trace info (Interrupt logs, register dumps)
    TTTY_INFO  = 10, // General status (Device detected, loading driver)
    TTTY_WARN  = 20, // Non-fatal issues (Checksum mismatch on non-critical file)
    TTTY_ERROR = 30, // Fatal issues (Hardware not found, boot failure)
    TTTY_NONE  = 255 // Silence everything
};

struct MuxSlot {
    Ttty                *device;
    enum TttyMuxPriority priority;
};

typedef struct {
    struct MuxSlot       slots[TTTY_MUX_MAX_TARGETS];
    enum TttyMuxPriority priority;
    u8                   count;
} TttyMux;

/* returns true on success */
bool ttty_init(Ttty *self, const char *name, enum TttyMode mode, u8 w, u8 h, u8 bpc, void *priv,
               u8 *default_att, void (*emit)(struct Ttty *, char),
               void (*setpos)(struct Ttty *, u8, u8));
void ttty_putc(Ttty *tty, char c);
void ttty_puts(Ttty *tty, const char *s);
void ttty_clear(Ttty *tty);

/* Returns false if all slots are full */
bool ttty_mux_subscribe(TttyMux *mux, Ttty *tty, enum TttyMuxPriority p);
void ttty_mux_set_priority(TttyMux *mux, enum TttyMuxPriority p);
void ttty_mux_putc(TttyMux *mux, char c);
void ttty_mux_puts(TttyMux *mux, const char *s);
void ttty_mux_clear(TttyMux *mux);

/* Returns true only when a complete token is ready */
bool ttty_ansi_parse(TttyAnsiParser *p, u8 c, TttyAnsiToken *out);

#ifdef TTTY_USE_PRINTF
void ttty_printf(Ttty *tty, const char *fmt, ...);
void ttty_mux_printf(TttyMux *mux, const char *fmt, ...);
#endif

// --------------------------------------------------------------------------
#ifdef TTTY_IMPLEMENTATION

bool ttty_init(Ttty *tty, const char *name, enum TttyMode mode, u8 w, u8 h, u8 bpc, void *priv,
               u8 *default_att, void (*emit)(struct Ttty *, char),
               void (*setpos)(struct Ttty *, u8, u8))
{
    u8 i;

    if (bpc > TTTY_MAX_BPC) return false;
    tty->name     = name;
    tty->mode     = mode & (~TTTY_ANSI); /*TODO: very ugly, use another interface*/
    tty->has_ansi = (mode & TTTY_ANSI) ? true : false;
    if (tty->has_ansi) {
        tty->ansi.current_num = 0;
        tty->ansi.is_private  = false;
        tty->ansi.p_idx       = 0;
        tty->ansi.state       = ANSI_STATE_GROUND;
    }
    tty->w         = w;
    tty->h         = h;
    tty->bpc       = bpc;
    tty->x         = 0;
    tty->y         = 0;
    tty->priv      = priv;
    tty->outchar   = emit;
    tty->setpos    = setpos;
    tty->backspace = false;
    tty->tabsize   = TTTY_TAB_W;

    for (i = 0; i < bpc - 1; i++) tty->default_att[i] = default_att[i];

    return true;
}

bool ttty_mux_subscribe(TttyMux *mux, Ttty *tty, enum TttyMuxPriority p)
{
    if (mux->count >= TTTY_MUX_MAX_TARGETS) return false;
    mux->slots[mux->count].device     = tty;
    mux->slots[mux->count++].priority = p;
    return true;
}

void ttty_mux_set_priority(TttyMux *mux, enum TttyMuxPriority p)
{
    mux->priority = p;
}

static bool ttty_mux_check_priority(TttyMux *mux, u8 slot)
{
    if (slot < mux->count)
        return mux->slots[slot].priority >= mux->priority;
    else
        return false;
}

static void ttty_update_cursor_pos(Ttty *tty)
{
    tty->setpos(tty, tty->x, tty->y);
}

static void ttty_set_xy(Ttty *tty, int x, int y)
{
    /* Clamp to screen boundaries */
    if (x < 0) x = 0;
    if (x >= tty->w) x = tty->w - 1;
    if (y < 0) y = 0;
    if (y >= tty->h) y = tty->h - 1;

    tty->x = x;
    tty->y = y;

    ttty_update_cursor_pos(tty);
}

static void ttty_scroll_up(Ttty *tty)
{
    if (tty->priv) {
        u8 *vram       = (u8 *)tty->priv;
        int row_bytes  = tty->w * tty->bpc;
        int total_size = tty->w * tty->h * tty->bpc;
        int i, k;

        /* Move rows up */
        for (i = 0; i < total_size - row_bytes; i++) {
            vram[i] = vram[i + row_bytes];
        }

        /* Clear last line */
        for (i = total_size - row_bytes; i < total_size; i += tty->bpc) {
            vram[i] = ' ';
            for (k = 1; k < tty->bpc; k++) vram[i + k] = tty->default_att[k - 1];
        }
    }
    tty->y = tty->h - 1;
}

static void ttty_handle_tab(Ttty *tty)
{
    int spaces = tty->tabsize - (tty->x % tty->tabsize);

    if (tty->x + spaces >= tty->w) {
        tty->x = 0;
        tty->y++;
    } else {
        usize i, k;
        u8   *vram      = tty->priv;
        u32   start_idx = (tty->y * tty->w + tty->x) * tty->bpc;
        u32   end_idx   = start_idx + (spaces * tty->bpc);

        for (i = start_idx; i < end_idx; i += tty->bpc) {
            vram[i] = ' ';
            for (k = 1; k < tty->bpc; k++) vram[i + k] = tty->default_att[k - 1];
        }

        tty->x += spaces;
    }

    if (tty->y >= tty->h) ttty_scroll_up(tty);
}

// Only scroll on end of screen, dont do any processing
void ttty_emit_raw(Ttty *tty, char c)
{
    if (!tty || !tty->outchar) return;

    if (tty->mode == TTTY_STREAM) {
        tty->outchar(tty, c);
        return;
    }

    /* Trigger output ONLY if within defined bounds */
    if (tty->outchar && (tty->x < tty->w) && (tty->y < tty->h)) {
        tty->outchar(tty, c);
    }
    
    tty->x++;

    if (tty->x >= tty->w) {
        tty->x = 0;
        tty->y++;
    }

    if (tty->y >= tty->h) ttty_scroll_up(tty);

    ttty_update_cursor_pos(tty); /* Sync hardware cursor index */
}

void ttty_emit(Ttty *tty, char c)
{
    if (!tty || !tty->outchar) return;

    if (tty->mode == TTTY_STREAM) {
        tty->outchar(tty, c);
        return;
    }

    /* Handle newline and c-return */
    switch (c) {
    case '\n':
        if (tty->mode == TTTY_XYLF) tty->x = 0;
        tty->y++;
        goto check_bounds;
    case '\r': tty->x = 0; goto sync_hw;
    case '\b':
        if (tty->x > 0) tty->x--;
        tty->backspace = true;  
        goto sync_hw;
    case '\t': ttty_handle_tab(tty); goto sync_hw;
    case 0x0c:
        /* formfeed, \f, clear screen */
        ttty_clear(tty);
        goto sync_hw;
    default: break;
    }

    /* Automatic Wrapping */
    if (tty->x >= tty->w) {
        tty->x = 0;
        tty->y++;
    }

check_bounds:
    /* Automatic Scrolling */
    if (tty->y >= tty->h) ttty_scroll_up(tty);

    if (c >= ' ') {
        /* Trigger output ONLY if within defined bounds */
        if (tty->outchar && (tty->x < tty->w) && (tty->y < tty->h)) {
            tty->outchar(tty, c);
        }
        tty->x++;
    }

sync_hw:
    ttty_update_cursor_pos(tty); /* Sync hardware cursor index */
}

static void ttty_execute_ansi(Ttty *tty, TttyAnsiToken *cmd)
{
    u8  *vram  = (u8 *)tty->priv;
    int *p     = cmd->params;
    int  curp0 = p[0] - (p[0] > 0);
    int  curp1 = p[1] - (p[1] > 0);
    int  curp2 = p[2] - (p[2] > 0);
    int  curp3 = p[3] - (p[3] > 0);
    int  n     = (p[0] > 0) ? p[0] : 1;

    switch (cmd->type) {
    case ANSI_CUU: ttty_set_xy(tty, tty->x, tty->y - n); break;
    case ANSI_CUD: ttty_set_xy(tty, tty->x, tty->y + n); break;
    case ANSI_CUF: ttty_set_xy(tty, tty->x + n, tty->y); break;
    case ANSI_CUB: ttty_set_xy(tty, tty->x - n, tty->y); break;
    case ANSI_CNL: ttty_set_xy(tty, 0, tty->y + n); break;
    case ANSI_CPL: ttty_set_xy(tty, 0, tty->y - n); break;
    case ANSI_CHA: ttty_set_xy(tty, curp0, tty->y); break;
    case ANSI_CUP: ttty_set_xy(tty, curp1, curp0); break;
    case ANSI_ED: {
        /* TODO: This is a mess. Figure it out*/
        int start_cell  = 0;
        int c           = 0;
        int end_cell    = tty->w * tty->h;
        int cursor_cell = tty->y * tty->w + tty->x;

        if (p[0] == 0)
            start_cell = cursor_cell; /* Clear from cursor to end */
        else if (p[0] == 1)
            end_cell = cursor_cell + 1; /* Clear beginning to cursor */
        else if (p[0] == 2) {
            /* Clear entire screen */
            ttty_clear(tty);
            break;
        }

        for (c = start_cell; c < end_cell; c++) {
            int addr   = c * tty->bpc;
            int j      = 0;
            vram[addr] = ' ';
            for (j = 1; j < tty->bpc; j++) vram[addr + j] = tty->default_att[j - 1];
        }
        break;
    }
    case ANSI_EL: {
        int row_start   = tty->y * tty->w;
        int row_end     = (tty->y + 1) * tty->w;
        int cursor_cell = tty->y * tty->w + tty->x;
        int c           = 0;

        int start_cell = row_start;
        int end_cell   = row_end;

        if (p[0] == 0)
            start_cell = cursor_cell; /* Cursor to end of line */
        else if (p[0] == 1)
            end_cell = cursor_cell + 1; /* Start of line to cursor */
        /* p[0] == 2 clears the full line (default start/end) */

        for (c = start_cell; c < end_cell; c++) {
            int addr   = c * tty->bpc;
            int j      = 0;
            vram[addr] = ' ';
            for (j = 1; j < tty->bpc; j++) vram[addr + j] = tty->default_att[j - 1];
        }

        break;
    }

    /* TODO: SU */
    /* TODO: SD *
    /* NOT PLANNED: HVP */
    case ANSI_SGR: /*TODO: SGR*/
    /* TODO: private modes */
    default: break;
    }
}

void ttty_putc_ansi(Ttty *tty, u8 c)
{
    TttyAnsiToken tk;

    if (tty->mode == TTTY_STREAM) {
        ttty_emit(tty, c);
        return;
    }

    if (ttty_ansi_parse(&tty->ansi, c, &tk)) {
        if (tk.type == ANSI_NONE)
            return;
        else if (tk.type == ANSI_CHAR)
            ttty_emit(tty, c);
        else {
            ttty_execute_ansi(tty, &tk);
        }
    }
}

void ttty_putc(Ttty *tty, char c)
{
    if (tty->has_ansi) {
        ttty_putc_ansi(tty, c);
    } else
        ttty_emit(tty, c);
}

void ttty_mux_putc(TttyMux *mux, char c)
{
    int i;
    for (i = 0; i < mux->count; i++) {
        // Only broadcast if target's priority meets or exceeds the current Mux
        // threshold
        if (mux->priority >= mux->slots[i].priority) {
            ttty_putc(mux->slots[i].device, c);
        }
    }
}

void ttty_puts(Ttty *tty, const char *s)
{
    while (*s) ttty_putc(tty, *s++);
}

void ttty_mux_puts(TttyMux *mux, const char *s)
{
    int i;
    for (i = 0; i < mux->count; i++) {
        /* Only broadcast if target's priority meets or exceeds the current Mux
         * threshold */
        if (mux->priority >= mux->slots[i].priority) {
            ttty_puts(mux->slots[i].device, s);
        }
    }
}

void ttty_clear(Ttty *tty)
{
    if (!tty || tty->mode == TTTY_STREAM) return;

    tty->x = 0;
    tty->y = 0;

    /*TODO: macro to use memset */
    if (tty->mode == TTTY_XY && tty->priv) {
        u8 *vram = (u8 *)tty->priv;
        int i, j;
        for (i = 0; i < (tty->w * tty->h) * tty->bpc; i += tty->bpc) {
            vram[i] = ' ';
            for (j = 1; j < tty->bpc; j++) vram[i + j] = tty->default_att[j - 1];
        }
    }
}

void ttty_mux_clear(TttyMux *mux)
{
    int i;
    for (i = 0; i < mux->count; i++) {
        /* Only broadcast if target's priority meets or exceeds the current Mux
         * threshold */
        if (mux->priority >= mux->slots[i].priority) {
            ttty_clear(mux->slots[i].device);
        }
    }
}

/* IMPORTANT: this function CANNOT return ANSI_CHAR */
static enum TttyAnsiCmd ansi_type(u8 c, bool is_private)
{
    if (is_private) {
        /* Add the private bitmask to the character */
        return (enum TttyAnsiCmd)(c | 0x100);
    }

    return (enum TttyAnsiCmd)c;
}

/* Returns true only when a complete token (Char or Sequence) is ready */
bool ttty_ansi_parse(TttyAnsiParser *p, u8 c, TttyAnsiToken *out)
{
    int i     = 0;
    out->type = ANSI_NONE;

    switch (p->state) {
    case ANSI_STATE_GROUND:
        if (c == 0x1B) {
            p->state = ANSI_STATE_ESC;
            return false;
        }

        out->type = ANSI_CHAR;
        out->chr  = c;
        return true;

    case ANSI_STATE_ESC:
        if (c == '[') {
            usize i;
            p->state       = ANSI_STATE_CSI;
            p->p_idx       = 0;
            p->current_num = 0;
            p->is_private  = false;
            for (i = 0; i < TTTY_MAX_ANSI_PARAMS; i++) out->params[i] = 0;
            return false;
        }
        p->state = ANSI_STATE_GROUND;
        return false;

    case ANSI_STATE_CSI:
        if (c == '?') {
            p->is_private = true;
            return false;
        }
        if (c >= '0' && c <= '9') {
            p->current_num = (p->current_num * 10) + (c - '0');
            return false;
        }
        if (c == ';') {
            p->params[p->p_idx++] = p->current_num;
            p->current_num        = 0;
            return false;
        }

        /* Terminal Character - Build the token */
        p->params[p->p_idx++] = p->current_num;
        out->param_count      = p->p_idx;
        for (i = 0; i < p->p_idx; i++) out->params[i] = p->params[i];

        out->type = ansi_type(c, p->is_private);
        p->state  = ANSI_STATE_GROUND;
        return true;
    }
    return false;
}

#ifdef TTTY_USE_PRINTF

/* The "Master Engine" that everything calls */
static void ttty_vformat(void (*emit)(void *, char), void *user, const char *fmt, va_list ap)
{
    static char *null_str = "(NULL)";
    char         c;
    char         specifier = 0;
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
                if (!s) s = null_str; /* print (NULL) if s is NULL */
                while (*s) emit(user, *s++);
                break;
            }
            case 'c': {
                char chr = va_arg(ap, int);
                emit(user, chr);
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
                    emit(user, pad_zero ? '0' : ' ');
                    content_len++;
                }

                /* Print the buffer in reverse */
                while (i > 0) emit(user, buf[--i]);
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

                if (pad_zero && is_neg) emit(user, '-');

emit_dec:
                content_len = i + (is_neg ? 1 : 0);

                while (content_len < width) {
                    emit(user, pad_zero ? '0' : ' ');
                    content_len++;
                }

                if (!pad_zero && is_neg) emit(user, '-');

                /* Print buffer in reverse (Left to Right) */
                while (i > 0) emit(user, buf[--i]);
                break;
            }
            case '%': {
                /* escaped % */
                emit(user, '%');
                break;
            }
            default: break;
            }
        } else {
            emit(user, c);
        }
    }
}

static void ttty_putc_wrapper(void *ttty, char c)
{
    ttty_putc((Ttty *)ttty, c);
}

static void ttty_mux_putc_wrapper(void *mux, char c)
{
    ttty_mux_putc((TttyMux *)mux, c);
}

void ttty_printf(Ttty *tty, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    ttty_vformat(ttty_putc_wrapper, tty, fmt, ap);
    va_end(ap);
}

void ttty_mux_printf(TttyMux *mux, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    ttty_vformat(ttty_mux_putc_wrapper, mux, fmt, ap);
    va_end(ap);
}

#endif /* TTTY_USE_PRINTF */
#endif /* TTTY_IMPLEMENTATION*/
#endif /* TTTY_H */