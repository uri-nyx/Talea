#ifndef AKAI_DEF
#define AKAI_DEF

#include "libsirius/types.h"

/* @AKAI: 002_DEF */
#define MAX_PROCESS 255

typedef u8 ProcessPID;
typedef int (*ProcessEntry)(int, char **);

// Syscall Error codes
#define A_OK               0U
#define A_ERROR            0xffffffffU // Just a general error
#define A_ERROR_IPC        0x00002000U // Bit 13 is reserved to signal an IPC error
#define A_ERROR_OOM        0xfffffff0U // Out of memory
#define A_ERROR_SEG        0xffffffe0U // Segfault
#define A_ERROR_INVAL      0xffffffd0U // Parameter to syscall invalid
#define A_ERROR_OOF        0xffffffc0U // Too many open files
#define A_ERROR_CLAIM      0xffffffb0U // Device already claimed
#define A_ERROR_CTL        0xffffffa0U // Error on device control layer
#define A_ERROR_FORBIDDEN  0xffffff90U // Forbidden action
#define A_ERROR_NOCHILDREN 0xffffff80U // Process has no children

#define A_ERROR_UNREACHEABLE 0xDEADBEEFU

#define FS_ERROR 0x80000000U // flag for FatFs errors
#define V_ERROR  0x90000000U // flag for video errors

// Process error
enum {
    P_ERROR_NONE = 0,
    P_ERROR_NO_SYSCALL,          // That syscall does not exist!
    P_ERROR_IPC_PID0,            // tried to initiate IPC as PID 0. Should be unreacheable
    P_ERROR_IPC_NOINIT,          // IPC is not initialized
    P_ERROR_IPC_INIT,            // Could not allocate IPC inbox page
    P_ERROR_BAD_ARGV,            // Argv could not be processed on exec()
    P_ERROR_FILE_TOO_LARGE,      // File to load was too large
    P_ERROR_BAD_POINTER,         // Pointer passed to a syscall was not valid
    P_ERROR_STACK_GROW,          // Failure to grow the stack
    P_ERROR_INC_TOO_LARGE,       // Increment to brk was too large
    P_ERROR_TOO_MANY_FILES,      // Too many open files
    P_ERROR_FILE_POOL_EXHAUSTED, // The file pool is full
    P_ERROR_NO_CTL_COMMAND,      // The issued command is not defined for the device
    P_ERROR_NO_DEV,              // Specified device does not exist
    P_ERROR_CANNOT_ATTACH,       // Cannot attach the specified device
    P_ERROR_NO_PORT,             // Driver error, no such port
    P_ERROR_NO_PERM,             // Process lacks permissions to attempt action
    P_ERROR_NOT_CHILD,           // The pid requested was not a child of the process
    P_ERROR_NO_PID,              // Could not acquire a PID
    P_ERROR_NO_EXEC,             // Not an executable file
    P_ERROR_NOENT,               // No directory entry
    P_ERROR_NOT_IMPLEMENTED,     //
};

#ifdef INCLUDE_DAYS_TABLE
static const u32 days_start_of_year[128] = {
    3652,  4017,  4382,  4747,  5113,  5478,  5843,  6208,  /* 1980 - 1987 */
    6574,  6939,  7304,  7669,  8035,  8400,  8765,  9130,  /* 1988 - 1995 */
    9496,  9861,  10226, 10591, 10957, 11322, 11687, 12052, /* 1996 - 2003 */
    12418, 12783, 13148, 13513, 13879, 14244, 14609, 14974, /* 2004 - 2011 */
    15340, 15705, 16070, 16435, 16801, 17166, 17531, 17896, /* 2012 - 2019 */
    18262, 18627, 18992, 19357, 19723, 20088, 20453, 20818, /* 2020 - 2027 */
    21184, 21549, 21914, 22279, 22645, 23010, 23375, 23740, /* 2028 - 2035 */
    24106, 24471, 24836, 25201, 25567, 25932, 26297, 26662, /* 2036 - 2043 */
    27028, 27393, 27758, 28123, 28489, 28854, 29219, 29584, /* 2044 - 2051 */
    29950, 30315, 30680, 31045, 31411, 31776, 32141, 32506, /* 2052 - 2059 */
    32872, 33237, 33602, 33967, 34333, 34698, 35063, 35428, /* 2060 - 2067 */
    35794, 36159, 36524, 36889, 37255, 37620, 37985, 38350, /* 2068 - 2075 */
    38716, 39081, 39446, 39811, 40177, 40542, 40907, 41272, /* 2076 - 2083 */
    41638, 42003, 42368, 42733, 43099, 43464, 43829, 44194, /* 2084 - 2091 */
    44560, 44925, 45290, 45655, 46021, 46386, 46751, 47116, /* 2092 - 2099 */
    47481, 47846, 48211, 48576, 48942, 49307, 49672, 50037  /* 2100 - 2107 */
};
#endif

/* @AKAI */

#endif /* AKAI_DEF */
