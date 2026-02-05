#ifndef HANDLERS_H
#define HANDLERS_H

#include "../include/system.h"
#include "../include/process.h"
#include "libsirius/discovery.h"
#include "libsirius/types.h"

extern struct Processes  processes;
extern struct SystemInfo sys;

enum AkaiSyscalls {
    SYSCALL_EXIT,
    SYSCALL_YIELD,
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

};

u32  akai_syscall(u32 service);
void akai_exception(u8 vector, u32 fault_addr);
void akai_interrupt(u8 vector);

void kernel_panic(u8 vector, u32 fault_addr, struct ThreadCtx *ctx);

#endif /* HANDLERS_H */
