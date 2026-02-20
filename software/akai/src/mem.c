#include "kernel.h"

void mem_init()
{
    struct PhysicalPages *pages = &A.pg;
    struct SystemInfo    *sys   = &A.info;

    usize i;
    usize tbsz = sys->textbuffer.h * sys->textbuffer.w * sys->textbuffer.bpc;
    usize fbsz = sys->framebuffer.h * sys->framebuffer.w;

    usize tb_pages = (tbsz + (PAGE_SIZE - 1)) >> 12;
    usize fb_pages = (fbsz + (PAGE_SIZE - 1)) >> 12;

    usize tb_start_page = sys->textbuffer.addr >> 12;
    usize fb_start_page = sys->framebuffer.addr >> 12;

    // mark the kernel pages as not free and owned by PID 0 (the Kernel)
    for (i = 0; i < (AKAI_KERNEL_END >> 12); i++) {
        BIT_SET(pages->free, i);
        pages->owners[i] = KERNEL_PID;
        pages->refs[i] += 1;
        pages->allocated++;
    }

    // mark the video buffer pages also as not free and owned by the kernel

    for (i = 0; i < tb_pages; i++) {
        BIT_SET(pages->free, tb_start_page + i);
        pages->owners[tb_start_page + i] = KERNEL_PID;
        pages->refs[tb_start_page + i] += 1;
        pages->allocated++;
    }

    for (i = 0; i < fb_pages; i++) {
        BIT_SET(pages->free, fb_start_page + i);
        pages->owners[fb_start_page + i] = KERNEL_PID;
        pages->refs[fb_start_page + i] += 1;
        pages->allocated++;
    }

    // mark the firmware pages as not free and owned by the kernel, and NEVER MAP THEM R
    for (i = 0xff0000 >> 12; i < 0x1000000 >> 12; i++) {
        BIT_SET(pages->free, i);
        pages->owners[i] = KERNEL_PID;
        pages->refs[i] += 1;
        pages->allocated++;
    }

    // mark also the kernel stack guard TODO: maybe remove this
    BIT_SET(pages->free, (AKAI_KERNEL_STACK_TOP >> 12) + 1);
    pages->owners[(AKAI_KERNEL_STACK_TOP >> 12) + 1] = KERNEL_PID;
    pages->refs[(AKAI_KERNEL_STACK_TOP >> 12) + 1] += 1;
    pages->allocated++;
}

void *alloc_pages_contiguous(ProcessPID pid, usize count)
{
    struct PhysicalPages *p = &A.pg;

    usize remaining = count;
    ssize first     = -1;
    usize i         = 0;

    if (pid == ORPHAN_PID) return NULL; // Do not alloc to the orphan

    // Not enough memory. // FIXME: maybe can do the trick of returning negative values, since phy-
    // sical pointers are 24 bit.
    if (NUM_PHYSICAL_PAGES - p->allocated < count) return NULL;

    do {
        bool allocated = BIT_TEST(p->free, i);
        if (!allocated) {
            remaining--;
            first = first < 0 ? i : first;
        } else {
            remaining = count;
            first     = -1;
        }
        if (++i >= NUM_PHYSICAL_PAGES) break;
    } while (remaining);

    // actually (void*)(0) would be a valid page, but its claimed by the kernel
    if (first < 0) return NULL; // Not found a contiguous region

    for (i = 0; i < count; i++) {
        BIT_SET(p->free, first + i);
        p->owners[first + i] = pid;
        p->refs[first + i] += 1;
        p->allocated++;
    }

    return (void *)(first << 12);
}

void free_page(void *page)
{
    struct PhysicalPages *p       = &A.pg;
    usize                 page_id = (u32)page >> 12;

    if (p->owners[page_id] == KERNEL_PID) return; // Don't free the kernel

    if (BIT_TEST(p->free, page_id) && !(p->refs[page_id] -= 1)) {
        BIT_CLR(p->free, page_id);
        // NOTE: while pid 0 is the kernel, the page is free because the bitmap is cleared.
        // Either way, its semantically sound that all relinquished pages belong to the kernel.
        p->owners[page_id] = KERNEL_PID;
        p->allocated--;
    }
}

void free_pages_contiguous(void *page, usize count)
{
    struct PhysicalPages *p       = &A.pg;
    usize                 page_id = (u32)page >> 12;
    usize                 i;

    for (i = 0; i < count; i++) {
        if (page_id + i >= NUM_PHYSICAL_PAGES) break;
        if (p->owners[page_id + i] == 0) continue; // Don't free the kernel
        if (BIT_TEST(p->free, page_id + i) && !(p->refs[page_id + i] -= 1)) {
            BIT_CLR(p->free, page_id + i);
            // NOTE: while pid 0 is the kernel, the page is free because the bitmap is cleared.
            // Either way, its semantically sound that all relinquished pages belong to the kernel.
            p->owners[page_id + i] = KERNEL_PID;
            p->allocated--;
        }
    }
}

void free_pages_by_owner(ProcessPID pid)
{
    struct PhysicalPages *p = &A.pg;
    usize                 i;

    if (pid == KERNEL_PID) return; // Dont free the kernel

    for (i = 0; i < NUM_PHYSICAL_PAGES; i++) {
        if (p->owners[i] == pid && BIT_TEST(p->free, i)) {
            p->owners[i] = ORPHAN_PID;
            if (!(p->refs[i] -= 1)) {
                BIT_CLR(p->free, i);
                // NOTE: while pid 0 is the kernel, the page is free because the bitmap is cleared.
                // Either way, its semantically sound that all relinquished pages belong to the
                // kernel.
                p->owners[i] = KERNEL_PID;
                p->allocated--;
            }
        }
    }
}

void *page_transfer(ProcessPID new_owner, void *page)
{
    struct PhysicalPages *p       = &A.pg;
    usize                 page_id = (u32)page >> 12;

    if (page == NULL) return NULL;
    if (page_id >= NUM_PHYSICAL_PAGES) return NULL;
    if (new_owner == KERNEL_PID || !BIT_TEST(p->free, page_id)) return NULL;

    p->owners[page_id] = new_owner;
    return page;
}

void *share_page(void *page)
{
    struct PhysicalPages *p = &A.pg;

    usize page_id = (u32)page >> 12;

    if (page_id >= NUM_PHYSICAL_PAGES) return NULL;

    if (BIT_TEST(p->free, page_id)) {
        p->refs[page_id]++;
        return page;
    }

    return NULL;
}
