#ifndef KERNEL_H
#define KERNEL_H

#include "akai_def.h"
#include "libsirius/devices.h"
#include "libsirius/discovery.h"

#include "kstring.h"

#include "system.h"

#include "process.h"

#include "mem.h"

#include "mmu.h"

#include "handlers.h"

#include "dev.h"

#include "drivers.h"

#include "ff.h"

#define FILE_POOL_MAX 64

#define BSWAP32(num)                                                                  \
    (((((unsigned)num) & 0xff000000) >> 24) | ((((unsigned)num) & 0x00ff0000) >> 8) | \
     ((((unsigned)num) & 0x0000ff00) << 8) | (((unsigned)num) << 24))

#define PAGE_ALIGN_UP(addr) (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

static const char *drive_labels[] = { "/A", "/B", "/H" };

struct FilePool {
    FIL files[FILE_POOL_MAX];
    u8  refs[FILE_POOL_MAX];
};

struct Akai {
    // Subsystems
    struct SystemInfo    info;
    struct Processes     pr;
    struct PhysicalPages pg;

    // Memory map
    u32  Kpt[PAGE_SIZE / sizeof(u32)];
    u32  Hpt[PAGE_SIZE / sizeof(u32)];
    u32 *dash;
    u8   _kernel_p_align[PAGE_SIZE * 2];

    // Interrupts
    KernelInterruptHook hooks[AKAI_NUM_INTERRUPTS];

    // Filesystem
    FATFS           fs[3]; // 0: tps A, 1: tps B, 2: hcs
    struct FilePool fp;

    // timing
    u32 time_start, uptime;

    // Devices
    struct Device devices[_DEV_NUM];

    // Hardware drivers
    struct DriverKb  kb;
    struct DriverTxt txt;
};

extern struct Akai A;

u8  *map_kernel_work_area(u32 page);
u8  *remap_kernel_work_area(u32 page);
void unmap_kernel_work_area();

void miniprint(const char *fmt, ...);

#endif /* KERNEL_H */
