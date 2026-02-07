#ifndef MMU_H
#define MMU_H

/* MMU abstractions for Akai */

#include "../include/system.h"
#include "libsirius/discovery.h"
#include "libsirius/types.h"

void mmu_init(struct SystemInfo *sys);

void map_pdt_entry(u32 id, u32 page, u32 flags);
void unmap_pdt_entry(u32 id);
void map_pt_entry(u32 *pt, u32 id, u32 page, u32 flags);
void chperm_pt_entry(u32 *pt, u32 id, u32 flags);
void map_pt_range(u32 *pt, u32 pstart, u32 vstart, usize num_pages, u32 flags);
void unmap_pt_entry(u32 *pt, u32 id);
void tlb_flush(void);
void mmu_on(void);
void mmu_off(void);

#endif /* MMU_H */
