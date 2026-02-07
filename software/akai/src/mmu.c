#include "../include/mmu.h"
#include "../include/hw.h"
#include "../libsirius/devices.h"
#include "../libsirius/types.h"

extern u32 *kernel_pt_align;
extern u32 *kernel_pt;
extern u32 *high_mem_pt;

void map_pdt_entry(u32 id, u32 page, u32 flags)
{
    _swd(sirius_pdt + ((id & 0x3FF) * 4), (page & 0xFFF000) | flags);
}

void unmap_pdt_entry(u32 id)
{
    _swd(sirius_pdt + ((id & 0x3FF) * 4), 0);
}

void map_pt_entry(u32 *pt, u32 id, u32 page, u32 flags)
{
    pt[id & 0x3FF] = (page & 0xFFF000) | flags;
    _trace(0x666, pt[id & 0x3FF], (u32)pt, id);
}

void chperm_pt_entry(u32 *pt, u32 id, u32 flags)
{
    pt[id & 0x3FF] &= ~0x1f;
    pt[id & 0x3FF] |= flags;
    _trace(0x666, pt[id & 0x3FF], (u32)pt, id);
}

void map_pt_range(u32 *pt, u32 pstart, u32 vstart, usize num_pages, u32 flags)
{
    // FIXME: this is not safe if it wraps. Maybe let it fail
    usize i;
    u32   id = (vstart >> 12);

    for (i = 0; i < num_pages; i++) {
        map_pt_entry(pt, (id + i) & 0x3FF, pstart + (i << 12), flags);
    }
}

void unmap_pt_entry(u32 *pt, u32 id)
{
    pt[id & 0x3FF] = 0;
}

void tlb_flush(void)
{
    _sbd(REG_SYSTEM_TLB, 0xff);
}

void mmu_on(void)
{
    _sbd(REG_SYSTEM_MMU, 0xff);
}

void mmu_off(void)
{
    _sbd(REG_SYSTEM_MMU, 0);
}