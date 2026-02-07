#include "../include/mem.h"
#include "../include/mmu.h"

void mem_init(struct PhysicalPages *pages, struct SystemInfo *sys)
{
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
        pages->allocated++;
    }

    // mark the video buffer pages also as not free and owned by the kernel

    for (i = 0; i < tb_pages; i++) {
        BIT_SET(pages->free, tb_start_page + i);
        pages->owners[tb_start_page + i] = KERNEL_PID;
        pages->allocated++;
    }

    for (i = 0; i < fb_pages; i++) {
        BIT_SET(pages->free, fb_start_page + i);
        pages->owners[fb_start_page + i] = KERNEL_PID;
        pages->allocated++;
    }

    // mark the firmware pages as not free and owned by the kernel, and NEVER MAP THEM R
    for (i = 0xff0000 >> 12; i < 0x1000000 >> 12; i++) {
        BIT_SET(pages->free, i);
        pages->owners[i] = KERNEL_PID;
        pages->allocated++;
    }

    // mark also the kernel stack guard
    BIT_SET(pages->free, (AKAI_KERNEL_END >> 12));
    pages->owners[(AKAI_KERNEL_END >> 12)] = KERNEL_PID;
    pages->allocated++;
}


void *alloc_pages_contiguous(struct PhysicalPages *p, ProcessPID pid, usize count)
{
    usize remaining = count;
    ssize first     = -1;
    usize i         = 0;

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
        p->allocated++;
    }

    return (void *)(first << 12);
}

void free_page(struct PhysicalPages *p, void *page)
{
    usize page_id = (u32)page >> 12;

    if (p->owners[page_id] == 0) return; // Don't free the kernel

    if (BIT_TEST(p->free, page_id)) {
        BIT_CLR(p->free, page_id);
        // NOTE: while pid 0 is the kernel, the page is free because the bitmap is cleared. Either
        // way, its semantically sound that all relinquished pages belong to the kernel.
        p->owners[page_id] = 0;
        p->allocated--;
    }
}

void free_pages_contiguous(struct PhysicalPages *p, void *page, usize count)
{
    usize page_id = (u32)page >> 12;
    usize i;

    for (i = 0; i < count; i++) {
        if (p->owners[page_id] == 0) continue; // Don't free the kernel
        if (BIT_TEST(p->free, page_id + i)) {
            BIT_CLR(p->free, page_id + i);
            // NOTE: while pid 0 is the kernel, the page is free because the bitmap is cleared.
            // Either way, its semantically sound that all relinquished pages belong to the kernel.
            p->owners[page_id + i] = 0;
            p->allocated--;
        }
    }
}

void free_pages_by_owner(struct PhysicalPages *p, ProcessPID pid)
{
    usize i;

    if (pid == 0) return; // Dont free the kernel

    for (i = 0; i < NUM_PHYSICAL_PAGES; i++) {
        if (p->owners[i] == pid && BIT_TEST(p->free, i)) {
            BIT_CLR(p->free, i);
            // NOTE: while pid 0 is the kernel, the page is free because the bitmap is cleared.
            // Either way, its semantically sound that all relinquished pages belong to the kernel.
            p->owners[i] = 0;
            p->allocated--;
        }
    }
}