/**
 * A very simple DOS-style kernel for the Taleä Computer System
 */

#include "libsirius/devices.h"
#include "libsirius/discovery.h"
#include "libsirius/types.h"

#include "../include/handlers.h"
#include "../include/hw.h"
#include "../include/mem.h"
#include "../include/mmu.h"
#include "../include/process.h"

u8   kernel_pt_align[PAGE_SIZE * 3]; // hack to align the page, because reasons
u32 *kernel_pt;
u32 *high_mem_pt;
u32 *dashboard_page;

struct PhysicalPages pages;
struct Processes     processes;
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

        *((volatile u32 *)(AKAI_TEXTBUFFER) + pos) = ((u32)c << 24) | 0x000f0000UL;

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
    kernel_pt   = (u32 *)(((u32)kernel_pt_align + 4095) & ~4095UL);
    high_mem_pt = kernel_pt + (PAGE_SIZE / sizeof(u32));

    // clear the page table
    memset(kernel_pt, 0, PAGE_SIZE);
    memset(high_mem_pt, 0, PAGE_SIZE);
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
    map_pt_range(kernel_pt, 0, 0, AKAI_KERNEL_END >> 12, PTE_V | PTE_R | PTE_W | PTE_X);

    // video buffers page table in the pdt
    map_pdt_entry(3, (u32)high_mem_pt, PTE_V);

    {
        // identity map the video buffers (maybe later remap them to be at the very top)
        usize tbsz = 128 * 1024;
        usize fbsz = sys->framebuffer.h * sys->framebuffer.w;

        usize tb_pages = (tbsz + (PAGE_SIZE - 1)) >> 12;
        usize fb_pages = (fbsz + (PAGE_SIZE - 1)) >> 12;

        map_pt_range(high_mem_pt, sys->textbuffer.addr, AKAI_TEXTBUFFER, tb_pages,
                     PTE_V | PTE_R | PTE_W);

        map_pt_range(high_mem_pt, sys->framebuffer.addr, AKAI_FRAMEBUFFER, fb_pages,
                     PTE_V | PTE_R | PTE_W);
    }

    // flush the TLB (though it should be empty)
    tlb_flush();

    // turn on, and pray!
    mmu_on();
}

static u8 test_proc_code[] = {
    0xa6, 0xc0, 0x00, 0x04, 0x04, 0x00, 0x00, 0x40, 0x60, 0x05, 0x00, 0x05, 0xa6, 0xc0, 0x00,
    0x00, 0x42, 0xd0, 0x00, 0x0d, 0xa6, 0xd6, 0x8e, 0xad, 0x04, 0x00, 0x00, 0x40, 0x42, 0x50,
    0x00, 0xb0, 0xa6, 0x52, 0x80, 0x0b, 0x0a, 0x50, 0x00, 0x20, 0xa6, 0xc0, 0x00, 0x01, 0x04,
    0x00, 0x00, 0x40, 0x0a, 0x10, 0x00, 0xa0, 0xa6, 0xc0, 0x00, 0x02, 0xa6, 0xd0, 0x0f, 0xff,
    0x44, 0xe0, 0x00, 0x00, 0xa6, 0xe7, 0x00, 0x2c, 0x04, 0x00, 0x00, 0x40, 0x42, 0x60, 0x00,
    0xff, 0xa6, 0x63, 0x0f, 0xff, 0xa6, 0x63, 0x7f, 0xff, 0x68, 0x03, 0x7f, 0xff, 0x0a, 0x10,
    0x28, 0xa0, 0xa6, 0xc0, 0x00, 0x01, 0x04, 0x00, 0x00, 0x40, 0x40, 0x0f, 0xff, 0xf9, 0x0b,
    0xff, 0xfc, 0x40, 0x34, 0xc1, 0x00, 0x00, 0x34, 0xd1, 0x00, 0x00, 0x42, 0xd0, 0x0f, 0x81,
    0xa6, 0xd6, 0x80, 0x00, 0xa6, 0xc0, 0x00, 0x05, 0x86, 0xd6, 0x80, 0x00, 0x04, 0x00, 0x00,
    0x40, 0x42, 0xc0, 0x0f, 0x7f, 0xa6, 0xc6, 0x00, 0x00, 0xa6, 0xd0, 0x00, 0x00, 0xe0, 0xd6,
    0x00, 0x00, 0x2e, 0xd1, 0x00, 0x00, 0x2e, 0xc1, 0x00, 0x00, 0x82, 0x0f, 0x80, 0x00
};

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

    // hook hernel interrupt handlers
    kernel_hook_interrupt(INT_KBD_SCAN, kbd_scan_hook);

    // allocate the dashboard page
    dashboard_page = alloc_pages_contiguous(&pages, KERNEL_PID, 1);
    // TODO: panic if failed to allocate

    // the process' view
    map_pt_entry(high_mem_pt, AKAI_IPC_SYSTEM_DASHBOARD >> 12, (u32)dashboard_page,
                 PTE_V | PTE_U | PTE_R);
    // the kernel's work area
    map_pt_entry(high_mem_pt, AKAI_IPC_KERNEL_DASH >> 12, (u32)dashboard_page,
                 PTE_V | PTE_W | PTE_R);

    tlb_flush();

    // lets enable kb interrupts to test hooks
    _sbd(sys.terminal + TERMINAL_KBD_CSR, KB_GLOBAL_EN | KB_IE_DOWN);
    {
        // test the processes
        struct Process *test;
        u32             result = 0;
        u8             *code, *stack;

        ProcessPID test_pid = process_create(&processes, "Test", (ProcessEntry)AKAI_PROCESS_BASE);

        if (!test_pid) {
            result = 1;
            goto proc_test_failed;
        }

        _trace(0xF0000, test_pid);

        // manual exec
        test  = &processes.proc[test_pid];
        code  = alloc_pages_contiguous(&pages, test_pid, 1);
        stack = alloc_pages_contiguous(&pages, test_pid, 1);

        if (!code || !stack) {
            process_terminate(&processes, test_pid);
            result = 2;
            goto proc_test_failed;
        }

        // map its page tables to us, so we can write to it
        map_pt_entry(kernel_pt, AKAI_PROCESS_PT0 >> 12, (u32)test->page_tables[0],
                     PTE_V | PTE_R | PTE_W);
        map_pt_entry(kernel_pt, AKAI_PROCESS_PT3 >> 12, (u32)test->page_tables[3],
                     PTE_V | PTE_R | PTE_W);
        // give r/w/x because its a test. otherwise this is very rare
        map_pt_entry((u32 *)AKAI_PROCESS_PT0, AKAI_PROCESS_BASE >> 12, (u32)code,
                     PTE_V | PTE_U | PTE_X | PTE_R | PTE_W);
        map_pt_entry((u32 *)AKAI_PROCESS_PT3, (AKAI_PROCESS_STACK_TOP >> 12) - 1, (u32)stack,
                     PTE_V | PTE_U | PTE_R | PTE_W);
        chperm_pt_entry((u32 *)AKAI_PROCESS_PT3, AKAI_IPC_SYSTEM_DASHBOARD >> 12,
                        PTE_V | PTE_U | PTE_R);
        tlb_flush();
        // map the code for us so we can write to it. should also memset the stack
        map_pt_entry(kernel_pt, AKAI_PROCESS_BASE >> 12, (u32)code, PTE_V | PTE_R | PTE_W);
        memcpy((u8 *)AKAI_PROCESS_BASE, test_proc_code, sizeof(test_proc_code));

        // unmap all the mappings to the kernel table
        unmap_pt_entry(kernel_pt, AKAI_PROCESS_PT0 >> 12);
        unmap_pt_entry(kernel_pt, AKAI_PROCESS_BASE >> 12);
        tlb_flush();

        test->ctx.usp = AKAI_PROCESS_STACK_TOP;

        // pray, and run
        process_run(&processes, test_pid, PARENT_KEEP_RUNNING);

proc_test_failed:
        _trace(0xdead, result);
    }

    _trace(0xb000b);
l:
    goto l;
}
