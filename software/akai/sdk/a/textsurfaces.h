#ifndef TEXTSURFACES_H
#define TEXTSURFACES_H

/* A simple header only library to do some bit of Tui windowing with ANSI sequences*/

#include "../akai.h"

#define FG 0
#define BG 1
#define AT 2

enum TextSurfaceCTL {
    CTL_CURSOR_SHOW,
    CTL_CURSOR_BLINK,
    CTL_CURSOR_HIDE,
    CTL_CURSOR_NO_BLINK,
};

struct TextSurface {
    void (*set_cursor)(struct TextSurface *self, u8 x, u8 y);
    void (*get_cursor)(struct TextSurface *self, u8 *outx, u8 *outy);
    void (*get_info)(struct TextSurface *self, u8 *w, u8 *h, u8 *bpc);
    void (*display)(struct TextSurface *self, char c);
    void (*clear_line)(struct TextSurface *self, u8 line);
    void (*clear_text)(struct TextSurface *self);
    void (*scroll_up)(struct TextSurface *self);
    void (*scroll_down)(struct TextSurface *self);

    void (*fill_span)(struct TextSurface *self, u32 start_idx, u32 end_idx, char c);

    void (*ctl)(struct TextSurface *self, int command);

    u8 x, y, sx, sy;
    u8 ox, oy, w, h;

    /* configurations */
    u8 tabsize;
    u8 canonical;

    /* ANSI */
    AnsiParser ansi;
    u32        capabilities;
    u8         attr[3], default_attr[3], saved_attr[3];

    bool reversed, saved;
    bool forward_sgr, forward_privates;

    void *priv;
};

void         ts_tab(struct TextSurface *S);
void         ts_update_cursor(struct TextSurface *S);
void         ts_set_xy(struct TextSurface *S, int x, int y);
void         ts_emit(struct TextSurface *S, char c);
enum AnsiCmd ts_ansi_type(u8 c, bool is_private);
bool         ts_ansi_parse(struct TextSurface *S, u8 c, AnsiToken *out);
void         ts_ansi_execute(struct TextSurface *S, AnsiToken *cmd);
void         ts_ansi_putc(struct TextSurface *S, char c);
void         ts_resize(struct TextSurface *S, u8 nw, u8 nh, void *priv);

#ifdef TEXTSURFACES_RAW_TEXTBUFFER_COLOR

#define TEXTBUFFER_COLOR_BPC 4U
extern struct TextSurface Akai_Textbuffer;
bool                      atxt_init_textbuffer(u8 omode);
#endif

#ifdef TEXTSURFACES_IMPLEMENTATION

#ifdef TEXTSURFACES_RAW_TEXTBUFFER_COLOR

static void atxt_set_cursor(struct TextSurface *self, u8 x, u8 y)
{
    ak_dev_out(DEV_TEXTBUFFER, VIDEO_CUR_X, x);
    ak_dev_out(DEV_TEXTBUFFER, VIDEO_CUR_Y, y);
}

static void atxt_get_cursor(struct TextSurface *self, u8 *outx, u8 *outy)
{
    *outx = ak_dev_in(DEV_TEXTBUFFER, VIDEO_CUR_X);
    *outy = ak_dev_in(DEV_TEXTBUFFER, VIDEO_CUR_Y);
}

static void atxt_get_info(struct TextSurface *self, u8 *w, u8 *h, u8 *bpc)
{
    ak_dev_out(DEV_TEXTBUFFER, VIDEO_COMMAND, VIDEO_COMMAND_SYS_INFO);

    *bpc = ak_dev_in(DEV_TEXTBUFFER, VIDEO_GPU3);
    *w   = ak_dev_in(DEV_TEXTBUFFER, VIDEO_GPU4);
    *h   = ak_dev_in(DEV_TEXTBUFFER, VIDEO_GPU5);
}

static void atxt_display(struct TextSurface *self, char c)
{
    u32 *tb = (u32 *)AKAI_TEXTBUFFER;

    tb[((u32)self->y * (u32)self->w) + self->x] = (u32)c << 24 | (u32)self->attr[FG] << 16 |
                                                  (u32)self->attr[BG] << 8 | (u32)self->attr[AT];
}

static void atxt_clear_line(struct TextSurface *self, u8 line)
{
    _fillw((u8 *)AKAI_TEXTBUFFER + (line * self->w * TEXTBUFFER_COLOR_BPC),
           (u32)' ' << 24 | (u32)self->attr[FG] << 16 | (u32)self->attr[BG] << 8 | self->attr[AT],
           self->w);
}

static void atxt_clear_text(struct TextSurface *self)
{
    self->x = 0;
    self->y = 0;

    _fillw((u8 *)AKAI_TEXTBUFFER,
           (u32)' ' << 24 | (u32)self->attr[FG] << 16 | (u32)self->attr[BG] << 8 | self->attr[AT],
           self->w * self->h);
}

static void atxt_scroll_up(struct TextSurface *self)
{
    u8   *src   = (u8 *)AKAI_TEXTBUFFER + (self->w * TEXTBUFFER_COLOR_BPC);
    usize count = (self->h - 1) * self->w * TEXTBUFFER_COLOR_BPC;

    memcpy((u8 *)AKAI_TEXTBUFFER, src, count);
    self->clear_line(self, self->h - 1);

    if (self->y > 0)
        self->y--;
    else
        self->y = 0;
}

static void atxt_scroll_down(struct TextSurface *self)
{
    u8   *dst   = (u8 *)AKAI_TEXTBUFFER + (self->w * TEXTBUFFER_COLOR_BPC);
    usize count = (self->h - 1) * self->w * TEXTBUFFER_COLOR_BPC;

    _copybck((u8 *)AKAI_TEXTBUFFER, dst, count);
    atxt_clear_line(self, 0);

    if (self->y < self->h - 1)
        self->y++;
    else
        self->y = self->h - 1;
}

static void atxt_fill_span(struct TextSurface *self, u32 start_idx, u32 end_idx, char c)
{
    if (start_idx < end_idx)
        return;
    else {
        _fillw((u8 *)AKAI_TEXTBUFFER + (start_idx * TEXTBUFFER_COLOR_BPC),
               (u32)' ' << 24 | (u32)self->attr[FG] << 16 | (u32)self->attr[BG] << 8 |
                   self->attr[AT],
               end_idx - start_idx);
    }
}

static void atxt_ctl(struct TextSurface *self, int command)
{
    switch (command) {
    case CTL_CURSOR_HIDE:
    case CTL_CURSOR_BLINK:
    case CTL_CURSOR_SHOW:
    case CTL_CURSOR_NO_BLINK: {
        u8 csr = ak_dev_in(DEV_TEXTBUFFER, VIDEO_CSR);
        switch (command) {
        case CTL_CURSOR_SHOW: csr |= VIDEO_CURSOR_EN; break;
        case CTL_CURSOR_BLINK: csr |= VIDEO_CURSOR_BLINK; break;
        case CTL_CURSOR_HIDE: csr &= ~VIDEO_CURSOR_EN; break;
        case CTL_CURSOR_NO_BLINK: csr &= ~VIDEO_CURSOR_BLINK; break;
        }
        ak_dev_out(DEV_TEXTBUFFER, VIDEO_CSR, csr);
        break;
    }
    default: break;
    }
}

struct TextSurface Akai_Textbuffer;

#ifndef TEXTBUFFER_DEFAULT_FG
#define TEXTBUFFER_DEFAULT_FG 0xf
#endif

#ifndef TEXTBUFFER_DEFAULT_BG
#define TEXTBUFFER_DEFAULT_BG 0
#endif

#ifndef TEXTBUFFER_DEFAULT_AT
#define TEXTBUFFER_DEFAULT_AT 0
#endif

bool atxt_init_textbuffer(u8 omode)
{
    u8 bpc;
    memset(&Akai_Textbuffer, 0, sizeof(Akai_Textbuffer));

    Akai_Textbuffer.set_cursor  = atxt_set_cursor;
    Akai_Textbuffer.get_cursor  = atxt_get_cursor;
    Akai_Textbuffer.get_info    = atxt_get_info;
    Akai_Textbuffer.display     = atxt_display;
    Akai_Textbuffer.clear_line  = atxt_clear_line;
    Akai_Textbuffer.clear_text  = atxt_clear_text;
    Akai_Textbuffer.scroll_up   = atxt_scroll_up;
    Akai_Textbuffer.scroll_down = atxt_scroll_down;
    Akai_Textbuffer.fill_span   = atxt_fill_span;
    Akai_Textbuffer.ctl         = atxt_ctl;

    Akai_Textbuffer.get_info(&Akai_Textbuffer, &Akai_Textbuffer.w, &Akai_Textbuffer.h, &bpc);

    if (bpc != TEXTBUFFER_COLOR_BPC) return false;

    Akai_Textbuffer.default_attr[FG] = TEXTBUFFER_DEFAULT_FG;
    Akai_Textbuffer.default_attr[BG] = TEXTBUFFER_DEFAULT_BG;
    Akai_Textbuffer.default_attr[AT] = TEXTBUFFER_DEFAULT_AT;
    Akai_Textbuffer.attr[FG]         = TEXTBUFFER_DEFAULT_FG;
    Akai_Textbuffer.attr[BG]         = TEXTBUFFER_DEFAULT_BG;
    Akai_Textbuffer.attr[AT]         = TEXTBUFFER_DEFAULT_AT;

    Akai_Textbuffer.x                = 0;
    Akai_Textbuffer.y                = 0;
    Akai_Textbuffer.ox               = 0;
    Akai_Textbuffer.oy               = 0;
    Akai_Textbuffer.canonical        = omode;
    Akai_Textbuffer.tabsize          = 4;
    Akai_Textbuffer.reversed         = false;
    Akai_Textbuffer.saved            = false;
    Akai_Textbuffer.forward_privates = false;
    Akai_Textbuffer.forward_sgr      = false;
    Akai_Textbuffer.capabilities     = CAP_COLOR;

    Akai_Textbuffer.priv = NULL;
    memset(&Akai_Textbuffer.ansi, 0, sizeof(Akai_Textbuffer.ansi));
    Akai_Textbuffer.set_cursor(&Akai_Textbuffer, 0, 0);
    return true;
}

#endif

void ts_tab(struct TextSurface *S)
{
    int spaces = S->tabsize - (S->x % S->tabsize);

    if (S->x + spaces >= S->w) {
        S->x = 0;
        S->y++;
    } else {
        u32 start_idx = (S->y * S->w + S->x);

        S->fill_span(S, start_idx, start_idx + spaces, ' ');
        S->x += spaces;
    }

    if (!(S->canonical & OUT_NOSCROLL) && S->y >= S->h) S->scroll_up(S);
}

void ts_update_cursor(struct TextSurface *S)
{
    S->set_cursor(S, S->x, S->y);
}

void ts_set_xy(struct TextSurface *S, int x, int y)
{
    /* Clamp to screen boundaries */
    if (x < 0) x = 0;
    if (x >= S->w) x = S->w - 1;
    if (y < 0) y = 0;
    if (y >= S->h) y = S->h - 1;

    S->x = x;
    S->y = y;

    ts_update_cursor(S);
}

void ts_emit(struct TextSurface *S, char c)
{
    bool control = false;

    /* Handle newline and c-return */
    switch (c) {
    case '\n':
        if (S->canonical & OUT_NLCR) S->x = 0;
        S->y++;
        goto check_bounds;
    case '\r': S->x = 0; goto sync_hw;
    case '\b':
        if (S->x > 0) S->x--;
        goto sync_hw;
    case '\t': ts_tab(S); goto sync_hw;
    case 0x0c: S->clear_text(S); goto sync_hw;
    default: control = true; break;
    }

    /* Automatic Wrapping */
    if (S->x >= S->w) {
        if (S->canonical & OUT_NOWRAP) goto sync_hw;
        S->x = 0;
        S->y++;
    }

check_bounds:
    /* Automatic Scrolling */

    if (S->y >= S->h) {
        if (S->canonical & OUT_NOSCROLL) goto sync_hw;
        S->scroll_up(S);
    }

    if (c >= ' ') {
        /* Trigger output ONLY if within defined bounds */
        if ((S->x < S->w) && (S->y < S->h)) {
            S->display(S, c);
        }
        S->x++;
    } else if (control) {
        if ((S->x + 1 < S->w) && (S->y + 1 < S->h)) {
            S->display(S, '^');
            S->x++;
            S->display(S, c + 0x40);
            S->x++;
        } else {
            S->x += 2;
        }
    }

sync_hw:

    ts_update_cursor(S); /* Sync hardware cursor index */
}

/* IMPORTANT: this function CANNOT return (signed)ANSI_CHAR */
enum AnsiCmd ts_ansi_type(u8 c, bool is_private)
{
    if (is_private) {
        /* Add the private bitmask to the character */
        return (enum AnsiCmd)(c | 0x100);
    }

    return (enum AnsiCmd)c;
}

bool ts_ansi_parse(struct TextSurface *S, u8 c, AnsiToken *out)
{
    AnsiParser *p = &S->ansi;

    int i     = 0;
    out->type = ANSI_NONE;

    switch (p->state) {
    case ANSI_STATE_GROUND:
        p->lit_i = 0;
        memset(out->lit, 0, 32);
        out->lit[p->lit_i++] = c;

        if (c == 0x1B) {
            p->state = ANSI_STATE_ESC;
            return false;
        }

        out->type = ANSI_CHAR;
        out->chr  = c;
        return true;

    case ANSI_STATE_ESC:
        out->lit[p->lit_i++] = c;
        if (c == '[') {
            p->state       = ANSI_STATE_CSI;
            p->p_idx       = 0;
            p->current_num = 0;
            p->is_private  = false;
            memset(out->params, 0, sizeof(out->params));
            return false;
        }
        p->state = ANSI_STATE_GROUND;
        return false;

    case ANSI_STATE_CSI:
        out->lit[p->lit_i++] = c;
        if (c == '?') {
            p->is_private = true;
            return false;
        }
        if (c >= '0' && c <= '9') {
            p->current_num = (p->current_num * 10) + (c - '0');
            return false;
        }
        if (c == ';') {
            if (p->p_idx < ANSI_MAX_PARAMS) {
                p->params[p->p_idx++] = p->current_num;
                p->current_num        = 0;
            }
            return false;
        }

        /* Terminal Character - Build the token */
        if (p->p_idx < ANSI_MAX_PARAMS) {
            p->params[p->p_idx++] = p->current_num;
        }

        out->param_count = p->p_idx;
        memcpy(out->params, p->params, sizeof(out->params));

        out->type = ts_ansi_type(c, p->is_private);
        p->state  = ANSI_STATE_GROUND;
        return true;
    }
    return false;
}

void ts_ansi_execute(struct TextSurface *S, AnsiToken *cmd)
{
    int *p     = cmd->params;
    int  curp0 = p[0] - (p[0] > 0);
    int  curp1 = p[1] - (p[1] > 0);
    int  curp2 = p[2] - (p[2] > 0);
    int  curp3 = p[3] - (p[3] > 0);
    int  n     = (p[0] > 0) ? p[0] : 1;

    usize x = S->x;
    usize y = S->y;
    usize w = S->w;
    usize h = S->h;

    if (x >= S->w) x = S->w - 1;
    if (y >= S->h) y = S->h - 1;

    switch (cmd->type) {
    case ANSI_CUU: ts_set_xy(S, x, y - n); break;
    case ANSI_CUD: ts_set_xy(S, x, y + n); break;
    case ANSI_CUF: ts_set_xy(S, x + n, y); break;
    case ANSI_CUB: ts_set_xy(S, x - n, y); break;
    case ANSI_CNL: ts_set_xy(S, 0, y + n); break;
    case ANSI_CPL: ts_set_xy(S, 0, y - n); break;
    case ANSI_CHA: ts_set_xy(S, curp0, y); break;
    case ANSI_CUP: ts_set_xy(S, curp1, curp0); break;
    case ANSI_ED: {
        /* TODO: This is a mess. Figure it out*/
        int start_cell  = 0;
        int c           = 0;
        int end_cell    = w * h;
        int cursor_cell = y * w + x;

        if (p[0] == 0)
            start_cell = cursor_cell; /* Clear from cursor to end */
        else if (p[0] == 1)
            end_cell = cursor_cell + 1; /* Clear beginning to cursor */
        else if (p[0] == 2) {
            /* Clear entire screen */
            S->clear_text(S);
            break;
        }

        S->fill_span(S, start_cell, end_cell, ' ');

        break;
    }
    case ANSI_EL: {
        int row_start   = y * w;
        int row_end     = (y + 1) * w;
        int cursor_cell = y * w + x;
        int c           = 0;

        int start_cell = row_start;
        int end_cell   = row_end;

        if (p[0] == 0)
            start_cell = cursor_cell; /* Cursor to end of line */
        else if (p[0] == 1)
            end_cell = cursor_cell + 1; /* Start of line to cursor */
                                        /* p[0] == 2 clears the full line (default start/end) */

        S->fill_span(S, start_cell, end_cell, ' ');

        break;
    }

    case ANSI_SU: {
        usize i;
        for (i = 0; i < n; i++) S->scroll_up(S);
        break;
    }
    case ANSI_SD: {
        usize i;
        for (i = 0; i < n; i++) S->scroll_down(S);
        break;
    }

        /* This is still very tied to the Taleä System*/
    case ANSI_SGR: {
        usize i;
        if (!(S->capabilities & CAP_COLOR)) break;

        for (i = 0; i < cmd->param_count; i++) {
            int p = cmd->params[i];
            switch (p) {
            case ANSI_SGR_RESET: S->attr[AT] = 0; break;
            case ANSI_SGR_BOLD: S->attr[AT] |= TEXTMODE_ATT_BOLD; break;
            case ANSI_SGR_DIM: S->attr[AT] |= TEXTMODE_ATT_DIM; break;
            case ANSI_SGR_ITALIC: S->attr[AT] |= TEXTMODE_ATT_OBLIQUE; break;
            case ANSI_SGR_UNDERLINE: S->attr[AT] |= TEXTMODE_ATT_UNDERLINE; break;
            case ANSI_SGR_BLINK: S->attr[AT] |= TEXTMODE_ATT_BLINK; break;
            case ANSI_SGR_TRANSPARENT: S->attr[AT] |= TEXTMODE_ATT_TRANSPARENT; break;
            case ANSI_SGR_REVERSE: {
                u8 tmp      = S->attr[FG];
                S->attr[FG] = S->attr[BG];
                S->attr[BG] = tmp;
                S->reversed = true;
                break;
            }
            case ANSI_SGR_FONT_0:
                S->attr[AT] &= ~(TEXTMODE_ATT_CODEPAGE | TEXTMODE_ATT_ALT_FONT);
                break;
            case ANSI_SGR_FONT_1:
                S->attr[AT] &= ~TEXTMODE_ATT_ALT_FONT;
                S->attr[AT] |= TEXTMODE_ATT_CODEPAGE;
                break;
            case ANSI_SGR_FONT_2:
                S->attr[AT] |= TEXTMODE_ATT_ALT_FONT;
                S->attr[AT] &= ~TEXTMODE_ATT_CODEPAGE;
                break;
            case ANSI_SGR_FONT_3:
                S->attr[AT] |= (TEXTMODE_ATT_CODEPAGE | TEXTMODE_ATT_ALT_FONT);
                break;
            case ANSI_SGR_NORMAL_INTENSITY:
                S->attr[AT] &= ~(TEXTMODE_ATT_BOLD | TEXTMODE_ATT_DIM);
                break;
            case ANSI_SGR_NOT_ITALIC: S->attr[AT] &= ~TEXTMODE_ATT_OBLIQUE; break;
            case ANSI_SGR_NOT_UNDERLINE: S->attr[AT] &= ~TEXTMODE_ATT_UNDERLINE; break;
            case ANSI_SGR_NOT_BLINK: S->attr[AT] &= ~TEXTMODE_ATT_BLINK; break;
            case ANSI_SGR_NOT_TRANSPARENT: S->attr[AT] &= ~TEXTMODE_ATT_TRANSPARENT; break;
            case ANSI_SGR_NOT_REVERSE: {
                if (S->reversed) {
                    u8 tmp      = S->attr[FG];
                    S->attr[FG] = S->attr[BG];
                    S->attr[BG] = tmp;
                    S->reversed = false;
                }
                break;
            }
            case ANSI_SGR_SET_FG:
            case ANSI_SGR_SET_FG + 1:
            case ANSI_SGR_SET_FG + 2:
            case ANSI_SGR_SET_FG + 3:
            case ANSI_SGR_SET_FG + 4:
            case ANSI_SGR_SET_FG + 5:
            case ANSI_SGR_SET_FG + 6:
            case ANSI_SGR_SET_FG + 7: S->attr[FG] = p - 30; break;
            case ANSI_SGR_SET_FG_256:
                if (i + 2 < cmd->param_count) {
                    if (cmd->params[i + 1] == 5) {
                        S->attr[FG] = cmd->params[i + 2];
                        i += 2;
                    } else if (cmd->params[i + 1] == 2) {
                        if (i + 4 < cmd->param_count)
                            i += 4;
                        else
                            return;
                    } else
                        break;
                } else {
                    return;
                }
                break;
            case ANSI_SGR_DEFAULT_FG: S->attr[FG] = S->default_attr[FG]; break;
            case ANSI_SGR_SET_BG:
            case ANSI_SGR_SET_BG + 1:
            case ANSI_SGR_SET_BG + 2:
            case ANSI_SGR_SET_BG + 3:
            case ANSI_SGR_SET_BG + 4:
            case ANSI_SGR_SET_BG + 5:
            case ANSI_SGR_SET_BG + 6:
            case ANSI_SGR_SET_BG + 7: S->attr[BG] = p - 40; break;
            case ANSI_SGR_SET_BG_256:
                if (i + 2 < cmd->param_count) {
                    if (cmd->params[i + 1] == 5) {
                        S->attr[BG] = cmd->params[i + 2];
                        i += 2;
                    } else if (cmd->params[i + 1] == 2) {
                        if (i + 4 < cmd->param_count)
                            i += 4;
                        else
                            return;
                    } else
                        break;
                } else {
                    return;
                }
                break;
            case ANSI_SGR_DEFAULT_BG: S->attr[BG] = S->default_attr[BG]; break;
            default: break;
            }
        }
        break;
    }

    /* Private SCO modes*/
    case ANSI_SCP:
        /* save cursor position, overwite if already saved */
        S->sx    = S->x;
        S->sy    = S->y;
        S->saved = true;
        memcpy(S->saved_attr, S->attr, sizeof(S->saved_attr));
        break;
    case ANSI_RCP:
        if (!S->saved) break;
        S->x = S->sx;
        S->y = S->sy;
        memcpy(S->attr, S->saved_attr, sizeof(S->attr));
        ts_set_xy(S, S->x, S->y);
        break;

    /* Private modes*/
    case ANSI_PRIV_H: {
        switch (p[0]) {
        case 25: /* Show Cursor */ S->ctl(S, CTL_CURSOR_SHOW); break;
        case 12: /* Start blink cursor */ S->ctl(S, CTL_CURSOR_BLINK); break;
        }
        break;
    }

    case ANSI_PRIV_L: {
        switch (p[0]) {
        case 25: /* Show Cursor */ S->ctl(S, CTL_CURSOR_HIDE); break;
        case 12: /* Start blink cursor */ S->ctl(S, CTL_CURSOR_NO_BLINK); break;
        }
        break;
    }
        /* NOT PLANNED: HVP */
    default: break;
    }
}

void ts_ansi_putc(struct TextSurface *S, char c)
{
    AnsiToken tk;

    if (ts_ansi_parse(S, c, &tk)) {
        if (tk.type == ANSI_NONE)
            return;
        else if (tk.type == ANSI_CHAR)
            ts_emit(S, c);
        else {
            if ((tk.type == ANSI_SGR && S->forward_sgr) ||
                ((tk.type & 0x100) && S->forward_privates)) {
                // forward
                usize i;
                for (i = 0; i < S->ansi.lit_i; i++) {
                    ts_emit(S, tk.lit[i]);
                }
            } else {
                ts_ansi_execute(S, &tk);
            }
        }
    }
}

void ts_resize(struct TextSurface *S, u8 nw, u8 nh, void *priv)
{
    S->w    = nw;
    S->h    = nh;
    S->priv = priv;
    atxt_set_cursor(S, 0, 0);
}

void ts_write(struct TextSurface *S, const char *ss, usize len)
{
    usize i;
    for (i = 0; i < len; i++) {
        ts_ansi_putc(S, ss[i]);
    }
}

#endif

#endif
