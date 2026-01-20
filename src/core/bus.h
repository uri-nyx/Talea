#ifndef BUS_H
#define BUS_H

#include "talea.h"

// Unified memory access

#ifdef DEBUG_LOG_MEMORY_ACCESS
// TODO: use Raylib's logger
#define WRITE_LOG(mem, dat) \
    TALEA_LOG_TRACE("[WRITE LOG]: value 0x%0" #dat "x written to 0x%08x (%s)\n", value, addr, mem)
#define READ_LOG(mem, dat) \
    TALEA_LOG_TRACE("[READ LOG]: value 0x%0" #dat "x read from 0x%08x (%s)\n", value, addr, mem)
#else
#define WRITE_LOG(mem, dat)
#define READ_LOG(mem, dat)
#endif

u8   Machine_ReadData8(TaleaMachine *m, u16 addr);
void Machine_WriteData8(TaleaMachine *m, u16 addr, u8 val);

u16  Machine_ReadData16(TaleaMachine *m, u16 addr);
void Machine_WriteData16(TaleaMachine *m, u16 addr, u16 val);

u32  Machine_ReadData32(TaleaMachine *m, u16 addr);
void Machine_WriteData32(TaleaMachine *m, u16 addr, u32 val);

#if TALEA_WITH_MMU

u8   Machine_ReadMain8(TaleaMachine *m, u32 addr);
void Machine_WriteMain8(TaleaMachine *m, u32 addr, u8 val);

u16  Machine_ReadMain16(TaleaMachine *m, u32 addr);
void Machine_WriteMain16(TaleaMachine *m, u32 addr, u16 val);

u32  Machine_ReadMain32(TaleaMachine *m, u32 addr);
void Machine_WriteMain32(TaleaMachine *m, u32 addr, u32 val);

#define ON_FAULT_RETURN \
    if (cpu->exception != EXCEPTION_NONE) return;

#define ON_FAULT_RETURN0 \
    if (cpu->exception != EXCEPTION_NONE) return 0;

#define ON_FAULT_RETURN_M \
    if (m->cpu.exception != EXCEPTION_NONE) return;

#define ON_FAULT_RETURN0_M \
    if (m->cpu.exception != EXCEPTION_NONE) return 0;

#else
// declare inline if compiling without mmu to see if it does something

#define ON_FAULT_RETURN
#define ON_FAULT_RETURN0

#define ON_FAULT_RETURN_M
#define ON_FAULT_RETURN0_M

static inline u8 Machine_ReadMain8(TaleaMachine *m, u32 addr)
{
    u8 value = m->main_memory[addr & 0x00FFFFFFU];
    READ_LOG("main", 2);
    return value;
}
static inline u16 Machine_ReadMain16(TaleaMachine *m, u32 addr)
{
    u16 value =
        ((u16)m->main_memory[addr & 0x00FFFFFFU] << 8 | m->main_memory[(addr + 1) & 0x00FFFFFFU]);
    READ_LOG("main", 4);
    return value;
}
static inline u32 Machine_ReadMain32(TaleaMachine *m, u32 addr)
{
    u32 value = ((u32)m->main_memory[addr & 0x00FFFFFFU] << 24 |
                 (u32)m->main_memory[(addr + 1) & 0x00FFFFFFU] << 16 |
                 (u32)m->main_memory[(addr + 2) & 0x00FFFFFFU] << 8 |
                 m->main_memory[(addr + 3) & 0x00FFFFFFU]);
    READ_LOG("main", 8);
    return value;
}

static inline void Machine_WriteMain8(TaleaMachine *m, u32 addr, u8 value)
{
    addr &= 0xffffff;
    m->main_memory[addr] = value;
    WRITE_LOG("main", 2);
}

static inline void Machine_WriteMain16(TaleaMachine *m, u32 addr, u16 value)
{
    m->main_memory[(addr) & 0xffffff]     = (value & 0xFF00) >> 8;
    m->main_memory[(addr + 1) & 0xffffff] = value & 0xFF;
    WRITE_LOG("main", 4);
}

static inline void Machine_WriteMain32(TaleaMachine *m, u32 addr, u32 value)
{
    m->main_memory[(addr) & 0xffffff]     = (value & 0xFF000000) >> 24;
    m->main_memory[(addr + 1) & 0xffffff] = (value & 0xFF0000) >> 16;
    m->main_memory[(addr + 2) & 0xffffff] = (value & 0xFF00) >> 8;
    m->main_memory[(addr + 3) & 0xffffff] = value & 0xFF;
    WRITE_LOG("main", 8);
}
#endif

#endif /* BUS_H */
