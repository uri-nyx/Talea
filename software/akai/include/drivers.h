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
    u8   w, h, bpc;
    u8   attr[3], default_attr[3], saved_attr[3];
    u8   tabsize;
    u8   canonical;
    bool reversed, saved;

    AnsiParser ansi;
};

/* KB */

void kbd_scan_hook(u32 *win, struct IPCMessage *msg_out);
u8   kbd_in(u8 port);
i32  kbd_out(u8 port, u8 value);
i32  kbd_reset(void);
i32  kbd_ctl(u32 command, void *buff, u32 len);

u8  txt_in(u8 port);
i32 txt_out(u8 port, u8 value);
i32 txt_reset(void);
i32 txt_ctl(u32 command, void *buff, u32 len);

#endif /* DRIVERS_H */
