#include "libsirius/types.h"
#include "../include/handlers.h"
#include "../include/process.h"

static const char *exception_names[] = {
    "EXCEPTION_RESET",         "EXCEPTION_BUS_ERROR",
    "EXCEPTION_ADDRESS_ERROR", "EXCEPTION_ILLEGAL_INSTRUCTION_TALEA",
    "EXCEPTION_DIVISION_ZERO", "EXCEPTION_PRIVILEGE_VIOLATION",
    "EXCEPTION_PAGE_FAULT",    "EXCEPTION_ACCESS_VIOLATION_TALEA",
    "EXCEPTION_DEBUG_STEP",    "EXCEPTION_OVERSPILL",
    "EXCEPTION_UNDERSPILL",
};

void akai_exception(u8 vector, u32 fault_addr)
{
    static u32 win[32];

    save_ctx(processes.curr);

    load_window(sirius_cwp - 1, &win);

    puts(&sys, "Exception: ");
    puts(&sys, exception_names[vector]);
    puts(&sys, " in process ");
    puts(&sys, processes.curr->name);
    puts(&sys, "\n");

    switch (vector) {
    default:
        process_kill(&processes, processes.curr->pid);
        process_yield(&processes);
        break;
    }

l:
    goto l;
}