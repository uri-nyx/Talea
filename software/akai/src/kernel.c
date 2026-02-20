/**
 * A very simple DOS-style kernel for the Taleä Computer System
 */

#include "kernel.h"
#include "hw.h"
#include "init_code.h"

struct Akai A;

extern void *_bss_start;
extern void *_data_start;

i32 vdev_ctl()
{
    return 0;
};

u8 vdev_in()
{
    return 0;
};

i32 vdev_out()
{
    return 0;
};

i32 vdev_reset()
{
    return 0;
};

void puts(const char *str)
{
    static u32 pos       = 0;
    u32        term_size = A.info.textbuffer.w * A.info.textbuffer.h;
    char      *p         = (char *)str;
    char       c;
    while (c = *p++) {
        if (c == '\n') {
            pos += A.info.textbuffer.w - (pos % A.info.textbuffer.w);
            continue;
        }

        *((volatile u32 *)(AKAI_TEXTBUFFER) + pos) = ((u32)c << 24) | 0x000f0000UL;

        pos = (pos + 1) % (term_size);
    }
}

static u8 work_areas_mapped = 0;

u8 *map_kernel_work_area(u32 page)
{
    u8 cache_work_area = work_areas_mapped;
    if (work_areas_mapped > 9) {
        _trace(0XBADC0DE, 0XC0FEBABE, 0XBADC0DE, 0XC0FEBABE);
        return NULL;
    }

    map_pt_entry((u32 *)AKAI_PROCESS_PT3, (AKAI_KERNEL_WORK0 >> 12) + work_areas_mapped, page,
                 PTE_V | PTE_R | PTE_W);
    tlb_flush();
    work_areas_mapped++;
    return (u8 *)(AKAI_KERNEL_WORK0 + (cache_work_area * PAGE_SIZE));
}

// only use after mapping a work area, be nice
u8 *remap_kernel_work_area(u32 page)

{
    u8 curr_work_area = work_areas_mapped ? work_areas_mapped - 1 : 0;
    map_pt_entry((u32 *)AKAI_PROCESS_PT3, (AKAI_KERNEL_WORK0 >> 12) + curr_work_area, page,
                 PTE_V | PTE_R | PTE_W);
    tlb_flush();
    return (u8 *)(AKAI_KERNEL_WORK0 + (curr_work_area * PAGE_SIZE));
}

void unmap_kernel_work_area()
{
    if (work_areas_mapped == 0) {
        // this is safe, just return
        return;
    }
    work_areas_mapped--;
    unmap_pt_entry((u32 *)AKAI_PROCESS_PT3, (AKAI_KERNEL_WORK0 >> 12) + work_areas_mapped);
    tlb_flush();
}

static void pdt_init(void)
{
    // set the pdt to 0xf000 in DATA
    u32 sreg = _gsreg();
    _ssreg((sreg & ~0x000FFF00) | (AKAI_PDT_BASE >> 4) << 8);
}

static void template_pt_init(void)
{
    // initialize the page table templates
    usize tbsz = 128 * 1024;
    usize fbsz = A.info.framebuffer.h * A.info.framebuffer.w;

    usize tb_pages = (tbsz + (PAGE_SIZE - 1)) >> 12;
    usize fb_pages = (fbsz + (PAGE_SIZE - 1)) >> 12;

    // find an aligned address for the dashboard page
    A.dash = (u32 *)(((u32)A._kernel_p_align + 4095) & ~4095UL);

    memset(A.Kpt, 0, PAGE_SIZE);
    memset(A.Hpt, 0, PAGE_SIZE);
    memset(A.dash, 0, PAGE_SIZE);

    // map the kernel in the template
    map_pt_range(A.Kpt, 0, 0, AKAI_KERNEL_END >> 12, PTE_RWX);

    // map the video buffers in the template
    map_pt_range(A.Hpt, A.info.textbuffer.addr, AKAI_TEXTBUFFER, tb_pages, PTE_V | PTE_R | PTE_W);
    map_pt_range(A.Hpt, A.info.framebuffer.addr, AKAI_FRAMEBUFFER, fb_pages, PTE_V | PTE_R | PTE_W);

    // map the dashboard page in the template (this is a global read-only page)
    map_pt_entry(A.Hpt, AKAI_IPC_KERNEL_DASH, (u32)A.dash, PTE_V | PTE_R | PTE_W);
    map_pt_entry(A.Hpt, AKAI_IPC_SYSTEM_DASHBOARD, (u32)A.dash, PTE_V | PTE_R | PTE_U);
}

void mmu_init(void)
{
    // intialize page directory
    pdt_init();

    // initialize the page table templates
    template_pt_init();

    // initialize the actual kernel page tables
    // check if this is safe for the underlying array! (u8   _kernel_p_align[PAGE_SIZE * 3];)

    // copy the templates in the page tables (they are identity mapped in template_pt_init)
    memcpy((u8 *)AKAI_PROCESS_PT0, A.Kpt, PAGE_SIZE);
    memcpy((u8 *)AKAI_PROCESS_PT3, A.Hpt, PAGE_SIZE);

    // Recursive mapping of the pts
    map_pt_entry((u32 *)AKAI_PROCESS_PT0, AKAI_PROCESS_PT0 >> 12, AKAI_PROCESS_PT0,
                 PTE_V | PTE_R | PTE_W);
    map_pt_entry((u32 *)AKAI_PROCESS_PT3, AKAI_PROCESS_PT3 >> 12, AKAI_PROCESS_PT3,
                 PTE_V | PTE_R | PTE_W);

    // map them in the PDT
    map_pdt_entry(AKAI_PDT_BASE, 0, AKAI_PROCESS_PT0, PTE_V);
    map_pdt_entry(AKAI_PDT_BASE, 3, AKAI_PROCESS_PT3, PTE_V);

    // flush the TLB (though it should be empty)
    tlb_flush();

    // turn on, and pray!
    mmu_on();
}

static void fs_mount()
{
    static const char *lab[] = { "A:", "B:", "HCS:" };

    usize i;
    for (i = 0; i < 3; i++) {
        f_mount(&A.fs[i], lab[i], 0);
    }

    f_chdrive("B:");
}

static void timer_init(void)
{
    // set timer to interval at 100hz
    u8 csr = _lbud(TERMINAL_TIMER_CSR);
    _sbd(TERMINAL_TIMER_PRESCALER, AKAI_TIMER_PRESCALER_1OOHZ);
    _shd(TERMINAL_TIMER_INTERVAL, AKAI_TIMER_INTERVAL_100HZ);
    _sbd(TERMINAL_TIMER_CSR, csr | TIM_INTERVAL_EN);
}

static void timer_on(void)
{
    u8 csr = _lbud(TERMINAL_TIMER_CSR);
    _sbd(TERMINAL_TIMER_CSR, csr | TIM_GLOBAL_EN);
}

static void timer_off(void)
{
    u8 csr = _lbud(TERMINAL_TIMER_CSR);
    _sbd(TERMINAL_TIMER_CSR, csr & ~TIM_GLOBAL_EN);
}

static u32 uptime(void)
{
    _sbd(REG_SYSTEM_MINUTE, TALEA_SYSTEM_MILLIS_MODE); /* Set the COUNTER MODE */
    return _lwd(REG_SYSTEM_MINUTE);
}

static void timer_interval_hook(u32 *win, struct IPCMessage *msg_out)
{
    u32 status = _lwd(AKAI_KERNEL_STATUS_SAVE);

    A.uptime = uptime() - A.time_start;

    if (!(status & CPU_STATUS_SUPERVISOR)) {
        // puts(A.pr.curr->name);
        process_yield();
    }
}

static ProcessPID init_proc(const char *name)
{
    struct Process *init;
    u32             result = 0;
    u8             *code, *stack;
    u32            *work;

    ProcessPID init_pid = process_create(name, (ProcessEntry)AKAI_PROCESS_BASE);
    //_trace(0xDF000); // FIXME: this trace is absolutely necesary, or an illegal instruction
    // triggers
    // a kernel panic

    if (!init_pid) {
        _trace(0xDF000E0); // FIXME: this trace is absolutely necesary, or an illegal instruction
                           // triggers a kernel panic

        return 0;
    }

    // manual exec
    init  = &A.pr.proc[init_pid];
    code  = alloc_pages_contiguous(init_pid, 1);
    stack = alloc_pages_contiguous(init_pid, 1);
    //_trace(0xDF001);

    if (!code || !stack) {
        //_trace(0xDF000E1);
        process_terminate(init_pid);
        //_trace(0xDF000E2);

        return 0;
    }

    // map its page tables to us, so we can write to it
    work = (u32 *)map_kernel_work_area((u32)init->page_tables[0]);
    if (!work) {
        process_terminate(init_pid);
        return 0;
    }

    // give r/w/x because its a init. otherwise this is very rare
    map_pt_entry(work, AKAI_PROCESS_BASE >> 12, (u32)code, PTE_U | PTE_RWX);

    work = (u32 *)remap_kernel_work_area((u32)init->page_tables[1]);
    map_pt_entry(work, (AKAI_PROCESS_STACK_TOP >> 12) - 1, (u32)stack, PTE_U | PTE_RW);

    work = (u32 *)remap_kernel_work_area((u32)stack);
    memset(work, 0, PAGE_SIZE);

    // map the code for us so we can write to it.
    work = (u32 *)remap_kernel_work_area((u32)code);
    memcpy((u8 *)work, init_bin, init_bin_len);

    unmap_kernel_work_area();
    tlb_flush();

    init->ctx.usp = AKAI_PROCESS_STACK_TOP;

    return init_pid;
}

void kmain(u16 sys_info_data_addr)
{
    ProcessPID init;
    memset(&A, 0, sizeof(A));
    // memset(_bss_start, 0, (AKAI_KERNEL_STACK_TOP - PAGE_SIZE) - (u32)_bss_start);

    // start time is now
    A.time_start = uptime();
    A.uptime     = uptime();

    // Copy the struct handed down by the firmware
    _copydm(sys_info_data_addr, &A.info, sizeof(A.info));

    // init mmu
    mmu_init();

    // init the physical memory manager
    mem_init();

    video_driver_init();
    kbd_driver_init();

    // init the process manager
    process_init(AKAI_IDLE_BASE);

    // hook hernel interrupt handlers
    kernel_hook_interrupt(INT_KBD_SCAN, kbd_scan_hook);
    kernel_hook_interrupt(INT_TIMER_INTERVAL, timer_interval_hook);

    // mount filesystems
    fs_mount();

    timer_init();
    init = init_proc("INIT");
    if (!init) goto fail;

    //timer_on(); // interupts are still disabled
    _sbd(A.info.terminal + TERMINAL_KBD_CSR, KB_GLOBAL_EN | KB_IE_DOWN); // enable keyboard
    process_run(init, PARENT_KEEP_RUNNING);
fail:
    _trace(0xDEAD, 0XB00B);
    // TODO: kernel panic here
l:
    goto l;
}
