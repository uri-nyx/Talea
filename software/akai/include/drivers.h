#ifndef DRIVERS_H
#define DRIVERS_H

#include "akai_def.h"
#include "ansi.h"
#include "process.h"

/* @AKAI: 210_DRIVERS */
#define LINE_BUFFER_LEN 1023
/* @AKAI */

struct DriverKb {
    struct akai_natural_ringbufferu32 events;

    u8          wait_queue[(MAX_PROCESS + 7) / 8];
    const char *ansi_seq;
    char        alt_buf[3];
    char        printable;

    u8    canonical;
    u8    line[LINE_BUFFER_LEN + 1];
    usize pos, next;
    bool  line_ready, blocking;
    // TODO: this can cause problems in the future, must keep per process state
};

struct DriverTxt {
    u8   x, y, sx, sy;
    u8   w, h, bpc, cw, ch;
    u8   top, bot, left, right;
    u8   attr[3], default_attr[3], saved_attr[3];
    u8   tabsize;
    u8   canonical;
    bool reversed, saved;

    AnsiParser ansi;
};

struct DriverFb {
    u8 wait_queue[(MAX_PROCESS + 7) / 8];
};

struct mous_ev {
    u16  x, y;
    bool left, right;
    u8   csr;
};

struct DriverMouse {
    struct mous_ev ev_prev, ev_curr;
    bool           new_ev;
    u8             cx, cy;
    u16            px, py;
    bool           lp, lr, rp, rr;
    bool           left, right;
    u8             mode;
};

u8   ser_in(u8 port);
i32  ser_out(u8 port, u8 value);
i32  ser_reset(void);
i32  ser_ctl(u32 command, void *buff, u32 len);
void ser_driver_init(void);

void kbd_scan_hook(u32 *win, struct IPCMessage *msg_out);
u8   kbd_in(u8 port);
i32  kbd_out(u8 port, u8 value);
i32  kbd_reset(void);
i32  kbd_ctl(u32 command, void *buff, u32 len);
void kbd_driver_init(void);
void kbd_process_input(void);

u8   txt_in(u8 port);
i32  txt_out(u8 port, u8 value);
i32  txt_reset(void);
i32  txt_ctl(u32 command, void *buff, u32 len);
void video_driver_init(void);
void fb_vblank_hook(u32 *win, struct IPCMessage *msg_out);

void mous_poll(u16 *outx, u16 *outy, u8 *csr, bool *left, bool *right);
u8   mous_in(u8 port);
i32  mous_out(u8 port, u8 value);
i32  mous_reset(void);
i32  mous_ctl(u32 command, void *buff, u32 len);
void mous_driver_init(void);
bool mous_inject_event();

#endif /* DRIVERS_H */
