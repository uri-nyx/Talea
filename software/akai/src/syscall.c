#include "../include/handlers.h"
#include "../include/hw.h"
#include "../include/process.h"

u32 akai_syscall(u32 service)
{
    // parameters passed to syscall in window, registers x13 onwards
    static u32 win[32];
    u32        result = -1;

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

        if (processes.curr->pid == 0) break; // Do not hook the idle task

        result = processes.curr->event_mask;
        processes.curr->event_mask |= event_mask;
        processes.curr->event_handler = (void *)event_handler;
        break;
    }
    case SYSCALL_UNHOOK: {
        u32 event_mask = win[13];

        if (processes.curr->pid == 0) break; // Do not unhook the idle task

        result = processes.curr->event_mask;
        processes.curr->event_mask &= ~event_mask;
        break;
    }
    default: break;
    }

    return result;
}