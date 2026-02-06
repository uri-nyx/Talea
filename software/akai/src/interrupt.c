#include "../include/handlers.h"
#include "../include/hw.h"
#include "../include/process.h"
#include "libsirius/types.h"

static const char *interrupt_names[] = {
    "INT_SER_RX",        "INT_KBD_CHAR",      "INT_KBD_SCAN",       "INT_TPS_FINISH",
    "INT_HCS_FINISH",    "INT_TIMER_TIMEOUT", "INT_TIMER_INTERVAL", "INT_VIDEO_VBLANK",
    "INT_MOUSE_PRESSED", "INT_TPS_EJECTED",   "INT_TPS_INSERTED",   "INT_AUDIO_NOTE_END",

};

static KernelInterruptHook kernel_interrupt_hooks[AKAI_NUM_INTERRUPTS];

KernelInterruptHook kernel_hook_interrupt(enum TaleaInterrupt interrupt, KernelInterruptHook hook)
{
    usize               idx  = interrupt - INT_SER_RX;
    KernelInterruptHook prev = kernel_interrupt_hooks[idx];

    kernel_interrupt_hooks[idx] = hook;
    return prev;
}

void akai_interrupt(u8 vector)
{
    static u32 win[32];

    u32 status = _lwd(AKAI_KERNEL_STATUS_SAVE);
    u32 pc     = _lwd(AKAI_KERNEL_PC_SAVE);

    _trace(0x1ba, vector, processes.curr->pid);
    save_ctx(processes.curr);

    load_window(sirius_cwp - 1, &win);

    if (vector >= INT_SER_RX && vector < AKAI_INVALID_INTERRUPT) {
        usize idx  = vector - INT_SER_RX;
        usize i    = 0;
        u32   flag = (1 << idx);

        if (kernel_interrupt_hooks[idx] != NULL) kernel_interrupt_hooks[idx](win);

        for (i = 0; i < MAX_PROCESS; i++) {
            if (processes.proc[i].event_mask & flag) processes.proc[i].pending_events |= flag;
        }
        
    } else {
        // TODO: better error, but this should never happen unless I add hardware
        _trace(0xBADC0DE, vector);
    }

    _trace(0x4503, status, pc);
    _swd(AKAI_KERNEL_STATUS_RESTORE, status);
    _swd(AKAI_KERNEL_PC_RESTORE, pc);
    return;
}
