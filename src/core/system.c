#include <time.h>

#include "bus.h"
#include "talea.h"

#define P_SYSTEM_MEMSIZE_FLASH (DEV_SYSTEM_BASE + 0)
#define P_SYSTEM_CLOCK         (DEV_SYSTEM_BASE + 1)
#define P_SYSTEM_INT           (DEV_SYSTEM_BASE + 2)
#define P_SYSTEM_POWER         (DEV_SYSTEM_BASE + 3)
#define P_SYSTEM_YEAR          (DEV_SYSTEM_BASE + 4)
#define P_SYSTEM_MONTH         (DEV_SYSTEM_BASE + 5)
#define P_SYSTEM_DAY           (DEV_SYSTEM_BASE + 6)
#define P_SYSTEM_HOUR          (DEV_SYSTEM_BASE + 7)
#define P_SYSTEM_MINUTE        (DEV_SYSTEM_BASE + 8)
#define P_SYSTEM_SECOND        (DEV_SYSTEM_BASE + 9)
#define P_SYSTEM_MILLIS        (DEV_SYSTEM_BASE + 10)
#define P_SYSTEM_COUNTER       (DEV_SYSTEM_BASE + 11)
#define P_SYSTEM_DEVICENUM     (DEV_SYSTEM_BASE + 12)
#define P_SYSTEM_ARCHID        (DEV_SYSTEM_BASE + 13)
#define P_SYSTEM_VENDORID      (DEV_SYSTEM_BASE + 14)

static inline u8 freq_to_byte(float target_mhz)
{
    TALEA_LOG_TRACE("%f MHZn", target_mhz);

    /* Quantization for the frequency byte */
    if (target_mhz < 10.0f) target_mhz = 10.0f;
    if (target_mhz > 100.0f) target_mhz = 100.0f;

    float byte_val = ((target_mhz - 10.0f) / 90.0f) * 255.0f;

    TALEA_LOG_TRACE("%f MHZ -> %d\n", target_mhz, (u8)(byte_val + 0.5f));
    // Add 0.5 for proper rounding before casting to int
    return (u8)(byte_val + 0.5f);
}

u8 System_ReadHandler(TaleaMachine *m, u16 addr)
{
    struct tm *timeinfo;

    time_t rawtime; // Make this more precise
    time(&rawtime);
    timeinfo      = localtime(&rawtime);
    m->sys.uptime = GetTime() * 1000; // to ms

    switch (addr) {
    case P_SYSTEM_MEMSIZE_FLASH:
        return (TALEA_MEM_SZ_MB);
    case P_SYSTEM_CLOCK:
        return (freq_to_byte(m->cpu.frequency / 1000000));
        // TODO: Document this. Get freequency back by 10000 + (reg_val * 353)
    case P_SYSTEM_INT:
        return (m->cpu.exception);
    case P_SYSTEM_POWER:
        break;
    case P_SYSTEM_YEAR:
        if (m->sys.unixtime_mode)
            return (rawtime >> 24);
        else
            return (timeinfo->tm_year);
    case P_SYSTEM_MONTH:
        if (m->sys.unixtime_mode)
            return (rawtime >> 16);
        else
            return (timeinfo->tm_mon);
    case P_SYSTEM_DAY:
        if (m->sys.unixtime_mode)
            return (rawtime >> 8);
        else
            return (timeinfo->tm_mday);
    case P_SYSTEM_HOUR:
        if (m->sys.unixtime_mode)
            return (rawtime);
        else
            return (timeinfo->tm_hour);
    case P_SYSTEM_MINUTE:
        if (m->sys.counter_mode)
            return (m->sys.uptime >> 24);
        else
            return (timeinfo->tm_min);
    case P_SYSTEM_SECOND:
        if (m->sys.counter_mode)
            return (m->sys.uptime >> 16);
        else
            return (timeinfo->tm_sec);
    case P_SYSTEM_MILLIS:
        if (m->sys.counter_mode)
            return (m->sys.uptime >> 8);
        else
            return (rawtime >> 8);
    case P_SYSTEM_COUNTER:
        if (m->sys.counter_mode) {
            m->sys.counter_mode = false;
            return (m->sys.uptime);
        } else {
            return (rawtime & 0xff);
        }
    case P_SYSTEM_DEVICENUM:
        return (TALEA_NUM_INSTALLED_DEVICES);
    case P_SYSTEM_ARCHID:
        return (ARCH_ID);
    case P_SYSTEM_VENDORID:
        return (VENDOR_ID);
    default:
        return (0);
    }
    return (0);
}

void System_WriteHandler(TaleaMachine *m, u16 addr, u8 value)
{
    FILE       *fp;
    static bool save_block_to_firmware_armed = false;
    static bool poweroff_sequence_armed      = false;

    switch (addr) {
    case P_SYSTEM_MEMSIZE_FLASH:
        // TODO: Document. Writing to this register saves a
        // block of 512 bytes from memory to the firmware file.
        // The block sits at 0x200
        if (value == TALEA_MAGIC_ARM_SEQUENCE &&
            !save_block_to_firmware_armed) {
            save_block_to_firmware_armed = true;
        } else if (value == TALEA_MAGIC_TRIGGER_SEQUENCE &&
                   save_block_to_firmware_armed) {
            TALEA_LOG_WARNING(
                "Should save block at 0x200 to firmware file!\n"); // TODO:
                                                                   // implement
                                                                   // this
            save_block_to_firmware_armed = false;
        } else {
            save_block_to_firmware_armed = false;
        }
        break;
    case P_SYSTEM_CLOCK:
    case P_SYSTEM_INT:
        // NON WRITABLE REGISTER
        break;
    case P_SYSTEM_POWER:
        if (value == TALEA_MAGIC_ARM_SEQUENCE && !poweroff_sequence_armed) {
            poweroff_sequence_armed = true;
        } else if (value == TALEA_MAGIC_TRIGGER_SEQUENCE &&
                   poweroff_sequence_armed) {
            TALEA_LOG_TRACE("Powering off!\n"); // TODO: implement this
            Machine_Poweroff(m);
            poweroff_sequence_armed = false;
        } else {
            poweroff_sequence_armed = false;
        }
        break;
    case P_SYSTEM_YEAR:
        m->sys.unixtime_mode = !m->sys.unixtime_mode;
        break;
    case P_SYSTEM_MONTH:
    case P_SYSTEM_DAY:
    case P_SYSTEM_HOUR:
        break;
    case P_SYSTEM_MINUTE:
        m->sys.counter_mode = true;
        break;
    case P_SYSTEM_SECOND:
    case P_SYSTEM_MILLIS:
    case P_SYSTEM_COUNTER:
    case P_SYSTEM_DEVICENUM:
    case P_SYSTEM_ARCHID:
    case P_SYSTEM_VENDORID:
    default:
        break;
    }
}
