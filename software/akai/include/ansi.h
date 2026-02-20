#ifndef ANSI_H
#define ANSI_H

#include "akai_def.h"

/* @AKAI: 910_ANSI */

#ifdef INCLUDE_ANSI_IN
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
#endif

enum AnsiCmd {
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
    ANSI_SU  = 'S',
    ANSI_SD  = 'T',

    /* Private SCO modes */
    ANSI_SCP = 's',
    ANSI_RCP = 'u',
    /* Private modes use a bitmask (e.g., 0x100) to stay unique */
    ANSI_PRIV_H = 'h' | 0x100,
    ANSI_PRIV_L = 'l' | 0x100
};

enum AnsiSGR {
    ANSI_SGR_RESET            = 0,
    ANSI_SGR_BOLD             = 1,
    ANSI_SGR_DIM              = 2,
    ANSI_SGR_ITALIC           = 3,
    ANSI_SGR_UNDERLINE        = 4,
    ANSI_SGR_BLINK            = 5,
    ANSI_SGR_REVERSE          = 7,
    ANSI_SGR_TRANSPARENT      = 8,
    ANSI_SGR_FONT_0           = 10,
    ANSI_SGR_FONT_1           = 11,
    ANSI_SGR_FONT_2           = 12,
    ANSI_SGR_FONT_3           = 13,
    ANSI_SGR_NORMAL_INTENSITY = 22,
    ANSI_SGR_NOT_ITALIC       = 23,
    ANSI_SGR_NOT_UNDERLINE    = 24,
    ANSI_SGR_NOT_BLINK        = 25,
    ANSI_SGR_NOT_REVERSE      = 27,
    ANSI_SGR_NOT_TRANSPARENT  = 28,
    ANSI_SGR_SET_FG           = 30,
    ANSI_SGR_SET_FG_256       = 38,
    ANSI_SGR_DEFAULT_FG       = 39,
    ANSI_SGR_SET_BG           = 40,
    ANSI_SGR_SET_BG_256       = 48,
    ANSI_SGR_DEFAULT_BG       = 49,

};

#define ANSI_MAX_PARAMS 16

/* @AKAI */

enum AnsiState { ANSI_STATE_GROUND, ANSI_STATE_ESC, ANSI_STATE_CSI };

typedef struct {
    enum AnsiState state; /* 0:GROUND, 1:ESC, 2:CSI */
    int            params[ANSI_MAX_PARAMS];
    int            p_idx;
    int            current_num;
    bool           is_private;
} AnsiParser;

typedef struct {
    enum AnsiCmd type;
    int          params[ANSI_MAX_PARAMS];
    int          param_count;
    u8           chr; /* The actual character if type == ANSI_CHAR */
} AnsiToken;

#endif /* ANSI_H */
