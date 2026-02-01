#include <stdlib.h>
#include <time.h>

#include "bus.h"
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
    static struct tm *timeinfo;
    static time_t     rawtime; // Make this more precise
    static double     uptime;
    static u64        instructionsRetired;

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
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        uptime   = GetTime();
        if (m->sys.unixtimeMode) {
            return rawtime >> 24;
        } else if (m->sys.calendarMode) {
            return timeinfo->tm_year;
        } else if (m->sys.microsMode) {
            return ((u64)(uptime * 1000000)) >> 56;
        } else {
            return 0xFF;
        }
    case REG_SYSTEM_MONTH:
        if (m->sys.unixtimeMode) {
            return rawtime >> 16;
        } else if (m->sys.calendarMode) {
            return timeinfo->tm_mon;
        } else if (m->sys.microsMode) {
            return ((u64)(uptime * 1000000)) >> 48;
        } else {
            return 0xFF;
        }
    case REG_SYSTEM_DAY:
        if (m->sys.unixtimeMode) {
            return rawtime >> 8;
        } else if (m->sys.calendarMode) {
            return timeinfo->tm_mday;
        } else if (m->sys.microsMode) {
            return ((u64)(uptime * 1000000)) >> 40;
        } else {
            return 0xFF;
        }
    case REG_SYSTEM_HOUR:
        if (m->sys.unixtimeMode) {
            return rawtime;
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
    case REG_SYSTEM_EXCEPTION: return m->cpu.exception;

    case REG_SYSTEM_FAULT_ADDR: return (m->cpu.faultAddr >> 24);
    case REG_SYSTEM_FAULT_ADDR + 1: return (m->cpu.faultAddr >> 16);
    case REG_SYSTEM_FAULT_ADDR + 2: return (m->cpu.faultAddr >> 8);
    case REG_SYSTEM_FAULT_ADDR + 3: return (m->cpu.faultAddr);

    /* CPU State */
    case REG_SYSTEM_CYCLES_INSTRET: {
        instructionsRetired = m->cpu.instructionsRetired;
        return (instructionsRetired >> 56);
    }
    case REG_SYSTEM_CYCLES_INSTRET + 1: return (instructionsRetired >> 48);
    case REG_SYSTEM_CYCLES_INSTRET + 2: return (instructionsRetired >> 40);
    case REG_SYSTEM_CYCLES_INSTRET + 3: return (instructionsRetired >> 32);
    case REG_SYSTEM_CYCLES_INSTRET + 4: return (instructionsRetired >> 24);
    case REG_SYSTEM_CYCLES_INSTRET + 5: return (instructionsRetired >> 16);
    case REG_SYSTEM_CYCLES_INSTRET + 6: return (instructionsRetired >> 8);
    case REG_SYSTEM_CYCLES_INSTRET + 7: return (instructionsRetired);

    /* Context Windowing */
    case REG_SYSTEM_CWP: return m->cpu.cwp;
    case REG_SYSTEM_WIN_SPILLED: return m->cpu.spilledWindows;
    case REG_SYSTEM_WIN_SEL: return m->sys.winSel;
    case REG_SYSTEM_WIN_OP: return m->sys.winOp;

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
    case REG_SYSTEM_CYCLES_INSTRET + 1:
    case REG_SYSTEM_CYCLES_INSTRET + 2:
    case REG_SYSTEM_CYCLES_INSTRET + 3:
    case REG_SYSTEM_CYCLES_INSTRET + 4:
    case REG_SYSTEM_CYCLES_INSTRET + 5:
    case REG_SYSTEM_CYCLES_INSTRET + 6:
    case REG_SYSTEM_CYCLES_INSTRET + 7: break;

    /* Context Windowing */
    case REG_SYSTEM_CWP: break;
    case REG_SYSTEM_WIN_SEL: m->sys.winSel = value & 0x3;
    case REG_SYSTEM_WIN_OP:
        if (value == TALEA_SYSTEM_WIN_OP_LOAD) {
            for (size_t i = 0; i < 32; i++) {
                u32 reg = 0;
                reg     = (u32)m->sys.winBuff[(i * 4)] << 24;
                reg |= (u32)m->sys.winBuff[(i * 4) + 1] << 16;
                reg |= m->sys.winBuff[(i * 4) + 2] << 8;
                reg |= m->sys.winBuff[(i * 4) + 3];

                reg = i ? reg : 0;

                if (m->sys.winSel < m->cpu.spilledWindows)
                    Machine_WriteData32(
                        m, TALEA_DATA_FIRMWARE_RES + (m->sys.winSel * 32 * 4) + (i*4), reg);
                else
                    m->cpu.gpr[(m->sys.winSel * 32) + i] = reg;
            }
        } else if (value == TALEA_SYSTEM_WIN_OP_STORE) {
            for (size_t i = 0; i < 32; i++) {
                u32 reg;

                if (m->sys.winSel < m->cpu.spilledWindows)
                    reg = Machine_ReadData32(m, TALEA_DATA_FIRMWARE_RES +
                                                    (m->sys.winSel * 32 * 4) + (i*4));
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

    default: break;
    }
}
