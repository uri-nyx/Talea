#ifndef HANDLERS_H
#define HANDLERS_H

#include "../include/process.h"
#include "../include/system.h"
#include "libsirius/discovery.h"
#include "libsirius/types.h"

extern struct Processes  processes;
extern struct SystemInfo sys;

enum AkaiSyscalls {
    SYSCALL_EXIT,
    SYSCALL_YIELD,
    SYSCALL_HOOK,
    SYSCALL_UNHOOK,
};

enum TaleaException {
    EXCEPTION_NONE = -1,
    EXCEPTION_RESET,
    EXCEPTION_BUS_ERROR = 0x2,
    EXCEPTION_ADDRESS_ERROR,
    EXCEPTION_ILLEGAL_INSTRUCTION_TALEA,
    EXCEPTION_DIVISION_ZERO,
    EXCEPTION_PRIVILEGE_VIOLATION,
    EXCEPTION_PAGE_FAULT,
    EXCEPTION_ACCESS_VIOLATION_TALEA,
    EXCEPTION_DEBUG_STEP,
    EXCEPTION_OVERSPILL,
    EXCEPTION_UNDERSPILL,
};

enum TaleaInterrupt {
    INT_SER_RX = 0x10,
    INT_KBD_CHAR,
    INT_KBD_SCAN,
    INT_TPS_FINISH,
    INT_HCS_FINISH,
    INT_TIMER_TIMEOUT,
    INT_TIMER_INTERVAL,
    INT_VIDEO_VBLANK,
    INT_MOUSE_PRESSED,
    INT_TPS_EJECTED,
    INT_TPS_INSERTED,
    INT_AUDIO_NOTE_END,
    AKAI_INVALID_INTERRUPT,
};

enum EventFlags {
    EVENT_SER_RX         = 1U << 0U,
    EVENT_KBD_CHAR       = 1U << 1U,
    EVENT_KBD_SCAN       = 1U << 2U,
    EVENT_TPS_FINISH     = 1U << 3U,
    EVENT_HCS_FINISH     = 1U << 4U,
    EVENT_TIMER_TIMEOUT  = 1U << 5U,
    EVENT_TIMER_INTERVAL = 1U << 6U,
    EVENT_VIDEO_VBLANK   = 1U << 7U,
    EVENT_MOUSE_PRESSED  = 1U << 8U,
    EVENT_TPS_EJECTED    = 1U << 9U,
    EVENT_TPS_INSERTED   = 1U << 10U,
    EVENT_AUDIO_NOTE_END = 1U << 11U,
};

#define AKAI_NUM_INTERRUPTS (AKAI_INVALID_INTERRUPT - INT_SER_RX)

u32  akai_syscall(u32 service);
void akai_exception(u8 vector, u32 fault_addr);
void akai_interrupt(u8 vector);

void kernel_panic(u8 vector, u32 fault_addr, struct ThreadCtx *ctx);
void kernel_panic_internal(const char *msg);

typedef void *(*KernelInterruptHook)(void *);

// Returns the address of the previous handler or NULL if there is non
KernelInterruptHook kernel_hook_interrupt(enum TaleaInterrupt interrupt, KernelInterruptHook hook);

#endif /* HANDLERS_H */
