#ifndef MEM_H
#define MEM_H

/* Physical memory manager for Akai */

#include "libsirius/types.h"

#include "../include/process.h"
#include "libsirius/discovery.h"

#define MAX_PHYSICAL_PAGES (1U << 12U)

// FIXME: this differs among memory configurations, but lets keep it simple for now
#define NUM_PHYSICAL_PAGES MAX_PHYSICAL_PAGES

// Size of page bitmap in bytes
#define PAGE_BITMAP_SIZE (NUM_PHYSICAL_PAGES / 8)

#define PAGE_OWNER_MAP_SIZE (NUM_PHYSICAL_PAGES)

enum { PAGE_FREE, PAGE_ALLOCATED };

struct PhysicalPages {
    u8         free[PAGE_BITMAP_SIZE];      // Free pages bitmap
    ProcessPID owners[PAGE_OWNER_MAP_SIZE]; // Owners of allocated pages
    usize      allocated;                   // count of allocated pages
};

void mem_init(struct PhysicalPages *pages, struct SystemInfo *sys);

void *alloc_pages_contiguous(struct PhysicalPages *p, ProcessPID pid, usize count);

void free_page(struct PhysicalPages *p, void *page);
void free_pages_contiguous(struct PhysicalPages *p, void *page, usize count);
void free_pages_by_owner(struct PhysicalPages *p, ProcessPID pid);

#endif /* MEM_H */
