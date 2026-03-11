#include "hw.h"
#include "kernel.h"

static const char *exception_names[] = {
    "EXCEPTION_RESET0",
    "EXCEPTION_RESET",
    "EXCEPTION_BUS_ERROR",
    "EXCEPTION_ADDRESS_ERROR",
    "EXCEPTION_ILLEGAL_INSTRUCTION_TALEA",
    "EXCEPTION_DIVISION_ZERO",
    "EXCEPTION_PRIVILEGE_VIOLATION",
    "EXCEPTION_PAGE_FAULT",
    "EXCEPTION_ACCESS_VIOLATION_TALEA",
    "EXCEPTION_DEBUG_STEP",
    "EXCEPTION_OVERSPILL",
    "EXCEPTION_UNDERSPILL",
};

static u32 panic_pos = 0;
static u32 panic_att = 0x000FFF00UL;

static void panic_putc(char c)
{
    u32 term_size = A.info.textbuffer.w * A.info.textbuffer.h;

    if (c == '\n') {
        panic_pos += A.info.textbuffer.w - (panic_pos % A.info.textbuffer.w);
        return;
    }

    *((volatile u32 *)(A.info.textbuffer.addr) + panic_pos) = ((u32)c << 24) | panic_att;

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
    u32  term_size   = A.info.textbuffer.w * A.info.textbuffer.h;
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

    for (i = 0; i < A.info.textbuffer.h * A.info.textbuffer.w; i++) {
        *((u32 *)A.info.textbuffer.addr + i) = ((u32)' ' << 24) | panic_att;
    }

    panic_att |= TEXTMODE_ATT_BOLD;
    panic_puts("CRITICAL ERROR. AKAI HALTED. KERNEL PANIC!\n");

    panic_att &= ~(unsigned)(TEXTMODE_ATT_BLINK | TEXTMODE_ATT_BOLD);
    panic_puts("PROCESS: ");
    panic_puts(A.pr.curr->name);
    panic_puts(" PID ");
    panic_puti(A.pr.curr->pid, 3);
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
        if (i > 0 && i % 4 == 0) panic_putc('\n');
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
    u32 status = _lwd(AKAI_KERNEL_STATUS_SAVE);
    u32 pc     = _lwd(AKAI_KERNEL_PC_SAVE);

    if (status & CPU_STATUS_SUPERVISOR || A.pr.curr->pid == 0) {
        // this is a kernel panic.
        kernel_panic(vector, fault_addr, &A.pr.curr->ctx);
    }

    switch (vector) {
    case EXCEPTION_PAGE_FAULT: {
        u32  faddr       = fault_addr;
        u8   cause       = _lbud(REG_SYSTEM_FAULT_CAUSE);
        u8   access_type = cause & 0x3;
        bool unmapped    = (cause & CAUSE_UNMAPPED) >> 6;
        bool leaf        = (cause & CAUSE_LEAF) >> 7;

        if (unmapped && access_type != ACCESS_EXEC) {
            if (faddr < AKAI_PROCESS_STACK_TOP && faddr >= AKAI_PROCESS_STACK_END) {
                u8 *new_page = alloc_pages_contiguous(A.pr.curr->pid, 1);
                if (!new_page) {
                    miniprint("Coulf not allocate page. Killing %d\n", A.pr.curr->pid);
                    A.pr.curr->last_error = P_ERROR_STACK_GROW;
                    process_terminate(A.pr.curr->pid, WARRANT_OOM);
                    process_yield();
                }

                if (!map_active_pt_entry((u32)new_page, faddr & ~0xFFFU, PTE_U | PTE_RW)) {
                    miniprint("Coulf not map page. Killing %d\n", A.pr.curr->pid);
                    A.pr.curr->last_error = P_ERROR_STACK_GROW;
                    process_terminate(A.pr.curr->pid, WARRANT_OOM);
                    process_yield();
                }

                memset((u8 *)(faddr & ~0xFFFU), 0, PAGE_SIZE);
            } else {
                miniprint(
                    "Constraints for stack growth do not hold. (faddr: %x, cause: %x). Killing %d\n",
                    faddr, cause, A.pr.curr->pid);

                process_terminate(A.pr.curr->pid, WARRANT_SEG);
                process_yield();
            }
        } else {
            miniprint(
                "Constraints for stack growth do not hold. (faddr: %x cause: %x, access: %x, unmapped: %d). Killing %d\n",
                faddr, cause, access_type, unmapped, A.pr.curr->pid);
            process_terminate(A.pr.curr->pid, WARRANT_SEG);
            process_yield();
        }
        break;
    }
    default:
        miniprint("Exception %s in process %s. Killing\n", exception_names[vector], A.pr.curr->name);
        A.pr.curr->exit_code = -vector;
        _trace(0xDEAD00B, A.pr.curr->pid, vector);
        process_terminate(A.pr.curr->pid, WARRANT_EXCEPTION);
        process_yield();
        break;
    }

    _swd(AKAI_KERNEL_STATUS_RESTORE, status);
    _swd(AKAI_KERNEL_PC_RESTORE, pc);
}
