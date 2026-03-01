#ifndef PROCESS_H
#define PROCESS_H

/* Simple processes and threads for Akai */

#include "akai_def.h"
#include "dev.h"

#define NUM_REGS 32

/* @AKAI: 400_PROCESS */
#define MAX_PROCESS     255
#define PROCESS_NAMELEN 10
#define MAX_OPEN_FILES  8
/* @AKAI */

#define CPU_STATUS_SUPERVISOR       (1U << 31U)
#define CPU_STATUS_INTERRUPT_ENABLE (1U << 30U)
#define CPU_STATUS_MMU_ENABLE       (1U << 29U)
#define CPU_STATUS_PRIORITY_MASK    (0x1C000000U)
#define CPU_STATUS_IVT_ADDR_MASK    (0x03F00000U) // NOTE: NOT USED
#define CPU_STATUS_PDT_ADDR_MASK    (0x000FFF00U)
#define CPU_STATUS_DEBUG_STEP       (1U << 1U)

#define ISALIVE(pid) (A.pr.proc[(pid)].state != FREE && A.pr.proc[(pid)].state != ZOMBIE)

struct ThreadCtx {
    u32 regs[NUM_REGS];
    u32 status;
    u32 pc;
    u32 usp;
    u8  wp;
};

enum ProcessFlags {
    PROC_TSR = 1 << 0,
    PROC_IPC = 1 << 2,
};

struct Subscriptions {
    ProcessPID publisher;
    u16        signal_mask;
};

enum {
    FREE_FD = -1,
};

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
    u32   last_error;
    i32   waiting_on;
    int   parent_state;

    // timing
    u32 user_ticks, system_ticks;

    // Event and interrupt code
    void *event_handler;
    u32   pending_events;
    u32   event_mask; // [31:16] general purpose signals [15] doorbell? [12:0]
                      // interrupts

    // subscriptions
    struct Subscriptions subs[4];

    // IPC
    void *inbox;
    void *outbox;
    u8    message_queue_flags;

    // Filesystem
    i16 fds[MAX_OPEN_FILES];
    u8  open_files;
    u32 cwd_cluster[3];
    u8  curr_drive;

    // IO
    struct IOStream stdin, stdout, stderr;
};

struct Processes {
    struct Process  proc[MAX_PROCESS];
    struct Process *curr;
    usize           count;
};

/* @AKAI: 410_PROCESS */

/*
HEADER AT THE START OF THE INBOX
00      1b  semaphore: for the kernel to inject an event, it must be 0. The kernel
initializes to 1. Before returning, the user must set it to a number greater than 1
to prevent recursive injection. The kernel will check every time it could inject the
process: if 0, its safe to inject if 1, it must never inject. if 2, it decrements to
0. if >2, it decrements by 1. 01      2b  tail of the message queue. read pointer
for user 03      2b  head of the message queue. write pointer for kernel 05      1b
missed. Number of missed messages since last injection. 06      1b  flags: bit 0:
overflow, messages were lost or dropped 07-08   2b  queue_max: maximum capacity of
message queue. 09-0F   6b  reserved for padding

TOTAL SIZE 16b
*/

enum {
    INBOX_HEADER_SEM       = 0,
    INBOX_HEADER_TAIL      = 1,
    INBOX_HEADER_HEAD      = 3,
    INBOX_HEADER_MISSED    = 5,
    INBOX_HEADER_FLAGS     = 6,
    INBOX_HEADER_QUEUE_MAX = 7,
    INBOX_HEADER_SIZE      = 16,
};

enum {
    INBOX_FLAG_QUEUE_OVERFLOW = 1U << 0,
};

// WARNING: THIS STRUCT IS ONLY FOR REFERENCE AND EASY HANDLING.
// ACTUAL MEMORY LAYOUT, ALIGNEMENT AND SIZE MUST BE ENSURED TO BE AS DESCRIBED
// ABOVE
struct InboxHeader {
    u8  semaphore;
    u16 tail;
    u16 head;
    u8  missed;
    u8  flags;
    u16 queue_max;
    u8  reserved[6];
};

/*
IPC MESSAGE STRUCTURE (MULTIBYTE BIG ENDIAN)
00      1b      sender PID
01      1b      type of message:
                    0-12    hardware events/interrupts
                    13-238  reserved for system events
                    239     IPC shared memory doorbell
                    240-255 16 general purpose signals
02      2b      msgid: identifier for this message of this type from this pid.
                       to be defined by user protocol implementation.
04      4b      subject: message subject, immediate information
08      4b      content: message content. May be immediate information or a pointer.
                       to be defined by user protocol implementation.
TOTAL SIZE: 12b
*/

enum Signal {
    SIGDOOR = 239,
    SIGGP0,
    SIGGP1,
    SIGGP2,
    SIGGP3,
    SIGGP4,
    SIGGP5,
    SIGGP6,
    SIGGP7,
    SIGGP8,
    SIGGP9,
    SIGGP10,
    SIGGP11,
    SIGGP12,
    SIGGP13,
    SIGGP14,
    SIGGP15,
};

enum {
    IPC_MSG_SENDER  = 0,
    IPC_MSG_TYPE    = 1,
    IPC_MSG_ID      = 2,
    IPC_MSG_SUBJECT = 4,
    IPC_MSG_CONTENT = 8,
    IPC_MSG_SIZE    = 12,
};

// WARNING: THIS STRUCT IS ONLY FOR REFERENCE AND EASY HANDLING.
// ACTUAL MEMORY LAYOUT, ALIGNEMENT AND SIZE MUST BE ENSURED TO BE AS DESCRIBED
// ABOVE
struct IPCMessage {
    // message header
    // like an address:port combination
    // (there are 16 general signals, then the interrupts, and the doorbell)
    ProcessPID sender;
    u8         type;
    u16        msgid; // identifier for this sender, type

    // message
    u32   subject; // immediate information
    void *content; // extended information
};

#define INBOX_QUEUE_MAX ((PAGE_SIZE - INBOX_HEADER_SIZE) / IPC_MSG_SIZE)

enum ExecutionWarrant {
    WARRANT_NONE      = 0,
    WARRANT_ABORT     = -1,
    WARRANT_EXCEPTION = -2,
    WARRANT_PARRICIDE = -3,
    WARRANT_REAPED    = -4,
    WARRANT_ERROR     = -4,
    WARRANT_OOM       = (signed)A_ERROR_OOM,
    WARRANT_SEG       = (signed)A_ERROR_SEG,

};

#define WAITON_ANY (-1)

enum WaitOptions {
    WAIT_HANG   = 0,
    WAIT_NOHANG = 1,
};

/* @AKAI */

enum ParentState {
    PARENT_KEEP_RUNNING,
    PARENT_WAIT,
    PARENT_DIE,
    PARENT_DETACH,
};

extern void _load_window(u8 wp, void *buf);
void       *load_window(u8 wp, void *buf);
void        store_window(u8 wp, void *buf);

void process_init(uptr idle_base);

ProcessPID process_create(const char *name, ProcessEntry entry);
bool       process_reset(ProcessPID pid, const char *name, u32 brk, u32 stack, ProcessEntry entry);
void       process_run(ProcessPID pid, enum ParentState parent_new_state);
void       process_stop(ProcessPID pid);
void       process_wait(ProcessPID pid);
void       process_set_ready(ProcessPID pid);
void       process_terminate(ProcessPID pid, i32 warrant);
void       process_reap(ProcessPID pid);
void       process_yield(void);

bool process_check_recv(ProcessPID sender_pid, ProcessPID target_pid, u8 signal);
bool process_queue_msg(ProcessPID pid, struct IPCMessage *msg);
bool process_check_addr(ProcessPID pid, uptr addr, u32 acces_flags, bool executable);

bool process_clone_memory(ProcessPID dst, ProcessPID src);
bool process_share_memory(ProcessPID dst, ProcessPID src);
void process_share_files(ProcessPID dst, ProcessPID src);

u32  save_ctx(struct Process *p);
void restore_ctx(struct Process *p);

extern void _load_and_switch(u32 pc, u32 status, u32 *regs);
extern void _load_init(u32 pc, u32 status, u32 *regs);
extern void _switch(u32 pc, u32 status);

#endif /* PROCESS_H */
