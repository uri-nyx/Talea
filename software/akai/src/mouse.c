#include "hw.h"
#include "kernel.h"
#include "libsirius/keys.h"

#define err A.pr.curr->last_error

bool mous_check_events(void)
{
    struct mous_ev ev;
    u8             pcx, pcy;
    bool           lp, lr, rp, rr;

    memcpy(&A.mous.ev_prev, &A.mous.ev_curr, sizeof(A.mous.ev_prev));
    mous_poll(&A.mous.ev_curr.x, &A.mous.ev_curr.y, &A.mous.ev_curr.csr, &A.mous.ev_curr.left,
              &A.mous.ev_curr.right);

    if (A.mous.mode & MOUS_REPORT) {
        pcx = A.mous.cx, pcy = A.mous.cy;
        lp = false, lr = false, rp = false, rr = false;

        A.mous.px = A.mous.ev_curr.x;
        A.mous.py = A.mous.ev_curr.y;
        A.mous.cx = (A.mous.ev_curr.x / A.txt.cw) + 1;
        A.mous.cy = (A.mous.ev_curr.y / A.txt.ch) + 1;
        A.mous.lp = lp = A.mous.ev_curr.left && !A.mous.ev_prev.left;
        A.mous.lr = lr = !A.mous.ev_curr.left && A.mous.ev_prev.left;
        A.mous.rp = rp = A.mous.ev_curr.right && !A.mous.ev_prev.right;
        A.mous.rr = rr = !A.mous.ev_curr.right && A.mous.ev_prev.right;

        A.mous.left  = A.mous.ev_curr.left;
        A.mous.right = A.mous.ev_curr.right;

        if (A.mous.cx != pcx || A.mous.cy != pcy || lp || lr || rp || rr) {
            A.mous.new_ev = true;
            return true;
        }
    }

    A.mous.new_ev = false;

    return false;
}

void inject_byte(u8 b)
{
    kbd_event_push(0x8000UL | (u32)b << 16);
}

void inject_uint(u8 v)
{
    /* v is always <= 255 because cx,cy are u8 */
    if (v >= 100) inject_byte('0' + (v / 100));
    if (v >= 10) inject_byte('0' + ((v / 10) % 10));
    inject_byte('0' + (v % 10));
}

void inject_sgr(u8 code, u8 cx, u8 cy, bool release)
{
    inject_byte('\x1b');
    inject_byte('[');
    inject_byte('<');

    /* code */
    inject_uint(code);

    inject_byte(';');
    inject_uint(cx);
    inject_byte(';');
    inject_uint(cy);

    inject_byte(release ? 'm' : 'M');
    _trace(0x18880, 4);

    kbd_process_input();
}

// Extended SGR (1006 xterm) mouse subset. my god, this is arcane
bool mous_inject_event()
{
#define MB1      0
#define MB2      1
#define MB3      2
#define MBNONE   3
#define MB_LEFT  MB1
#define MB_RIGHT MB3

#define MOUSE_MOTION_INDICATOR 32

    u8 cx, cy;
    if (!mous_check_events()) return false;

    cx = A.mous.cx;
    cy = A.mous.cy;

    /* 1. Left press */
    if (A.mous.lp) {
        inject_sgr(MB_LEFT, cx, cy, false);
        return true;
    }

    /* 2. Left release */
    if (A.mous.lr) {
        inject_sgr(MB_LEFT, cx, cy, true);
        return true;
    }

    /* 3. Right press */
    if (A.mous.rp) {
        inject_sgr(MB_RIGHT, cx, cy, false);
        return true;
    }

    /* 4. Right release */
    if (A.mous.rr) {
        inject_sgr(MB_RIGHT, cx, cy, true);
        return true;
    }

    /* 5. Movement (hover or drag) */
    if (A.mous.left) {
        /* left drag */
        inject_sgr(MOUSE_MOTION_INDICATOR + MB_LEFT, cx, cy, false);
    } else if (A.mous.right) {
        /* right drag */
        inject_sgr(MOUSE_MOTION_INDICATOR + MB_RIGHT, cx, cy, false);
    } else {
        /* hover */
        inject_sgr(MOUSE_MOTION_INDICATOR + MBNONE, cx, cy, false);
    }

    return true;
}

void mous_poll(u16 *outx, u16 *outy, u8 *csr, bool *left, bool *right)
{
    *csr  = _lbud(A.info.mouse + MOUSE_CSR);
    *outx = _lhud(A.info.mouse + MOUSE_X);
    *outy = _lhud(A.info.mouse + MOUSE_Y);

    *left  = !!(*csr & MOUSE_BUTT_LEFT);
    *right = !!(*csr & MOUSE_BUTT_RIGHT);
}

u8 mous_in(u8 port)
{
    if (port > A.devices[DEV_MOUSE].ports) {
        err = P_ERROR_NO_PORT;
        return 0;
    }

    return _lbud(A.devices[DEV_MOUSE].base + port);
}

i32 mous_out(u8 port, u8 value)
{
    if (port > A.devices[DEV_MOUSE].ports) {
        err = P_ERROR_NO_PORT;
        return (signed)A_ERROR;
    }

    _sbd(A.devices[DEV_MOUSE].base + port, value);
    return (signed)A_OK;
}

i32 mous_reset(void)
{
    // should revert to visible and no sprite
    u8 csr = _lbud(A.info.mouse + MOUSE_CSR);
    csr &= ~(MOUSE_CUSTOM);
    csr |= MOUSE_VISIBLE;
    _sbd(A.info.mouse + MOUSE_CSR, csr);
    return (signed)A_OK;
}

i32 mous_ctl(u32 command, void *buff, u32 len)
{
    switch (command) {
    case DEV_RESET: return mous_reset();
    case MCTL_GET_MODE:
    case PX_GETCANON: return (signed)A.mous.mode;
    case MCTL_SET_MODE: {
        u8 mode;
        if (len != 1) {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
        }

        mode = *(u8 *)buff;

        A.mous.mode = mode;

        return (signed)A_OK;
        break;
    }

    default: err = P_ERROR_NO_CTL_COMMAND; return (signed)A_ERROR_CTL;
    }
}

void mous_driver_init(void)
{
    A.devices[DEV_MOUSE].base  = A.info.mouse;
    A.devices[DEV_MOUSE].ports = 10;
    A.devices[DEV_MOUSE].id    = 'M';
    A.devices[DEV_MOUSE].num   = DEV_MOUSE;
    A.devices[DEV_MOUSE].reset = mous_reset;
    A.devices[DEV_MOUSE].in    = mous_in;
    A.devices[DEV_MOUSE].out   = mous_out;
    A.devices[DEV_MOUSE].ctl   = mous_ctl;
    mous_reset();
}
