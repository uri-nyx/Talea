#ifndef MEM_H
#define MEM_H

/* Physical memory manager for Akai */

#include "akai_def.h"

#define MAX_PHYSICAL_PAGES (1U << 12U)

// FIXME: this differs among memory configurations, but lets keep it simple for now
#define NUM_PHYSICAL_PAGES MAX_PHYSICAL_PAGES

// Size of page bitmap in bytes
#define PAGE_BITMAP_SIZE (NUM_PHYSICAL_PAGES / 8)

#define PAGE_OWNER_MAP_SIZE (NUM_PHYSICAL_PAGES)

enum { PAGE_FREE, PAGE_ALLOCATED };

struct PhysicalPages {
    u8         free[PAGE_BITMAP_SIZE];      // Free pages bitmap
    u8         refs[PAGE_OWNER_MAP_SIZE];   // Reference count
    ProcessPID owners[PAGE_OWNER_MAP_SIZE]; // original owners of allocated pages
    usize      allocated;                   // count of allocated pages
};

void mem_init(void);

void *alloc_pages_contiguous(ProcessPID pid, usize count);

void  free_page(void *page);
void  free_pages_contiguous(void *page, usize count);
void  free_pages_by_owner(ProcessPID pid);
void *page_transfer(ProcessPID new_owner, void *page);
void *share_page(void *page);

#endif /* MEM_H */
