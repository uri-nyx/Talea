#include "../include/handlers.h"
#include "../include/hw.h"
#include "../include/mem.h"
#include "../include/mmu.h"
#include "../include/process.h"

u32 akai_syscall(u32 service)
{
    // parameters passed to syscall in window, registers x13 onwards
    static u32 win[32];
    u32        result = -1;

    u32 status = _lwd(AKAI_KERNEL_STATUS_SAVE);
    u32 pc     = _lwd(AKAI_KERNEL_PC_SAVE);

    _trace(0xFef0);
    save_ctx(processes.curr);

    // load_window(sirius_cwp - 1, &win);
    memcpy(win, processes.curr->ctx.regs, sizeof(win));

    switch (service) {
    case SYSCALL_EXIT: {
        u32 exit_code = win[13];

        _trace(0xe1, exit_code);
        puts(&sys, "Exit ");
        puts(&sys, processes.curr->name);
        puts(&sys, "\n");

        processes.curr->exit_code = exit_code;

        process_terminate(&processes, processes.curr->pid);

        if (processes.proc[processes.curr->parent].state == WAITING) {
            // here, waiting means that a process has spawned another.
            // waiting for anything else does not make sense without a scheduler
            process_run(&processes, processes.proc[processes.curr->parent].pid,
                        PARENT_KEEP_RUNNING);
        } else {
            process_yield(&processes);
        }
        result = 0;
        break;
    }
    case SYSCALL_YIELD: {
        static yields = 0;
        _trace(0xe2, yields++);
        if (processes.curr->pid != 0) {
            puts(&sys, "Yield ");
            puts(&sys, processes.curr->name);
            puts(&sys, "\n");
        }
        process_yield(&processes);
        result = 0;
        break;
    }

    case SYSCALL_HOOK: {
        u32 event_mask    = win[13];
        u32 event_handler = win[14];

        _trace(0xe3, processes.curr->pid, event_mask, event_handler);

        if (processes.curr->pid == 0) break; // Do not hook the idle task
        if ((processes.curr->flags & PROC_IPC) != PROC_IPC)
            break; // must initialize IPC. TODO: return error code

        result = processes.curr->event_mask;
        processes.curr->event_mask |= event_mask;
        processes.curr->event_handler = (void *)event_handler;
        break;
    }
    case SYSCALL_UNHOOK: {
        u32 event_mask = win[13];

        _trace(0xe4, processes.curr->pid, event_mask);

        if (processes.curr->pid == 0) break; // Do not unhook the idle task

        result = processes.curr->event_mask;
        processes.curr->event_mask &= ~event_mask;
        break;
    }
    case SYSCALL_IPC_INIT: {
        u8   hdrbuf[INBOX_HEADER_SIZE];
        u32 *inbox_page = alloc_pages_contiguous(&pages, processes.curr->pid, 1);

        if (!inbox_page) {
            // out of memory
            result = -1; // TODO: design a coherent convention for syscall return values & errors
            break;
        }

        // the page tables should be mapped in the process' space per process_create
        map_pt_entry((u32 *)AKAI_PROCESS_PT3, AKAI_IPC_INBOX >> 12, (u32)inbox_page,
                     PTE_V | PTE_U | PTE_R | PTE_W);
        map_pt_entry((u32 *)AKAI_PROCESS_PT3, AKAI_IPC_KERNEL_IN >> 12, (u32)inbox_page,
                     PTE_V | PTE_R | PTE_W);
        tlb_flush();
        memset((u8 *)AKAI_IPC_KERNEL_IN, 0, PAGE_SIZE);

        processes.curr->flags |= PROC_IPC;
        processes.curr->inbox = inbox_page;

        memset(hdrbuf, 0, INBOX_HEADER_SIZE);
        hdrbuf[INBOX_HEADER_QUEUE_MAX]     = INBOX_QUEUE_MAX >> 8;
        hdrbuf[INBOX_HEADER_QUEUE_MAX + 1] = INBOX_QUEUE_MAX;
        memcpy((u8*)AKAI_IPC_KERNEL_IN, hdrbuf, INBOX_HEADER_SIZE);

        result = 0;
    }
    case SYSCALL_PUTC: {
        char         c       = win[13];
        static usize pos     = 0;
        usize        tb_size = sys.textbuffer.w * sys.textbuffer.h;

        *((u32 *)AKAI_TEXTBUFFER + pos) = ((u32)c << 24) | 0x000f0000;
        pos++;
    }
    default: break;
    }

    _trace(0x4504, status, pc);
    _swd(AKAI_KERNEL_STATUS_RESTORE, status);
    _swd(AKAI_KERNEL_PC_RESTORE, pc);

    return result;
}