/**
 * A very simple DOS-style kernel for the Taleä Computer System
 */

#include "libsirius/devices.h"
#include "libsirius/discovery.h"
#include "libsirius/types.h"

#include "../include/hw.h"
#include "../include/mem.h"
#include "../include/mmu.h"
#include "../include/process.h"

u8 kernel_pt_align[PAGE_SIZE * 3]; // hack to align the page, because reasons
u32      *kernel_pt;
u32      *video_pt;

struct PhysicalPages pages;
struct Processes processes;
#define SIZE_PROC sizeof(processes);
struct SystemInfo sys;

void puts(struct SystemInfo *s, const char *str)
{
    static u32 pos       = 0;
    u32        term_size = s->textbuffer.w * s->textbuffer.h;
    char      *p         = (char *)str;
    char       c;
    while (c = *p++) {
        if (c == '\n') {
            pos += sys.textbuffer.w - (pos % sys.textbuffer.w);
            continue;
        }

        *((volatile u32 *)(s->textbuffer.addr) + pos) = ((u32)c << 24) | 0x000f0000UL;

        pos = (pos + 1) % (term_size);
    }
}

static void pdt_init(void)
{
    // set the pdt to 0xf000 in DATA
    u32 sreg = _gsreg();
    _ssreg((sreg & ~0x000FFF00) | (AKAI_PDT_BASE >> 4) << 8);
}

static void pt_init(void)
{
    // find an aligned region for the kernel page table
    kernel_pt = (u32 *)(((u32)kernel_pt_align + 4095) & ~4095UL);
    video_pt  = kernel_pt + (PAGE_SIZE / sizeof(u32));

    // clear the page table
    memset(kernel_pt, 0, PAGE_SIZE);
    memset(video_pt, 0, PAGE_SIZE);
}

void mmu_init(struct SystemInfo *sys)
{
    // intialize page directory
    pdt_init();

    // initialize kernel page tables
    pt_init();

    // identity map the kernel page table in the pdt
    map_pdt_entry(0, (u32)kernel_pt, PTE_V);

    // identity map the kernel (0-0x2_0000) (//TODO: mark data, bss and stack as not exectuable )
    map_pt_range(kernel_pt, 0, 0, KERNEL_END >> 12, PTE_V | PTE_R | PTE_W | PTE_X);

    // identity map the video buffers page table in the pdt
    map_pdt_entry(sys->textbuffer.addr >> 22, (u32)video_pt, PTE_V);

    {
        // identity map the video buffers (maybe later remap them to be at the very top)
        usize tbsz = sys->textbuffer.h * sys->textbuffer.w * sys->textbuffer.bpc * 2;
        usize fbsz = sys->framebuffer.h * sys->framebuffer.w;

        usize tb_pages = (tbsz + (PAGE_SIZE - 1)) >> 12;
        usize fb_pages = (fbsz + (PAGE_SIZE - 1)) >> 12;

        map_pt_range(video_pt, sys->textbuffer.addr, sys->textbuffer.addr, tb_pages,
                     PTE_V | PTE_R | PTE_W | PTE_X);

        map_pt_range(video_pt, sys->framebuffer.addr, sys->framebuffer.addr, fb_pages,
                     PTE_V | PTE_R | PTE_W | PTE_X);
    }

    // flush the TLB (though it should be empty)
    tlb_flush();

    // turn on, and pray!
    mmu_on();
}

static u8 test_proc_code[] = { 0x42, 0x50, 0x00, 0xb0, 0xa6, 0x52, 0x80, 0x0b, 0x0a, 0x50, 0x00,
                               0x20, 0xa6, 0xc0, 0x00, 0x01, 0x04, 0x00, 0x00, 0x40, 0x0a, 0x10,
                               0x00, 0xa0, 0xa6, 0xc0, 0x00, 0x01, 0x04, 0x00, 0x00, 0x40, 0xa6,
                               0xc0, 0x00, 0x00, 0xa6, 0xd2, 0x80, 0x00, 0x04, 0x00, 0x00, 0x40 };

void kmain(u16 sys_info_data_addr)
{
    // Copy the struct handed down by the firmware
    _copydm(sys_info_data_addr, &sys, sizeof(sys));

    // init mmu
    mmu_init(&sys);

    // init the physical memory manager
    mem_init(&pages, &sys);

    // init the process manager
    process_init(&processes, AKAI_IDLE_BASE);

    // CRASH
    {
        // test the processes
        struct Process *test;
        u32             result = 0;
        u8             *code;

        ProcessPID test_pid = process_create(&processes, "Test", (ProcessEntry)AKAI_PROCESS_BASE);

        if (!test_pid) {
            result = 1;
            goto proc_test_failed;
        }

        _trace(0xF0000, test_pid);

        // manual exec
        test = &processes.proc[test_pid];
        code = alloc_pages_contiguous(&pages, test_pid, 1);

        if (!code) {
            process_terminate(&processes, test_pid);
            result = 2;
            goto proc_test_failed;
        }

        // map its page table 0 to us, so we can write to it
        map_pt_entry(kernel_pt, AKAI_PROCESS_PT0 >> 12, (u32)test->page_tables[0],
                     PTE_V | PTE_R | PTE_W);
        // give r/w/x because its a test. otherwise this is very rare
        map_pt_entry((u32 *)AKAI_PROCESS_PT0, AKAI_PROCESS_BASE >> 12, (u32)code,
                     PTE_V | PTE_U | PTE_X | PTE_R | PTE_W);
        // map the code for us so we can write to it
        map_pt_entry(kernel_pt, AKAI_PROCESS_BASE >> 12, (u32)code, PTE_V | PTE_R | PTE_W);
        memcpy((u8 *)AKAI_PROCESS_BASE, test_proc_code, sizeof(test_proc_code));

        // unmap all the mappings to the kernel table
        unmap_pt_entry(kernel_pt, AKAI_PROCESS_PT0 >> 12);
        unmap_pt_entry(kernel_pt, AKAI_PROCESS_BASE >> 12);
        tlb_flush();

        // pray, and run
        process_run(&processes, test_pid, PARENT_KEEP_RUNNING);

proc_test_failed:
        _trace(0xdead, result);
    }

_trace(0xb000b);
l:
    goto l;
}
