#include "../include/handlers.h"
#include "../include/hw.h"
#include "../include/process.h"

static const char *exception_names[] = {
    "EXCEPTION_RESET0",
    "EXCEPTION_RESET",         "EXCEPTION_BUS_ERROR",
    "EXCEPTION_ADDRESS_ERROR", "EXCEPTION_ILLEGAL_INSTRUCTION_TALEA",
    "EXCEPTION_DIVISION_ZERO", "EXCEPTION_PRIVILEGE_VIOLATION",
    "EXCEPTION_PAGE_FAULT",    "EXCEPTION_ACCESS_VIOLATION_TALEA",
    "EXCEPTION_DEBUG_STEP",    "EXCEPTION_OVERSPILL",
    "EXCEPTION_UNDERSPILL",
};

static u32 panic_pos = 0;
static u32 panic_att = 0x000FFF00UL;

static void panic_putc(char c)
{
    u32 term_size = sys.textbuffer.w * sys.textbuffer.h;

    if (c == '\n') {
        panic_pos += sys.textbuffer.w - (panic_pos % sys.textbuffer.w);
        return;
    }

    *((volatile u32 *)(sys.textbuffer.addr) + panic_pos) = ((u32)c << 24) | panic_att;

    panic_pos = (panic_pos + 1) % (term_size);
}

static void panic_puts(const char *str)
{
    char *p = (char *)str;
    char  c;
    while (c = *p++) {
        panic_putc(c);
    }
}

static void panic_putx(u32 x)
{
    u32  term_size   = sys.textbuffer.w * sys.textbuffer.h;
    char hex_chars[] = "0123456789ABCDEF";

    /* a 32 bit int fits in 8 characters */
    char buf[16];
    int  i = 0;
    u8   content_len;

    if (x == 0) {
        buf[i++] = '0';
        goto emit_hex;
    }

    while (x > 0) {
        buf[i++] = hex_chars[x & 0xF];
        x >>= 4;
    }

emit_hex:
    content_len = i;
    while (content_len < 8) {
        panic_putc('0');
        content_len++;
    }

    /* Print the buffer in reverse */
    while (i > 0) panic_putc(buf[--i]);
}

static void panic_puti(i32 n, u8 pad)
{
    char buf[16]; /* Enough for -2,147,483,648 plus null (no commas) */
    i32  i = 0;
    u32  num;
    bool is_neg = n < 0;

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

emit_dec:
    if (is_neg) panic_putc('-');

    while (i < pad) {
        panic_putc('0');
        pad--; 
    }

    /* Print buffer in reverse (Left to Right) */
    while (i > 0) panic_putc(buf[--i]);
}

void kernel_panic(u8 vector, u32 fault_addr, struct ThreadCtx *ctx)
{
    usize i;
    u32   sreg;
    _cli(); // disable interrupts
    sreg = _gsreg();
    _ssreg(sreg & ~CPU_STATUS_MMU_ENABLE); // disable mmu

    _trace(0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF);
    _trace(0XBADC0DE, vector, fault_addr, 0xDEADBEEF);
    for (i = 0; i < NUM_REGS; i++) {
        _trace(0xDEADBEEF, i, ctx->regs[i], 0xDEADBEEF);
    }
    _trace(ctx->pc, ctx->status, ctx->usp, ctx->wp);
    _trace(0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF);

    for (i = 0; i < sys.textbuffer.h * sys.textbuffer.w; i++) {
        *((u32*)sys.textbuffer.addr + i) = ((u32)' ' << 24) | panic_att;
    }

    panic_att |= TEXTMODE_ATT_BOLD;
    panic_puts("CRITICAL ERROR. AKAI HALTED. KERNEL PANIC!\n");

    panic_att &= ~(TEXTMODE_ATT_BLINK | TEXTMODE_ATT_BOLD);
    panic_puts("PROCESS: ");
    panic_puts(processes.curr->name);
    panic_puts(" PID ");
    panic_puti(processes.curr->pid, 3);
    panic_puts("\nException ");
    panic_puts(exception_names[vector]);
    panic_putc(' ');
    panic_putx(vector);
    panic_puts(" at ");
    panic_putx(fault_addr);
    panic_putc('\n');

    panic_puts("Registers (window ");
    panic_putx(ctx->wp);
    panic_puts(")\n");

    panic_puts("PC: ");
    panic_putx(ctx->pc);
    panic_putc(' ');
    panic_puts("STATUS: ");
    panic_putx(ctx->status);
    panic_putc(' ');
    panic_puts("SP: ");
    panic_putx(ctx->usp);
    panic_putc('\n');

    for (i = 0; i < NUM_REGS; i++) {
        panic_putc('R');
        panic_puti(i, 2);
        panic_puts(": ");
        panic_putx(ctx->regs[i]);
        panic_puts("  ");
        if (i % 4 == 0) panic_putc('\n');
    }

    panic_putc('\n');
    panic_putc('\n');

    panic_att |= TEXTMODE_ATT_BLINK | TEXTMODE_ATT_BOLD;
    panic_puts("KERNEL PANIC! KERNEL PANIC! KERNEL PANIC! KERNEL PANIC!\n");

halt:
    goto halt;
}

void akai_exception(u8 vector, u32 fault_addr)
{
    u32        status;
    static u32 win[32];
    status = _lwd(AKAI_KERNEL_STATUS_SAVE);

    save_ctx(processes.curr);

    if (status & CPU_STATUS_SUPERVISOR || processes.curr->pid == 0) {
        // this is a kernel panic.
        kernel_panic(vector, fault_addr, &processes.curr->ctx);
    }

    load_window(sirius_cwp - 1, &win);

    puts(&sys, "Exception: ");
    puts(&sys, exception_names[vector]);
    puts(&sys, " in process ");
    puts(&sys, processes.curr->name);
    puts(&sys, "\n");

    processes.curr->exit_code = -vector;

    switch (vector) {
    default:
        process_terminate(&processes, processes.curr->pid);
        process_yield(&processes);
        break;
    }
}