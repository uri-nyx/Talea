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

void kbd_scan_hook(u32 *win, struct IPCMessage *msg_out)
{
    char c = _lwd(sys.terminal + TERMINAL_KBD_CSR) >> 16;

    *(u8 *)AKAI_IPC_KERNEL_DASH = c;

    // TODO: define messages internal protocol for interrupts
    msg_out->sender  = KERNEL_PID;
    msg_out->type    = 1; // I dont know, KB, or something
    msg_out->msgid   = 0;
    msg_out->subject = c;
    msg_out->content = NULL;

    {
        static usize pos     = 80;
        usize        tb_size = sys.textbuffer.w * sys.textbuffer.h;

        *((u32 *)AKAI_TEXTBUFFER + pos) = ((u32)c << 24) | 0x000f0000;
        pos++;
    }
}

void akai_interrupt(u8 vector)
{
    static u32 win[32];

    u32 status = _lwd(AKAI_KERNEL_STATUS_SAVE);
    u32 pc     = _lwd(AKAI_KERNEL_PC_SAVE);

    if (status & CPU_STATUS_SUPERVISOR) {
        _trace(0xADDDDD, vector, status);
    }

    _trace(0x1ba, vector, processes.curr->pid);
    save_ctx(processes.curr);

    load_window(sirius_cwp - 1, &win);

    if (vector >= INT_SER_RX && vector < AKAI_INVALID_INTERRUPT) {
        static struct IPCMessage msg;
        usize                    signal = vector - INT_SER_RX;
        usize                    i      = 0;
        u32                      flag   = (1 << signal);

        memset(&msg, 0, sizeof(msg));

        if (kernel_interrupt_hooks[signal] != NULL) {
            kernel_interrupt_hooks[signal](win, &msg);
        }

        for (i = 0; i < MAX_PROCESS; i++) {
            if (!(processes.proc[i].flags & PROC_IPC)) continue;
            if (process_check_recv(&processes, KERNEL_PID, processes.proc[i].pid, signal)) {
                bool overflow;
                processes.proc[i].pending_events |= flag;
                overflow = process_queue_msg(&processes, processes.proc[i].pid, &msg);
                if (overflow) processes.proc[i].message_queue_flags |= INBOX_FLAG_QUEUE_OVERFLOW;
            }
        }

    } else {
        // TODO: better error, but this should never happen unless I add hardware
        _trace(0xBADC0DE, vector, 0xBADC0DE);
    }

    _trace(0x4503, status, pc);
    _swd(AKAI_KERNEL_STATUS_RESTORE, status);
    _swd(AKAI_KERNEL_PC_RESTORE, pc);
    return;
}
