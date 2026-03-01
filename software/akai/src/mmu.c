#include "hw.h"
#include "kernel.h"

void tlb_flush_addr(u32 vaddr)
{
    _trace(0xbbb);
    tlb_flush();
    _trace(0xbbb);
    return;
}

void map_pdt_entry(u32 pdt_base, u32 id, u32 page, u32 flags)
{
    _swd(pdt_base + ((id & 0x3FF) * 4), (page & 0xFFF000) | flags);
}

void unmap_pdt_entry(u32 pdt_base, u32 id)
{
    _swd(pdt_base + ((id & 0x3FF) * 4), 0);
}

void map_pt_entry(u32 *pt, u32 id, u32 page, u32 flags)
{
    pt[id & 0x3FF] = (page & 0xFFF000) | flags;
    _trace(0x666, pt[id & 0x3FF], (u32)pt, id);
}

bool map_active_range(u32 pstart, u32 vstart, usize num_pages, u32 flags)
{
    u32  i;
    u32  vaddr             = vstart;
    u32  paddr             = pstart;
    bool structure_changed = false;

    if (((vstart & 0xfff) != 0) || ((pstart & 0xfff) != 0)) {
        _trace(0xDEADC0DE, 0X10001);
    }

    for (i = 0; i < num_pages; i++) {
        u32  pdt_idx = vaddr >> 22;
        u32  vpn     = (vaddr >> 12) & 0x3FF;
        u32 *active_pt;

        if (pdt_idx > 3) return false;

        if (A.pr.curr->page_tables[pdt_idx] == NULL) {
            u32 *work;
            u32  bridge_vpn;
            u32 *new_pt = (u32 *)alloc_pages_contiguous(A.pr.curr->pid, 1);
            if (!new_pt) return false;

            work = (u32 *)map_kernel_work_area((u32)new_pt);
            memset(work, 0, PAGE_SIZE);

            work             = (u32 *)remap_kernel_work_area((u32)A.pr.curr->page_tables[0]);
            bridge_vpn       = (AKAI_PROCESS_PAGE_TABLES >> 12) + pdt_idx;
            work[bridge_vpn] = ((u32)new_pt & 0xFFF000) | PTE_V | PTE_R | PTE_W;
            unmap_kernel_work_area();

            A.pr.curr->page_tables[pdt_idx] = new_pt;
            map_pdt_entry(A.pr.curr->pdt, pdt_idx, (u32)new_pt, PTE_V);

            structure_changed = true;
        }

        active_pt      = (u32 *)(AKAI_PROCESS_PAGE_TABLES + (pdt_idx * PAGE_SIZE));
        active_pt[vpn] = (paddr & 0xFFF000) | flags;
        _trace(0xd1e2, active_pt, vaddr, active_pt[vpn]);
        _trace(0xd1e2, flags);

        vaddr += PAGE_SIZE;
        paddr += PAGE_SIZE;
    }

    tlb_flush();

    return true;
}

bool map_active_pt_entry(u32 paddr, u32 vaddr, u32 flags)
{
    u32  pdt_idx  = vaddr >> 22;
    u32  vpn      = (vaddr >> 12) & 0x3FF;
    u32 *pde_phys = A.pr.curr->page_tables[pdt_idx];
    u32 *active_pt;

    if (pdt_idx > 3) {
        // this is not allowed
        return false;
    }

    if (pde_phys == NULL) {
        u32 *new_pt_phys = (u32 *)alloc_pages_contiguous(A.pr.curr->pid, 1);
        u32  recursive_vpn;
        u32 *work;

        if (!new_pt_phys) {
            A.pr.curr->last_error = A_ERROR_OOM;
            return false;
        }

        work = (u32 *)map_kernel_work_area((u32)new_pt_phys);
        if (!work) {
            free_page(new_pt_phys);
            return false;
        }

        memset(work, 0, PAGE_SIZE);

        work = (u32 *)remap_kernel_work_area((u32)A.pr.curr->page_tables[0]);

        recursive_vpn       = ((AKAI_PROCESS_PAGE_TABLES + (pdt_idx * PAGE_SIZE)) >> 12) & 0x3FF;
        work[recursive_vpn] = ((u32)new_pt_phys & 0xFFF000) | PTE_V | PTE_R | PTE_W;

        A.pr.curr->page_tables[pdt_idx] = new_pt_phys;
        map_pdt_entry(A.pr.curr->pdt, pdt_idx, (u32)new_pt_phys, PTE_V);

        unmap_kernel_work_area();
        tlb_flush();
    }

    active_pt = (u32 *)map_kernel_work_area((u32)A.pr.curr->page_tables[pdt_idx]);
    if (!active_pt) {
        return false;
    };

    active_pt[vpn] = (paddr & 0xFFF000) | flags;
    _trace(0xffaaf, active_pt, vpn, active_pt[vpn]);
    unmap_kernel_work_area();
    _trace(0xffeef0);
    tlb_flush(); // FIXME: calling tlb_flush_addr(vaddr) here triggers an illegal isntruction
                 // exception. who knows why
    return true;
}

// NOTE: it does not shrink the page tables
void unmap_active_pt_entry(u32 vaddr)
{
    u32  pdt_idx   = vaddr >> 22;
    u32  vpn       = (vaddr >> 12) & 0x3FF;
    u32 *pde_phys  = A.pr.curr->page_tables[pdt_idx];
    u32 *active_pt = (u32 *)(AKAI_PROCESS_PAGE_TABLES + pdt_idx * PAGE_SIZE);

    if (pdt_idx > 3) return;
    if (pde_phys == NULL) return;
    // must be mapped
    active_pt[vpn] = 0;
    tlb_flush();
}

void *active_phys_from_v(u32 vaddr)
{
    u32  pdt_idx   = vaddr >> 22;
    u32  vpn       = (vaddr >> 12) & 0x3FF;
    u32 *pde_phys  = A.pr.curr->page_tables[pdt_idx];
    u32 *active_pt = (u32 *)(AKAI_PROCESS_PAGE_TABLES + pdt_idx * PAGE_SIZE);

    if (pdt_idx > 3) return NULL;
    if (pde_phys == NULL) return NULL;
    // must be mapped
    return (void *)(active_pt[vpn] & ~0xFFFU);
}

void *phys_from_v(struct Process *proc, u32 vaddr)
{
    u32  pdt_idx = vaddr >> 22;
    u32  vpn     = (vaddr >> 12) & 0x3FF;
    u32 *pt_phys = proc->page_tables[pdt_idx];
    u32 *pt_work;
    u32  phys_page;

    if (pdt_idx > 3 || pt_phys == NULL) return NULL;

    /* Map the process's page table into kernel work area to read it */
    pt_work = (u32 *)map_kernel_work_area((u32)pt_phys);
    if (!pt_work) return NULL;

    phys_page = pt_work[vpn] & 0xFFF000;
    unmap_kernel_work_area();

    if (phys_page == 0) return NULL;
    return (void *)phys_page;
}

usize copy_to_user(struct Process *dest_proc, void *user_dst, void *kernel_src, usize len)
{
    u8   *src    = (u8 *)kernel_src;
    u8   *dst    = (u8 *)user_dst;
    usize copied = 0;

    while (len > 0) {
        u32 vaddr  = (u32)dst;
        u32 offset = vaddr & 0xFFF;
        u32 chunk  = 4096 - offset;
        u8 *work_area;
        void *phys_page;
        
        if (chunk > len) chunk = len;

        /* Get the physical address for this chunk */
        phys_page = phys_from_v(dest_proc, vaddr);
        if (!phys_page) break; /* Segmentation fault style failure */

        /* Map that user page into our work area */
        work_area = (u8 *)map_kernel_work_area((u32)phys_page);
        if (!work_area) break;

        /* Copy the chunk */
        memcpy(work_area + offset, src, chunk);

        unmap_kernel_work_area();

        /* Advance pointers */
        len -= chunk;
        src += chunk;
        dst += chunk;
        copied += chunk;
    }
    return copied;
}

void chperm_pt_entry(u32 *pt, u32 id, u32 flags)
{
    pt[id & 0x3FF] &= ~0x1fU;
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
