#include "hw.h"
#include "kernel.h"

static const char *interrupt_names[] = {
    "INT_SER_RX",        "INT_KBD_CHAR",      "INT_KBD_SCAN",       "INT_TPS_FINISH",
    "INT_HCS_FINISH",    "INT_TIMER_TIMEOUT", "INT_TIMER_INTERVAL", "INT_VIDEO_VBLANK",
    "INT_MOUSE_PRESSED", "INT_TPS_EJECTED",   "INT_TPS_INSERTED",   "INT_AUDIO_NOTE_END",

};

static void no_interrupt_hook(u32 *win, struct IPCMessage *msg_out)
{
    return;
}

void interrupt_init(void)
{
    usize i = 0;
    for (i = 0; i < AKAI_NUM_INTERRUPTS; i++) {
        A.hooks[i] = no_interrupt_hook;
    }
}

KernelInterruptHook kernel_hook_interrupt(enum TaleaInterrupt interrupt, KernelInterruptHook hook)
{
    usize               idx  = interrupt - INT_SER_RX;
    KernelInterruptHook prev = A.hooks[idx];

    A.hooks[idx] = hook;
    return prev;
}

extern void timer_interval_hook(u32 *win, struct IPCMessage *msg_out);

void akai_interrupt(u8 vector)
{
    u32 *win;

    u32 status = _lwd(AKAI_KERNEL_STATUS_SAVE);
    u32 pc     = _lwd(AKAI_KERNEL_PC_SAVE);

    if (status & CPU_STATUS_SUPERVISOR) {
        _trace(0xADDDDD, vector, status);
    }

    _trace(0x1ba, vector, A.pr.curr->pid);
    save_ctx(A.pr.curr);
    win = A.pr.curr->ctx.regs;
    _trace(0x1ba, 0xaaaa);

    if (vector >= INT_SER_RX && vector < AKAI_INVALID_INTERRUPT) {
        struct IPCMessage msg;
        usize             signal = vector - INT_SER_RX;
        usize             i      = 0;
        u32               flag   = (1 << signal);

        memset(&msg, 0, sizeof(msg));

        if (A.hooks[signal] != NULL) {
            _trace(0x1ba, 0xbbbb, A.hooks[signal], signal);
            _trace(0x1ba, 0xbbbb, timer_interval_hook);
            A.hooks[signal](win, &msg);
        }

        for (i = 0; i < MAX_PROCESS; i++) {
            if (!(A.pr.proc[i].flags & PROC_IPC)) continue;
            if (process_check_recv(KERNEL_PID, A.pr.proc[i].pid, signal)) {
                bool overflow;
                A.pr.proc[i].pending_events |= flag;
                overflow = process_queue_msg(A.pr.proc[i].pid, &msg);
                if (overflow) A.pr.proc[i].message_queue_flags |= INBOX_FLAG_QUEUE_OVERFLOW;
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
