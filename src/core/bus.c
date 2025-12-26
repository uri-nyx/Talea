#include "bus.h"
#include "talea.h"

#ifdef DEBUG_LOG_MEMORY_ACCESS
// TODO: use Raylib's logger
#define WRITE_LOG(mem, dat)                        \
    TALEA_LOG_TRACE("[WRITE LOG]: value 0x%0" #dat \
                    "x written to 0x%08x (%s)\n",  \
                    value, addr, mem)
#define READ_LOG(mem, dat)                                                     \
    TALEA_LOG_TRACE("[READ LOG]: value 0x%0" #dat "x read from 0x%08x (%s)\n", \
                    value, addr, mem)
#else
#define WRITE_LOG(mem, dat)
#define READ_LOG(mem, dat)
#endif

/* MAIN MEMORY FUNCTIONS --------------------------------- */

// Reads

u8 Machine_ReadMain8(TaleaMachine *m, u32 addr)
{
    u8 value = m->main_memory[addr & 0x00FFFFFFU];
    READ_LOG("main", 2);
    return value;
}
u16 Machine_ReadMain16(TaleaMachine *m, u32 addr)
{
    u16 value = ((u16)m->main_memory[addr & 0x00FFFFFFU] << 8 |
                 m->main_memory[(addr + 1) & 0x00FFFFFFU]);
    READ_LOG("main", 4);
    return value;
}
u32 Machine_ReadMain32(TaleaMachine *m, u32 addr)
{
    u32 value = ((u32)m->main_memory[addr & 0x00FFFFFFU] << 24 |
                 (u32)m->main_memory[(addr + 1) & 0x00FFFFFFU] << 16 |
                 (u32)m->main_memory[(addr + 2) & 0x00FFFFFFU] << 8 |
                 m->main_memory[(addr + 3) & 0x00FFFFFFU]);
    READ_LOG("main", 8);
    return value;
}

// Writes

void Machine_WriteMain8(TaleaMachine *m, u32 addr, u8 value)
{
    addr &= 0xffffff;
    m->main_memory[addr] = value;
    WRITE_LOG("main", 2);
}

void Machine_WriteMain16(TaleaMachine *m, u32 addr, u16 value)
{
    m->main_memory[(addr) & 0xffffff]     = (value & 0xFF00) >> 8;
    m->main_memory[(addr + 1) & 0xffffff] = value & 0xFF;
    WRITE_LOG("main", 4);
}

void Machine_WriteMain32(TaleaMachine *m, u32 addr, u32 value)
{
    m->main_memory[(addr) & 0xffffff]     = (value & 0xFF000000) >> 24;
    m->main_memory[(addr + 1) & 0xffffff] = (value & 0xFF0000) >> 16;
    m->main_memory[(addr + 2) & 0xffffff] = (value & 0xFF00) >> 8;
    m->main_memory[(addr + 3) & 0xffffff] = value & 0xFF;
    WRITE_LOG("main", 8);
}

/* DATA MEMORY FUNCTIONS --------------------------------- */

u8 Machine_ReadData8(TaleaMachine *m, u16 addr)
{
    u8 value = 0;

    u8  port;
    u16 device;

    port   = addr & 0x0f;
    device = addr & 0xfff0;

    switch (device) {
    case DEV_TTY_BASE:
        value = Terminal_ReadHandler(m, addr);
        break;
    case DEV_VIDEO_BASE:
        value = Video_ReadHandler(m, addr);
        break;
    case DEV_TPS_BASE:
        value = Storage_ReadHandler(m, addr);
        break;
    case DEV_MOUSE_BASE:
        value = (m->data_memory[addr]);
        break;
    case DEV_AUDIO_BASE:
    case DEV_AUDIO_BASE + 0x10:
    case DEV_AUDIO_BASE + 0x20:
    case DEV_AUDIO_BASE + 0x30:
        value = 0;
        break; //(AudioHandlerReadU8(addr));
    case DEV_SYSTEM_BASE:
        value = System_ReadHandler(m, addr);
        break;
    default:
        value = (m->data_memory[addr]);
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
    case DEV_TTY_BASE:
        Terminal_WriteHandler(m, addr, value);
        break;
    case DEV_VIDEO_BASE:
        Video_WriteHandler(m, addr, value);
        break;
    case DEV_TPS_BASE:
        Storage_WriteHandler(m, addr, value);
        break;
    case DEV_MOUSE_BASE:
        m->data_memory[addr] = value;
        break;
    case DEV_AUDIO_BASE:
    case DEV_AUDIO_BASE + 0x10:
    case DEV_AUDIO_BASE + 0x20:
    case DEV_AUDIO_BASE + 0x30:
        // AudioHandlerWriteU8(addr, value);
        break;
    case DEV_SYSTEM_BASE:
        System_WriteHandler(m, addr, value);
        break;
    default:
        m->data_memory[addr] = value;
    }
}

u16 Machine_ReadData16(TaleaMachine *m, u16 addr)
{
    u16 value = 0;

    u16 device = addr & 0xfff0;
    if ((((addr + 1) & 0xf0) != device) && (addr < 0x100)) {
        TALEA_LOG_ERROR(
            "Error reading 16 bits from data memory: misaligned read\n");
        // FIXME: logging, please
        return (0xdead);
    }

    value =
        ((u16)Machine_ReadData8(m, addr) << 8 | Machine_ReadData8(m, addr + 1));

    READ_LOG("data", 4);
    return value;
}

void Machine_WriteData16(TaleaMachine *m, u16 addr, u16 value)
{
    WRITE_LOG("data", 4);

    u16 device = addr & 0xfff0;
    if ((((addr + 2) & 0xf0) != device) && (addr < 0x100)) {
        TALEA_LOG_ERROR(
            "Error writing 16 bits to data memory: misaligned write");
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
        TALEA_LOG_ERROR(
            "Error reading 32 bits from data memory: misaligned read\n");
        // FIXME: logging, please
        return (0xdeadbeef);
    }

    value = ((u32)Machine_ReadData8(m, addr) << 24 |
             (u32)Machine_ReadData8(m, addr + 1) << 16 |
             (u32)Machine_ReadData8(m, addr + 2) << 8 |
             Machine_ReadData8(m, addr + 3));

    READ_LOG("data", 8);
    return value;
}
void Machine_WriteData32(TaleaMachine *m, u16 addr, u32 value)
{
    WRITE_LOG("data", 8);

    u16 device = addr & 0xfff0;
    if ((((addr + 3) & 0xf0) != device) && (addr < 0x100)) {
        TALEA_LOG_ERROR(
            "Error writing 32 bits to data memory: misaligned write");
        return;
    }

    Machine_WriteData8(m, addr, (value & 0xFF000000) >> 24);
    Machine_WriteData8(m, addr + 1, (value & 0xFF0000) >> 16);
    Machine_WriteData8(m, addr + 2, (value & 0xFF00) >> 8);
    Machine_WriteData8(m, addr + 3, value & 0xFF);
}

void Bus_RegisterDevices(TaleaMachine *m, const int *id_array,
                         unsigned int start_index, unsigned int end_index)
{
    for (size_t i = start_index; i < (end_index + 1); i++) {
        m->data_memory[DEV_MAP_BASE + i] = id_array[i];
    }
}
