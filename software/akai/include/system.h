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

#define AKAI_PROCESS_BASE        0x25000
#define AKAI_IDLE_BASE           (sys.textbuffer.addr - PAGE_SIZE * 4)
#define AKAI_PROCESS_PAGE_TABLES 0x21000
#define AKAI_PROCESS_PT0         0x21000
#define AKAI_PROCESS_PT1         0x22000
#define AKAI_PROCESS_PT2         0x23000
#define AKAI_PROCESS_PT3         0x24000
#define AKAI_PROCESS_STACK       0x250F0 // TODO: change this back to 0x3FF000
#define AKAI_PROCESS_STATUS      (CPU_STATUS_INTERRUPT_ENABLE | CPU_STATUS_MMU_ENABLE)

#define AKAI_PDT_BASE (0xF000U)

#define KERNEL_END 0x20000
#define KERNEL_PID ((ProcessPID)(0))

#endif /* SYSTEM_H */
