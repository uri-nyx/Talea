#ifndef SYSTEM_H
#define SYSTEM_H

#include "libsirius/devices.h"

#define sirius_pdt ((_gsreg() & 0x000FFF00) >> 4)
#define sirius_cwp (_lbud(REG_SYSTEM_CWP))

/* Critical kernel scratchpad in DATA memory */
#define AKAI_KERNEL_PC_SAVE        0x1000
#define AKAI_KERNEL_STATUS_SAVE    0x1004
#define AKAI_KERNEL_PC_RESTORE     0x1008
#define AKAI_KERNEL_STATUS_RESTORE 0x100C
#define AKAI_KERNEL_X5SAVE         0x1010

/* AKAI MEMORY MAP */
#define AKAI_KERNEL_BASE          0X000000
#define AKAI_KERNEL_DATA_END      // TODO: calculate kernel size
#define AKAI_KERNEL_STACK_TOP     0X020000
#define AKAI_KERNEL_END           0x020000
#define AKAI_PROCESS_PAGE_TABLES  0x021000
#define AKAI_PROCESS_PT0          0x021000
#define AKAI_PROCESS_PT1          0x022000
#define AKAI_PROCESS_PT2          0x023000
#define AKAI_PROCESS_PT3          0x024000
#define AKAI_PROCESS_BASE         0x025000
#define AKAI_PROCESS_END          0xF6A000
#define AKAI_PROCESS_STACK_END    0xF6B000
#define AKAI_PROCESS_STACK_TOP    0xF7B000
#define AKAI_IPC_KERNEL_IN        0xF7C000
#define AKAI_IPC_KERNEL_OUT       0xF7D000
#define AKAI_IPC_KERNEL_DASH      0xF7E000
#define AKAI_IPC_INBOX            0xF7F000
#define AKAI_IPC_OUTBOX           0xF80000
#define AKAI_IPC_SYSTEM_DASHBOARD 0xF81000
#define AKAI_IDLE_BASE            0xF82000
#define AKAI_TEXTBUFFER           0xF84000
#define AKAI_FRAMEBUFFER          0xFA5000
#define AKAI_ADDR_SPACE_END       0xFF0000

#define AKAI_PROCESS_STATUS (CPU_STATUS_INTERRUPT_ENABLE | CPU_STATUS_MMU_ENABLE)

#define AKAI_PDT_BASE (0xF000U)

#define KERNEL_PID ((ProcessPID)(0))

#endif /* SYSTEM_H */
