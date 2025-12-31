#ifndef TINS_H
#define TINS_H

/**
 *      Tins is a header only library that implement hardware agnostic input streams in C89.
 */

/* tino.h */

#ifndef HAS_LIBSIRIUS_TYPES
typedef unsigned char u8;
typedef unsigned int  u32;
typedef i8 bool;
#define true  1
#define false 0
#endif

#ifndef TINS_WAIT
#define TINS_WAIT                    \
    {                                \
        int i;                       \
        for (i = 0; i < 10000; i++); \
    }
#endif

enum TinsErr { TINS_NO_DATA = -1 };

typedef struct Tins {
    void *priv; /* Pointer to the hardware-specific driver/queue */
    u8    prev; /* Previous character read */
    /* A user-defined function to fetch the next byte from the stream */
    /* Should return TINS_NO_DATA if no data is available */
    int (*get_byte)(void *priv);

    /* checks a byte without consuming it */
    int (*peek)(void *priv);
} Tins;

static const char *ANSI_UP     = "\x1B[A";
static const char *ANSI_DOWN   = "\x1B[B";
static const char *ANSI_LEFT   = "\x1B[D";
static const char *ANSI_RIGHT  = "\x1B[C";
static const char *ANSI_HOME   = "\x1B[H";
static const char *ANSI_END    = "\x1B[F";
static const char *ANSI_DELETE = "\x1B[3~";
static const char *ANSI_PGUP   = "\x1B[5~";
static const char *ANSI_PGDOWN = "\x1B[6~";

static const char *ANSI_F1 = "\x1BOP";
static const char *ANSI_F2 = "\x1BOQ";
static const char *ANSI_F3 = "\x1BOR";
static const char *ANSI_F4 = "\x1BOS";

struct tins_ansi {
    const char *seq;            /* Pointer to a constant string of the escape sequence */
    u8          printable_char; /* if seq == NULL, this is a printable character */
};

u8   tins_getc(Tins *t);
bool tins_read(Tins *t, u8 *out_char);
void tins_skip_ansi_sequence(Tins *in);
int  tins_readline(Tins *in, void (*echo)(void *, char), void *user, char *buffer, int limit);

#ifdef TINS_IMPLEMENTATION

/* Blocking read: wait until the driver finally provides a byte */
u8 tins_getc(Tins *t)
{
    int result;
    while ((result = t->get_byte(t->priv)) == TINS_NO_DATA) {
        TINS_WAIT;
    };
    return (u8)result;
}

/* Non-blocking read: check once and return. Returns true if there's data */
bool tins_read(Tins *t, u8 *out_char)
{
    int result = t->get_byte(t->priv);
    if (result != TINS_NO_DATA) {
        *out_char = (u8)result;
        return true;
    }
    return false;
}

#endif
#endif /* TINS_H */
