#ifndef HANDLERS_H
#define HANDLERS_H

#include "akai_def.h"
#include "ff.h"
#include "process.h"

/* @AKAI: 300_SYSCALLS */
#define X(serivice, function, args, ret, doc)
/* BEGIN_SYSCALL_LIST */
#define SYSCALL_LIST                                                                                       \
    /* Process management */                                                                               \
    X(SYSCALL_EXIT, ak_exit, "int x13: exit code", "none", "the process exits")                            \
    X(SYSCALL_YIELD, ak_yield, "none", "none", "the process yields execution to the scheduler")            \
    X(SYSCALL_RFORK, ak_rfork, "u32 x13: flags, u32 x14: heirloom", "pid/ERROR",                           \
      "fork a process with selected configuration")                                                        \
    X(SYSCALL_EXEC, ak_exec,                                                                               \
      "const char* x13: path, int x14: argc, char** x15: argv, int x16: flags", "none/ERROR",              \
      "process is replaced with executable image")                                                         \
    X(SYSCALL_WAIT, ak_wait, "int x13: pid, int* x14: status, int x15: options",                           \
      "ProcessPID/ERROR", "waits on a child or any child to exit")                                         \
    /* IPC and Hardware*/                                                                                  \
    X(SYSCALL_HOOK, ak_hook, "MODIFY API", "MODIFY", "MODIFY")                                             \
    X(SYSCALL_UNHOOK, ak_unhook, "MODIFY API", "MODIFY", "MODIFY")                                         \
    X(SYSCALL_IPC_SUB, ak_ipc_sub, "NOT IMPLEMENTED", "NOT IMPLEMENTED", "NOT IMPLEMENTED")                \
    X(SYSCALL_IPC_UNSUB, ak_ipc_unsub, "NOT IMPLEMENTED", "NOT IMPLEMENTED", "NOT IMPLEMENTED")            \
    X(SYSCALL_IPC_INIT, ak_ipc_init, "none", "OK/ERROR", "initialize buffers for IPC")                     \
    X(SYSCALL_IPC_RECV, ak_ipc_recv, "NOT IMPLEMENTED", "NOT IMPLEMENTED", "NOT IMPLEMENTED")              \
    X(SYSCALL_IPC_SEND, ak_ipc_send, "NOT IMPLEMENTED", "NOT IMPLEMENTED", "NOT IMPLEMENTED")              \
    X(SYSCALL_DEV_IN, ak_dev_in, "u32 x13: devnum, u8 x14: port", "u8/ERROR",                              \
      "read an owned device's port")                                                                       \
    X(SYSCALL_DEV_OUT, ak_dev_out, "u32 x13: devnum, u8 x14: port, u8 x15: value", "OK/ERROR",             \
      "write to an owned device's port")                                                                   \
    X(SYSCALL_DEV_CTL, ak_dev_ctl, "u32 x13: devnum, u32 x14: cmd, void* x15: buf, u32 x16: len",          \
      "res/ERROR", "send a command to an owned device")                                                    \
    X(SYSCALL_DEV_CLAIM, ak_dev_claim, "u32 x13: devnum", "OK/ERROR",                                      \
      "claim ownership of a harware device")                                                               \
    /* Filesystem */                                                                                       \
    X(SYSCALL_OPEN, ak_open, "const char* x13: path, int x14: open mode", "fd/ERROR",                      \
      "opens a file for the specified operation")                                                          \
    X(SYSCALL_DUP, ak_dup, "NOT IMPLEMENTED", "NOT IMPLEMENTED", "NOT IMPLEMENTED")                        \
    X(SYSCALL_CLOSE, ak_close, "int x13: fd", "OK/ERROR", "closes an already open file")                   \
    X(SYSCALL_UNLINK, ak_unlink, "const char* x13: path", "OK/ERROR", "removes a file")                    \
    X(SYSCALL_RENAME, ak_rename, "const char* x13: old name, const char* x14: new name",                   \
      "OK/ERROR", "changes a file's name")                                                                 \
    X(SYSCALL_MKDIR, ak_mkdir, "const char* x13: path", "OK/ERROR",                                        \
      "creates a new empty directory")                                                                     \
    X(SYSCALL_CHMOD, ak_chmod, "NOT IMPLEMENTED", "NOT IMPLEMENTED", "NOT IMPLEMENTED")                    \
    X(SYSCALL_UTIME, ak_utime, "NOT IMPLEMENTED", "NOT IMPLEMENTED", "NOT IMPLEMENTED")                    \
    X(SYSCALL_READ, ak_read, "int x13: fd, void* x14: buf, u32 x15: count", "bytes read/ERROR",            \
      "read from an opened file into a buffer")                                                            \
    X(SYSCALL_WRITE, ak_write, "int x13: fd, void *x14: buf, u32 x15: count",                              \
      "bytes written/ERROR", "writes from a buffer to a file")                                             \
    X(SYSCALL_SEEK, ak_seek, "int x13: fd, i32 x14: offset, int x15: whence", "offset/ERROR",              \
      "positions the file read write pointer (in bytes) according to offset and whence")                   \
    X(SYSCALL_TRUNC, ak_trunc, "NOT IMPLEMENTED", "NOT IMPLEMENTED", "NOT IMPLEMENTED")                    \
    X(SYSCALL_STAT, ak_stat, "const char* x13: path, AkaiDirEntry* x14: out entry", "OK/ERROR",            \
      "gives information on given path")                                                                   \
    X(SYSCALL_SYNC, ak_sync, "NOT IMPLEMENTED", "NOT IMPLEMENTED", "NOT IMPLEMENTED")                      \
    X(SYSCALL_OPENDIR, ak_opendir, "AkaiDir* x13: dir, const char* x14: path, int x15: flags",             \
      "OK/ERROR", "open a directory")                                                                      \
    X(SYSCALL_CLOSEDIR, ak_closedir, "AkaiDir* x13: dir, int x14: flags", "OK/ERROR",                      \
      "close a directory")                                                                                 \
    X(SYSCALL_READDIR, ak_readdir,                                                                         \
      "AkaiDir* x13: dir, AkaiDirEntry* x14: out entry, int x15: flags", "OK/ERROR",                       \
      "reads a directory entry")                                                                           \
    X(SYSCALL_CHDIR, ak_chdir, "const char* x13: path", "OK/ERROR",                                        \
      "changes a process' current directory")                                                              \
    X(SYSCALL_GETCWD, ak_getcwd, "void* x13: buf, u32 x14: len", "OK/ERROR",                               \
      "writes the current directory path into buf up to len characters")                                   \
    X(SYSCALL_MOUNT, ak_mount, "NOT IMPLEMENTED", "NOT IMPLEMENTED", "NOT IMPLEMENTED")                    \
    X(SYSCALL_EXPAND, ak_expand, "NOT IMPLEMENTED", "NOT IMPLEMENTED", "NOT IMPLEMENTED")                  \
    X(SYSCALL_FORWARD, ak_forward, "NOT IMPLEMENTED", "NOT IMPLEMENTED", "NOT IMPLEMENTED")                \
    /* Memory */                                                                                           \
    X(SYSCALL_SBRK, ak_sbrk, "u32 x13: brk increment", "old bkr/ERROR (-1)",                               \
      "increments a process brk, allocating memory as needed")                                             \
    X(SYSCALL_SHM_MAKE, ak_shm_make, "NOT IMPLEMENTED", "NOT IMPLEMENTED", "NOT IMPLEMENTED")              \
    X(SYSCALL_SHM_UNMAKE, ak_shm_unmake, "NOT IMPLEMENTED", "NOT IMPLEMENTED", "NOT IMPLEMENTED")          \
    /* Queries */                                                                                          \
    X(SYSCALL_ERROR, ak_error, "ProcessPID x13: pid", "ERROR",                                             \
      "returns the queried process' last error, 0 for self")                                               \
    X(SYSCALL_GETPID, ak_getpid, "none", "pid/ERROR", "returns the caller's pid")                          \
    X(SYSCALL_ABORT, ak_abort, "none", "none", "the process is aborted")                                   \
    /* Time */                                                                                             \
    X(SYSCALL_TIME, ak_time, "none", "int/ERROR", "returns time (in seconds) since the epoch")             \
    X(SYSCALL_CLOCK, ak_clock, "u32* x13: user, u32* x14: system, ProcessPID x15: pid",                    \
      "ticks/ERROR", "returns the process' ticks elapsed")                                                 \
    X(SYSCALL_CALENDAR, ak_calendar, "u32* x13: calendar 1, u32* x14: calendar 2", "ERROR",                \
      "returns the calendar time in a compressed format")                                                  \
    X(SYSCALL_GETPPID, ak_getppid, "none", "pid/ERROR", "returns the caller's parent pid")                 \
    X(SYSCALL_SET_PREEMPT, ak_set_preempt, "u32 x13: mode", "OK/ERROR",                                    \
      "sets the kernel preemption mode")                                                                   \
    X(SYSCALL_ASM, ak_asm,                                                                                 \
      "const char* x13: line, usize x14: len, usize x15: curr address, u8* x16: out, usize x17: out size", \
      "bytes/ERROR", "Assembles one instruction to *out buffer")

/* END_SYSCALL_LIST */
#undef X

enum AkaiSyscalls {
#define X(service, function, args, ret, doc) service,
    SYSCALL_LIST
#undef X
};

enum PreemptMode {
    NO_PREEMPT    = 0,
    PREEMPT_ROBIN = 1,
};

enum FsCtl {
    FSCTL_GETFREE,
    FSCTL_SETLABEL,
    FSCTL_GETLABEL,
    FSCTL_GETCP,
};

#define MAX_EXEC_FSIZE   (3 * 1024 * 1024) // this is a bit arbitrary, but 3MB should be enough
#define MAX_EXEC_ARGS    32
#define MAX_EXEC_ARG_LEN 64

#define EXEC_FILE_FORMAT_MASK 0xff

enum ExecFlags {
    O_EXEC_AOUT,        // a.out executable, with sections aligned to page boundaries
    O_EXEC_AOUT_FLAT,   // a.out executable, with sections not aligned to page boundaries
    O_EXEC_BIN,         // flat binary.
    O_EXEC_BIN_SIGNED,  // a flat binary with a signature prepended (first u32: any value, then
                        // 'AKAIBIN!')
    O_EXEC_GUESS = 255, // Guess the format based on information encoded in the file and
                        // heuristics, never will guess flat binary
};

/*
 * RFORK API
 * ----------------------
 * int ak_rfork(int flags, int heirloom);
 *
 * ## Memory configuration flags (RF_MEM_CFG):
 *     Exactly one of the following must be selected:
 *         - RF_MEM_COPY   – Child receives a full copy of the parent's memory.
 *         - RF_MEM_SHARE  – Child shares all memory with the parent.
 *         - RF_MEM_FRESH  – Child starts with no memory mapped.
 *
 * ## File descriptor configuration flags (RF_FIL_CFG):
 *     Exactly one of the following must be selected:
 *         - RF_FIL_SHARE  – Child inherits the parent's file table.
 *         - RF_FIL_CLEAN  – Child starts with an empty file table.
 *
 * ## Parent behavior configuration flags (RF_PARENT_CFG):
 *     Exactly one of the following must be selected:
 *         - RF_PARENT_WAIT         – Parent blocks until child exits.
 *                                  - Child's exit status is not recoverable
 *         - RF_PARENT_DIE          – Parent is terminated immediately.
 *         - RF_PARENT_DETACH       – Child is reparented to kernel.
 *         - RF_PARENT_KEEP_RUNNING – Parent continues execution normally.
 *
 * ## Child behavior:
 *     - RF_CHILD_WAIT  - Child is created in STOPPED state.
 *
 *         If RF_CHILD_WAIT is set, the parent MUST be KEEP_RUNNING.
 *         Otherwise a deadlock is guaranteed. Thus, not doing so, reusults
 *         A_ERROR_INVAL.
 *
 * ## Heirloom device mask:
 *     'heirloom' is a bitmask of devices the child inherits ownership of.
 *
 *         - heirloom < (1U << _DEV_NUM) must be true
 *         - For each bit set in heirloom:
 *               The current process MUST be the owner of that device.
 */
enum RForkFlags {
    RF_MEM_COPY            = 0X0000, // Clone all parent's memory
    RF_MEM_SHARE           = 0X0001, // Share all parent's memory
    RF_MEM_FRESH           = 0X0002, // No memory whatsoever
    RF_FIL_SHARE           = 0X0004, // Share all parents files
    RF_FIL_CLEAN           = 0X0008, // Clean file descriptor table
    RF_LEASE_STDIN         = 0X0010, // Get parent's stdin in lease
    RF_LEASE_STDOUT        = 0X0020, // Get parent's stdout in lease
    RF_LEASE_STDERR        = 0X0040, // Get parent's stderr in lease
    RF_PARENT_WAIT         = 0X0100, // Parent to wait on child
    RF_PARENT_DIE          = 0X0200, // Parent to die on child spawn
    RF_PARENT_DETACH       = 0X0400, // Reparent to Kenel on spawn
    RF_PARENT_KEEP_RUNNING = 0X0800, // Parent to run on spawn
    RF_CHILD_WAIT          = 0x1000, // Child to be stopped on spawn
};

#define RF_MEM_CFG    (RF_MEM_COPY | RF_MEM_SHARE | RF_MEM_FRESH)
#define RF_FIL_CFG    (RF_FIL_SHARE | RF_FIL_CLEAN)
#define RF_PARENT_CFG (RF_PARENT_DETACH | RF_PARENT_DIE | RF_PARENT_WAIT | RF_PARENT_KEEP_RUNNING)

#define RF_INHERIT_FRAMEBUFFER (1U << DEV_FRAMEBUFFER)
#define RF_INHERIT_TEXTBUFFER  (1U << DEV_TEXTBUFFER)
#define RF_INHERIT_SYNTH       (1U << DEV_SYNTH)
#define RF_INHERIT_PCM         (1U << DEV_PCM)
#define RF_INHERIT_SERIAL      (1U << DEV_SERIAL)
#define RF_INHERIT_KEYBOARD    (1U << DEV_KEYBOARD)

enum OpenFlags {
    O_OPEN_EXISTING = 0,    // FA_OPEN_EXISTING,
    O_READ          = 0x1,  // FA_READ,
    O_WRITE         = 0x2,  // FA_WRITE,
    O_CREATE_ALWAYS = 0x8,  // FA_CREATE_ALWAYS,
    O_OPEN_ALWAYS   = 0x10, // FA_OPEN_ALWAYS,
    O_APPEND        = 0x30, // FA_OPEN_APPEND,
};

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

enum TaleaException {
    EXCEPTION_NONE = -1,
    EXCEPTION_RESET,
    EXCEPTION_BUS_ERROR = 0x2,
    EXCEPTION_ADDRESS_ERROR,
    EXCEPTION_ILLEGAL_INSTRUCTION_TALEA,
    EXCEPTION_DIVISION_ZERO,
    EXCEPTION_PRIVILEGE_VIOLATION,
    EXCEPTION_PAGE_FAULT,
    EXCEPTION_ACCESS_VIOLATION_TALEA,
    EXCEPTION_DEBUG_STEP,
    EXCEPTION_OVERSPILL,
    EXCEPTION_UNDERSPILL,
};

enum TaleaInterrupt {
    INT_SER_RX = 0x10,
    INT_KBD_CHAR,
    INT_KBD_SCAN,
    INT_TPS_FINISH,
    INT_HCS_FINISH,
    INT_TIMER_TIMEOUT,
    INT_TIMER_INTERVAL,
    INT_VIDEO_VBLANK,
    INT_MOUSE_PRESSED,
    INT_TPS_EJECTED,
    INT_TPS_INSERTED,
    INT_AUDIO_NOTE_END,
    AKAI_INVALID_INTERRUPT,
};

enum EventFlags {
    EVENT_SER_RX         = 1U << 0U,
    EVENT_KBD_CHAR       = 1U << 1U,
    EVENT_KBD_SCAN       = 1U << 2U,
    EVENT_TPS_FINISH     = 1U << 3U,
    EVENT_HCS_FINISH     = 1U << 4U,
    EVENT_TIMER_TIMEOUT  = 1U << 5U,
    EVENT_TIMER_INTERVAL = 1U << 6U,
    EVENT_VIDEO_VBLANK   = 1U << 7U,
    EVENT_MOUSE_PRESSED  = 1U << 8U,
    EVENT_TPS_EJECTED    = 1U << 9U,
    EVENT_TPS_INSERTED   = 1U << 10U,
    EVENT_AUDIO_NOTE_END = 1U << 11U,
};

#define AKAI_NUM_INTERRUPTS (AKAI_INVALID_INTERRUPT - INT_SER_RX)

#define AKAI_FS_FAT12 1
#define AKAI_FS_FAT16 2
#define AKAI_FS_FAT32 3

#define AKAI_DIR_SIZE 256

typedef struct AkaiDir {
    u8 data[AKAI_DIR_SIZE];
} AkaiDir;

#define ENTRY_FSIZE      0
#define ENTRY_FMOD       4
#define ENTRY_FCREAT     8
#define ENTRY_FATTRIB    12
#define ENTRY_FS         13
#define ENTRY_FNAME      16
#define ENTRY_FNAME_SIZE 32

#define AK_ATTR_READONLY (1 << 0)
#define AK_ATTR_HIDDEN   (1 << 1)
#define AK_ATTR_SYSTEM   (1 << 2)
#define AK_ATTR_DIR      (1 << 3)
#define AK_ATTR_ARCHIVE  (1 << 4)

#define ADIR_FSIZE(entry)   *(u32 *)((entry) + ENTRY_FSIZE)
#define ADIR_FMOD(entry)    *(u32 *)((entry) + ENTRY_FMOD)
#define ADIR_FCREAT(entry)  *(u32 *)((entry) + ENTRY_FCREAT)
#define ADIR_FATTRIB(entry) *(u8 *)((entry) + ENTRY_FATTRIB)
#define ADIR_FS(entry)      *(u8 *)((entry) + ENTRY_FS)
#define ADIR_FNAME(entry)   (char *)((entry) + ENTRY_FNAME)

#define AKAI_DIR_ENTRY_SIZE 128
typedef struct AkaiDirEntry {
    u8 data[AKAI_DIR_ENTRY_SIZE];
} AkaiDirEntry;

/* @AKAI */

i32  akai_syscall(u32 service);
void akai_exception(u8 vector, u32 fault_addr);
void akai_interrupt(u8 vector);

void kernel_panic(u8 vector, u32 fault_addr, struct ThreadCtx *ctx);
void kernel_panic_internal(const char *msg);

typedef void (*KernelInterruptHook)(u32 *, struct IPCMessage *);

// Returns the address of the previous handler or NULL if there is non
KernelInterruptHook kernel_hook_interrupt(enum TaleaInterrupt interrupt, KernelInterruptHook hook);

#endif /* HANDLERS_H */
