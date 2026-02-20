#include "hw.h"
#include "kernel.h"

/*
_start:
    trace x0,x0,x0,x2
    li x12, 1 	# SYSCALL_YIELD
    syscall x0, 0x40
    j _start
*/
static u8 idle_code[] = { 0x0a, 0x00, 0x00, 0x40, 0xa6, 0xc0, 0x00, 0x01,
                          0x04, 0x00, 0x00, 0x40, 0x40, 0x0f, 0xff, 0xfd };

static bool setup_pt(struct Process *p, u32 phys_pt, u32 *template, u32 pdt_idx)
{
    u32 *work = (u32 *)map_kernel_work_area(phys_pt);
    u32  v, vpn;

    if (!work) {
        return false;
    }

    memcpy(work, template, PAGE_SIZE);

    work = (u32 *)remap_kernel_work_area((u32)p->page_tables[0]);

    v         = AKAI_PROCESS_PAGE_TABLES + (pdt_idx * PAGE_SIZE);
    vpn       = (v >> 12) & 0x3FF;
    work[vpn] = (phys_pt & 0xFFF000) | PTE_V | PTE_R | PTE_W;

    unmap_kernel_work_area();
    return true;
}

void process_init(uptr idle_base)
{
    struct Processes *p        = &A.pr;
    const char        name[10] = "AKAI IDLE";
    u32              *code;

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

    code = alloc_pages_contiguous(KERNEL_PID, 1);
    if (!code) {
        // TODO: implement -> kernel_panic_internal("In process init: could not allocate pages for
        // idle process.");
        _trace(0xDEADC0DE);
l:
        goto l;
    }

    p->proc[0].pdt = AKAI_PDT_BASE;

    p->proc[0].page_tables[0] = (u32 *)AKAI_PROCESS_PT0; // this is identity mapped
    p->proc[0].page_tables[1] = NULL;
    p->proc[0].page_tables[2] = NULL;
    p->proc[0].page_tables[3] = (u32 *)AKAI_PROCESS_PT3;

    map_pt_entry((u32 *)AKAI_PROCESS_PT3, idle_base >> 12, (uptr)code,
                 PTE_V | PTE_U | PTE_X | PTE_R | PTE_W);
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

enum {
    PROCESS_NOREUSE_PT,
    PROCESS_REUSE_PT,
};

// fill the process with default values
static bool populate_process(ProcessPID pid, const char *name, ProcessEntry entry, int flags)
{
    usize           i;
    struct Process *p = &A.pr.proc[pid];

    if (flags == PROCESS_NOREUSE_PT) {
        p->page_tables[0] = alloc_pages_contiguous(pid, 1);
        p->page_tables[3] = alloc_pages_contiguous(pid, 1);

        if (!p->page_tables[0] || !p->page_tables[3]) {
            p->state = FREE;
            A.pr.count--;
            _trace(0xdead1);
            return false;
        }

        if (!setup_pt(p, (u32)p->page_tables[0], A.Kpt, 0)) {
            p->state = FREE;
            free_page(p->page_tables[3]);
            free_page(p->page_tables[0]);
            p->page_tables[0] = NULL;
            p->page_tables[3] = NULL;
            A.pr.count--;
            _trace(0xdead2);
            return false;
        }

        if (!setup_pt(p, (u32)p->page_tables[3], A.Hpt, 3)) {
            p->state = FREE;
            free_page(p->page_tables[3]);
            free_page(p->page_tables[0]);
            p->page_tables[0] = NULL;
            p->page_tables[3] = NULL;
            A.pr.count--;
            _trace(0xdead3);
            return false;
        }

    } else {
        // Reuse page tables, but wipe userspace (recursive mapping hold)
        usize i;
        u32  *work = (u32 *)map_kernel_work_area((u32)p->page_tables[0]);

        if (!work) {
            p->state = ZOMBIE;
            free_page(p->page_tables[3]);
            free_page(p->page_tables[0]);
            p->page_tables[0] = NULL;
            p->page_tables[3] = NULL;
            A.pr.count--;
            _trace(0xdead3);
            return false;
        }

        for (i = (AKAI_PROCESS_BASE >> 12); i < 1024; i++) {
            u32 phys = work[i] & ~0xFFF;
            if (phys) free_page((void *)phys);
            work[i] = 0;
        }

        if (p->page_tables[1]) {
            work = (u32 *)remap_kernel_work_area((u32)p->page_tables[1]);
            for (i = 0; i < 1024; i++) {
                u32 phys = work[i] & ~0xFFF;
                if (phys) free_page((void *)phys);
                work[i] = 0;
            }
            free_page(p->page_tables[1]);
            p->page_tables[1] = NULL;
        }

        if (p->page_tables[2]) {
            work = (u32 *)remap_kernel_work_area((u32)p->page_tables[2]);
            for (i = 0; i < 1024; i++) {
                u32 phys = work[i] & ~0xFFF;
                if (phys) free_page((void *)phys);
                work[i] = 0;
            }
            free_page(p->page_tables[2]);
            p->page_tables[2] = NULL;
        }

        work = (u32 *)remap_kernel_work_area((u32)p->page_tables[3]);
        for (i = 0; i < ((AKAI_PROCESS_STACK_TOP >> 12) & 0x3FF); i++) {
            u32 phys = work[i] & ~0xFFF;
            if (phys) free_page((void *)phys);
            work[i] = 0;
        }

        if (p->inbox != NULL) {
            free_page(p->inbox);
            work[(AKAI_IPC_INBOX >> 12) & 0x3FF] = 0;
        }

        unmap_kernel_work_area();
    }

    // Set the pid
    p->pid = pid;

    // Set the pdt as pid AKAI_BASE_PDT + (0-255) << 4
    p->pdt = AKAI_PDT_BASE + ((u16)pid << 4);

    // map pt0 and A.Hpt in the page directory.
    // the index in the page directory is (pid << 2) + pt_number,
    // since the kernel has pdt pointer to AKAI_PDT_BASE
    map_pdt_entry(p->pdt, 0, (u32)p->page_tables[0], PTE_V);

    // map the A.Hpt in the same way
    map_pdt_entry(p->pdt, 3, (u32)p->page_tables[3], PTE_V);

    // Set it to NEWBORN
    p->state = NEWBORN;

    // Set flags. start as normal program
    p->flags = 0;

    // Set the name
    memcpy(p->name, name, 10); // TODO: use strncpy
    p->name[9] = 0;

    // Set the entry point
    p->entry = entry;

    // reset open files
    for (i = 0; i < MAX_OPEN_FILES; i++) {
        p->fds[i] = FREE_FD;
    }

    return true;
}

// returns 0 (the kernel PID) on error. Make user to init the process module (to create the Kernel
// process) first
ProcessPID process_create(const char *name, ProcessEntry entry)
{
    struct Processes *p = &A.pr;
    usize             i;
    ProcessPID        pid;

    _trace(0xc00f);
    for (i = 0; i < MAX_PROCESS; i++) {
        _trace(i, p->proc[i].state);
        // reap zombies
        if (p->proc[i].state == ZOMBIE && p->proc[i].parent == 0) {
            process_reap(i);
        }

        // allocate a pid
        if (p->proc[i].state == FREE) {
            pid = i;
            _trace(0xf2eeeee, pid);
            p->count++;
            break;
        }
    }

    if (p->count >= MAX_PROCESS) return 0;

    if (!populate_process(pid, name, entry, PROCESS_NOREUSE_PT)) return 0;

    // Fill regs and pc, usp, and status
    save_ctx(&p->proc[pid]);

    // Set parent as curr process
    p->proc[pid].parent         = p->curr->pid;
    p->proc[pid].curr_drive     = p->curr->curr_drive;
    p->proc[pid].cwd_cluster[0] = p->curr->cwd_cluster[0];
    p->proc[pid].cwd_cluster[1] = p->curr->cwd_cluster[1];
    p->proc[pid].cwd_cluster[2] = p->curr->cwd_cluster[2];

    // brk left to zero, only initialize on exec() or rfork()
    return pid;
}

// reset a process to NEWBORN. Wipe all, for overlaying a new image. true on succes
bool process_reset(ProcessPID pid, const char *name, u32 brk, u32 stack, ProcessEntry entry)
{
    struct Processes *p = &A.pr;
    usize             i;

    ProcessPID parent = p->proc[pid].parent; // cache the parent

    if (!populate_process(pid, name, entry, PROCESS_REUSE_PT)) return false;
    // Fill only zero out regs, we set the stack, pc and status
    memset(p->proc[pid].ctx.regs, 0, 32 * 4);

    p->proc[pid].brk        = (void *)brk;
    p->proc[pid].ctx.pc     = (u32)AKAI_PROCESS_BASE;
    p->proc[pid].ctx.usp    = AKAI_PROCESS_STACK_TOP;
    p->proc[pid].ctx.status = ((p->proc[pid].pdt >> 4) << 8) | AKAI_PROCESS_STATUS;
    // Restore parent
    p->proc[pid].parent = parent;

    return true;
}

bool process_check_addr(ProcessPID pid, uptr addr, u32 acces_flags, bool executable)
{
    struct Processes *p = &A.pr;
    u32              *work;
    u32               addr_pt   = addr >> 22; // this is the page table number
    u32               entry_idx = (addr >> 12) & 0x3FF;
    u32               entry     = 0;

    _trace(0xfbbba, addr_pt, addr);
    if (!addr) return false;
    if (addr >= AKAI_IDLE_BASE && addr < AKAI_IDLE_BASE + PAGE_SIZE) return false;
    if (executable && ((addr & 0x3) != 0)) return false;
    if (addr_pt > 3) return false;
    if (!p->proc[pid].page_tables[addr_pt]) return false;

    // map the pt to the kernel to read the entry
    work = (u32 *)map_kernel_work_area((u32)p->proc[pid].page_tables[addr_pt]);
    // read the entry
    entry = work[entry_idx];
    _trace(0xfbbba, entry, entry_idx);
    // unmap from kernel
    unmap_kernel_work_area();

    if ((entry & acces_flags) != acces_flags) return false;

    return true;
}

static void inject_event(u32 active_events)
{
    struct Processes *p = &A.pr;
    u8                semaphore;

    // load semaphore
    semaphore = *(u8 *)AKAI_IPC_INBOX;

    if (semaphore == 0) {
        // inject!
        if (process_check_addr(p->curr->pid, (uptr)p->curr->event_handler, PTE_V | PTE_U | PTE_X,
                               true)) {
            p->curr->ctx.regs[31] = p->curr->ctx.pc;
            _trace(0xf19, p->curr->ctx.pc, p->curr->ctx.regs[31], p->curr->ctx.status);
            p->curr->ctx.pc = (uptr)p->curr->event_handler;
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

    *(u8 *)AKAI_IPC_INBOX = semaphore;

    p->curr->pending_events &= ~active_events;
}

void process_run(ProcessPID pid, enum ParentState parent_new_state)
{
    struct Processes *p = &A.pr;
    u32               sreg;
    ProcessPID        old_pid = p->curr->pid;

    struct Process *target = &p->proc[pid];

    _trace(0xAAAAAAAA, pid, p->curr->pid, target->state);

    if (target->state == ZOMBIE || target->state == FREE) {
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
            case PARENT_WAIT: process_wait(parent->pid); break;
            case PARENT_DIE: process_terminate(parent->pid); break;
            case PARENT_DETACH:
                process_stop(parent->pid);
                target->parent = 0;
                break;
            case PARENT_KEEP_RUNNING:
            default: process_stop(parent->pid); break;
            }
        } else {
            process_stop(p->curr->pid);
        }
    }

skip:
    _trace(0xEEEEEE);
    p->curr                       = target;
    A.fs[target->curr_drive].cdir = target->cwd_cluster[target->curr_drive];

    // switch pdt. There should be no problem, since the kernel is mapped there too
    sreg = _gsreg();
    sreg &= ~(0x000FFF00);
    sreg |= (p->curr->pdt >> 4) << 8;
    _trace(0XBBBBB, sreg, _gsreg());
    _trace(0XBBBBB, p->curr->page_tables[0], p->curr->page_tables[3]);
    _trace(0XBBBBB, _lwd(p->curr->pdt), _lwd(p->curr->pdt + 12));
    if (p->curr->pid == 0) {
    }
    _ssreg(sreg);
    tlb_flush();
    _trace(0XDDDDD);

    if (target->state == NEWBORN) {
        // a newborn cannot possibly run an event
        process_set_ready(pid);
        if (pid == 1 && old_pid == 0 && sirius_cwp == 0) {
            _load_init(p->curr->ctx.pc, p->curr->ctx.status, p->curr->ctx.regs);

        } else {
            _load_and_switch(p->curr->ctx.pc, p->curr->ctx.status, p->curr->ctx.regs);
        }
    } else {
        u32 active_events = p->curr->event_mask & p->curr->pending_events;

        _trace(0xf1A, p->curr->ctx.pc, p->curr->ctx.status);

        if (active_events) inject_event(active_events);

        restore_ctx(p->curr);
        _trace(0xf10, p->curr->ctx.pc, p->curr->ctx.status, p->curr->ctx.usp);
        _trace(0xf10, p->curr->ctx.regs[1]);
        _switch(p->curr->ctx.pc, p->curr->ctx.status);
    }
}

void process_stop(ProcessPID pid)
{
    struct Processes *p = &A.pr;

    if (p->proc[pid].state == ZOMBIE || p->proc[pid].state == FREE || p->proc[pid].state == WAITING)
        return;
    process_set_ready(pid);
    save_ctx(&p->proc[pid]);
}

void process_wait(ProcessPID pid)
{
    struct Processes *p = &A.pr;

    if (p->proc[pid].state == ZOMBIE || p->proc[pid].state == FREE) return;
    p->proc[pid].state = WAITING;
    save_ctx(&p->proc[pid]);
}

void process_set_ready(ProcessPID pid)
{
    struct Processes *p = &A.pr;

    if (p->proc[pid].state == ZOMBIE || p->proc[pid].state == FREE) return;
    p->proc[pid].state = READY;
}

void process_terminate(ProcessPID pid)
{
    struct Processes *p = &A.pr;

    usize i;

    _trace(0xDEADB001, pid);
    if (pid == 0) return; // idle process is not killable

    for (i = 0; i < MAX_PROCESS; i++) {
        if (p->proc[i].parent == pid) p->proc[i].parent = 0;
    }

    free_pages_by_owner(pid);

    for (i = 0; i < MAX_OPEN_FILES; i++) {
        // close open files
        i16 fi = p->proc[pid].fds[i];
        if (fi >= 0) {
            A.fp.refs[fi] -= A.fp.refs[fi] ? 1 : 0;
            if (A.fp.refs[fi] == 0) {
                // dont care about errors
                f_close(&A.fp.files[fi]);
            }
            p->proc[pid].fds[i] = FREE_FD;
        }
    }

    for (i = 0; i < _DEV_NUM; i++) {
        // relinquish owned devices
        if (A.device_owners[i] == pid) {
            A.device_owners[i] = p->proc[pid].parent;
        }
    }

    if (p->proc[pid].parent == 0) {
        _trace(0xDEADDD, p->curr->pid, pid);
        p->proc[pid].state = FREE;
        p->count--;
    } else {
        p->proc[pid].state = ZOMBIE;
    }
}

void process_reap(ProcessPID pid)
{
    struct Processes *p = &A.pr;

    if (pid == 0 || p->proc[pid].state == FREE) return; // idle process is not killable

    if (p->proc[pid].state != ZOMBIE) {
        process_terminate(pid);
    }

    if (p->proc[pid].state == FREE) return;

    p->proc[pid].state = FREE;
    p->count--;
}

void process_yield(void)
{
    struct Processes *p = &A.pr;

    usize      i;
    ProcessPID n;
    // some scheduler logic would be neat. for now, KISS, first ready process runs

    if (p->curr->state == RUNNING) p->curr->state = READY;

    n = (p->curr->pid + 1) % MAX_PROCESS;

    for (i = 0; i < MAX_PROCESS; i++) {
        ProcessPID k = (n + i) % MAX_PROCESS;
        if ((p->proc[k].state == READY || p->proc[k].state == NEWBORN) && p->proc[k].pid != 0) {
            _trace(0xAAA, k, p->proc[k].state);
            process_run(k, PARENT_KEEP_RUNNING);
        }
    }

    _trace(0xBBB0, p->curr->pid, p->curr->state);

    // if we got here, no processes to run, so we go to idle
    if (p->curr->state == READY || p->curr->state == NEWBORN)
        process_run(p->curr->pid, PARENT_KEEP_RUNNING);
    else
        process_run(0, PARENT_KEEP_RUNNING);
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

    _trace(0xFBD, wp);
    for (i = 0; i < 32; i++) {
        _swd(REG_SYSTEM_WIN_BUFF + (i * 4), regs[i]);
    }

    _trace(0xFBD, 1);
    _sbd(REG_SYSTEM_WIN_SEL, wp);
    _trace(0xFBD, 3);
    _sbd(REG_SYSTEM_WIN_OP, TALEA_SYSTEM_WIN_OP_LOAD); // get the window in register file
    _trace(0xFBD, 4);
}

u32 save_ctx(struct Process *p)
{
    u32 sreg = _disable_interrupts();
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
    } else if (p->state != FREE && p->state != ZOMBIE) {
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

    _trace(0x1002, p->pid, sreg);
    _restore_interrupts(sreg);
    return 0;
}

void restore_ctx(struct Process *p)
{
    if (p->state == NEWBORN) {
        // false restore, to transfer to usermode
        _trace(0xfa1, p->pid);
        save_ctx(p);                      // we just want a clean register file<
        _swd(REG_SYSTEM_USP, p->ctx.usp); // store saved usp
    } else if (p->state != FREE && p->state != ZOMBIE) {
        _trace(0xfa0, p->pid, p->ctx.wp, p->ctx.regs[1]);
        store_window(p->ctx.wp, p->ctx.regs);
        _swd(REG_SYSTEM_USP, p->ctx.usp); // store saved usp
    }
};

bool process_check_recv(ProcessPID sender_pid, ProcessPID target_pid, u8 signal)
{
    struct Process *target = &A.pr.proc[target_pid];

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

bool process_queue_msg(ProcessPID pid, struct IPCMessage *msg)
{
    struct Processes *p = &A.pr;

    u8                 msgbuf[IPC_MSG_SIZE], *hdrbuf, *work_inbox;
    bool               overflow = false;
    struct InboxHeader header;

    if (pid == 0) return false; // dont send messages to the Kernel, syscalls exist
    if (p->proc[pid].state == FREE || p->proc[pid].state == ZOMBIE) return false;
    if (!(p->proc[pid].flags & PROC_IPC)) return false;
    if (!(p->proc[pid].inbox)) return false;
    if (!msg) return false;
    if (msg->type == 0) return false; // its a NULL message

    work_inbox = map_kernel_work_area((u32)p->proc[pid].inbox);
    if (!work_inbox) return false;

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

    // TODO: this memcpy calls are superfluous, use the pointer directly
    hdrbuf = work_inbox;

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
            memcpy(work_inbox + INBOX_HEADER_SIZE + (header.head * IPC_MSG_SIZE), msgbuf,
                   IPC_MSG_SIZE);
            header.head = next_head;
        }

        header.missed++;

        // store changed values
        hdrbuf[INBOX_HEADER_HEAD]     = header.head >> 8;
        hdrbuf[INBOX_HEADER_HEAD + 1] = header.head;
        hdrbuf[INBOX_HEADER_FLAGS]    = header.flags;
        hdrbuf[INBOX_HEADER_MISSED]   = header.missed;
    }

    unmap_kernel_work_area();
    return overflow;
}
