#ifndef MMU_H
#define MMU_H

/* MMU abstractions for Akai */

void mmu_init(void);

bool map_active_range(u32 pstart, u32 vstart, usize num_pages, u32 flags);
bool map_active_pt_entry(u32 paddr, u32 vaddr, u32 flags);
void unmap_active_pt_entry(u32 vaddr);
void *active_phys_from_v(u32 vaddr);

void *phys_from_v(struct Process *proc, u32 vaddr);
usize copy_to_user(struct Process *dest_proc, void *user_dst, void *kernel_src, usize len);
void map_pdt_entry(u32 pdt_base, u32 id, u32 page, u32 flags);
void unmap_pdt_entry(u32 pdt_base, u32 id);
void map_pt_entry(u32 *pt, u32 id, u32 page, u32 flags);
void chperm_pt_entry(u32 *pt, u32 vaddr, u32 flags);
void map_pt_range(u32 *pt, u32 pstart, u32 vstart, usize num_pages, u32 flags);
void unmap_pt_entry(u32 *pt, u32 id);
void tlb_flush(void);
void mmu_on(void);
void mmu_off(void);

#endif /* MMU_H */
