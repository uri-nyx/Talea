#include <stdlib.h>
#include <time.h>

#include "bus.h"
#include "cpu.h"
#include "talea.h"

static inline u8 freq_to_byte(float targetMhz)
{
    /* Quantization for the frequency byte */
    if (targetMhz < 10.0f) targetMhz = 10.0f;
    if (targetMhz > 100.0f) targetMhz = 100.0f;

    float val = ((targetMhz - 10.0f) / 90.0f) * 255.0f;

    return (u8)(val + 0.5f);
}

u8 System_Read(TaleaMachine *m, u16 addr)
{
    static u64        instructionsRetired;
    static u64        cycles;
    static double     uptime;
    static time_t     now;
    static struct tm *timeinfo;
    time(&now);
    timeinfo = localtime(&now);

    switch (addr) {
    case REG_SYSTEM_ARCH_ID: return TAELA_ARCH_ID;
    case REG_SYSTEM_VENDOR_ID: return TALEA_VENDOR_ID;
    case REG_SYSTEM_VERSION:
        return (TALEA_SYSTEM_VERSION_MAJOR << 4) | (TALEA_SYSTEM_VERSION_MINOR & 0xf);
    case REG_SYSTEM_MEMSIZE_FLASH:
        // TODO: DOCUMENT THAT THIS GIVES THE NUMBER OF 64KB blocks - 1 (0 means the system has 64Kb
        // ram)
        return (TALEA_MAIN_MEM_SZ / (1U << 16)) - 1;
    case REG_SYSTEM_CLOCK:
        // TODO: Document this. Get freequency back by 10000 + (reg_val * 353)
        return (freq_to_byte(m->cpu.frequency / 1000000));
    case REG_SYSTEM_DEVICE_NUM: return DEV_SLOT_COUNT;

    /* Entropy */
    case REG_SYSTEM_RNG:
        m->sys.seed = (m->sys.seed * 1103515245U) + 12345U;
        return m->sys.seed >> 24;

    /* Power */
    case REG_SYSTEM_POWER: break;

    /* Time */
    case REG_SYSTEM_YEAR:
        time_t n;
        time(&n);
        uptime             = GetTime();
        m->sys.lastRawtime = n;
        m->sys.uptime      = GetTime();

        if (m->sys.unixtimeMode) {
            return m->sys.lastRawtime >> 24;
        } else if (m->sys.calendarMode) {
            return timeinfo->tm_year;
        } else if (m->sys.microsMode) {
            return ((u64)(uptime * 1000000)) >> 56;
        } else {
            return 0xFF;
        }
    case REG_SYSTEM_MONTH:
        if (m->sys.unixtimeMode) {
            return m->sys.lastRawtime >> 16;
        } else if (m->sys.calendarMode) {
            return timeinfo->tm_mon;
        } else if (m->sys.microsMode) {
            return ((u64)(uptime * 1000000)) >> 48;
        } else {
            return 0xFF;
        }
    case REG_SYSTEM_DAY:
        if (m->sys.unixtimeMode) {
            return m->sys.lastRawtime >> 8;
        } else if (m->sys.calendarMode) {
            return timeinfo->tm_mday;
        } else if (m->sys.microsMode) {
            return ((u64)(uptime * 1000000)) >> 40;
        } else {
            return 0xFF;
        }
    case REG_SYSTEM_HOUR:
        if (m->sys.unixtimeMode) {
            return m->sys.lastRawtime;
        } else if (m->sys.calendarMode) {
            return timeinfo->tm_hour;
        } else if (m->sys.microsMode) {
            return ((u64)(uptime * 1000000)) >> 32;
        } else {
            return 0xFF;
        }
    case REG_SYSTEM_MINUTE:
        if (m->sys.calendarMode) {
            return timeinfo->tm_min;
        } else if (m->sys.microsMode) {
            return ((u64)(uptime * 1000000)) >> 24;
        } else if (m->sys.millisMode) {
            uptime = GetTime();
            return ((u64)(uptime * 1000)) >> 24;
        } else {
            return 0xFF;
        }
    case REG_SYSTEM_SECOND:
        if (m->sys.calendarMode) {
            return timeinfo->tm_sec;
        } else if (m->sys.microsMode) {
            return ((u64)(uptime * 1000000)) >> 16;
        } else if (m->sys.millisMode) {
            return ((u64)(uptime * 1000)) >> 16;
        } else {
            return 0xFF;
        }
    case REG_SYSTEM_MILLIS:
        if (m->sys.microsMode) {
            return ((u64)(uptime * 1000000)) >> 8;
        } else if (m->sys.millisMode) {
            return ((u64)(uptime * 1000)) >> 8;
        } else {
            return 0xFF;
        }
    case REG_SYSTEM_COUNTER:
        if (m->sys.microsMode) {
            return ((u64)(uptime * 1000000));
        } else if (m->sys.millisMode) {
            return ((u64)(uptime * 1000));
        } else {
            return 0xFF;
        }

    /* Exceptions */
    case REG_SYSTEM_EXCEPTION: return m->cpu.LastException;

    case REG_SYSTEM_FAULT_ADDR: return (m->cpu.faultAddr >> 24);
    case REG_SYSTEM_FAULT_ADDR + 1: return (m->cpu.faultAddr >> 16);
    case REG_SYSTEM_FAULT_ADDR + 2: return (m->cpu.faultAddr >> 8);
    case REG_SYSTEM_FAULT_ADDR + 3: {
        u8 addr          = m->cpu.faultAddr;
        TALEA_LOG_TRACE("Clearing fault addr\n");
        m->cpu.faultAddr = 0;
        return addr;
    };

    /* CPU State */
    case REG_SYSTEM_CYCLES_INSTRET: {
        instructionsRetired = m->cpu.instructionsRetired;
        cycles              = m->cpu.cycles;

        if (m->sys.instMode) {
            return (instructionsRetired >> 56);

        } else {
            return (cycles >> 56);
        }
    }
    case REG_SYSTEM_CYCLES_INSTRET + 1:
        if (m->sys.instMode)
            return (instructionsRetired >> 48);
        else
            return (cycles >> 48);
    case REG_SYSTEM_CYCLES_INSTRET + 2:
        if (m->sys.instMode)
            return (instructionsRetired >> 40);
        else
            return (cycles >> 40);
    case REG_SYSTEM_CYCLES_INSTRET + 3:
        if (m->sys.instMode)
            return (instructionsRetired >> 32);
        else
            return (cycles >> 32);
    case REG_SYSTEM_CYCLES_INSTRET + 4:
        if (m->sys.instMode)
            return (instructionsRetired >> 24);
        else
            return (cycles >> 24);
    case REG_SYSTEM_CYCLES_INSTRET + 5:
        if (m->sys.instMode)
            return (instructionsRetired >> 16);
        else
            return (cycles >> 16);
    case REG_SYSTEM_CYCLES_INSTRET + 6:
        if (m->sys.instMode)
            return (instructionsRetired >> 8);
        else
            return (cycles >> 8);
    case REG_SYSTEM_CYCLES_INSTRET + 7:
        if (m->sys.instMode)
            return (instructionsRetired);
        else
            return (cycles);

    /* Context Windowing */
    case REG_SYSTEM_CWP: return m->cpu.cwp;
    case REG_SYSTEM_WIN_SPILLED: return m->cpu.spilledWindows;
    case REG_SYSTEM_WIN_SEL: return m->sys.winSel;
    case REG_SYSTEM_WIN_OP: return m->sys.winOp;

    /* MMU */
    case REG_SYSTEM_PDT: return SR_GET_PDT(m->cpu.status) >> 4;
    case REG_SYSTEM_PDT + 1: return SR_GET_PDT(m->cpu.status) & 0xf;
    case REG_SYSTEM_TLB: return 0;
    case REG_SYSTEM_MMU: return SR_GET_MMU(m->cpu.status) >> 29;

    /* Context switching */
    case REG_SYSTEM_USP: return (m->cpu.usp >> 24);
    case REG_SYSTEM_USP + 1: return (m->cpu.usp >> 16);
    case REG_SYSTEM_USP + 2: return (m->cpu.usp >> 8);
    case REG_SYSTEM_USP + 3: return (m->cpu.usp);

    case REG_SYSTEM_INTERRUPT: return m->cpu.interrupt;
    case REG_SYTEM_FAULT_CAUSE: {
        u8 cause          = m->cpu.faultCause;
        m->cpu.faultCause = 0;
        return cause;
    }

    default:
        if (addr >= REG_SYSTEM_WIN_BUFF && addr < REG_SYSTEM_WIN_BUFF + (32 * 4)) {
            return m->sys.winBuff[addr - REG_SYSTEM_WIN_BUFF];
        } else {
            return 0;
        }
    }
    return 0;
}

void System_Write(TaleaMachine *m, u16 addr, u8 value)
{
    FILE       *fp;
    static bool save_block_to_firmware_armed = false;
    static bool poweroffSequenceArmed        = false;

    // TALEA_LOG_TRACE("SYSTEM WRITE TO %d, v: %x\n", addr, value);

    switch (addr) {
    case REG_SYSTEM_ARCH_ID:
    case REG_SYSTEM_VENDOR_ID:
    case REG_SYSTEM_VERSION: break;
    case REG_SYSTEM_MEMSIZE_FLASH:
        // TODO: Document that this writes a block to flash (a tiny file that gets loaded at boot)
        TaleaMemoryView view = Bus_GetView(m, 0, 512, BUS_ACCESS_READ);
        if (!view.ptr || view.length != 512) {
            TALEA_LOG_WARNING("Could not write to flash!\n");
            break;
        }

        u8     buff[512];
        size_t written = Bus_ReadBlock(m, &view, buff, 512);
        if (written != 512) {
            TALEA_LOG_WARNING("Could not write to flash (2)!\n");
            break;
        }

        FILE *flash = fopen(TALEA_FLASH_FILE, "wb");
        if (flash == NULL) {
            TALEA_LOG_WARNING("Could not open flash file!\n");
            break;
        }

        written = fwrite(buff, 1, 512, flash);
        if (written != 512) {
            TALEA_LOG_WARNING("Write error writing to flash!\n");
            break;
        }

        break;
    case REG_SYSTEM_CLOCK:
    case REG_SYSTEM_DEVICE_NUM: break;

    /* Entropy */
    case REG_SYSTEM_RNG: break;

    /* Power */
    case REG_SYSTEM_POWER:
        if (value == TALEA_MAGIC_ARM_SEQUENCE && !poweroffSequenceArmed) {
            poweroffSequenceArmed = true;
        } else if (value == TALEA_MAGIC_TRIGGER_SEQUENCE && poweroffSequenceArmed) {
            TALEA_LOG_TRACE("Powering off!\n");
            Machine_Poweroff(m);
            poweroffSequenceArmed = false;
        } else {
            poweroffSequenceArmed = false;
        }
        break;

    /* Time */
    case REG_SYSTEM_YEAR:
        m->sys.unixtimeMode = false;
        m->sys.calendarMode = false;
        m->sys.microsMode   = false;
        m->sys.millisMode   = false;
        if (value == TALEA_SYSTEM_UNIXTIME_MODE)
            m->sys.unixtimeMode = true;
        else if (value == TALEA_SYSTEM_CALENDAR_MODE)
            m->sys.calendarMode = true;
        else if (value == TALEA_SYSTEM_MICROS_MODE)
            m->sys.microsMode = true;
        break;
    case REG_SYSTEM_MONTH: break;
    case REG_SYSTEM_DAY: break;
    case REG_SYSTEM_HOUR: break;
    case REG_SYSTEM_MINUTE:
        m->sys.unixtimeMode = false;
        m->sys.calendarMode = false;
        m->sys.microsMode   = false;
        m->sys.millisMode   = false;
        if (value == TALEA_SYSTEM_UNIXTIME_MODE)
            m->sys.unixtimeMode = true;
        else if (value == TALEA_SYSTEM_CALENDAR_MODE)
            m->sys.calendarMode = true;
        else if (value == TALEA_SYSTEM_MICROS_MODE)
            m->sys.microsMode = true;
        else if (value == TALEA_SYSTEM_MILLIS_MODE)
            m->sys.millisMode = true;
        break;
    case REG_SYSTEM_SECOND:
    case REG_SYSTEM_MILLIS:
    case REG_SYSTEM_COUNTER: break;

    /* Exceptions */
    case REG_SYSTEM_EXCEPTION:
    case REG_SYSTEM_FAULT_ADDR:
    case REG_SYSTEM_FAULT_ADDR + 1:
    case REG_SYSTEM_FAULT_ADDR + 2:
    case REG_SYSTEM_FAULT_ADDR + 3: break;

    /* CPU State */
    case REG_SYSTEM_CYCLES_INSTRET:
        if (value == TALEA_SYSTEM_INST_MODE) {
            m->sys.instMode = true;
        } else if (value == 0) {
            m->sys.instMode = false;
        }
        break;
    case REG_SYSTEM_CYCLES_INSTRET + 1:
    case REG_SYSTEM_CYCLES_INSTRET + 2:
    case REG_SYSTEM_CYCLES_INSTRET + 3:
    case REG_SYSTEM_CYCLES_INSTRET + 4:
    case REG_SYSTEM_CYCLES_INSTRET + 5:
    case REG_SYSTEM_CYCLES_INSTRET + 6:
    case REG_SYSTEM_CYCLES_INSTRET + 7:
        break;

        /* Context Windowing */
        /* clang-format off */
    case REG_SYSTEM_CWP: break;
    case REG_SYSTEM_WIN_SEL: m->sys.winSel = value & 0x3; break;
    
    case REG_SYSTEM_WIN_OP:
        if (value == TALEA_SYSTEM_WIN_OP_LOAD) {
            for (size_t i = 0; i < 32; i++) {
                u32 reg = 0;

                reg = (u32)m->sys.winBuff[(i * 4)] << 24;
                reg |= (u32)m->sys.winBuff[(i * 4) + 1] << 16;
                reg |= (u32)m->sys.winBuff[(i * 4) + 2] << 8;
                reg |= m->sys.winBuff[(i * 4) + 3];

                reg = i ? reg : 0;

                if (m->sys.winSel < m->cpu.spilledWindows)
                    Machine_WriteData32(m, TALEA_DATA_FIRMWARE_RES + (m->sys.winSel * 32 * 4) + (i * 4), reg);
                else
                    m->cpu.gpr[(m->sys.winSel * 32) + i] = reg;
            }
        } else if (value == TALEA_SYSTEM_WIN_OP_STORE) {
            for (size_t i = 0; i < 32; i++) {
                u32 reg;

                if (m->sys.winSel < m->cpu.spilledWindows)
                    reg = Machine_ReadData32(m, TALEA_DATA_FIRMWARE_RES + (m->sys.winSel * 32 * 4) + (i * 4));
                else
                    reg = m->cpu.gpr[(m->sys.winSel * 32) + i];

                reg = i ? reg : 0;

                m->sys.winBuff[(i * 4)]     = reg >> 24;
                m->sys.winBuff[(i * 4) + 1] = reg >> 16;
                m->sys.winBuff[(i * 4) + 2] = reg >> 8;
                m->sys.winBuff[(i * 4) + 3] = reg;
            }
        } else {
            break;
        }

        m->sys.winOp = value;
        break;

    /* MMU */
    case REG_SYSTEM_PDT:
        m->cpu.status = (m->cpu.status & 0xFFF0FFFF) | (u32)((value & 0xf) << 16);
        break;
    case REG_SYSTEM_PDT + 1:
        m->cpu.status = (m->cpu.status & 0xFFFF00FF) | ((u32)value << 8);
        break;
    case REG_SYSTEM_TLB:
        //TALEA_LOG_TRACE("TLB REG written %x\n", value);
        if (value == 0xFF) {
            // TODO: document this
            MMU_FlushTLB(m);
        }
        break;
    case REG_SYSTEM_MMU:
        //TALEA_LOG_TRACE("MMU REG written %x\n", value);
        if (value == 0xFF) {
            // TODO: document this
          //  TALEA_LOG_TRACE("Switching MMU ON!\n");
            m->cpu.status |= 0x20000000;
        } else if (value == 0) {
            m->cpu.status &= ~(0x20000000);
        }
        break;

    /* context switching */
    case REG_SYSTEM_USP: m->cpu.usp = (m->cpu.usp & 0x00ffffff) | ((u32)value << 24); break;
    case REG_SYSTEM_USP + 1: m->cpu.usp = (m->cpu.usp & 0xff00ffff) | ((u32)value << 16); break;
    case REG_SYSTEM_USP + 2: m->cpu.usp = (m->cpu.usp & 0xffff00ff) | ((u32)value << 8); break;
    case REG_SYSTEM_USP + 3: m->cpu.usp = (m->cpu.usp & 0xffffff00) | ((u32)value); break;

    case REG_SYSTEM_INTERRUPT: break;
    case REG_SYTEM_FAULT_CAUSE: break;
    default: 
        if (addr >= REG_SYSTEM_WIN_BUFF && addr < REG_SYSTEM_WIN_BUFF + (32 * 4)) {
            m->sys.winBuff[addr - REG_SYSTEM_WIN_BUFF]=value;
        }
        break;
    }
}
