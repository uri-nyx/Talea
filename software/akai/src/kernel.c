/**
 * A very simple DOS-style kernel for the Taleä Computer System
 */

#include "kernel.h"
#include "hw.h"
#include "init_code.h"
#include "kstdarg.h"

struct Akai A;

extern void *_bss_start;
extern void *_data_start;

i32 vdev_ctl()
{
    return 0;
}

u8 vdev_in()
{
    return 0;
}

i32 vdev_out()
{
    return 0;
}

i32 vdev_reset()
{
    return 0;
}

void emit(c)
{
    static u32 vpos      = 0;
    u32        term_size = A.info.textbuffer.w * A.info.textbuffer.h;

    if (c == '\n') {
        vpos += A.info.textbuffer.w - (vpos % A.info.textbuffer.w);
        return;
    }

    *((volatile u32 *)(AKAI_TEXTBUFFER) + vpos) = ((u32)c << 24) | 0x000f0000UL;

    vpos = (vpos + 1) % (term_size);
}

void puts(const char *str)
{
    char *p = (char *)str;
    char  c;
    while ((c = *p++)) {
        emit(c);
    }
}

void miniprint(const char *fmt, ...)
{
    static const char *null_str = "(NULL)";
    char               c;
    char               specifier = 0;

    va_list ap;
    va_start(ap, fmt);

    while ((c = *fmt++)) {
        if (c == '%') {
            u8   width    = 0;
            bool pad_zero = false;

            specifier = *fmt++;
            if (!specifier) return; /* end printing if no specifier found */

            if (specifier == '0') {
                pad_zero  = true;
                specifier = *fmt++;
            }

            if (!specifier) return; /* end printing if no specifier found */

            if (specifier >= '0' && specifier <= '9') {
                width     = (width * 10) + (specifier - '0');
                specifier = *fmt++;
            }

            if (!specifier) return; /* end printing if no specifier found */

            switch (specifier) {
            case 's': {
                char *s = va_arg(ap, char *);
                if (!s) s = (char *)null_str; /* print (NULL) if s is NULL */
                while (*s) emit(*s++);
                break;
            }
            case 'c': {
                char chr = va_arg(ap, int);
                emit(chr);
                break;
            }
            case 'x': {
                char hex_chars[] = "0123456789ABCDEF";
                /* a 32 bit int fits in 8 characters */
                char buf[16];
                int  i = 0;
                u32  x = va_arg(ap, int);
                u8   content_len;

                if (x == 0) {
                    buf[i++] = '0';
                    goto emit_hex;
                }

                /* Extract digits from least significant to most significant */
                while (x > 0) {
                    buf[i++] = hex_chars[x & 0xF];
                    x >>= 4;
                }
emit_hex:
                content_len = i;
                while (content_len < width) {
                    emit(pad_zero ? '0' : ' ');
                    content_len++;
                }

                /* Print the buffer in reverse */
                while (i > 0) emit(buf[--i]);
                break;
            }
            case 'd': {
                char buf[16]; /* Enough for -2,147,483,648 plus null (no commas) */
                i32  i = 0;
                u32  num;
                int  n           = va_arg(ap, int);
                int  content_len = 0;
                bool is_neg      = n < 0;

                /* Handle Negative Numbers */
                if (is_neg) {
                    /* Use unsigned to avoid overflow issues with INT_MIN */
                    num = (u32)(-n);
                } else {
                    num = (u32)n;
                }

                /* Extract digits (Right to Left) */
                if (num == 0) {
                    buf[i++] = '0';
                    goto emit_dec;
                }

                while (num > 0) {
                    buf[i++] = (num % 10) + '0';
                    num /= 10;
                }

                if (pad_zero && is_neg) emit('-');

emit_dec:
                content_len = i + (is_neg ? 1 : 0);

                while (content_len < width) {
                    emit(pad_zero ? '0' : ' ');
                    content_len++;
                }

                if (!pad_zero && is_neg) emit('-');

                /* Print buffer in reverse (Left to Right) */
                while (i > 0) emit(buf[--i]);
                break;
            }
            case '%': {
                /* escaped % */
                emit('%');
                break;
            }
            default: break;
            }
        } else {
            emit(c);
        }
    }

    va_end(ap);
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
    _ssreg((sreg & ~0x000FFF00U) | (AKAI_PDT_BASE >> 4) << 8);
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
    FRESULT res;

    usize i;
    for (i = 0; i < 3; i++) {
        res = f_mount(&A.fs[i], drive_labels[i], 0);
        if (res != FR_OK) {
            miniprint("[KERNEL] Fatal error mounting filesystems. halt (%d)\n", res);
            _halt();
        }
    }

    res = f_chdrive("/B/"); // drive B, universal path format
    if (res != FR_OK) {
        miniprint("[KERNEL] Fatal error setting drive B. halt (%d)\n", res);
        _halt();
    }
}

static void timer_init(void)
{
    // set timer to interval at 100hz
    u8 csr = _lbud(TERMINAL_TIMER_CSR);
    _sbd(TERMINAL_TIMER_PRESCALER, AKAI_TIMER_PRESCALER_1OOHZ);
    _shd(TERMINAL_TIMER_INTERVAL, AKAI_TIMER_INTERVAL_100HZ);
    _sbd(TERMINAL_TIMER_CSR, csr | TIM_INTERVAL_EN);
}

void timer_on(void)
{
    u8 csr = _lbud(TERMINAL_TIMER_CSR);
    _sbd(TERMINAL_TIMER_CSR, csr | TIM_GLOBAL_EN);
}

void timer_off(void)
{
    u8 csr = _lbud(TERMINAL_TIMER_CSR);
    _sbd(TERMINAL_TIMER_CSR, csr & ~TIM_GLOBAL_EN);
}

static u32 uptime(void)
{
    _sbd(REG_SYSTEM_MINUTE, TALEA_SYSTEM_MILLIS_MODE); /* Set the COUNTER MODE */
    return _lwd(REG_SYSTEM_MINUTE);
}

void timer_interval_hook(u32 *win, struct IPCMessage *msg_out)
{
    u32          status;
    static usize ticks = 0;

    status = _lwd(AKAI_KERNEL_STATUS_SAVE);

    ticks++;
    A.uptime = uptime() - A.time_start;
    _trace(0x1888, 2);

    if (ticks % 2 && (A.mous.mode & MOUS_REPORT)) {
        _trace(0x18880, 1);
        mous_inject_event();
    }

    // TODO: IPC message out
    if (!(status & CPU_STATUS_SUPERVISOR)) {
        _trace(0x1888, 3);
        A.pr.curr->user_ticks++;
        process_yield();
    } else {
        A.pr.curr->system_ticks++;
    }
}

static ProcessPID init_proc(const char *name)
{
    struct Process *init;
    u32             result = 0;
    u8             *code, *stack;
    u32            *work;

    ProcessPID init_pid = process_create(name, (ProcessEntry)AKAI_PROCESS_BASE);

    if (!init_pid) return 0;

    // manual exec
    init  = &A.pr.proc[init_pid];
    code  = alloc_pages_contiguous(init_pid, 1);
    stack = alloc_pages_contiguous(init_pid, 1);

    if (!code || !stack) {
        process_terminate(init_pid, WARRANT_NONE);
        return 0;
    }

    _trace(0x888555, 0x777888, code, init_bin_len);

    // map its page tables to us, so we can write to it
    work = (u32 *)map_kernel_work_area((u32)init->page_tables[0]);
    if (!work) {
        process_terminate(init_pid, WARRANT_NONE);
        return 0;
    }

    // give r/w/x because its a init. otherwise this is very rare
    map_pt_entry(work, AKAI_PROCESS_BASE >> 12, (u32)code, PTE_U | PTE_RWX);

    work = (u32 *)remap_kernel_work_area((u32)init->page_tables[3]);
    map_pt_entry(work, (AKAI_PROCESS_STACK_TOP >> 12) - 1, (u32)stack, PTE_U | PTE_RW);

    work = (u32 *)remap_kernel_work_area((u32)stack);
    memset(work, 0, PAGE_SIZE);

    // map the code for us so we can write to it.
    work = (u32 *)remap_kernel_work_area((u32)code);
    _trace(0x888666, 0x777888, code, init_bin_len);
    memcpy((u8 *)work, init_bin, init_bin_len);

    unmap_kernel_work_area();
    tlb_flush();

    init->ctx.usp = AKAI_PROCESS_STACK_TOP;
    init->brk     = (void *)(AKAI_PROCESS_BASE + PAGE_SIZE);

    // intialize standard streams
    init->stdin.res_type  = HW;
    init->stdout.res_type = HW;
    init->stderr.res_type = HW;

    return init_pid;
}

static void lineages_init(void)
{
    usize i;
    for (i = 0; i < _DEV_NUM; i++) {
        A.devices[i].deed.depth = 0;
        memset(A.devices[i].deed.lineage, 0, sizeof(A.devices[i].deed.lineage));
    }
}

void kmain(u16 sys_info_data_addr)
{
    ProcessPID init;
    u32        usp;
    memset(&A, 0, sizeof(A));
    // memset(_bss_start, 0, (AKAI_KERNEL_STACK_TOP - PAGE_SIZE) - (u32)_bss_start);
    // start time is now
    A.time_start = uptime();
    A.uptime     = uptime();

    // Copy the struct handed down by the firmware
    _copydm(sys_info_data_addr, &A.info, sizeof(A.info));

    miniprint("[KERNEL] [%09d]    Akai kernel boot!     [OK]\n", A.time_start);

    // init mmu
    mmu_init();

    miniprint("[KERNEL] [%09d]    MMU initialized        [OK]\n", uptime());

    // init the physical memory manager
    mem_init();

    miniprint("[KERNEL] [%09d]    PMM initialized        [OK]\n", uptime());

    lineages_init();
    miniprint("[KERNEL] [%09d]    Lineages initialized   [OK]\n", uptime());

    video_driver_init();
    miniprint("[KERNEL] [%09d]    Video initialized      [OK]\n", uptime());

    kbd_driver_init();
    miniprint("[KERNEL] [%09d]    Keyboard initialized   [OK]\n", uptime());

    ser_driver_init();
    miniprint("[KERNEL] [%09d]    Serial initialized     [OK]\n", uptime());

    mous_driver_init();
    miniprint("[KERNEL] [%09d]    Mous   initialized     [OK]\n", uptime());

    // init the process manager
    process_init(AKAI_IDLE_BASE);
    miniprint("[KERNEL] [%09d]    Scheduler initialized  [OK]\n", uptime());
    _trace(0xAFF);

    // hook hernel interrupt handlers
    interrupt_init();
    kernel_hook_interrupt(INT_KBD_SCAN, kbd_scan_hook);
    kernel_hook_interrupt(INT_TIMER_INTERVAL, timer_interval_hook);
    kernel_hook_interrupt(INT_VIDEO_VBLANK, fb_vblank_hook);
    miniprint("[KERNEL] [%09d]    Interrupts initialized [OK]\n", uptime());

    // mount filesystems
    fs_mount();
    miniprint("[KERNEL] [%09d]    Mounted filesystems    [OK]\n", uptime());

    timer_init();
    miniprint("[KERNEL] [%09d]    Timers initialized     [OK]\n", uptime());

    init = init_proc("INIT");
    if (!init) goto fail;

    miniprint("[KERNEL] [%09d]    INIT process loaded    [OK]\n", uptime());

    timer_on(); // interupts are still disabled
    _sbd(A.info.terminal + TERMINAL_KBD_CSR, KB_GLOBAL_EN | KB_IE_DOWN); // enable keyboard

    miniprint("[KERNEL] [%09d]    Launching INIT!\n", uptime());
    process_run(init, PARENT_KEEP_RUNNING);
fail:
    _trace(0xDEAD, 0XB00B);
    miniprint("[KERNEL] [%09d]    Error in boot sequence. Halting.\n", uptime());
    // TODO: kernel panic here
l:
    goto l;
}
