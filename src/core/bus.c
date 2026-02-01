#include "bus.h"
#include "core/cpu.h"
#include "machine_description.h"
#include "talea.h"
#include "types.h"

#define BUS_MAIN_POINTER_MASK 0xFFFFFFU // 24 bit pointer
#define BUS_DATA_POINTER_MASK 0xFFFFU   // 16 bit pointer

#define MAIN_BUS_ACCESS(addr) bus->main_memory[(addr) & BUS_MAIN_POINTER_MASK]
#define DATA_BUS_ACCESS(addr) bus->data_memory[(addr) & BUS_DATA_POINTER_MASK]
#define ROM_ACCESS(addr)      bus->rom[(addr - bus->rom_start) & BUS_DATA_POINTER_MASK]

#define BUS_IO_DEV_MASK  0xFFF0
#define BUS_IO_PORT_MASK 0xF

typedef struct TaleaBus {
    u8     main_memory[TALEA_MAIN_MEM_SZ];
    u8     rom[TALEA_ROM_SIZE];
    u8     data_memory[TALEA_DATA_MEM_SZ];
    size_t mainsz;
    size_t main_end;
    size_t datasz;
    size_t romsz;
    size_t rom_start;
    size_t rom_end;
} TaleaBus;

static TaleaBus MachineBus = { 0 };

void Bus_Reset(TaleaMachine *m)
{
    m->bus = &MachineBus;

    memset(MachineBus.main_memory, 0, sizeof(MachineBus.main_memory));
    memset(MachineBus.data_memory, 0, sizeof(MachineBus.data_memory));
    memset(MachineBus.rom, 0, sizeof(MachineBus.rom));

    MachineBus.mainsz = sizeof(MachineBus.main_memory);
    MachineBus.datasz = sizeof(MachineBus.data_memory);
    MachineBus.romsz  = sizeof(MachineBus.rom);

    // This is a bit redundant but its clearer
    MachineBus.main_end  = MachineBus.mainsz;
    MachineBus.rom_start = TALEA_FIRMWARE_ADDRESS; // This is purposefully set at build time
    MachineBus.rom_end   = MachineBus.rom_start + MachineBus.romsz;
}

void Bus_LoadFirmware(TaleaMachine *m, u8 *firmware, size_t firmware_size)
{
    // we're going to parse the bare minimum of the a.out header, just to load the code at
    // FIRMWARE_ADDRESS and the data+bss at FIRMWARE_DATA_ADDDRESS
    // Also, assume firmware is not NULL and everything because this is only called once, by us

    u32 code_size = fromBe32(((u32 *)firmware)[1]); // code size starts at offset 4 in the a.out
                                                    // header
    u32 data_size = fromBe32(((u32 *)firmware)[2]); // data size starts at offset 8 in the a.out
                                                    // header

    // Don't bother zeroing out the bss, its already 0
    u8 *stripped = firmware + 32; // the actual code starts at offset 32 in the a.out header
    memcpy(m->bus->rom, stripped, MIN(code_size, TALEA_ROM_SIZE)); // load code
    memcpy(m->bus->main_memory + TALEA_FIRMWARE_DATA_ADDRESS, stripped + code_size,
           MIN(data_size, TALEA_MAIN_MEM_SZ - TALEA_FIRMWARE_DATA_ADDRESS)); // load data
    // and we're done!
    return;
}

/* DATA MEMORY FUNCTIONS --------------------------------- */

u8 Machine_ReadData8(TaleaMachine *m, u16 addr)
{
    u8 value = 0;

    u8  port;
    u16 device;

    port   = addr & BUS_IO_PORT_MASK;
    device = addr & BUS_IO_DEV_MASK;

    switch (device) {
    case DEV_BASE_TERMINAL: value = Terminal_Read(m, port); break;
    case DEV_BASE_VIDEO: value = Video_Read(m, port); break;
    case DEV_BASE_STORAGE: value = Storage_Read(m, port); break;
    case DEV_BASE_MOUSE: value = Mouse_Read(m, port); break;
    case DEV_BASE_AUDIO: value = Audio_Read(m, port); break;
    case TALEA_DATA_SYSTEM_START: value = System_Read(m, addr); break;
    default: value = (m->bus->data_memory[addr]);
    }

    READ_LOG("data", 2);
    return value;
}
void Machine_WriteData8(TaleaMachine *m, u16 addr, u8 value)
{
    TaleaBus *bus = m->bus;
    WRITE_LOG("data", 2);

    u8  port   = addr & 0x0f;
    u16 device = addr & 0xfff0;

    switch (device) {
    case DEV_BASE_TERMINAL: Terminal_Write(m, port, value); break;
    case DEV_BASE_VIDEO: Video_Write(m, port, value); break;
    case DEV_BASE_STORAGE: Storage_Write(m, port, value); break;
    case DEV_BASE_MOUSE: Mouse_Write(m, port, value); break;
    case DEV_BASE_AUDIO: Audio_Write(m, port, value); break;
    case TALEA_DATA_SYSTEM_START: System_Write(m, addr, value); break;
    default: {
        DATA_BUS_ACCESS(addr) = value;
        break;
    }
    }
}

u16 Machine_ReadData16(TaleaMachine *m, u16 addr)
{
    u16 value = 0;

    u16 device = addr & BUS_IO_DEV_MASK;
    if ((((addr + 1) & 0xf0) != device) && (addr < 0x100)) {
        TALEA_LOG_ERROR("Error reading 16 bits from data memory: misaligned IO read\n");
        // FIXME: logging, please
        return (0xdead);
    }

    value = (u16)Machine_ReadData8(m, addr) << 8;
    value |= Machine_ReadData8(m, addr + 1);

    READ_LOG("data", 4);
    return value;
}

void Machine_WriteData16(TaleaMachine *m, u16 addr, u16 value)
{
    WRITE_LOG("data", 4);

    u16 device = addr & BUS_IO_DEV_MASK;
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
    u16 device = addr & BUS_IO_DEV_MASK;
    if ((((addr + 3) & 0xf0) != device) && (addr < 0x100)) {
        TALEA_LOG_ERROR("Error reading 32 bits from data memory: misaligned read\n");
        // FIXME: logging, please
        return (0xdeadbeef);
    }

    value = (u32)Machine_ReadData8(m, addr) << 24;
    value |= (u32)Machine_ReadData8(m, addr + 1) << 16;
    value |= (u32)Machine_ReadData8(m, addr + 2) << 8; 
    value |= Machine_ReadData8(m, addr + 3);

    READ_LOG("data", 8);
    return value;
}
void Machine_WriteData32(TaleaMachine *m, u16 addr, u32 value)
{
    WRITE_LOG("data", 8);

    u16 device = addr & BUS_IO_DEV_MASK;
    if ((((addr + 3) & 0xf0) != device) && (addr < 0x100)) {
        TALEA_LOG_ERROR("Error writing 32 bits to data memory: misaligned write");
        return;
    }

    Machine_WriteData8(m, addr, (value & 0xFF000000) >> 24);
    Machine_WriteData8(m, addr + 1, (value & 0xFF0000) >> 16);
    Machine_WriteData8(m, addr + 2, (value & 0xFF00) >> 8);
    Machine_WriteData8(m, addr + 3, value & 0xFF);
}

void Bus_RegisterDevices(TaleaMachine *m)
{
    TaleaBus *bus = m->bus;
    TALEA_LOG_TRACE("Registering %d devices:\n", DEV_SLOT_COUNT);
    for (size_t i = 0; i < DEV_SLOT_COUNT; i++) {
        DATA_BUS_ACCESS(TALEA_DATA_DEV_MAP + i) = Talea_DeviceMap[i];
        TALEA_LOG_TRACE("Registered device 0x%x: 0x%02x (%c == %c)\n", i,
                        DATA_BUS_ACCESS(TALEA_DATA_DEV_MAP + i),
                        DATA_BUS_ACCESS(TALEA_DATA_DEV_MAP + i), Talea_DeviceMap[i]);
    }

    TALEA_LOG_TRACE("DEVICE MAP REGISTRY\n");
    for (size_t i = 0; i < DEV_SLOT_COUNT; i++) {
        TALEA_LOG_TRACE("0x%04x: 0x%02x (%c)\n", TALEA_DATA_DEV_MAP + i,
                        DATA_BUS_ACCESS(TALEA_DATA_DEV_MAP + i),
                        DATA_BUS_ACCESS(TALEA_DATA_DEV_MAP + i));
    }
}

#if TALEA_WITH_MMU

u8 Machine_ReadMain8(TaleaMachine *m, u32 addr)
{
    TaleaBus *bus   = m->bus;
    u8        value = 0xFF;

    u32 paddr = addr & BUS_MAIN_POINTER_MASK;

    if (SR_GET_MMU(m->cpu.status)) {
        paddr = MMU_TranslateAddr(m, addr, ACCESS_READ);
        if (m->cpu.exception != EXCEPTION_NONE) return 0;
    }

    if (paddr < bus->main_end) {
        value = MAIN_BUS_ACCESS(paddr);
    } else if (paddr >= bus->rom_start && paddr < bus->rom_end) {
        value = ROM_ACCESS(paddr);
    }
#ifdef TALEA_DEBUG_BUS_ERROR_ON_ILLEGAL_READ
    else {
        // THIS ADDRESS IS NOT MAPPED TO PHYSICAL MEMORY!
        m->cpu.exception = EXCEPTION_BUS_ERROR;
    }
#endif

    READ_LOG("main", 2);
    return value;
}

u16 Machine_ReadMain16(TaleaMachine *m, u32 addr)
{
    TaleaBus *bus   = m->bus;
    u16       value = 0xDEAD; // open bus
    u32       paddr = addr & BUS_MAIN_POINTER_MASK;

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

    if ((paddr + 1) < bus->main_end) {
        value = ((u16)MAIN_BUS_ACCESS(paddr) << 8 | MAIN_BUS_ACCESS(paddr + 1));
    } else if ((paddr + 1) >= bus->rom_start && (paddr + 1) < bus->rom_end) {
        value = ((u16)ROM_ACCESS(paddr) << 8 | ROM_ACCESS(paddr + 1));
    }
#ifdef TALEA_DEBUG_BUS_ERROR_ON_ILLEGAL_READ
    else {
        // THIS ADDRESS IS NOT MAPPED TO PHYSICAL MEMORY!
        m->cpu.exception = EXCEPTION_BUS_ERROR;
    }
#endif

    READ_LOG("main", 4);
    return value;
}

u32 Machine_ReadMain32(TaleaMachine *m, u32 addr)
{
    TaleaBus *bus   = m->bus;
    u32       value = 0xDEADBEEF; // open bus
    u32       paddr = addr & BUS_MAIN_POINTER_MASK;

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

    if ((paddr + 3) < bus->main_end) {
        value = ((u32)MAIN_BUS_ACCESS(paddr) << 24 | (u32)MAIN_BUS_ACCESS(paddr + 1) << 16 |
                 (u32)MAIN_BUS_ACCESS(paddr + 2) << 8 | MAIN_BUS_ACCESS(paddr + 3));
    } else if ((paddr + 3) >= bus->rom_start && (paddr + 3) < bus->rom_end) {
        value = ((u32)ROM_ACCESS(paddr) << 24 | (u32)ROM_ACCESS(paddr + 1) << 16 |
                 (u32)ROM_ACCESS(paddr + 2) << 8 | ROM_ACCESS(paddr + 3));
    }
#ifdef TALEA_DEBUG_BUS_ERROR_ON_ILLEGAL_READ
    else {
        // THIS ADDRESS IS NOT MAPPED TO PHYSICAL MEMORY!
        m->cpu.exception = EXCEPTION_BUS_ERROR;
    }
#endif

    READ_LOG("main", 8);
    return value;
}

u32 Machine_ReadMain32Physical(TaleaMachine *m, u32 paddr)
{
    TaleaBus *bus   = m->bus;
    u32       value = 0xDEADBEEF; // open bus
    paddr           = paddr & BUS_MAIN_POINTER_MASK;

    if ((paddr + 3) < bus->main_end) {
        value = ((u32)MAIN_BUS_ACCESS(paddr) << 24 | (u32)MAIN_BUS_ACCESS(paddr + 1) << 16 |
                 (u32)MAIN_BUS_ACCESS(paddr + 2) << 8 | MAIN_BUS_ACCESS(paddr + 3));
    } else if ((paddr + 3) >= bus->rom_start && (paddr + 3) < bus->rom_end) {
        value = ((u32)ROM_ACCESS(paddr) << 24 | (u32)ROM_ACCESS(paddr + 1) << 16 |
                 (u32)ROM_ACCESS(paddr + 2) << 8 | ROM_ACCESS(paddr + 3));
    }
#ifdef TALEA_DEBUG_BUS_ERROR_ON_ILLEGAL_READ
    else {
        // THIS ADDRESS IS NOT MAPPED TO PHYSICAL MEMORY!
        m->cpu.exception = EXCEPTION_BUS_ERROR;
    }
#endif

    READ_LOG("main", 8);
    return value;
}

void Machine_WriteMain8(TaleaMachine *m, u32 addr, u8 value)
{
    TaleaBus *bus   = m->bus;
    u32       paddr = addr & BUS_MAIN_POINTER_MASK;

    if (SR_GET_MMU(m->cpu.status)) {
        paddr = MMU_TranslateAddr(m, addr, ACCESS_WRITE);
        if (m->cpu.exception != EXCEPTION_NONE) return;
    }

    if (paddr < bus->main_end) {
        MAIN_BUS_ACCESS(paddr) = value;
    }

#ifdef TALEA_DEBUG_BUS_ERROR_ON_ILLEGAL_WRITE
    if (paddr >= bus->rom_start && paddr < bus->rom_end) {
        // TODO: DOCUMENT. Writing to ROM is a nop, but don't raise expecption;
        m->cpu.exception = EXCEPTION_BUS_ERROR;
    } else {
        // THIS ADDRESS IS NOT MAPPED TO PHYSICAL MEMORY!
        m->cpu.exception = EXCEPTION_BUS_ERROR;
    }
#endif

    WRITE_LOG("main", 2);
}

void Machine_WriteMain16(TaleaMachine *m, u32 addr, u16 value)
{
    TaleaBus *bus   = m->bus;
    u32       paddr = addr & BUS_MAIN_POINTER_MASK;

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

    if ((paddr + 1) < bus->main_end) {
        MAIN_BUS_ACCESS(paddr)     = (value & 0xFF00) >> 8;
        MAIN_BUS_ACCESS(paddr + 1) = value & 0xFF;
    }
#ifdef TALEA_DEBUG_BUS_ERROR_ON_ILLEGAL_WRITE
    if ((paddr + 1) >= bus->rom_start && (paddr + 1) < bus->rom_end) {
        m->cpu.exception = EXCEPTION_BUS_ERROR;
    } else {
        // THIS ADDRESS IS NOT MAPPED TO PHYSICAL MEMORY!
        m->cpu.exception = EXCEPTION_BUS_ERROR;
    }
#endif

    WRITE_LOG("main", 4);
}

void Machine_WriteMain32(TaleaMachine *m, u32 addr, u32 value)
{
    TaleaBus *bus   = m->bus;
    u32       paddr = addr & 0xffffff;

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

    if ((paddr + 3) < bus->main_end) {
        MAIN_BUS_ACCESS(paddr)     = (value & 0xFF000000) >> 24;
        MAIN_BUS_ACCESS(paddr + 1) = (value & 0xFF0000) >> 16;
        MAIN_BUS_ACCESS(paddr + 2) = (value & 0xFF00) >> 8;
        MAIN_BUS_ACCESS(paddr + 3) = value & 0xFF;
    }

#ifdef TALEA_DEBUG_BUS_ERROR_ON_ILLEGAL_WRITE
    if ((paddr + 3) >= bus->rom_start && (paddr + 3) < bus->rom_end) {
        m->cpu.exception = EXCEPTION_BUS_ERROR;
    } else {
        // THIS ADDRESS IS NOT MAPPED TO PHYSICAL MEMORY!
        m->cpu.exception = EXCEPTION_BUS_ERROR;
    }
#endif

    WRITE_LOG("main", 8);
}

void Machine_WriteMain32Physical(TaleaMachine *m, u32 paddr, u32 value)
{
    TaleaBus *bus = m->bus;
    paddr         = paddr & 0xffffff;

    if ((paddr + 3) < bus->main_end) {
        MAIN_BUS_ACCESS(paddr)     = (value & 0xFF000000) >> 24;
        MAIN_BUS_ACCESS(paddr + 1) = (value & 0xFF0000) >> 16;
        MAIN_BUS_ACCESS(paddr + 2) = (value & 0xFF00) >> 8;
        MAIN_BUS_ACCESS(paddr + 3) = value & 0xFF;
    }

#ifdef TALEA_DEBUG_BUS_ERROR_ON_ILLEGAL_WRITE
    if ((paddr + 3) >= bus->rom_start && (paddr + 3) < bus->rom_end) {
        m->cpu.exception = EXCEPTION_BUS_ERROR;
    } else {
        // THIS ADDRESS IS NOT MAPPED TO PHYSICAL MEMORY!
        m->cpu.exception = EXCEPTION_BUS_ERROR;
    }
#endif

    WRITE_LOG("main", 8);
}

// DMA Bus controller

void *Bus_GetDataPointer(TaleaMachine *m, u16 addr, size_t len)
{
    size_t start = addr & BUS_DATA_POINTER_MASK;

    if (start < TALEA_DATA_FIRMWARE_RES) return NULL;
    if (start + len >= TALEA_DATA_MEM_SZ) return NULL;

    return &m->bus->data_memory[start];
}

TaleaMemoryView Bus_GetView(TaleaMachine *m, u32 addr, size_t len, enum MemoryViewAccess flags)
{
    TaleaBus       *bus = m->bus;
    TaleaMemoryView view =
        (TaleaMemoryView){ .ptr = NULL, .guest_addr = addr, .length = 0, .access_flags = flags };

    addr = addr & BUS_MAIN_POINTER_MASK;
    /*
        TALEA_LOG_TRACE("Requesting DMA memory view to addr %06x, len %d, flags %d\n", addr, len,
                        flags);

    */
    if (addr >= bus->rom_start && (addr + len) < bus->rom_end && flags == BUS_ACCESS_READ) {
        view.ptr    = &bus->rom[addr - bus->rom_start];
        view.length = len;
        /*
                TALEA_LOG_TRACE("Granting DMA memory view to addr %06x [ROM], len %d, flags %d\n",
           addr, len, flags);
        */
    }

    if ((addr + len) < bus->main_end) {
        view.ptr    = &bus->main_memory[addr];
        view.length = len;
        /*
        TALEA_LOG_TRACE("Granting DMA memory view to addr %06x [RAM], len %d, flags %d\n", addr,
                        len, flags);
                        */
    }

    return view;
}

size_t Bus_Copy(TaleaMachine *m, TaleaMemoryView *view_src, TaleaMemoryView *view_dest, size_t len)
{
    if (!view_src->ptr || view_src->length <= 0) {
        TALEA_LOG_WARNING("Bus DMA copy request denied: invalid source memory view\n");
        return 0;
    }

    if (!view_dest->ptr || view_dest->length <= 0 ||
        !(view_dest->access_flags & BUS_ACCESS_WRITE)) {
        if (!(view_dest->access_flags & BUS_ACCESS_WRITE))
            TALEA_LOG_WARNING("Bus DMA copy request denied: source lacks write access\n");
        else
            TALEA_LOG_WARNING("Bus DMA copy request denied: invalid destinantion memory view\n");
        return 0;
    }

    if (view_dest->guest_addr == view_src->guest_addr) return len;

    size_t bytes_to_copy = MIN(len, MIN(view_src->length, view_dest->length));
    memmove(view_dest->ptr, view_src->ptr, bytes_to_copy);
    return bytes_to_copy;
}

// Reads from Guest to write to Host
// NOTE: the buffer should be at leat of len size
size_t Bus_ReadBlock(TaleaMachine *m, TaleaMemoryView *view_src, void *restrict buff_dest,
                     size_t len)
{
    if (!view_src->ptr || view_src->length <= 0 || !(view_src->access_flags & BUS_ACCESS_READ)) {
        if (!(view_src->access_flags & BUS_ACCESS_READ))
            TALEA_LOG_WARNING("Bus DMA read request denied: source lacks read access\n");
        else
            TALEA_LOG_WARNING("Bus DMA read request denied: invalid destinantion memory view\n");
        return 0;
    }

    size_t bytes_to_copy = MIN(len, view_src->length);
    memcpy(buff_dest, view_src->ptr, bytes_to_copy);
    return bytes_to_copy;
}

// Reads from Host to write to Guest
// NOTE: the buffer should be at leat of len size
size_t Bus_WriteBlock(TaleaMachine *m, void *restrict buff_src, TaleaMemoryView *view_dest,
                      size_t len)
{
    if (!view_dest->ptr || view_dest->length <= 0 ||
        !(view_dest->access_flags & BUS_ACCESS_WRITE)) {
        if (!(view_dest->access_flags & BUS_ACCESS_WRITE))
            TALEA_LOG_WARNING("Bus DMA write request denied: destination lacks write access\n");
        else
            TALEA_LOG_WARNING("Bus DMA write request denied: invalid destinantion memory view\n");
        return 0;
    }

    size_t bytes_to_copy = MIN(len, view_dest->length);
    memcpy(view_dest->ptr, buff_src, bytes_to_copy);
    return bytes_to_copy;
}

size_t Bus_Memset(TaleaMachine *m, TaleaMemoryView *view_dest, u8 pattern, size_t len)
{
    if (!view_dest->ptr || view_dest->length <= 0 ||
        !(view_dest->access_flags & BUS_ACCESS_WRITE)) {
        if (!(view_dest->access_flags & BUS_ACCESS_WRITE))
            TALEA_LOG_WARNING(
                "Bus DMA memset request denied: destination lacks write access [addr: %06x]\n",
                view_dest->guest_addr);
        else
            TALEA_LOG_WARNING("Bus DMA memset request denied: invalid destinantion memory view\n");
        return 0;
    }

    size_t bytes_to_set = MIN(len, view_dest->length);
    memset(view_dest->ptr, pattern, bytes_to_set);
    return bytes_to_set;
}

// NOTE: this is endianness agnostic it just fills the pattern. Be sure to reverse if needed
// NOTE: count is in elements (no of 16 bit patterns) return same
size_t Bus_Memset16(TaleaMachine *m, TaleaMemoryView *view_dest, u16 pattern, size_t count)
{
    if (!view_dest->ptr || view_dest->length <= 0 ||
        !(view_dest->access_flags & BUS_ACCESS_WRITE)) {
        if (!(view_dest->access_flags & BUS_ACCESS_WRITE))
            TALEA_LOG_WARNING("Bus DMA memset16 request denied: destination lacks write access\n");
        else
            TALEA_LOG_WARNING(
                "Bus DMA memset16 request denied: invalid destinantion memory view\n");
        return 0;
    }

    size_t elements_to_set = (MIN(count * 2, view_dest->length)) / 2;
    u16   *raw_ptr         = (u16 *)view_dest->ptr;
    for (size_t i = 0; i < elements_to_set; i++) raw_ptr[i] = pattern;
    return elements_to_set;
}

// NOTE: this is endianness agnostic it just fills the pattern.
// NOTE: count is in elements (no of 16 bit patterns) return same
size_t Bus_Memset32(TaleaMachine *m, TaleaMemoryView *view_dest, u32 pattern, size_t count)
{
    if (!view_dest->ptr || view_dest->length <= 0 ||
        !(view_dest->access_flags & BUS_ACCESS_WRITE)) {
        if (!(view_dest->access_flags & BUS_ACCESS_WRITE))
            TALEA_LOG_WARNING(
                "Bus DMA memset32 request denied: destination lacks write access [addr: %06x]\n",
                view_dest->guest_addr);
        else
            TALEA_LOG_WARNING(
                "Bus DMA memset32 request denied: invalid destinantion memory view\n");
        return 0;
    }

    size_t elements_to_set = (MIN(count * 4, view_dest->length)) / 4;
    if (count != elements_to_set) {
        TALEA_LOG_TRACE(
            "Out of bounds prevented in Memset32, wanted to set %d elements, view only has %d",
            view_dest->length);
    }
    u32 *raw_ptr = (u32 *)view_dest->ptr;
    for (size_t i = 0; i < elements_to_set; i++) raw_ptr[i] = pattern;
    return elements_to_set;
}

#endif
