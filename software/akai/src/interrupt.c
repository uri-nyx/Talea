#include "../include/handlers.h"
#include "../include/hw.h"
#include "../include/process.h"
#include "libsirius/types.h"

static const char *interrupt_names[] = {
    "INT_SER_RX",        "INT_KBD_CHAR",      "INT_KBD_SCAN",       "INT_TPS_FINISH",
    "INT_HCS_FINISH",    "INT_TIMER_TIMEOUT", "INT_TIMER_INTERVAL", "INT_VIDEO_VBLANK",
    "INT_MOUSE_PRESSED", "INT_TPS_EJECTED",   "INT_TPS_INSERTED",   "INT_AUDIO_NOTE_END",

};

void akai_interrupt(u8 vector)
{
    static u32 win[32];

    u32 status = _lwd(AKAI_KERNEL_STATUS_SAVE);
    u32 pc     = _lwd(AKAI_KERNEL_PC_SAVE);

    _trace(0x1ba, vector, processes.curr->pid);
    save_ctx(processes.curr);

    load_window(sirius_cwp - 1, &win);

    switch (vector) {
    default: break;
    }

    _trace(0x4503, status, pc);
    _swd(AKAI_KERNEL_STATUS_RESTORE, status);
    _swd(AKAI_KERNEL_PC_RESTORE, pc);
    return;
}
