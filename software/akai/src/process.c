#include "../include/process.h"
#include "../include/handlers.h"
#include "../include/hw.h"
#include "../include/mem.h"
#include "../include/mmu.h"
#include "libsirius/devices.h"

extern u32                 *kernel_pt;
extern u32                 *high_mem_pt;
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

    _trace(0xdaaaaa, idle_base);

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
        // TODO: implement -> kernel_panic_internal("In process init: could not allocate pages for
        // idle process.");
        _trace(0xDEADC0DE);
l:
        goto l;
    }

    p->proc[0].pdt = AKAI_PDT_BASE;

    p->proc[0].page_tables[0] = kernel_pt;
    p->proc[0].page_tables[1] = NULL;
    p->proc[0].page_tables[2] = NULL;
    p->proc[0].page_tables[3] = high_mem_pt;

    map_pt_entry(high_mem_pt, idle_base >> 12, (u32)code, PTE_V | PTE_U | PTE_X | PTE_R | PTE_W);
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
    u32       *pt0, *pt3;

    // reap zombies
    for (i = 0; i < MAX_PROCESS; i++) {
        if (p->proc[i].state == ZOMBIE && p->proc[i].parent == 0) {
            process_reap(p, i);
        }
    }

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

    // Give page tables, for the high memory and the kernel/process mappings
    // Do it here, because if it bails we have to undo the PID allocation
    pt0 = alloc_pages_contiguous(&pages, pid, 1);
    pt3 = alloc_pages_contiguous(&pages, pid, 1);

    if (!pt0 || !pt3) {
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

    // map pt0 and high_mem_pt in the page directory.
    // the index in the page directory is (pid << 2) + pt_number,
    // since the kernel has pdt pointer to AKAI_PDT_BASE
    map_pdt_entry(((u16)pid << 2) + 0, (u32)pt0, PTE_V);

    // map the high_mem_pt in the same way
    map_pdt_entry(((u16)pid << 2) + 3, (u32)pt3, PTE_V);

    // assing pt0 to pid.page_tables[0]
    p->proc[pid].page_tables[0] = pt0;
    // assign pt3 to pid.page_tabls[3] and map it in pt0 to the PT3 address
    p->proc[pid].page_tables[3] = pt3;

    // map the pt to the kernel to copy and access it
    map_pt_entry(kernel_pt, AKAI_PROCESS_PT0 >> 12, (u32)pt0, PTE_V | PTE_R | PTE_W);
    map_pt_entry(kernel_pt, AKAI_PROCESS_PT3 >> 12, (u32)pt3, PTE_V | PTE_R | PTE_W);
    tlb_flush();

    // map the pt into itself
    memset((u8 *)AKAI_PROCESS_PT0, 0, PAGE_SIZE);
    memcpy((u8 *)AKAI_PROCESS_PT0, kernel_pt, PAGE_SIZE);
    memset((u8 *)AKAI_PROCESS_PT3, 0, PAGE_SIZE);
    memcpy((u8 *)AKAI_PROCESS_PT3, high_mem_pt, PAGE_SIZE);

    map_pt_entry((u32 *)AKAI_PROCESS_PT0, AKAI_PROCESS_PT0 >> 12, (u32)pt0, PTE_V | PTE_R | PTE_W);
    map_pt_entry((u32 *)AKAI_PROCESS_PT0, AKAI_PROCESS_PT3 >> 12, (u32)pt3, PTE_V | PTE_R | PTE_W);

    _trace(0x555);
    // unmap from kernel
    unmap_pt_entry(kernel_pt, AKAI_PROCESS_PT0 >> 12);
    unmap_pt_entry(kernel_pt, AKAI_PROCESS_PT3 >> 12);
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

bool process_check_addr(struct Processes *p, ProcessPID pid, u32 addr, u32 acces_flags,
                        bool executable)
{
    u32 addr_pt   = addr >> 22; // this is the page table number
    u32 mapped_pt = (AKAI_PROCESS_PT0 + (addr_pt * 0x1000));
    u32 entry_idx = (addr >> 12) & 0x3FF;
    u32 entry     = 0;

    _trace(0xfbbba, addr_pt, addr, mapped_pt);
    if (!addr) return false;
    if (addr >= AKAI_IDLE_BASE && addr < AKAI_IDLE_BASE + PAGE_SIZE) return false;
    if (executable && ((addr & 0x3) != 0)) return false;
    if (addr_pt > 3) return false;
    if (!p->proc[pid].page_tables[addr_pt]) return false;

    // map the pt to the kernel to read the entry
    map_pt_entry(kernel_pt, mapped_pt >> 12, (u32)p->proc[pid].page_tables[addr_pt], PTE_V | PTE_R);
    tlb_flush();
    // read the entry
    entry = ((u32 *)mapped_pt)[entry_idx];
    _trace(0xfbbba, entry, entry_idx);
    // unmap from kernel
    unmap_pt_entry(kernel_pt, mapped_pt >> 12);
    tlb_flush();

    if ((entry & acces_flags) != acces_flags) return false;

    return true;
}

static void inject_event(struct Processes *p, u32 active_events)
{
    u8 semaphore;

    // load semaphore
    semaphore = *(u8 *)AKAI_IPC_KERNEL_IN;

    if (semaphore == 0) {
        // inject!
        if (process_check_addr(p, p->curr->pid, (u32)p->curr->event_handler, PTE_V | PTE_U | PTE_X,
                               true)) {
            p->curr->ctx.regs[31] = p->curr->ctx.pc;
            _trace(0xf19, p->curr->ctx.pc, p->curr->ctx.regs[31], p->curr->ctx.status);
            p->curr->ctx.pc = (u32)p->curr->event_handler;
            semaphore       = 1;
        }
    } else if (semaphore == 1) {
        // never inject! just clear active events
    } else if (semaphore == 2) {
        // inject next time
        semaphore = 0;
    } else {
        // cooldown
        semaphore--;
    }

    *(u8 *)AKAI_IPC_KERNEL_IN = semaphore;

    p->curr->pending_events &= ~active_events;
}

void process_run(struct Processes *p, ProcessPID pid, enum ParentState parent_new_state)
{
    u32 sreg;

    struct Process *target = &p->proc[pid];

    _trace(0xAAAAAAAA, pid, p->curr->pid, target->state);

    if (target->state == ZOMBIE) {
        _trace(0xBADC0DE, pid, 0XDEAD);
        return;
    }

    if (p->curr && p->curr->pid != pid) {
        if (p->curr->pid == 0 || p->curr->state == NEWBORN) goto skip;
        if (target->state == NEWBORN) {
            // should, in the odd case that a process creates,
            // and another runs for the first time, the latter process adopt the child?
            struct Process *parent = &p->proc[p->proc[pid].parent];

            if (target->pid == parent->pid) goto skip;

            switch (parent_new_state) {
            case PARENT_WAIT: process_wait(p, parent->pid); break;
            case PARENT_DIE: process_terminate(p, parent->pid); break;
            case PARENT_DETACH:
                process_stop(p, parent->pid);
                target->parent = 0;
                break;
            case PARENT_KEEP_RUNNING:
            default: process_stop(p, parent->pid); break;
            }
        } else {
            process_stop(p, p->curr->pid);
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
        // a newborn cannot possibly run an event
        process_set_ready(p, pid);
        _load_and_switch(p->curr->ctx.pc, p->curr->ctx.status, p->curr->ctx.regs);
    } else {
        u32 active_events = p->curr->event_mask & p->curr->pending_events;

        _trace(0xf1A, p->curr->ctx.pc, p->curr->ctx.status);

        if (active_events) inject_event(p, active_events);

        restore_ctx(p->curr);
        _trace(0xf10, p->curr->ctx.pc, p->curr->ctx.status, p->curr->ctx.usp);
        _switch(p->curr->ctx.pc, p->curr->ctx.status);
    }
}

u32 process_stop(struct Processes *p, ProcessPID pid)
{
    if (p->proc[pid].state == ZOMBIE) return;
    process_set_ready(p, pid);
    return save_ctx(&p->proc[pid]);
}

u32 process_wait(struct Processes *p, ProcessPID pid)
{
    if (p->proc[pid].state == ZOMBIE) return;
    p->proc[pid].state = WAITING;
    return save_ctx(&p->proc[pid]);
}

void process_set_ready(struct Processes *p, ProcessPID pid)
{
    if (p->proc[pid].state == ZOMBIE) return;
    p->proc[pid].state = READY;
}

void process_terminate(struct Processes *p, ProcessPID pid)
{
    usize i;
    if (pid == 0) return; // idle process is not killable

    for (i = 0; i < MAX_PROCESS; i++) {
        if (p->proc[i].parent == pid) p->proc[i].parent = 0;
    }
    free_pages_by_owner(&pages, pid);

    if (p->proc[pid].parent == 0) {
        p->proc[pid].state = FREE;
        p->count--;
    } else {
        p->proc[pid].state = ZOMBIE;
    }
}

void process_reap(struct Processes *p, ProcessPID pid)
{
    if (pid == 0 || p->proc[pid].state == FREE) return; // idle process is not killable

    if (p->proc[pid].state != ZOMBIE) {
        process_terminate(p, pid);
    }

    if (p->proc[pid].state == FREE) return;

    p->proc[pid].state = FREE;
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
        if (p->proc[k].state == READY && p->proc[k].pid != 0)
            process_run(p, k, PARENT_KEEP_RUNNING);
    }

    // if we got here, no processes to run, so we go to idle
    if (p->curr->state == READY)
        process_run(p, p->curr->pid, PARENT_KEEP_RUNNING);
    else
        process_run(p, 0, PARENT_KEEP_RUNNING);
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
        p->ctx.usp    = AKAI_PROCESS_STACK_TOP;
        p->ctx.status = ((p->pdt >> 4) << 8) | AKAI_PROCESS_STATUS;
        _swd(REG_SYSTEM_USP, p->ctx.usp);
    } else if (sirius_cwp == 0 && p->pid != 0 && p->state != NEWBORN) {
        // panic here
    } else {
        u8 cwp        = sirius_cwp;
        p->ctx.pc     = _lwd(AKAI_KERNEL_PC_SAVE);     // 0x1000); // save user pc
        p->ctx.status = _lwd(AKAI_KERNEL_STATUS_SAVE); // 0x1004); // save user status register
        p->ctx.usp    = _lwd(REG_SYSTEM_USP);
        _trace(0x1001, p->ctx.pc, p->ctx.status, p->ctx.usp);
        _trace(0x67, p->pid, cwp);
        p->ctx.wp = cwp ? cwp - 1 : cwp;
        load_window(p->ctx.wp, p->ctx.regs);
        // Assume it was saved upon entering the kernel. Otherwise this will fault
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

bool process_check_recv(struct Processes *p, ProcessPID sender_pid, ProcessPID target_pid,
                        u8 signal)
{
    struct Process *target = &p->proc[target_pid];

    u32   signal_bit;
    bool  is_signal = false;
    usize i;

    if (target_pid == 0)
        return false; // idle does not receive messages (for now).
                      // it does not send them either, but the sender_pid 0 is interpreted as
                      // the kernel

    if (signal < AKAI_INVALID_INTERRUPT) {
        signal_bit = (1UL << signal); // Interrupt
    } else if (signal == SIGDOOR) {
        signal_bit = 1UL << 15;
    } else if (signal > SIGDOOR) {
        is_signal  = true;
        signal_bit = (u32)(1UL << (signal - SIGGP0)) << 16;
    } else {
        return false; // unsupported signal
    }

    if (target->event_mask & signal_bit) return true; // event_mask is a master override

    if (!is_signal) return false;

    signal_bit >>= 16;

    for (i = 0; i < 4; i++) {
        if (target->subs[i].publisher == sender_pid && target->subs[i].signal_mask & signal_bit)
            return true;
    }

    return false;
}

bool process_queue_msg(struct Processes *p, ProcessPID pid, struct IPCMessage *msg)
{
    u8                 msgbuf[IPC_MSG_SIZE], hdrbuf[INBOX_HEADER_SIZE];
    bool               overflow = false;
    struct InboxHeader header;

    if (pid == 0) return false; // dont send messages to the Kernel, syscalls exist
    if (p->proc[pid].state == FREE || p->proc[pid].state == ZOMBIE) return false;
    if (!(p->proc[pid].flags & PROC_IPC)) return false;
    if (!(p->proc[pid].inbox)) return false;
    if (!msg) return false;
    if (msg->type == 0) return false; // its a NULL message

    msgbuf[IPC_MSG_SENDER]      = msg->sender;
    msgbuf[IPC_MSG_TYPE]        = msg->type;
    msgbuf[IPC_MSG_ID]          = msg->msgid >> 8;
    msgbuf[IPC_MSG_ID + 1]      = msg->msgid;
    msgbuf[IPC_MSG_SUBJECT]     = msg->subject >> 24;
    msgbuf[IPC_MSG_SUBJECT + 1] = msg->subject >> 16;
    msgbuf[IPC_MSG_SUBJECT + 2] = msg->subject >> 8;
    msgbuf[IPC_MSG_SUBJECT + 3] = msg->subject;
    msgbuf[IPC_MSG_CONTENT]     = (u32)msg->content >> 24;
    msgbuf[IPC_MSG_CONTENT + 1] = (u32)msg->content >> 16;
    msgbuf[IPC_MSG_CONTENT + 2] = (u32)msg->content >> 8;
    msgbuf[IPC_MSG_CONTENT + 3] = (u32)msg->content;

    // map this process inbox into the shadow scratchpad
    map_pt_entry((u32 *)AKAI_PROCESS_PT3, AKAI_IPC_KERNEL_IN >> 12, (u32)p->proc[pid].inbox,
                 PTE_V | PTE_R | PTE_W);
    tlb_flush();

    // TODO: this memcpy calls are superfluous, use the pointer directly
    memcpy(hdrbuf, (u8 *)AKAI_IPC_KERNEL_IN, INBOX_HEADER_SIZE);

    header.semaphore = hdrbuf[INBOX_HEADER_SEM];
    header.tail      = (u16)hdrbuf[INBOX_HEADER_TAIL] << 8;
    header.tail |= hdrbuf[INBOX_HEADER_TAIL + 1];
    header.head = (u16)hdrbuf[INBOX_HEADER_HEAD] << 8;
    header.head |= hdrbuf[INBOX_HEADER_HEAD + 1];
    header.missed    = hdrbuf[INBOX_HEADER_MISSED];
    header.flags     = hdrbuf[INBOX_HEADER_FLAGS];
    header.queue_max = (u16)hdrbuf[INBOX_HEADER_QUEUE_MAX] << 8;
    header.queue_max |= hdrbuf[INBOX_HEADER_QUEUE_MAX + 1];

    /* insert */
    {
        u16 next_head = (header.head + 1) % header.queue_max;
        if (next_head == header.tail) {
            // overrun
            header.flags |= INBOX_FLAG_QUEUE_OVERFLOW;
            overflow = true;
        } else {
            memcpy((u8 *)AKAI_IPC_KERNEL_IN + INBOX_HEADER_SIZE + (header.head * IPC_MSG_SIZE),
                   msgbuf, IPC_MSG_SIZE);
            header.head = next_head;
        }

        header.missed++;

        // store changed values
        hdrbuf[INBOX_HEADER_HEAD]     = header.head >> 8;
        hdrbuf[INBOX_HEADER_HEAD + 1] = header.head;
        hdrbuf[INBOX_HEADER_FLAGS]    = header.flags;
        hdrbuf[INBOX_HEADER_MISSED]   = header.missed;
    }

    memcpy((u8 *)AKAI_IPC_KERNEL_IN, hdrbuf, INBOX_HEADER_SIZE);

    // unmap this process inbox into the shadow scratchpad by mapping p->curr's
    map_pt_entry((u32 *)AKAI_PROCESS_PT3, AKAI_IPC_KERNEL_IN >> 12, (u32)p->curr->inbox,
                 PTE_V | PTE_R | PTE_W);
    tlb_flush();
    return overflow;
}