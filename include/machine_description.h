#ifndef C3B7CF5D_94B6_4D22_8C31_96A68EFF4AD8
#define C3B7CF5D_94B6_4D22_8C31_96A68EFF4AD8
#ifndef MACHINE_DESCRIPTION_H
#define MACHINE_DESCRIPTION_H

/* Machine identification */

#define TALEA_SYSTEM_VERSION_MAJOR 0
#define TALEA_SYSTEM_VERSION_MINOR 9

#define TALEA_VENDOR_ID 'T' // The House of Taleä
#define TAELA_ARCH_ID   'S' // Sirius Mk I Cpu

/* Hardware configuration options */
#define TALEA_WITH_MMU 1 // compile with MMU
#define TALEA_WITH_3D  1 // compile with 3d support in the video device
#define TALEA_WITH_HCS 1 // compile with hcs (fixed storage) installed

#define TALEA_MAPPED_FILE_ADDR 0x10000 // Map a file into this address at boot

/* Cpu configuration */
#define SIRIUS_BASE_FREQ_HZ 10200000 // The Sirius runs at 10 Mhz

/* Memory configuration */

#define KB (1024)
#define MB (KB * KB)

#define TALEA_ADDRESS_SPACE_SIZE (1U << 24U)                  // A 24-bit address bus
#define TALEA_DATA_MEM_SZ        (64 * KB)                    // Supervisor data memory is 64KB
#define TALEA_ROM_SIZE           (64 * KB)                    // 64KB of ROM
#define TALEA_MAIN_MEM_SZ        ((16 * MB) - TALEA_ROM_SIZE) // ~16MB for this config

#define TALEA_FIRMWARE_ADDRESS      (TALEA_ADDRESS_SPACE_SIZE - TALEA_ROM_SIZE)
#define TALEA_RESET_ADDRESS         TALEA_FIRMWARE_ADDRESS // Starts executing code at this addr
#define TALEA_FIRMWARE_DATA_ADDRESS (32 * KB) // Data section of firmware is mapped to this addr

static_assert(TALEA_MAIN_MEM_SZ % (1U << 16) == 0, "Main memory size must be a multiple of 64Kb");
static_assert(TALEA_MAIN_MEM_SZ < TALEA_ADDRESS_SPACE_SIZE,
              "Main memory size must be lower than the address space");

/* DATA memory map */
#define TALEA_DATA_IO_PORTS_END 0x00FF // Device IO ports (16 per device)
#define TALEA_DATA_DEV_MAP      0x0100 // Device map
#define TALEA_DATA_SYSTEM_START 0x0110 // System csrs (256 bytes)
#define TALEA_DATA_SYSTEM_END   0x01FF
#define TALEA_DATA_IVT_BASE     0x0200 // Interrupt vector tables (256 vectors @ 32 bits)
#define TALEA_DATA_IVT_END      0x05FF
#define TALEA_DATA_FIRMWARE_RES 0x0600 // Data memory reserved for firmware use (convention)
#define TALEA_DATA_FREE_BASE    0x1000 // Data memory free for use in Supervisor Mode

/* Device map */
#define TALEA_DEVICE_MAP   \
    X(0x00, TERMINAL, 'K') \
    X(0x10, VIDEO, 'V')    \
    X(0x20, STORAGE, 'D')  \
    X(0x30, AUDIO, 'A')    \
    X(0x40, MOUSE, 'M')

#define X(addr, name, id) DEV_SLOT_##name,
enum { TALEA_DEVICE_MAP DEV_SLOT_COUNT };
#undef X

#define X(addr, name, id) DEV_BASE_##name = addr,
enum { TALEA_DEVICE_MAP };
#undef X

#define X(addr, name, id) DEV_ID_##name = id,
enum { TALEA_DEVICE_MAP };
#undef X

#define X(addr, name, id) id,
static const char Talea_DeviceMap[16] = { TALEA_DEVICE_MAP };
#undef X

/* SYSTEM REGISTERS */
enum {
    /* Identity */
    REG_SYSTEM_ARCH_ID = TALEA_DATA_SYSTEM_START,
    REG_SYSTEM_VENDOR_ID,
    REG_SYSTEM_VERSION,       // The system version as [7:4] Major [3:0] Minor
    REG_SYSTEM_MEMSIZE_FLASH, // The memory size as a multiple of 64KB - 1
    REG_SYSTEM_CLOCK,         // The clock frequency
    REG_SYSTEM_DEVICE_NUM,    // The number of device slots used

    /* Entropy */
    REG_SYSTEM_RNG, // A random byte

    /* Power */
    REG_SYSTEM_POWER, // Poweroff register

    /* Time */
    REG_SYSTEM_YEAR,
    REG_SYSTEM_MONTH,
    REG_SYSTEM_DAY,
    REG_SYSTEM_HOUR,
    REG_SYSTEM_MINUTE,
    REG_SYSTEM_SECOND,
    REG_SYSTEM_MILLIS,
    REG_SYSTEM_COUNTER,

    /* Exceptions */
    REG_SYSTEM_EXCEPTION,  // Last or current exception raised
    REG_SYSTEM_FAULT_ADDR, // Last address where an exception originated

    /* CPU State */
    REG_SYSTEM_CYCLES_INSTRET = REG_SYSTEM_FAULT_ADDR + 4, // number of cycles/instructions executed

    /* Context Windowing */
    REG_SYSTEM_CWP = REG_SYSTEM_CYCLES_INSTRET + 8,
    REG_SYSTEM_WIN_SPILLED,
    REG_SYSTEM_WIN_SEL,
    REG_SYSTEM_WIN_OP,
    REG_SYSTEM_WIN_BUFF,

    /* MMU registers */
    REG_SYSTEM_PDT = REG_SYSTEM_WIN_BUFF + 128, // Current pdt base in DATA memory
    REG_SYSTEM_TLB = REG_SYSTEM_PDT + 2,        // Writing flushes the TLB
    REG_SYSTEM_MMU,                             // enable/disable MMU

    /* Context switching */
    REG_SYSTEM_USP, // Saved user stack pointer
    
    /* Interrupts */
    REG_SYSTEM_INTERRUPT = REG_SYSTEM_USP + 4, // Last interrupt acknowledged
    REG_SYTEM_FAULT_CAUSE,
};

enum {
    TALEA_SYSTEM_CALENDAR_MODE,
    TALEA_SYSTEM_UNIXTIME_MODE,
    TALEA_SYSTEM_MICROS_MODE,
    TALEA_SYSTEM_MILLIS_MODE,
    TALEA_SYSTEM_INST_MODE
};

enum {
    TALEA_SYSTEM_WIN_OP_LOAD,
    TALEA_SYSTEM_WIN_OP_STORE,
};

/* --- INTERRUPTS AND CPU EXCEPTIONS --- */

#define PRIORITY_STORAGE_INTERRUPT  1
#define PRIORITY_VBLANK_INTERRUPT   2
#define PRIORITY_AUDIO_INTERRUPT    3
#define PRIORITY_SERIAL_INTERRUPT   4
#define PRIORITY_KEYBOARD_INTERRUPT 5
#define PRIORITY_TIMEOUT_INTERRUPT  6
#define PRIORITY_INTERVAL_INTERRUPT 6

enum TaleaInterrupt {
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

#endif

#endif /* C3B7CF5D_94B6_4D22_8C31_96A68EFF4AD8 */
