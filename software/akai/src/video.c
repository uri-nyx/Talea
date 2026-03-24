#include "ansi.h"
#include "hw.h"
#include "kernel.h"
#include "mmu.h"
#include "sgl.h"

#define err A.pr.curr->last_error

#define FG 0
#define BG 1
#define AT 2

static void set_cursor(u8 x, u8 y)
{
    _sbd(A.devices[DEV_TEXTBUFFER].base + VIDEO_CUR_X, x);
    _sbd(A.devices[DEV_TEXTBUFFER].base + VIDEO_CUR_Y, y);
}

static void get_cursor(u8 *outx, u8 *outy)
{
    *outx = _lbud(A.devices[DEV_TEXTBUFFER].base + VIDEO_CUR_X);
    *outy = _lbud(A.devices[DEV_TEXTBUFFER].base + VIDEO_CUR_Y);
}

static void get_info(u8 *w, u8 *h, u8 *bpc, u8 *cw, u8 *ch)
{
    _sbd(A.devices[DEV_TEXTBUFFER].base + VIDEO_COMMAND, VIDEO_COMMAND_SYS_INFO);

    *bpc = _lbud(A.devices[DEV_TEXTBUFFER].base + VIDEO_GPU3);
    *w   = _lbud(A.devices[DEV_TEXTBUFFER].base + VIDEO_GPU4);
    *h   = _lbud(A.devices[DEV_TEXTBUFFER].base + VIDEO_GPU5);
    *cw  = _lbud(A.devices[DEV_TEXTBUFFER].base + VIDEO_GPU0);
    *ch  = _lbud(A.devices[DEV_TEXTBUFFER].base + VIDEO_GPU1);
}

static void display(char c)
{
    _trace(0xDAFEB, c);
    if (A.txt.bpc == 1) {
        u8 *tb = (u8 *)AKAI_TEXTBUFFER;

        tb[((u32)A.txt.y * (u32)A.txt.w) + A.txt.x] = c;
    } else {
        u32 *tb = (u32 *)AKAI_TEXTBUFFER;

        tb[((u32)A.txt.y * (u32)A.txt.w) + A.txt.x] = (u32)c << 24 | (u32)A.txt.attr[FG] << 16 |
                                                      (u32)A.txt.attr[BG] << 8 |
                                                      (u32)A.txt.attr[AT];
    }
}

static void clear_line(u8 line)
{
    if (A.txt.bpc == 4) {
        _fillw((u8 *)AKAI_TEXTBUFFER + (line * A.txt.w * A.txt.bpc),
               (u32)' ' << 24 | (u32)A.txt.attr[FG] << 16 | (u32)A.txt.attr[BG] << 8 |
                   A.txt.attr[AT],
               A.txt.w);
    } else {
        memset((u8 *)AKAI_TEXTBUFFER + (line * A.txt.w * A.txt.bpc), ' ', A.txt.w * A.txt.bpc);
    }
}

static void scroll_up(void)
{
    u8   *src   = (u8 *)AKAI_TEXTBUFFER + (A.txt.w * A.txt.bpc);
    usize count = (A.txt.h - 1) * A.txt.w * A.txt.bpc;

    memcpy((u8 *)AKAI_TEXTBUFFER, src, count);
    clear_line(A.txt.h - 1);

    if (A.txt.y > 0)
        A.txt.y--;
    else
        A.txt.y = 0;
}

static void scroll_down(void)
{
    u8   *dst   = (u8 *)AKAI_TEXTBUFFER + (A.txt.w * A.txt.bpc);
    usize count = (A.txt.h - 1) * A.txt.w * A.txt.bpc;

    _copybck((u8 *)AKAI_TEXTBUFFER, dst, count);
    clear_line(0);

    if (A.txt.y < A.txt.h - 1)
        A.txt.y++;
    else
        A.txt.y = A.txt.h - 1;
}

static void tab(void)
{
    int spaces = A.txt.tabsize - (A.txt.x % A.txt.tabsize);

    if (A.txt.x + spaces >= A.txt.w) {
        A.txt.x = 0;
        A.txt.y++;
    } else {
        usize i, k;
        u8   *tb        = (u8 *)AKAI_TEXTBUFFER;
        u32   start_idx = (A.txt.y * A.txt.w + A.txt.x) * A.txt.bpc;
        u32   len       = spaces * A.txt.bpc;

        if (A.txt.bpc == 4) {
            _fillw((u8 *)AKAI_TEXTBUFFER + start_idx,
                   (u32)' ' << 24 | (u32)A.txt.attr[FG] << 16 | (u32)A.txt.attr[BG] << 8 |
                       A.txt.attr[AT],
                   spaces);
        } else {
            memset((u8 *)AKAI_TEXTBUFFER + start_idx, ' ', len);
        }

        A.txt.x += spaces;
    }

    if (A.txt.y >= A.txt.h) scroll_up();
}

static void clear_txt(void)
{
    A.txt.x = 0;
    A.txt.y = 0;

    if (A.txt.bpc == 4) {
        _fillw((u8 *)AKAI_TEXTBUFFER,
               (u32)' ' << 24 | (u32)A.txt.attr[FG] << 16 | (u32)A.txt.attr[BG] << 8 |
                   A.txt.attr[AT],
               A.txt.w * A.txt.h);
    } else {
        memset((u8 *)AKAI_TEXTBUFFER, ' ', (A.txt.w * A.txt.h) * A.txt.bpc);
    }
}

static void update_cursor(void)
{
    set_cursor(A.txt.x, A.txt.y);
}

static void set_xy(int x, int y)
{
    /* Clamp to screen boundaries */
    if (x < 0) x = 0;
    if (x >= A.txt.w) x = A.txt.w - 1;
    if (y < 0) y = 0;
    if (y >= A.txt.h) y = A.txt.h - 1;

    A.txt.x = x;
    A.txt.y = y;

    update_cursor();
}

static void emit(char c)
{
    bool control = false;
    _trace(0xDAFA);
    /* Handle newline and c-return */
    switch (c) {
    case '\n':
        _trace(0xDAFA, 1);

        if (A.txt.canonical & OUT_NLCR) A.txt.x = 0;
        A.txt.y++;
        goto check_bounds;
    case '\r':
        _trace(0xDAFA, 2);
        A.txt.x = 0;
        goto sync_hw;
    case '\b':
        _trace(0xDAFA, 3);
        if (A.txt.x > 0) A.txt.x--;
        goto sync_hw;
    case '\t':
        _trace(0xDAFA, 4);
        tab();
        goto sync_hw;
    case 0x0c:
        _trace(0xDAFA, 5);
        /* formfeed, \f, clear screen */
        clear_txt();
        goto sync_hw;
    default: control = true; break;
    }

    /* Automatic Wrapping */
    if (A.txt.x >= A.txt.w) {
        if (A.txt.canonical & OUT_NOWRAP) goto sync_hw;
        A.txt.x = 0;
        A.txt.y++;
    }

check_bounds:
    /* Automatic Scrolling */
    _trace(0xDAFA, 6, A.txt.y, A.txt.h);
    if (A.txt.y >= A.txt.h) {
        if (A.txt.canonical & OUT_NOSCROLL) goto sync_hw;
        scroll_up();
    }

    if (c >= ' ') {
        _trace(0xDAFA1);
        /* Trigger output ONLY if within defined bounds */
        if ((A.txt.x < A.txt.w) && (A.txt.y < A.txt.h)) {
            display(c);
        }
        A.txt.x++;
    } else if (control) {
        if ((A.txt.x + 1 < A.txt.w) && (A.txt.y + 1 < A.txt.h)) {
            display('^');
            A.txt.x++;
            display(c + 0x40);
            A.txt.x++;
        } else {
            A.txt.x += 2;
        }
    }

sync_hw:
    _trace(0xDAFA, 7);
    update_cursor(); /* Sync hardware cursor index */
}

/* IMPORTANT: this function CANNOT return (signed)ANSI_CHAR */
static enum AnsiCmd ansi_type(u8 c, bool is_private)
{
    if (is_private) {
        /* Add the private bitmask to the character */
        return (enum AnsiCmd)(c | 0x100);
    }

    return (enum AnsiCmd)c;
}

static bool ansi_parse(u8 c, AnsiToken *out)
{
    AnsiParser *p = &A.txt.ansi;

    int i     = 0;
    out->type = ANSI_NONE;

    _trace(0xDAF01, p->state);

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
            _trace(0xDAF02);
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

        out->type = ansi_type(c, p->is_private);
        p->state  = ANSI_STATE_GROUND;
        return true;
    }
    return false;
}

static void ansi_execute(AnsiToken *cmd)
{
    u8  *tb    = (u8 *)AKAI_TEXTBUFFER;
    int *p     = cmd->params;
    int  curp0 = p[0] - (p[0] > 0);
    int  curp1 = p[1] - (p[1] > 0);
    int  curp2 = p[2] - (p[2] > 0);
    int  curp3 = p[3] - (p[3] > 0);
    int  n     = (p[0] > 0) ? p[0] : 1;

    usize x = A.txt.x;
    usize y = A.txt.y;
    usize w = A.txt.w;
    usize h = A.txt.h;

    if (x >= A.txt.w) x = A.txt.w - 1;
    if (y >= A.txt.h) y = A.txt.h - 1;

    switch (cmd->type) {
    case ANSI_CUU: set_xy(x, y - n); break;
    case ANSI_CUD: set_xy(x, y + n); break;
    case ANSI_CUF: set_xy(x + n, y); break;
    case ANSI_CUB: set_xy(x - n, y); break;
    case ANSI_CNL: set_xy(0, y + n); break;
    case ANSI_CPL: set_xy(0, y - n); break;
    case ANSI_CHA: set_xy(curp0, y); break;
    case ANSI_CUP: set_xy(curp1, curp0); break;
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
            clear_txt();
            break;
        }

        if (A.txt.bpc == 4) {
            _fillw((u8 *)AKAI_TEXTBUFFER + (start_cell * A.txt.bpc),
                   (u32)' ' << 24 | (u32)A.txt.attr[FG] << 16 | (u32)A.txt.attr[BG] << 8 |
                       A.txt.attr[AT],
                   end_cell - start_cell);
        } else {
            memset((u8 *)AKAI_TEXTBUFFER + (start_cell * A.txt.bpc), ' ',
                   (end_cell - start_cell) * A.txt.bpc);
        }

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

        if (A.txt.bpc == 4) {
            _fillw((u8 *)AKAI_TEXTBUFFER + (start_cell * A.txt.bpc),
                   (u32)' ' << 24 | (u32)A.txt.attr[FG] << 16 | (u32)A.txt.attr[BG] << 8 |
                       A.txt.attr[AT],
                   end_cell - start_cell);
        } else {
            memset((u8 *)AKAI_TEXTBUFFER + (start_cell * A.txt.bpc), ' ',
                   (end_cell - start_cell) * A.txt.bpc);
        }

        break;
    }

    case ANSI_SU: {
        usize i;
        for (i = 0; i < n; i++) scroll_up();
        break;
    }
    case ANSI_SD: {
        usize i;
        for (i = 0; i < n; i++) scroll_down();
        break;
    }

    case ANSI_DSR: {
        if (p[0] == 6) {
            u16 row = A.txt.x + 1; // 1‑based row
            u16 col = A.txt.y + 1; // 1‑based column

            inject_byte('\x1b');
            inject_byte('[');
            inject_uint(row);
            inject_byte(';');
            inject_uint(col);
            inject_byte('R');

            kbd_process_input();
        }
        break;
    }

    case ANSI_SGR: {
        usize i;
        if (A.txt.bpc != 4) break;

        for (i = 0; i < cmd->param_count; i++) {
            int p = cmd->params[i];
            switch (p) {
            case ANSI_SGR_RESET: A.txt.attr[AT] = 0; break;
            case ANSI_SGR_BOLD: A.txt.attr[AT] |= TEXTMODE_ATT_BOLD; break;
            case ANSI_SGR_DIM: A.txt.attr[AT] |= TEXTMODE_ATT_DIM; break;
            case ANSI_SGR_ITALIC: A.txt.attr[AT] |= TEXTMODE_ATT_OBLIQUE; break;
            case ANSI_SGR_UNDERLINE: A.txt.attr[AT] |= TEXTMODE_ATT_UNDERLINE; break;
            case ANSI_SGR_BLINK: A.txt.attr[AT] |= TEXTMODE_ATT_BLINK; break;
            case ANSI_SGR_TRANSPARENT: A.txt.attr[AT] |= TEXTMODE_ATT_TRANSPARENT; break;
            case ANSI_SGR_REVERSE: {
                u8 tmp         = A.txt.attr[FG];
                A.txt.attr[FG] = A.txt.attr[BG];
                A.txt.attr[BG] = tmp;
                A.txt.reversed = true;
                break;
            }
            case ANSI_SGR_FONT_0:
                A.txt.attr[AT] &= ~(TEXTMODE_ATT_CODEPAGE | TEXTMODE_ATT_ALT_FONT);
                break;
            case ANSI_SGR_FONT_1:
                A.txt.attr[AT] &= ~TEXTMODE_ATT_ALT_FONT;
                A.txt.attr[AT] |= TEXTMODE_ATT_CODEPAGE;
                break;
            case ANSI_SGR_FONT_2:
                A.txt.attr[AT] |= TEXTMODE_ATT_ALT_FONT;
                A.txt.attr[AT] &= ~TEXTMODE_ATT_CODEPAGE;
                break;
            case ANSI_SGR_FONT_3:
                A.txt.attr[AT] |= (TEXTMODE_ATT_CODEPAGE | TEXTMODE_ATT_ALT_FONT);
                break;
            case ANSI_SGR_NORMAL_INTENSITY:
                A.txt.attr[AT] &= ~(TEXTMODE_ATT_BOLD | TEXTMODE_ATT_DIM);
                break;
            case ANSI_SGR_NOT_ITALIC: A.txt.attr[AT] &= ~TEXTMODE_ATT_OBLIQUE; break;
            case ANSI_SGR_NOT_UNDERLINE: A.txt.attr[AT] &= ~TEXTMODE_ATT_UNDERLINE; break;
            case ANSI_SGR_NOT_BLINK: A.txt.attr[AT] &= ~TEXTMODE_ATT_BLINK; break;
            case ANSI_SGR_NOT_TRANSPARENT: A.txt.attr[AT] &= ~TEXTMODE_ATT_TRANSPARENT; break;
            case ANSI_SGR_NOT_REVERSE: {
                if (A.txt.reversed) {
                    u8 tmp         = A.txt.attr[FG];
                    A.txt.attr[FG] = A.txt.attr[BG];
                    A.txt.attr[BG] = tmp;
                    A.txt.reversed = false;
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
            case ANSI_SGR_SET_FG + 7: A.txt.attr[FG] = p - 30; break;
            case ANSI_SGR_SET_FG_256:
                if (i + 2 < cmd->param_count) {
                    if (cmd->params[i + 1] == 5) {
                        A.txt.attr[FG] = cmd->params[i + 2];
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
            case ANSI_SGR_DEFAULT_FG: A.txt.attr[FG] = A.txt.default_attr[FG]; break;
            case ANSI_SGR_SET_BG:
            case ANSI_SGR_SET_BG + 1:
            case ANSI_SGR_SET_BG + 2:
            case ANSI_SGR_SET_BG + 3:
            case ANSI_SGR_SET_BG + 4:
            case ANSI_SGR_SET_BG + 5:
            case ANSI_SGR_SET_BG + 6:
            case ANSI_SGR_SET_BG + 7: A.txt.attr[BG] = p - 40; break;
            case ANSI_SGR_SET_BG_256:
                if (i + 2 < cmd->param_count) {
                    if (cmd->params[i + 1] == 5) {
                        A.txt.attr[BG] = cmd->params[i + 2];
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
            case ANSI_SGR_DEFAULT_BG: A.txt.attr[BG] = A.txt.default_attr[BG]; break;
            default: break;
            }
        }
        break;
    }

    /* Private SCO modes*/
    case ANSI_SCP:
        /* save cursor position, overwite if already saved */
        A.txt.sx    = A.txt.x;
        A.txt.sy    = A.txt.y;
        A.txt.saved = true;
        memcpy(A.txt.saved_attr, A.txt.attr, sizeof(A.txt.saved_attr));
        //miniprint("SCP: %d,%d\n", A.txt.x, A.txt.y);
        break;
    case ANSI_RCP:
        if (!A.txt.saved) break;
        A.txt.x = A.txt.sx;
        A.txt.y = A.txt.sy;
        memcpy(A.txt.attr, A.txt.saved_attr, sizeof(A.txt.attr));
        set_xy(A.txt.x, A.txt.y);
        //miniprint("RCP: %d,%d\n", A.txt.x, A.txt.y);
        break;

    /* Private modes*/
    case ANSI_PRIV_H: {
        u8 csr = _lbud(A.devices[DEV_TEXTBUFFER].base + VIDEO_CSR);
        switch (p[0]) {
        case 25: /* Show Cursor */ csr |= VIDEO_CURSOR_EN; break;
        case 12: /* Start blink cursor */ csr |= VIDEO_CURSOR_BLINK; break;
        case 1006: /*Enable Mouse Report*/ A.mous.mode |= MOUS_REPORT; break;
        }
        _sbd(A.devices[DEV_TEXTBUFFER].base + VIDEO_CSR, csr);
        break;
    }

    case ANSI_PRIV_L: {
        u8 csr = _lbud(A.devices[DEV_TEXTBUFFER].base + VIDEO_CSR);
        switch (p[0]) {
        case 25: /* Hide Cursor */ csr &= ~VIDEO_CURSOR_EN; break;
        case 12: /* static cursor */ csr &= ~VIDEO_CURSOR_BLINK; break;
        case 1006: /*Disable Mouse Report*/ A.mous.mode &= ~MOUS_REPORT; break;
        }
        _sbd(A.devices[DEV_TEXTBUFFER].base + VIDEO_CSR, csr);
        break;
    }

        /* NOT PLANNED: HVP */
    default: break;
    }
}

static void ansi_putc(char c)
{
    AnsiToken tk;

    _trace(0xDAFEE, c);

    if (ansi_parse(c, &tk)) {
        _trace(0xdafee2);
        if (tk.type == ANSI_NONE)
            return;
        else if (tk.type == ANSI_CHAR)
            emit(c);
        else {
            ansi_execute(&tk);
        }
    }
}

i32 txt_reset(void)
{
    u8 w, h, cw, ch, bpc;

    memset(&A.txt, 0, sizeof(A.txt));
    get_info(&w, &h, &bpc, &cw, &ch);

    _trace(0xEF, w, h, bpc);

    set_cursor(0, 0);

    if (bpc == 4) {
        A.txt.default_attr[FG] = 0xf;
        A.txt.default_attr[BG] = 0;
        A.txt.default_attr[AT] = 0;
        A.txt.attr[FG]         = 0xf;
        A.txt.attr[BG]         = 0;
        A.txt.attr[AT]         = 0;
    }

    A.txt.x         = 0;
    A.txt.y         = 0;
    A.txt.w         = w;
    A.txt.h         = h;
    A.txt.cw        = cw;
    A.txt.ch        = ch;
    A.txt.bpc       = bpc;
    A.txt.canonical = 0;
    A.txt.tabsize   = 4;
    A.txt.reversed  = false;
    memset(&A.txt.ansi, 0, sizeof(A.txt.ansi));
    return (signed)A_OK;
}

u8 txt_in(u8 port)
{
    if (port > A.devices[DEV_TEXTBUFFER].ports) {
        err = P_ERROR_NO_PORT;
        return 0;
    }

    return _lbud(A.devices[DEV_TEXTBUFFER].base + port);
}

i32 txt_out(u8 port, u8 value)
{
    if (port > A.devices[DEV_TEXTBUFFER].ports) {
        err = P_ERROR_NO_PORT;
        return (signed)A_ERROR;
    }

    _sbd(A.devices[DEV_TEXTBUFFER].base + port, value);
    return (signed)A_OK;
}

i32 txt_ctl(u32 command, void *buff, u32 len)
{
    _trace(0xDAFE, command);
    switch (command) {
    case DEV_RESET: return txt_reset();
    case TCTL_GET_INFO: {
        return (i32)((u32)A.txt.bpc << 16 | (u32)A.txt.w << 8 | (u32)A.txt.h);
    }
    case TCTL_GET_CURSOR: {
        return (i32)((u16)A.txt.x << 8 | A.txt.y);
    }
    case TCTL_SET_CURSOR: {
        if (len == 1) {
            u8 csr          = _lbud(A.devices[DEV_TEXTBUFFER].base + VIDEO_CSR);
            u8 option       = *(u8 *)buff;
            u8 cursor_state = 0;

            csr &= ~(VIDEO_CURSOR_BLINK | VIDEO_CURSOR_EN);

            if (option & VIDEO_CURSOR_EN) cursor_state = VIDEO_CURSOR_EN;
            if (option & VIDEO_CURSOR_BLINK) cursor_state = VIDEO_CURSOR_BLINK;

            _sbd(A.devices[DEV_TEXTBUFFER].base + VIDEO_CSR, csr | cursor_state);
            return (signed)A_OK;
        } else {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
        }
    };
    case TCTL_SET_ATTR: {
        if (A.txt.bpc == 1 && len == 1) {
            // only tab size
            A.txt.tabsize = *(u8 *)buff;
            return (signed)A_OK;
        } else if (A.txt.bpc == 4 && len == 4) {
            u8 *att = (u8 *)buff;

            A.txt.default_attr[FG] = att[FG];
            A.txt.default_attr[BG] = att[BG];
            A.txt.default_attr[AT] = att[AT];
            A.txt.tabsize          = att[3];
            return (signed)A_OK;
        } else {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
        }
    }
    case TCTL_GET_ATTR: {
        if (A.txt.bpc == 1 && len == 1) {
            // only tab size
            *(u8 *)buff = A.txt.tabsize;
            return (signed)A_OK;
        } else if (A.txt.bpc == 4 && len == 4) {
            u8 *att = (u8 *)buff;

            att[FG] = A.txt.default_attr[FG];
            att[BG] = A.txt.default_attr[BG];
            att[AT] = A.txt.default_attr[AT];
            att[3]  = A.txt.tabsize;
            return (signed)A_OK;
        } else {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
        }
    }

    case TCTL_MAP_TEXTBUFFER: {
        usize i;
        usize tbsz = 128 * 1024;

        usize tb_pages = (tbsz + (PAGE_SIZE - 1)) >> 12;

        u32 *pde_phys  = A.pr.curr->page_tables[3];
        u32 *active_pt = (u32 *)(AKAI_PROCESS_PAGE_TABLES + 3 * PAGE_SIZE);
        if (pde_phys == NULL) return (signed)A_ERROR_CTL;
        // must be mapped

        for (i = 0; i < tb_pages; i++) {
            chperm_pt_entry(active_pt, AKAI_TEXTBUFFER + i * PAGE_SIZE, PTE_U | PTE_RW);
        }

        tlb_flush();
        return (signed)A_OK;
    }

    case VCTL_MAP_FRAMEBUFFER: {
        usize i;
        usize fbsz = A.info.framebuffer.h * A.info.framebuffer.w;

        usize fb_pages = (fbsz + (PAGE_SIZE - 1)) >> 12;

        u32 *pde_phys  = A.pr.curr->page_tables[3];
        u32 *active_pt = (u32 *)(AKAI_PROCESS_PAGE_TABLES + 3 * PAGE_SIZE);
        if (pde_phys == NULL) return (signed)A_ERROR_CTL;
        // must be mapped

        for (i = 0; i < fb_pages; i++) {
            chperm_pt_entry(active_pt, AKAI_FRAMEBUFFER + i * PAGE_SIZE, PTE_U | PTE_RW);
        }

        tlb_flush();
        return (signed)A_OK;
    }

    case PX_WRITE: {
        usize i;
        u8   *s = (u8 *)buff;

        if (len == 0) return 0;

        _trace(0xDAFEA, buff);

        for (i = 0; i < len; i++) {
            ansi_putc(*s++);
        }

        return len;
    }

    case VCTL_LOAD_PALETTE: {
        int verror;
        if (len != 1024) {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
        }

        _copymd(buff, AKAI_PALETTE, len);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_BEGIN_DRAWING);
        _sbd(A.info.video + VIDEO_ERR, 0); // clear video error
        _sbd(A.info.video + VIDEO_GPU0, 4);
        _shd(A.info.video + VIDEO_GPU1, AKAI_PALETTE);
        _sbd(A.info.video + VIDEO_GPU3, (len / 4) - 1);
        _sbd(A.info.video + VIDEO_GPU4, 0);
        _sbd(A.info.video + VIDEO_GPU7, 0); // latch
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_LOAD);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_END_DRAWING);
        verror = _lbud(A.info.video + VIDEO_ERR); // read video error

        if (verror) {
            err = V_ERROR | verror;
            return (signed)A_ERROR_CTL;
        }

        return A_OK;
    }

    case VCTL_LOAD_PALETTE_DEFAULT: {
        int verror;

        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_BEGIN_DRAWING);
        _sbd(A.info.video + VIDEO_ERR, 0); // clear video error
        _sbd(A.info.video + VIDEO_GPU0, 5);
        _shd(A.info.video + VIDEO_GPU1, AKAI_PALETTE);
        _sbd(A.info.video + VIDEO_GPU3, 255);
        _sbd(A.info.video + VIDEO_GPU4, 0);
        _sbd(A.info.video + VIDEO_GPU7, 0); // latch
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_LOAD);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_END_DRAWING);

        verror = _lbud(A.info.video + VIDEO_ERR); // read video error

        if (verror) {
            err = V_ERROR | verror;
            return (signed)A_ERROR_CTL;
        }

        return A_OK;
    }

    case PX_FLUSH: return (signed)A_OK;

    case PX_SETCANON: {
        u8 mode;
        if (len != 1) {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
        }

        mode = *(u8 *)buff;

        A.txt.canonical = mode;

        return (signed)A_OK;
    }

    case PX_GETCANON: return (signed)A.txt.canonical;

    default: err = P_ERROR_NO_CTL_COMMAND; return (signed)A_ERROR_CTL;
    }
}

i32 fb_reset(void)
{
    return (signed)A_OK;
}

u8 fb_in(u8 port)
{
    if (port > A.devices[DEV_FRAMEBUFFER].ports) {
        err = P_ERROR_NO_PORT;
        return 0;
    }

    return _lbud(A.devices[DEV_FRAMEBUFFER].base + port);
}

i32 fb_out(u8 port, u8 value)
{
    if (port > A.devices[DEV_FRAMEBUFFER].ports) {
        err = P_ERROR_NO_PORT;
        return (signed)A_ERROR;
    }

    _sbd(A.devices[DEV_FRAMEBUFFER].base + port, value);
    return (signed)A_OK;
}

i32 fb_ctl(u32 command, void *buff, u32 len)
{
    _trace(0xDAFE, command);
    switch (command) {
    case DEV_RESET: return fb_reset();
    case VCTL_MAP_FRAMEBUFFER: {
        usize i;
        usize fbsz = A.info.framebuffer.h * A.info.framebuffer.w;

        usize fb_pages = (fbsz + (PAGE_SIZE - 1)) >> 12;

        u32 *pde_phys  = A.pr.curr->page_tables[3];
        u32 *active_pt = (u32 *)(AKAI_PROCESS_PAGE_TABLES + 3 * PAGE_SIZE);
        if (pde_phys == NULL) return (signed)A_ERROR_CTL;
        // must be mapped

        for (i = 0; i < fb_pages; i++) {
            chperm_pt_entry(active_pt, AKAI_FRAMEBUFFER + i * PAGE_SIZE, PTE_U | PTE_RW);
        }

        tlb_flush();
        return (signed)A_OK;
    }
    case VCTL_LOAD_PALETTE: {
        int verror;
        if (len != 1024) {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
        }

        _copymd(buff, AKAI_PALETTE, len);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_BEGIN_DRAWING);
        _sbd(A.info.video + VIDEO_ERR, 0); // clear video error
        _sbd(A.info.video + VIDEO_GPU0, 4);
        _shd(A.info.video + VIDEO_GPU1, AKAI_PALETTE);
        _sbd(A.info.video + VIDEO_GPU3, (len / 4) - 1);
        _sbd(A.info.video + VIDEO_GPU4, 0);
        _sbd(A.info.video + VIDEO_GPU7, 0); // latch
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_LOAD);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_END_DRAWING);
        verror = _lbud(A.info.video + VIDEO_ERR); // read video error

        if (verror) {
            err = V_ERROR | verror;
            return (signed)A_ERROR_CTL;
        }

        return A_OK;
    }

    case VCTL_LOAD_PALETTE_DEFAULT: {
        int verror;

        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_BEGIN_DRAWING);
        _sbd(A.info.video + VIDEO_ERR, 0); // clear video error
        _sbd(A.info.video + VIDEO_GPU0, 5);
        _shd(A.info.video + VIDEO_GPU1, AKAI_PALETTE);
        _sbd(A.info.video + VIDEO_GPU3, 255);
        _sbd(A.info.video + VIDEO_GPU4, 0);
        _sbd(A.info.video + VIDEO_GPU7, 0); // latch
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_LOAD);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_END_DRAWING);

        verror = _lbud(A.info.video + VIDEO_ERR); // read video error

        if (verror) {
            err = V_ERROR | verror;
            return (signed)A_ERROR_CTL;
        }

        return A_OK;
    }

    case GL_GET_FRAMEBUFFER_PHYS: return (i32)active_phys_from_v(AKAI_FRAMEBUFFER);
    case GL_DO_ROP: {
        u8 rop = *(u8 *)buff;
        u8 csr = _lbud(A.info.video + VIDEO_CSR);
        csr &= ~(0x7 << 4);
        csr |= (rop & 0x7) << 4;

        _sbd(A.info.video + VIDEO_GPU0, csr);
        _sbd(A.info.video + VIDEO_GPU7, 0);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_SET_CSR);
        return (signed)A_OK;
    }
    case GL_DO_BIND: {
        sglSurface *surf = (sglSurface *)buff;
        u32         buf  = (u32)surf->buff;
        void       *phys;

        if (len != sizeof(sglSurface)) {
            err = P_ERROR_BAD_POINTER;
            return (signed)A_ERROR_CTL;
        }

        phys = active_phys_from_v(buf + (buf & 0xFFF));

        if (!phys) {
            err = P_ERROR_BAD_POINTER;
            return (signed)A_ERROR_CTL;
        }

        surf->phys_addr = (sgl_phys)phys;

        // miniprint("BINDING surface %d, phys %x, w %d, h %d\n", surf->id, (sgl_phys)phys,
        // surf->width, surf->height);

        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_BEGIN_DRAWING);
        _swd(A.info.video + VIDEO_GPU0, (sgl_phys)phys);
        _sbd(A.info.video + VIDEO_GPU0, surf->id);
        _shd(A.info.video + VIDEO_GPU4, surf->width);
        _shd(A.info.video + VIDEO_GPU6, surf->height);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_BIND_CTX);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_END_DRAWING);
        return (signed)A_OK;
    }

    case GL_DO_CLEAR: {
        sgl_color color = *(sgl_color *)buff;

        _swd(A.info.video + VIDEO_GPU0,
             (u32)(' ' << 24) | 0U << 16 | 0U << 8 | (TEXTMODE_ATT_TRANSPARENT));
        _sbd(A.info.video + VIDEO_GPU4, color);
        _sbd(A.info.video + VIDEO_GPU5, 0x3);
        _sbd(A.info.video + VIDEO_GPU7, 0);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_CLEAR);
        return (signed)A_OK;
    }

    case GL_DO_CALL: {
        sglDrawCall *call = (sglDrawCall *)buff;
        if (len != sizeof(sglDrawCall)) {
            err = P_ERROR_BAD_POINTER;
            return (signed)A_ERROR_CTL;
        }

        return video_do_call(call);
    }

    case VCTL_WAIT_VBLANK: {
        u8 csr = _lbud(A.info.video + VIDEO_CSR);
        if (!(csr & VIDEO_VBLANK_EN)) return A_ERROR;

        // miniprint("sending %d to sleep\n", A.pr.curr->pid);
        BIT_SET(A.fb.wait_queue, A.pr.curr->pid);
        process_wait(A.pr.curr->pid);
        process_yield();

        return (signed)A_OK;
    }

    default: err = P_ERROR_NO_CTL_COMMAND; return (signed)A_ERROR_CTL;
    }
}

i32 video_do_call(sglDrawCall *call)
{
    switch (call->type) {
    case SGL_LINE:
        _sbd(A.info.video + VIDEO_GPU0, call->c.line.t_id);
        _sbd(A.info.video + VIDEO_GPU1, call->c.line.color);
        _shd(A.info.video + VIDEO_GPU4, call->c.line.x0);
        _shd(A.info.video + VIDEO_GPU6, call->c.line.y0);
        _shd(A.info.video + VIDEO_GPU0, call->c.line.x1);
        _shd(A.info.video + VIDEO_GPU2, call->c.line.y1);
        _sbd(A.info.video + VIDEO_GPU7, 0);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_DRAW_LINE);
        break;
    case SGL_RECT:
        _sbd(A.info.video + VIDEO_GPU0, call->c.rect.t_id);
        _sbd(A.info.video + VIDEO_GPU1, call->c.rect.color);
        _shd(A.info.video + VIDEO_GPU4, call->c.rect.w);
        _shd(A.info.video + VIDEO_GPU6, call->c.rect.h);
        _shd(A.info.video + VIDEO_GPU0, call->c.rect.dx);
        _shd(A.info.video + VIDEO_GPU2, call->c.rect.dy);
        _sbd(A.info.video + VIDEO_GPU7, 0);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_DRAW_RECT);
        break;
    case SGL_CIRCLE:
        _sbd(A.info.video + VIDEO_GPU0, call->c.circle.t_id);
        _sbd(A.info.video + VIDEO_GPU1, call->c.circle.outline);
        _sbd(A.info.video + VIDEO_GPU2, call->c.circle.interior);
        _sbd(A.info.video + VIDEO_GPU3, call->c.circle.mode);
        _shd(A.info.video + VIDEO_GPU4, call->c.circle.xm);
        _shd(A.info.video + VIDEO_GPU6, call->c.circle.ym);
        _shd(A.info.video + VIDEO_GPU0, call->c.circle.r);
        _sbd(A.info.video + VIDEO_GPU7, 0);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_DRAW_CIRCLE);
        break;
    case SGL_TRI:
        _sbd(A.info.video + VIDEO_GPU0, call->c.tri.t_id);
        _sbd(A.info.video + VIDEO_GPU1, call->c.tri.color);
        _shd(A.info.video + VIDEO_GPU4, call->c.tri.x0);
        _shd(A.info.video + VIDEO_GPU6, call->c.tri.y0);
        _shd(A.info.video + VIDEO_GPU0, call->c.tri.x1);
        _shd(A.info.video + VIDEO_GPU2, call->c.tri.y1);
        _shd(A.info.video + VIDEO_GPU4, call->c.tri.x2);
        _shd(A.info.video + VIDEO_GPU6, call->c.tri.y2);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_DRAW_TRI);
        break;
    case SGL_BLIT: {
        u32 buff      = (u32)call->c.blit.buff;
        u32 phys_addr = ((u32)active_phys_from_v(buff) + (buff & 0xFFF)) & 0xFFFFFF;
        if (!phys_addr) {
            err = P_ERROR_BAD_POINTER;
            return (signed)A_ERROR_INVAL;
        }
        _swd(A.info.video + VIDEO_GPU0, ((u32)call->c.blit.t_id << 24) | phys_addr);
        _shd(A.info.video + VIDEO_GPU4, call->c.blit.sw);
        _shd(A.info.video + VIDEO_GPU6, call->c.blit.sh);
        _shd(A.info.video + VIDEO_GPU0, call->c.blit.dx);
        _shd(A.info.video + VIDEO_GPU2, call->c.blit.dy);
        _sbd(A.info.video + VIDEO_GPU4, call->c.blit.rot);
        _sbd(A.info.video + VIDEO_GPU7, 0);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_BLIT);
        break;
    }
    case SGL_BLIT_STRETCH: {
        u32 buff      = (u32)call->c.blit_stretched.buff;
        u32 phys_addr = (((u32)active_phys_from_v(buff) + (buff & 0xFFF))) & 0xFFFFFF;
        if (!phys_addr) {
            err = P_ERROR_BAD_POINTER;
            return (signed)A_ERROR_INVAL;
        }
        _swd(A.info.video + VIDEO_GPU0, ((u32)call->c.blit_stretched.t_id << 24) | phys_addr);
        _shd(A.info.video + VIDEO_GPU4, call->c.blit_stretched.sw);
        _shd(A.info.video + VIDEO_GPU6, call->c.blit_stretched.sh);
        _shd(A.info.video + VIDEO_GPU0, call->c.blit_stretched.dx);
        _shd(A.info.video + VIDEO_GPU2, call->c.blit_stretched.dy);
        _shd(A.info.video + VIDEO_GPU4, call->c.blit_stretched.dw);
        _shd(A.info.video + VIDEO_GPU6, call->c.blit_stretched.dh);
        _sbd(A.info.video + VIDEO_GPU0, call->c.blit_stretched.rot);
        _sbd(A.info.video + VIDEO_GPU7, 0);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_STRETCH_BLIT);
        break;
    }
    case SGL_PATTERN_FILL: {
        u32 buff      = (u32)call->c.pattern.buff;
        u32 phys_addr = ((u32)active_phys_from_v(buff + (buff & 0xFFF))) & 0xFFFFFF;
        if (!phys_addr) {
            err = P_ERROR_BAD_POINTER;
            return (signed)A_ERROR_INVAL;
        }
        _swd(A.info.video + VIDEO_GPU0, ((u32)call->c.pattern.t_id << 24) | phys_addr);
        _sbd(A.info.video + VIDEO_GPU4, call->c.pattern.pw);
        _sbd(A.info.video + VIDEO_GPU5, call->c.pattern.ph);
        _sbd(A.info.video + VIDEO_GPU6, call->c.pattern.u);
        _sbd(A.info.video + VIDEO_GPU7, call->c.pattern.v);
        _shd(A.info.video + VIDEO_GPU0, call->c.pattern.dx);
        _shd(A.info.video + VIDEO_GPU2, call->c.pattern.dy);
        _sbd(A.info.video + VIDEO_GPU4, call->c.pattern.dw);
        _sbd(A.info.video + VIDEO_GPU6, call->c.pattern.dh);
        _sbd(A.info.video + VIDEO_GPU0, call->c.pattern.rot);
        _sbd(A.info.video + VIDEO_GPU7, 0);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_PATTERN_FILL);
        break;
    }
    case SGL_FILL_HSPAN:
        _sbd(A.info.video + VIDEO_GPU0, call->c.hspan.t_id);
        _sbd(A.info.video + VIDEO_GPU1, call->c.hspan.color);
        _shd(A.info.video + VIDEO_GPU2, call->c.hspan.x0);
        _shd(A.info.video + VIDEO_GPU4, call->c.hspan.x1);
        _shd(A.info.video + VIDEO_GPU6, call->c.hspan.y);
        _sbd(A.info.video + VIDEO_GPU7, 0);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_FILL_SPAN);
        break;
    case SGL_FILL_VSPAN:
        _sbd(A.info.video + VIDEO_GPU0, call->c.vspan.t_id);
        _sbd(A.info.video + VIDEO_GPU1, call->c.vspan.color);
        _shd(A.info.video + VIDEO_GPU2, call->c.vspan.x);
        _shd(A.info.video + VIDEO_GPU4, call->c.vspan.y0);
        _shd(A.info.video + VIDEO_GPU6, call->c.vspan.y1);
        _sbd(A.info.video + VIDEO_GPU7, 0);
        _sbd(A.info.video + VIDEO_COMMAND, VIDEO_COMMAND_FILL_VSPAN);
        break;
    default: err = A_ERROR_INVAL; return (signed)A_ERROR_CTL;
    }

    (signed)A_OK;
}

void fb_vblank_hook(u32 *win, struct IPCMessage *msg_out)
{
    usize i;
    u8   *wait_queue;
    u32   sreg;

    sreg = _disable_interrupts();

    // TODO: define messages internal protocol for interrupts
    msg_out->sender  = KERNEL_PID;
    msg_out->type    = 1; // I dont know, KB, or something
    msg_out->msgid   = 0;
    msg_out->subject = 0; // vblank
    msg_out->content = NULL;

    for (i = 0; i < MAX_PROCESS; i++) {
        if (BIT_TEST(A.fb.wait_queue, i)) {
            // miniprint("VBLANK waking %d\n", i);
            process_set_ready(i); // maybe prioritize schedulen for owner
            BIT_CLR(A.fb.wait_queue, i);
        }
    }
    _restore_interrupts(sreg);
}

// Wrapper if we ever implement terminal emulator in graphic mode
void video_emit(char c)
{
    emit(c);
}

void video_driver_init(void)
{
    A.devices[DEV_TEXTBUFFER].base  = A.info.video;
    A.devices[DEV_TEXTBUFFER].ports = 15;
    A.devices[DEV_TEXTBUFFER].id    = 'V';
    A.devices[DEV_TEXTBUFFER].num   = DEV_TEXTBUFFER;
    A.devices[DEV_TEXTBUFFER].reset = txt_reset;
    A.devices[DEV_TEXTBUFFER].in    = txt_in;
    A.devices[DEV_TEXTBUFFER].out   = txt_out;
    A.devices[DEV_TEXTBUFFER].ctl   = txt_ctl;
    txt_reset();

    A.devices[DEV_FRAMEBUFFER].base  = A.info.video;
    A.devices[DEV_FRAMEBUFFER].ports = 15;
    A.devices[DEV_FRAMEBUFFER].id    = 'V';
    A.devices[DEV_FRAMEBUFFER].num   = DEV_FRAMEBUFFER;
    A.devices[DEV_FRAMEBUFFER].reset = fb_reset;
    A.devices[DEV_FRAMEBUFFER].in    = fb_in;
    A.devices[DEV_FRAMEBUFFER].out   = fb_out;
    A.devices[DEV_FRAMEBUFFER].ctl   = fb_ctl;
    fb_reset();
}
