#include "libsirius/types.h"
#include "../include/handlers.h"
#include "../include/process.h"

static const char *interrupt_names[] = {
    "INT_SER_RX",        "INT_KBD_CHAR",      "INT_KBD_SCAN",       "INT_TPS_FINISH",
    "INT_HCS_FINISH",    "INT_TIMER_TIMEOUT", "INT_TIMER_INTERVAL", "INT_VIDEO_VBLANK",
    "INT_MOUSE_PRESSED", "INT_TPS_EJECTED",   "INT_TPS_INSERTED",   "INT_AUDIO_NOTE_END",

};

void akai_interrupt(u8 vector)
{
    static u32 win[32];

    save_ctx(processes.curr);

    load_window(sirius_cwp - 1, &win);

    _trace(0x1ba, vector);

    switch (vector) {
    default: break;
    }
}
