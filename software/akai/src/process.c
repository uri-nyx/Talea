#include "../include/process.h"
#include "../include/hw.h"
#include "../include/mem.h"
#include "../include/mmu.h"
#include "libsirius/devices.h"

extern u32                 *kernel_pt;
extern u32                 *video_pt;
extern struct PhysicalPages pages;

/*
_start:
    trace x0,x0,x0,x2
    li x12, 1 	# SYSCALL_YIELD
    syscall x0, 0x40
    j _start
*/
static u8 idle_code[] = { 0x0a, 0x00, 0x00, 0x40, 0xa6, 0xc0, 0x00, 0x01,
                          0x04, 0x00, 0x00, 0x40, 0x40, 0x0f, 0xff, 0xfd };

void process_init(struct Processes *p, u32 idle_base)
{
    const char name[10] = "AKAI IDLE";
    u32       *code;

    memset(p, 0, sizeof(struct Processes));

    p->count       = 1;
    p->proc[0].pid = 0;

    p->proc[0].ctx.pc     = idle_base;
    p->proc[0].ctx.status = AKAI_PROCESS_STATUS;
    // the idle task should not need a stack
    p->proc[0].ctx.usp = idle_base + 255;
    p->proc[0].ctx.wp  = 0;

    memset(p->proc[0].ctx.regs, 0, NUM_REGS * sizeof(u32));

    p->proc[0].parent = 0;
    p->proc[0].pid    = 0;
    p->proc[0].entry  = (ProcessEntry)idle_base;

    code = alloc_pages_contiguous(&pages, 0, 1);
    if (!code) {
        // panic
        _trace(0xdeadbee0);
l:
        goto l;
    }

    p->proc[0].pdt = AKAI_PDT_BASE;

    p->proc[0].page_tables[0] = kernel_pt;
    p->proc[0].page_tables[1] = NULL;
    p->proc[0].page_tables[2] = NULL;
    p->proc[0].page_tables[3] = video_pt;

    map_pt_entry(video_pt, idle_base >> 12, (u32)code, PTE_V | PTE_U | PTE_X | PTE_R | PTE_W);
    memcpy((u8 *)idle_base, idle_code, sizeof(idle_code));
    tlb_flush();

    memcpy(p->proc[0].name, name, 10);

    p->proc[0].state   = NEWBORN;
    p->proc[0].brk     = NULL; // idle must not have a heap
    p->proc[0].flags   = 0;
    p->proc->exit_code = 0;

    save_ctx(&p->proc[0]);
    p->curr = &p->proc[0];
}

// returns 0 (the kernel PID) on error. Make user to init the process module (to create the Kernel
// process) first
ProcessPID process_create(struct Processes *p, const char *name, ProcessEntry entry)
{
    usize      i;
    ProcessPID pid;
    u32       *pt0;

    if (p->count >= MAX_PROCESS) return 0;

    // allocate a pid
    for (i = 0; i < MAX_PROCESS; i++) {
        if (p->proc[i].state == FREE) {
            pid = i;
            _trace(0xf2eeeee, pid);
            p->count++;
            break;
        }
    }

    // clear the struct
    memset(&p->proc[pid], 0, sizeof(struct Process));

    // Give page tables. for now, only page table 0 (0-4mb),
    // and page table 3 is shared (pointer to video_pt)
    // Do it here, because if it bails we have to undo the PID allocation
    pt0 = alloc_pages_contiguous(&pages, pid, 1);

    if (!pt0) {
        p->proc[pid].state = FREE;
        // how do I decrement count here?
        p->count--;
        _trace(0xdead1);
        return 0;
    }

    // Set the pid
    p->proc[pid].pid = pid;

    // Set the pdt as pid AKAI_BASE_PDT + (0-255) << 4
    p->proc[pid].pdt = AKAI_PDT_BASE + ((u16)pid << 4);

    // map pt0 and video_pt in the page directory.
    // the index in the page directory is (pid << 2) + pt_number,
    // since the kernel has pdt pointer to AKAI_PDT_BASE
    map_pdt_entry(((u16)pid << 2) + 0, (u32)pt0, PTE_V);

    // map the video_pt in the same way
    map_pdt_entry(((u16)pid << 2) + 3, (u32)video_pt, PTE_V);

    // assing pt0 to pid.page_tables[0]
    p->proc[pid].page_tables[0] = pt0;

    // map the pt to the kernel to copy and access it
    map_pt_entry(kernel_pt, AKAI_PROCESS_PT0 >> 12, (u32)pt0, PTE_V | PTE_R | PTE_W);
    // map the pt into itself
    memset((u8 *)AKAI_PROCESS_PT0, 0, PAGE_SIZE);
    memcpy((u8 *)AKAI_PROCESS_PT0, kernel_pt, PAGE_SIZE);
    map_pt_entry((u32 *)AKAI_PROCESS_PT0, AKAI_PROCESS_PT0 >> 12, (u32)pt0, PTE_V | PTE_R | PTE_W);
    // assign video_pt to pid.page_tabls[3] and map it in pt0 to the PT3 address
    p->proc[pid].page_tables[3] = video_pt;
    map_pt_entry((u32 *)AKAI_PROCESS_PT0, AKAI_PROCESS_PT3 >> 12, (u32)video_pt,
                 PTE_V | PTE_R | PTE_W);

    _trace(0x555);
    // unmap from kernel
    unmap_pt_entry(kernel_pt, AKAI_PROCESS_PT0 >> 12);
    tlb_flush();

    // Set it to NEWBORN
    p->proc[pid].state = NEWBORN;

    // Set flags. start as normal program
    p->proc[pid].flags = 0;

    // Set the name
    memcpy(p->proc[pid].name, name, 10);
    p->proc[pid].name[9] = 0;

    // Set the entry point
    p->proc[pid].entry = entry;

    // Fill regs and pc, usp, and status
    save_ctx(&p->proc[pid]); // TODO: if is_system, the usp and the status should be different

    // Set paretn as curr process
    p->proc[pid].parent = p->curr->pid;

    // brk left to zero, only initialize on exec()

    return pid;
}

void process_run(struct Processes *p, ProcessPID pid, enum ParentState parent_new_state)
{
    u32 sreg;

    struct Process *target = &p->proc[pid];
    struct Process *parent = p->curr;

    if (parent && parent->pid != pid) {
        if (parent->pid == 0 || parent->state == NEWBORN) goto skip;

        switch (parent_new_state) {
        case PARENT_KEEP_RUNNING: break;
        case PARENT_WAIT: process_wait(p, p->curr->pid); break;
        case PARENT_DIE: break;
        case PARENT_DETACH: break;
        default: break;
        }
    }

skip:

    p->curr = target;

    // switch pdt. There should be no problem, since the kernel is mapped there too
    sreg = _gsreg();
    sreg &= ~(0x000FFF00);
    sreg |= (p->curr->pdt >> 4) << 8;
    _ssreg(sreg);
    tlb_flush();

    if (target->state == NEWBORN) {
        process_set_ready(p, pid);
        _load_and_switch(p->curr->ctx.pc, p->curr->ctx.status, p->curr->ctx.regs);
    } else {
        _trace(0xf1A, p->curr->ctx.pc, p->curr->ctx.status);
        restore_ctx(p->curr);
        _trace(0xf10, p->curr->ctx.pc, p->curr->ctx.status);
        _switch(p->curr->ctx.pc, p->curr->ctx.status);
    }
}

u32 process_stop(struct Processes *p, ProcessPID pid)
{
    process_set_ready(p, pid);
    return save_ctx(&p->proc[pid]);
}

u32 process_wait(struct Processes *p, ProcessPID pid)
{
    p->proc[pid].state = WAITING;
    return save_ctx(&p->proc[pid]);
}

void process_set_ready(struct Processes *p, ProcessPID pid)
{
    p->proc[pid].state = READY;
}

void process_kill(struct Processes *p, ProcessPID pid)
{
    if (pid == 0) return; // idle process is not killable
    p->proc[pid].state = FREE;
    free_pages_by_owner(&pages, pid);
    p->count--;
}

void process_yield(struct Processes *p)
{
    usize      i;
    ProcessPID n;
    // some scheduler logic would be neat. for now, KISS, first ready process runs

    if (p->curr->state == RUNNING) p->curr->state = READY;

    n = (p->curr->pid + 1) % MAX_PROCESS;

    for (i = 0; i < MAX_PROCESS; i++) {
        ProcessPID k = (n + i) % MAX_PROCESS;
        if (p->proc[k].state == READY && p->proc[k].pid != 0) process_run(p, k);
    }

    // if we got here, no processes to run, so we go to idle
    if (p->curr->state == READY)
        process_run(p, p->curr->pid);
    else
        process_run(p, 0);
}

void *load_window(u8 wp, void *buf)
{
    usize i;
    u32  *regs = (u32 *)buf;

    _sbd(REG_SYSTEM_WIN_SEL, wp);
    _sbd(REG_SYSTEM_WIN_OP, TALEA_SYSTEM_WIN_OP_STORE); // get the window in WIN_BUFF

    for (i = 0; i < 32; i++) {
        regs[i] = _lwd(REG_SYSTEM_WIN_BUFF + (i * 4));
    }
    return regs;
}

void store_window(u8 wp, void *buf)
{
    usize i;
    u32  *regs = (u32 *)buf;

    for (i = 0; i < 32; i++) {
        _swd(REG_SYSTEM_WIN_BUFF + (i * 4), regs[i]);
    }

    _sbd(REG_SYSTEM_WIN_SEL, wp);
    _sbd(REG_SYSTEM_WIN_OP, TALEA_SYSTEM_WIN_OP_LOAD); // get the window in register file
}

u32 save_ctx(struct Process *p)
{
    if (p->state == NEWBORN) {
        // process being created in the kernel
        u32 stat;
        load_window(p->ctx.wp, p->ctx.regs);
        p->ctx.pc     = (u32)p->entry;
        p->ctx.usp    = AKAI_PROCESS_STACK;
        p->ctx.status = ((p->pdt >> 4) << 8) | AKAI_PROCESS_STATUS;
    } else if (sirius_cwp == 0 && p->pid != 0 && p->state != NEWBORN) {
        // panic here
        _trace(0xdeadbeef, p->state, p->pid, 1);
l:
        goto l;
    } else {
        _trace(0x67, p->pid);
        p->ctx.wp = sirius_cwp - 1;
        load_window(p->ctx.wp, p->ctx.regs);
        // Assume it was saved upon entering the kernel. Otherwise this will fault
        p->ctx.pc = _lwd(AKAI_KERNEL_PC_SAVE); // 0x1000); // save user pc
        _trace(0x1001, p->ctx.pc);
        p->ctx.status = _lwd(AKAI_KERNEL_STATUS_SAVE); // 0x1004); // save user status register
        p->ctx.usp    = _lwd(REG_SYSTEM_USP);
    }

    return 0;
}

void restore_ctx(struct Process *p)
{
    if (p->state == NEWBORN) {
        // false restore, to transfer to usermode
        _trace(0xfa1, p->pid);
        save_ctx(p);                      // we just want a clean register file<
        _swd(REG_SYSTEM_USP, p->ctx.usp); // store saved usp
    } else {
        _trace(0xfa0, p->pid);
        store_window(p->ctx.wp, p->ctx.regs);
        _swd(REG_SYSTEM_USP, p->ctx.usp); // store saved usp
    }
};
