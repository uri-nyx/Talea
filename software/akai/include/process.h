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

typedef u8 ProcessPID;

typedef int (*ProcessEntry)(int, char **);

enum ProcessFlags {
    PROC_TSR = 1 << 0,
    PROC_IPC = 1 << 2,
};

struct Subscriptions {
    ProcessPID publisher;
    u16        signal_mask;
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

    // Event and interrupt code
    void *event_handler;
    u32   pending_events;
    u32   event_mask; // [31:16] general purpose signals [15] doorbell? [12:0] interrupts

    // subscriptions
    struct Subscriptions subs[4];

    // IPC
    void *inbox;
    void *outbox;
    u8    message_queue_flags;
};

struct Processes {
    struct Process  proc[MAX_PROCESS];
    struct Process *curr;
    usize           count;
};

/*
HEADER AT THE START OF THE INBOX
00      1b  semaphore: for the kernel to inject an event, it must be 0. The kernel initializes to 1.
        Before returning, the user must set it to a number greater than 1 to prevent recursive
        injection. The kernel will check every time it could inject the process:
            if 0, its safe to inject
            if 1, it must never inject.
            if 2, it decrements to 0.
            if >2, it decrements by 1.
01      2b  tail of the message queue. read pointer for user
03      2b  head of the message queue. write pointer for kernel
05      1b  missed. Number of missed messages since last injection.
06      1b  flags:
            bit 0: overflow, messages were lost or dropped
07-08   2b  queue_max: maximum capacity of message queue.
09-0F   6b  reserved for padding

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
// ACTUAL MEMORY LAYOUT, ALIGNEMENT AND SIZE MUST BE ENSURED TO BE AS DESCRIBED ABOVE
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
// ACTUAL MEMORY LAYOUT, ALIGNEMENT AND SIZE MUST BE ENSURED TO BE AS DESCRIBED ABOVE
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

bool process_check_recv(struct Processes *p, ProcessPID sender_pid, ProcessPID target_pid,
                        u8 signal);
bool process_queue_msg(struct Processes *p, ProcessPID pid, struct IPCMessage *msg);
bool process_check_addr(struct Processes *p, ProcessPID pid, u32 addr, u32 acces_flags,
                        bool executable);

u32  save_ctx(struct Process *p);
void restore_ctx(struct Process *p);

extern void _load_and_switch(u32 pc, u32 status, u32 *regs);
extern void _switch(u32 pc, u32 status);

#endif /* PROCESS_H */
