#ifndef PROCESS_H
#define PROCESS_H

/* Simple processes and threads for Akai */

#include "libsirius/types.h"

#define MAX_PROCESS     256
#define NUM_REGS        32
#define PROCESS_NAMELEN 10

enum {
    CPU_STATUS_SUPERVISOR       = (1U << 31U),
    CPU_STATUS_INTERRUPT_ENABLE = (1U << 30U),
    CPU_STATUS_MMU_ENABLE       = (1U << 29U),
    CPU_STATUS_PRIORITY_MASK    = 0x1C000000,
    CPU_STATUS_IVT_ADDR_MASK    = 0x03F00000, // NOTE: NOT USED
    CPU_STATUS_PDT_ADDR_MASK    = 0x000FFF00,
    CPU_STATUS_DEBUG_STEP       = (1U << 1U),
};

struct ThreadCtx {
    u32 regs[NUM_REGS];
    u32 status;
    u32 pc;
    u32 usp;
    u8  wp;
};

enum ProcessFlags {
    PROC_TSR = 1 << 0,
};

typedef u8 ProcessPID;

typedef int (*ProcessEntry)(int, char **);

struct Process {
    struct ThreadCtx ctx;
    ProcessPID       parent;
    ProcessPID       pid;
    ProcessEntry     entry;
    u32             *page_tables[4];
    u16              pdt;
    char             name[10];
    enum { FREE, NEWBORN, WAITING, READY, RUNNING, ZOMBIE } state;
    void *brk;
    u32   flags;
    i32   exit_code;
};

struct Processes {
    struct Process  proc[MAX_PROCESS];
    struct Process *curr;
    usize           count;
};

enum ParentState {
    PARENT_KEEP_RUNNING,
    PARENT_WAIT,
    PARENT_DIE,
    PARENT_DETACH,
};

extern void _load_window(u8 wp, void *buf);
void       *load_window(u8 wp, void *buf);
void        store_window(u8 wp, void *buf);

void process_init(struct Processes *p, u32 idle_base);

ProcessPID process_create(struct Processes *p, const char *name, ProcessEntry entry);
void       process_run(struct Processes *p, ProcessPID pid, enum ParentState parent_new_state);
u32        process_stop(struct Processes *p, ProcessPID pid);
u32        process_wait(struct Processes *p, ProcessPID pid);
void       process_set_ready(struct Processes *p, ProcessPID pid);
void       process_terminate(struct Processes *p, ProcessPID pid);
void       process_reap(struct Processes *p, ProcessPID pid);
void       process_yield(struct Processes *p);

u32  save_ctx(struct Process *p);
void restore_ctx(struct Process *p);

extern void _load_and_switch(u32 pc, u32 status, u32 *regs);
extern void _switch(u32 pc, u32 status);

#endif /* PROCESS_H */
