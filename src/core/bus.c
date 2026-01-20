#include "bus.h"
#include "core/cpu.h"
#include "talea.h"

/* DATA MEMORY FUNCTIONS --------------------------------- */

u8 Machine_ReadData8(TaleaMachine *m, u16 addr)
{
    u8 value = 0;

    u8  port;
    u16 device;

    port   = addr & 0x0f;
    device = addr & 0xfff0;

    switch (device) {
    case DEV_TTY_BASE: value = Terminal_ReadHandler(m, addr); break;
    case DEV_VIDEO_BASE: value = Video_ReadHandler(m, addr); break;
    case DEV_TPS_BASE: value = Storage_ReadHandler(m, addr); break;
    case DEV_MOUSE_BASE: value = (m->data_memory[addr]); break;
    case DEV_AUDIO_BASE: value = Synth_ReadHandler(m, addr); break;
    case DEV_SYSTEM_BASE: value = System_ReadHandler(m, addr); break;
    default: value = (m->data_memory[addr]);
    }

    READ_LOG("data", 2);
    return value;
}
void Machine_WriteData8(TaleaMachine *m, u16 addr, u8 value)
{
    WRITE_LOG("data", 2);

    u8  port   = addr & 0x0f;
    u16 device = addr & 0xfff0;

    switch (device) {
    case DEV_TTY_BASE: Terminal_WriteHandler(m, addr, value); break;
    case DEV_VIDEO_BASE: Video_WriteHandler(m, addr, value); break;
    case DEV_TPS_BASE: Storage_WriteHandler(m, addr, value); break;
    case DEV_MOUSE_BASE: m->data_memory[addr] = value; break;
    case DEV_AUDIO_BASE: Synth_WriteHandler(m, addr, value); break;
    case DEV_SYSTEM_BASE: System_WriteHandler(m, addr, value); break;
    default: m->data_memory[addr] = value;
    }
}

u16 Machine_ReadData16(TaleaMachine *m, u16 addr)
{
    u16 value = 0;

    u16 device = addr & 0xfff0;
    if ((((addr + 1) & 0xf0) != device) && (addr < 0x100)) {
        TALEA_LOG_ERROR("Error reading 16 bits from data memory: misaligned read\n");
        // FIXME: logging, please
        return (0xdead);
    }

    value = ((u16)Machine_ReadData8(m, addr) << 8 | Machine_ReadData8(m, addr + 1));

    READ_LOG("data", 4);
    return value;
}

void Machine_WriteData16(TaleaMachine *m, u16 addr, u16 value)
{
    WRITE_LOG("data", 4);

    u16 device = addr & 0xfff0;
    if ((((addr + 2) & 0xf0) != device) && (addr < 0x100)) {
        TALEA_LOG_ERROR("Error writing 16 bits to data memory: misaligned write");
        return;
    }

    Machine_WriteData8(m, addr, (value & 0xFF00) >> 8);
    Machine_WriteData8(m, addr + 1, value & 0xFF);
}

u32 Machine_ReadData32(TaleaMachine *m, u16 addr)
{
    u32 value  = 0;
    u16 device = addr & 0xfff0;
    if ((((addr + 3) & 0xf0) != device) && (addr < 0x100)) {
        TALEA_LOG_ERROR("Error reading 32 bits from data memory: misaligned read\n");
        // FIXME: logging, please
        return (0xdeadbeef);
    }

    value = ((u32)Machine_ReadData8(m, addr) << 24 | (u32)Machine_ReadData8(m, addr + 1) << 16 |
             (u32)Machine_ReadData8(m, addr + 2) << 8 | Machine_ReadData8(m, addr + 3));

    READ_LOG("data", 8);
    return value;
}
void Machine_WriteData32(TaleaMachine *m, u16 addr, u32 value)
{
    WRITE_LOG("data", 8);

    u16 device = addr & 0xfff0;
    if ((((addr + 3) & 0xf0) != device) && (addr < 0x100)) {
        TALEA_LOG_ERROR("Error writing 32 bits to data memory: misaligned write");
        return;
    }

    Machine_WriteData8(m, addr, (value & 0xFF000000) >> 24);
    Machine_WriteData8(m, addr + 1, (value & 0xFF0000) >> 16);
    Machine_WriteData8(m, addr + 2, (value & 0xFF00) >> 8);
    Machine_WriteData8(m, addr + 3, value & 0xFF);
}

void Bus_RegisterDevices(TaleaMachine *m, const int *id_array, u8 start_index,
                         u8 end_index)
{
    TALEA_LOG_TRACE("Registering %d devices:\n", end_index - start_index);
    for (size_t i = start_index; i < (end_index + 1); i++) {
        m->data_memory[DEV_MAP_BASE + i] = id_array[i];
        TALEA_LOG_TRACE("Registered device 0x%x: 0x%02x (%c == %c)\n", i, m->data_memory[DEV_MAP_BASE + i], m->data_memory[DEV_MAP_BASE + i], id_array[i]);
    }

    TALEA_LOG_TRACE("DEVICE MAP REGISTRY\n");
    for (size_t i = start_index; i < (end_index + 1); i++) {
        TALEA_LOG_TRACE("0x%04x: 0x%02x (%c)\n", DEV_MAP_BASE + i, m->data_memory[DEV_MAP_BASE + i],
                        m->data_memory[DEV_MAP_BASE + i]);
    }
}

#if TALEA_WITH_MMU

u8 Machine_ReadMain8(TaleaMachine *m, u32 addr)
{
    u32 paddr = addr  & 0x00FFFFFFU;

    if (SR_GET_MMU(m->cpu.status)) {
        paddr = MMU_TranslateAddr(m, addr, ACCESS_READ);
        if (m->cpu.exception != EXCEPTION_NONE) return 0;
    }

    u8 value = m->main_memory[paddr];
    READ_LOG("main", 2);
    return value;
}

u16 Machine_ReadMain16(TaleaMachine *m, u32 addr)
{
    u32 paddr = addr;

    if (SR_GET_MMU(m->cpu.status)) {
        if ((addr & 0xFFF) > (4096 - 2)) {
            u8 h = Machine_ReadMain8(m, addr);
            u8 l = Machine_ReadMain8(m, addr + 1);
            return (u16)h << 8 | l;
        } else {
            paddr = MMU_TranslateAddr(m, addr, ACCESS_READ);
            if (m->cpu.exception != EXCEPTION_NONE) return 0;
        }
    }

    // clang-format off
    u16 value = ((u16)m->main_memory[paddr & 0x00FFFFFFU] << 8 | 
                m->main_memory[(paddr + 1) & 0x00FFFFFFU]);
    // clang-format on
    READ_LOG("main", 4);
    return value;
}

u32 Machine_ReadMain32(TaleaMachine *m, u32 addr)
{
    u32 paddr = addr & 0x00FFFFFFU;

    if (SR_GET_MMU(m->cpu.status)) {
        if ((addr & 0xFFF) > (4096 - 4)) {
            u8 h  = Machine_ReadMain8(m, addr);
            u8 hm = Machine_ReadMain8(m, addr + 1);
            u8 lm = Machine_ReadMain8(m, addr + 2);
            u8 l  = Machine_ReadMain8(m, addr + 3);
            return (u32)h << 24 | (u32)hm << 16 | (u32)lm << 8 | l;
        } else {
            paddr = MMU_TranslateAddr(m, addr, ACCESS_READ);
            if (m->cpu.exception != EXCEPTION_NONE) return 0;
        }
    }

    u32 value = ((u32)m->main_memory[paddr] << 24 | (u32)m->main_memory[(paddr + 1) & 0x00FFFFFFU] << 16 |
                 (u32)m->main_memory[(paddr + 2 )  & 0x00FFFFFFU] << 8 | m->main_memory[(paddr + 3) & 0x00FFFFFFU]);
    READ_LOG("main", 8);
    return value;
}

void Machine_WriteMain8(TaleaMachine *m, u32 addr, u8 value)
{
    u32 paddr = addr & 0xffffff;

    if (SR_GET_MMU(m->cpu.status)) {
        paddr = MMU_TranslateAddr(m, addr, ACCESS_WRITE);
        if (m->cpu.exception != EXCEPTION_NONE) return;
    }

    m->main_memory[paddr] = value;
    WRITE_LOG("main", 2);
}

void Machine_WriteMain16(TaleaMachine *m, u32 addr, u16 value)
{
    u32 paddr = addr & 0xffffff;

    if (SR_GET_MMU(m->cpu.status)) {
        if ((addr & 0xFFF) > (4096 - 2)) {
            // TODO: undo partial write if it happens
            Machine_WriteMain8(m, addr, value >> 8);
            Machine_WriteMain8(m, addr + 1, value);
        } else {
            paddr = MMU_TranslateAddr(m, addr, ACCESS_WRITE);
            if (m->cpu.exception != EXCEPTION_NONE) return;
        }
    }

    m->main_memory[paddr]     = (value & 0xFF00) >> 8;
    m->main_memory[(paddr + 1)  & 0x00FFFFFFU] = value & 0xFF;
    WRITE_LOG("main", 4);
}

void Machine_WriteMain32(TaleaMachine *m, u32 addr, u32 value)
{
    u32 paddr = addr & 0xffffff;

    if (SR_GET_MMU(m->cpu.status)) {
        if ((addr & 0xFFF) > (4096 - 4)) {
            // TODO: undo partial write if it happens
            Machine_WriteMain8(m, addr, value >> 24);
            Machine_WriteMain8(m, addr + 1, value >> 16);
            Machine_WriteMain8(m, addr + 2, value >> 8);
            Machine_WriteMain8(m, addr + 3, value);
        } else {
            paddr = MMU_TranslateAddr(m, addr, ACCESS_WRITE);
            if (m->cpu.exception != EXCEPTION_NONE) return;
        }
    }

    m->main_memory[paddr]     = (value & 0xFF000000) >> 24;
    m->main_memory[(paddr + 1)  & 0x00FFFFFFU] = (value & 0xFF0000) >> 16;
    m->main_memory[(paddr + 2)  & 0x00FFFFFFU] = (value & 0xFF00) >> 8;
    m->main_memory[(paddr + 3)  & 0x00FFFFFFU] = value & 0xFF;
    WRITE_LOG("main", 8);
}
#endif
