#ifndef BUS_H
#define BUS_H

#include "talea.h"

// Unified memory access

// Opaque bus to access any memory
typedef struct TaleaBus TaleaBus;

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

u8   Machine_ReadMain8(TaleaMachine *m, u32 addr);
void Machine_WriteMain8(TaleaMachine *m, u32 addr, u8 val);

u16  Machine_ReadMain16(TaleaMachine *m, u32 addr);
void Machine_WriteMain16(TaleaMachine *m, u32 addr, u16 val);

u32  Machine_ReadMain32(TaleaMachine *m, u32 addr);
u32  Machine_ReadMain32Physical(TaleaMachine *m, u32 paddr);
void Machine_WriteMain32(TaleaMachine *m, u32 addr, u32 val);
void Machine_WriteMain32Physical(TaleaMachine *m, u32 paddr, u32 value);

void Bus_Reset(TaleaMachine *m);
void Bus_LoadFirmware(TaleaMachine *m, u8 *firmware, size_t firmware_size);

TaleaMemoryView Bus_GetView(TaleaMachine *m, u32 addr, size_t len, enum MemoryViewAccess);
void           *Bus_GetDataPointer(TaleaMachine *m, u16 addr, size_t len);

size_t Bus_Copy(TaleaMachine *m, TaleaMemoryView *view_src, TaleaMemoryView *view_dest, size_t len);
size_t Bus_ReadBlock(TaleaMachine *m, TaleaMemoryView *view_src, void *restrict buff_dest,
                     size_t len);
size_t Bus_WriteBlock(TaleaMachine *m, void *restrict buff_src, TaleaMemoryView *view_dest,
                      size_t len);
size_t Bus_Memset(TaleaMachine *m, TaleaMemoryView *view_dest, u8 pattern, size_t len);
size_t Bus_Memset16(TaleaMachine *m, TaleaMemoryView *view_dest, u16 pattern, size_t count);
size_t Bus_Memset32(TaleaMachine *m, TaleaMemoryView *view_dest, u32 pattern, size_t count);

#if TALEA_WITH_MMU

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

#endif

#endif /* BUS_H */
