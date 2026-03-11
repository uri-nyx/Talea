#ifndef AKAI_H
#define AKAI_H


#define PAGE_SIZE 4096U

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long  usize;

typedef signed char  i8;
typedef signed short i16;
typedef signed int   i32;
typedef signed long  ssize;

typedef float f32;

typedef unsigned long  uptr;
typedef unsigned short dptr;

typedef i8 bool;
#define true  1
#define false 0

#define NULL ((void *)0)

struct akai_ringbuffer {
    u8   *data;
    usize size;
    usize head;
    usize tail;
};

struct akai_natural_ringbufferu8 {
    u8 data[256];
    u8 head;
    u8 tail;
};

struct akai_natural_ringbufferu16 {
    u16 data[256];
    u8  head;
    u8  tail;
};

struct akai_natural_ringbufferu32 {
    u32 data[256];
    u8  head;
    u8  tail;
};

#define BITMAP(T, bits) [((bits) + (sizeof((T)) - 1)) / sizeof((T))] 
#define BIT_TEST(bitmap, bit) (((bitmap)[(bit) >> 3] >> (bit & 0x7)) & 1)
#define BIT_SET(bitmap, bit)  ((bitmap)[(bit) >> 3] |= 1 << (bit & 0x7))
#define BIT_CLR(bitmap, bit)  ((bitmap)[(bit) >> 3] &= ~(1 << (bit & 0x7)))

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

/* STARNDARD TALEA DEVICE IDS */
#define DEVICE_ID_TTY     'K'
#define DEVICE_ID_VIDEO   'V'
#define DEVICE_ID_STORAGE 'D'
#define DEVICE_ID_AUDIO   'A'
#define DEVICE_ID_MOUSE   'M'

enum SerialStatus {
    SER_STATUS_DATA_AVAILABLE = 0x1, // RX FIFO not empty
    SER_STATUS_CARRIER_DETECT = 0x2, // Socket is connected
};

enum SerialControl {
    SER_CONTROL_INT_EN       = 0x1,  // Enable interrupt upon receiving data
    SER_CONTROL_MASTER_RESET = 0x80, // Master reset (clear all buffers)
};

enum TimerCSR {
    TIM_TIMEOUT_EN  = 0x01, // start/stop timeout
    TIM_INTERVAL_EN = 0x02, // start/stop interval
    TIM_GLOBAL_EN   = 0x80, // global enable
};

#define CONTROL    0x01
#define SHIFT      0x02
#define LALT       0x04
#define RGUI       0x08
#define CAPSLOCK   0x10
#define SCROLLLOCK 0x20
#define RALT       0x40

/* KEYBOARD CSR CONFIG */
enum KeyboardCSR {
    KB_IE_DOWN = 0x01,   // Fire interrupt when a key is pressed.
    KB_IE_UP   = 0x02,   // Fire interrupt when a key is released.
    KB_IE_CHAR = 0x04,   // Filter: Only fire if the CHARACTER register is
                         // non-zero (ASCII).
    KB_GLOBAL_EN = 0x80, // Master switch for Keyboard Interrupts.
    KB_GET_CSR   = 0x10, // If set, when read, CSR register returns the status of
                         // the Interrupt enable flags and this flag is cleared
                         // //TODO: Document this
};

enum PortsTerminal {
    TERMINAL_SERIAL_DATA     = 0x0,
    TERMINAL_SERIAL_STATUS   = 0x1,
    TERMINAL_SERIAL_CTRL     = 0x2,
    TERMINAL_SERIAL_RXCOUNT  = 0x3,
    TERMINAL_TIMER_TIMEOUT   = 0x6,
    TERMINAL_TIMER_INTERVAL  = 0x8,
    TERMINAL_TIMER_PRESCALER = 0xa,
    TERMINAL_TIMER_CSR       = 0xb,
    TERMINAL_KBD_CSR         = 0xc,
    TERMINAL_KBD_CHAR        = 0xd,
    TERMINAL_KBD_CODE        = 0xe,
};

enum PortsMouse {

    MOUSE_CSR          = 0,
    MOUSE_X            = 1,
    MOUSE_Y            = 3,
    MOUSE_HOTSPOT      = 5,
    MOUSE_BORDER_COLOR = 6,
    MOUSE_FILL_COLOR   = 7,
    MOUSE_ACCENT_COLOR = 8,
    MOUSE_SPRITE       = 9,
};

enum MouseCsr {
    MOUSE_BUTT_RIGHT = 1UL << 0,
    MOUSE_BUTT_LEFT  = 1UL << 1,
    MOUSE_CUSTOM     = 1UL << 5,
    MOUSE_VISIBLE    = 1UL << 6,
    MOUSE_IE         = 1UL << 7,
};

enum TaleaVideoMode {
    VIDEO_MODE_TEXT_MONO,
    VIDEO_MODE_TEXT_COLOR,
    VIDEO_MODE_GRAPHIC = 4,
    VIDEO_MODE_TEXT_AND_GRAPHIC
};

/* VIDEO DEVICE PORTS */
enum PortsVideo {
    VIDEO_COMMAND = 0x00,
    VIDEO_GPU0    = 1,
    VIDEO_GPU1    = 2,
    VIDEO_GPU2    = 3,
    VIDEO_GPU3    = 4,
    VIDEO_GPU4    = 5,
    VIDEO_GPU5    = 6,
    VIDEO_GPU6    = 7,
    VIDEO_GPU7    = 8,
    VIDEO_GPU8    = 9,
    VIDEO_GPU9    = 10,
    VIDEO_GPU10   = 11,
    VIDEO_CUR_X   = 0x0c,
    VIDEO_CUR_Y   = 0x0d,
    VIDEO_CSR     = 0x0e,
    VIDEO_ERR     = 0x0f,
};

enum VideoCommand {
    VIDEO_COMMAND_NOP,
    VIDEO_COMMAND_END_DRAWING,
    VIDEO_COMMAND_BEGIN_DRAWING,

    // Immediate commands
    VIDEO_COMMAND_SYS_INFO,
    VIDEO_COMMAND_BUFFER_INFO,

    // Queued commands
    VIDEO_COMMAND_CLEAR,
    VIDEO_COMMAND_SET_MODE,
    VIDEO_COMMAND_SET_ADDR,
    VIDEO_COMMAND_LOAD,
    VIDEO_COMMAND_BLIT,
    VIDEO_COMMAND_STRETCH_BLIT,
    VIDEO_COMMAND_PATTERN_FILL,

    // GPU commands (queued)
    VIDEO_COMMAND_DRAW_RECT,
    VIDEO_COMMAND_DRAW_LINE,
    VIDEO_COMMAND_DRAW_CIRCLE,
    VIDEO_COMMAND_DRAW_TRI,
    VIDEO_COMMAND_DRAW_BATCH,
    VIDEO_COMMAND_FILL_SPAN,
    VIDEO_COMMAND_FILL_VSPAN,

    VIDEO_COMMAND_SET_CSR, // queued for use in blits
    VIDEO_COMMAND_BIND_CTX,
    VIDEO_COMMAND_GET_MARKER,
};

enum VideoCSR {
    VIDEO_VBLANK_EN    = 0X01,
    VIDEO_RESET_REGS   = 0X02,
    VIDEO_CURSOR_EN    = 0X04,
    VIDEO_CURSOR_BLINK = 0X08,
    VIDEO_ROP0         = 0X10,
    VIDEO_ROP1         = 0X20,
    VIDEO_ROP2         = 0X40,
    VIDEO_QUEUE_FULL   = 0X80,
};

#define TEXTMODE_CHAR (0xFF000000U)
#define TEXTMODE_FG   (0X00FF0000U)
#define TEXTMODE_BG   (0X0000FF00U)
#define TEXTMODE_ATT  (0X000000FFU)

enum TextModeAttrib {

    TEXTMODE_ATT_CODEPAGE    = 0x01,
    TEXTMODE_ATT_ALT_FONT    = 0x02,
    TEXTMODE_ATT_TRANSPARENT = 0x04,
    TEXTMODE_ATT_OBLIQUE     = 0x08,
    TEXTMODE_ATT_DIM         = 0x10,
    TEXTMODE_ATT_UNDERLINE   = 0x20,
    TEXTMODE_ATT_BOLD        = 0x40,
    TEXTMODE_ATT_BLINK       = 0x80,
};

enum VideoClearFlag {
    VIDEO_CLEAR_FLAG_TB = 1 << 0,
    VIDEO_CLEAR_FLAG_FB = 1 << 1,
};

enum VideoBatchFlags {
    VIDEO_BATCH_TYPE0            = 1UL << 0,
    VIDEO_BATCH_TYPE1            = 1UL << 1,
    VIDEO_BATCH_BACKFACE_CULLING = 1UL << 2,
    VIDEO_BATCH_ZSHADING         = 1UL << 3,
    VIDEO_BATCH_MODIFY_TOPOLOGY  = 1UL << 4,
    VIDEO_BATCH_ABSOLUTE         = 1UL << 5,
    VIDEO_BATCH_PERSPECTIVE      = 1UL << 6,
    VIDEO_BATCH_DEPTH_SORT       = 1UL << 7,
};

enum VideoBatchType {
    VIDEO_BATCH_TYPE_POINT    = 0,
    VIDEO_BATCH_TYPE_LINE     = 1,
    VIDEO_BATCH_TYPE_TRISTRIP = 2,
    VIDEO_BATCH_TYPE_TRILIST  = 3,
};

enum VertexFlags {
    VX_END_OF_STRIP = 1UL << 0,
    VX_BRIGHT       = 1UL << 1,
    VX_HIDE         = 1UL << 2,
    VX_MARK         = 1UL << 3,
    VX_MARKID0      = 1UL << 4,
    VX_MARKID1      = 1UL << 5,
    VX_MARKID2      = 1UL << 6,
    VX_MARKID3      = 1UL << 7,
};

typedef i32 fx16;
#define FX16         16
#define TO_FX16(n)   ((fx16)((n) << FX16))
#define FROM_FX16(n) ((int)((n) >> FX16))

typedef struct {
    fx16 x, y, z;
    u8   color;
    u8   flags;
} vxfx16;

typedef struct {
    i16 x, y, z;
    u8  color;
    u8  flags;
} vxi16;

#define VXI16_REPR_SZ 8

#ifdef INCLUDE_SIN_TABLE
static const i16 sin_table[256] = {
    0,      402,    803,    1205,   1606,   2006,   2404,   2801,   3196,   3589,   3981,   4370,
    4756,   5139,   5519,   5896,   6269,   6639,   7004,   7366,   7723,   8075,   8423,   8765,
    9102,   9434,   9759,   10079,  10393,  10701,  11002,  11297,  11585,  11866,  12139,  12406,
    12665,  12916,  13159,  13395,  13622,  13842,  14053,  14255,  14449,  14634,  14810,  14978,
    15136,  15286,  15426,  15557,  15678,  15791,  15893,  15986,  16069,  16142,  16206,  16260,
    16305,  16339,  16364,  16379,  16384,  16379,  16364,  16339,  16305,  16260,  16206,  16142,
    16069,  15986,  15893,  15791,  15678,  15557,  15426,  15286,  15136,  14978,  14810,  14634,
    14449,  14255,  14053,  13842,  13622,  13395,  13159,  12916,  12665,  12406,  12139,  11866,
    11585,  11297,  11002,  10701,  10393,  10079,  9759,   9434,   9102,   8765,   8423,   8075,
    7723,   7366,   7004,   6639,   6269,   5896,   5519,   5139,   4756,   4370,   3981,   3589,
    3196,   2801,   2404,   2006,   1606,   1205,   803,    402,    0,      -402,   -803,   -1205,
    -1606,  -2006,  -2404,  -2801,  -3196,  -3589,  -3981,  -4370,  -4756,  -5139,  -5519,  -5896,
    -6269,  -6639,  -7004,  -7366,  -7723,  -8075,  -8423,  -8765,  -9102,  -9434,  -9759,  -10079,
    -10393, -10701, -11002, -11297, -11585, -11866, -12139, -12406, -12665, -12916, -13159, -13395,
    -13622, -13842, -14053, -14255, -14449, -14634, -14810, -14978, -15136, -15286, -15426, -15557,
    -15678, -15791, -15893, -15986, -16069, -16142, -16206, -16260, -16305, -16339, -16364, -16379,
    -16384, -16379, -16364, -16339, -16305, -16260, -16206, -16142, -16069, -15986, -15893, -15791,
    -15678, -15557, -15426, -15286, -15136, -14978, -14810, -14634, -14449, -14255, -14053, -13842,
    -13622, -13395, -13159, -12916, -12665, -12406, -12139, -11866, -11585, -11297, -11002, -10701,
    -10393, -10079, -9759,  -9434,  -9102,  -8765,  -8423,  -8075,  -7723,  -7366,  -7004,  -6639,
    -6269,  -5896,  -5519,  -5139,  -4756,  -4370,  -3981,  -3589,  -3196,  -2801,  -2404,  -2006,
    -1606,  -1205,  -803,   -402
};

/* Input: 0-255. Output: 1.14 Fixed Point */
#define FX1_14_SIN(a) (sin_table[(u8)(a)])
#define FX1_14_COS(a) (sin_table[(u8)((a) + 64)])
#endif

#define FX1_14_MUL(a, b) ((i32)(a) * (b) >> 14)

enum StorageMedium {
    NoMedia, // No medium detected
    Tps128K, // Tps, 256 sectors, 1 bank of 256 sectors, sector is 512 bytes
    Tps512K, // Tps, 1024 sectors, 4 banks of 256 sectors, sector is 512 bytes
    Tps1M,   // Tps, 2048 sectors, 8 banks of 256 sectors, sector is 512 bytes
    Hcs32M,  // Hcs, 65536 sectors, 1 banks of 65536 sectors, sector is 512 bytes
    Hcs64M,  // Hcs, 131072 sectors, 2 banks of 65536 sectors, sector is 512 bytes
    Hcs128M  // Hcs, 262144 sectors, 4 banks of 65536 sectors , sector is 512 bytes
};

enum StorageMediumLookup {
    STORAGE_MEDIUM_LOOKUP_TYPE,
    STORAGE_MEDIUM_LOOKUP_SECTORS,
    STORAGE_MEDIUM_LOOKUP_BANKS,
    STORAGE_MEDIUM_LOOKUP_BANK_SZ,
    STORAGE_MEDIUM_LOOKUP_SECTOR_SZ,
};

#define NOMEDIA 0
#define TPS     1
#define HCS     2

#ifdef INCLUDE_STORAGE_MEDIUM_LOOKUP
static const char *Storage_MediumNames[] = {
    "NoMedia", // No medium detected
    "Tps128K", // Tps, 256 sectors, 1 bank of 256 sectors, sector is 512 bytes
    "Tps512K", // Tps, 1024 sectors, 4 banks of 256 sectors, sector is 512 bytes
    "Tps1M",   // Tps, 2048 sectors, 8 banks of 256 sectors, sector is 512 bytes
    "Hcs32M",  // Hcs, 65536 sectors, 1 banks of 65536 sectors, sector is 512 bytes
    "Hcs64M",  // Hcs, 131072 sectors, 2 banks of 65536 sectors, sector is 512 bytes
    "Hcs128M"  // Hcs, 262144 sectors, 4 banks of 65536 sectors , sector is 512 bytes
};

static u32 Storage_MediumLookup[7][5] = {
    // TYPE, SECTOR COUNT, BANKS, BANK SIZE, SECTOR SIZE
    { NOMEDIA, 0, 0, 0, 0 },        { TPS, 256, 1, 256, 512 },     { TPS, 1024, 4, 256, 512 },
    { TPS, 2048, 8, 256, 512 },     { HCS, 65536, 1, 65536, 512 }, { HCS, 131072, 2, 65536, 512 },
    { HCS, 262144, 4, 65536, 512 },
};
#endif

enum StorageStatus {
    STOR_STATUS_READY = 0x01,
    STOR_STATUS_ERROR = 0x02,    // an error happened in the last operation
    STOR_STATUS_DONE  = 0x04,    // thread is done processing, it is safe to raise
                                 // interrupt
    STOR_STATUS_INSERTED = 0x08, // always high if inserted
    STOR_STATUS_WPROT    = 0x10, // always high if write protected
    STOR_STATUS_BOOT     = 0x20, // always high if disk is bootable. Also marked in
                                 // first sector
    STOR_STATUS_BUSY = 0x80,     // currently doing something, locked
};

enum TpsId { TPS_ID_A, TPS_ID_B, TPS_TOTAL_DRIVES };

/* TPS DEVICE PORTS DEPRECATED*/
enum PortsTps {
    TPS_COMMAND = 0x00,
    TPS_DATA    = 0x01,
    TPS_POINT   = 0x02,
    TPS_RESULT  = 0x04,
    TPS_STATUS  = 0x05
};

enum PortsStorage {
    STORAGE_TPS_COMMAND = 0x00,
    STORAGE_TPS_DATA    = 0x01,
    STORAGE_TPS_POINT   = 0x02,
    STORAGE_TPS_RESULT  = 0x04,
    STORAGE_TPS_STATUS  = 0x05,
    STORAGE_HCS_COMMAND = 0x06,
    STORAGE_HCS_DATA    = 0x07,
    STORAGE_HCS_SECTOR  = 0x08,
    STORAGE_HCS_POINT   = 0x0a,
    STORAGE_HCS_RESULT  = 0x0c,
    STORAGE_HCS_STATUS  = 0x0d,
};

enum StorageCommand {
    STORAGE_COMMAND_NOP,        // does nothing
    STORAGE_COMMAND_STORE,      // stores to sector DATA of current bank from POINT_H:POINT_L
    STORAGE_COMMAND_LOAD,       // loads from sector DATA of current bank in POINT_H:POINT_L
    STORAGE_COMMAND_SETCURRENT, // Sets the drive (A or B)
    STORAGE_COMMAND_GETCURRENT, // returns the current selected drive on STATUS_H
    STORAGE_COMMAND_MEDIUM,     // Returns the medium type in STATUS_H. Can infer size,
                                // sector count, etc based on a lookup table
    STORAGE_COMMAND_BANK,       // changes bank
    STORAGE_COMMAND_GETBANK,    // Return the current selected bank on STATUS_H
};

/* AUDIO DEVICE PORTS */
enum PortsAudio {
    AUDIO_ADDR           = 0x0,
    AUDIO_DATA           = 0x1,
    AUDIO_CSR            = 0x2,
    AUDIO_FNUMH          = 0x3,
    AUDIO_FNUML          = 0x4,
    AUDIO_DURH           = 0x5,
    AUDIO_DURL           = 0x6,
    AUDIO_CHANNEL_SELECT = 0x7,
    AUDIO_GLOBAL_STATUS0 = 0x8,
    AUDIO_GLOBAL_STATUS1 = 0x9,
    AUDIO_GLOBAL_STATUS2 = 0xa,
    AUDIO_GLOBAL_STATUS3 = 0xb,
    AUDIO_MASTER_VOL     = 0xc,
    AUDIO_PCM_FIFOH      = 0xd,
    AUDIO_PCM_FIFOL      = 0xe,
};

#define AUDIO_GLOB_NOTE_ENDED0       (1U << 0U)
#define AUDIO_GLOB_NOTE_ENDED1       (1U << 1U)
#define AUDIO_GLOB_NOTE_ENDED2       (1U << 2U)
#define AUDIO_GLOB_NOTE_ENDED3       (1U << 3U)
#define AUDIO_GLOB_NOTE_ENDED4       (1U << 4U)
#define AUDIO_GLOB_NOTE_ENDED5       (1U << 5U)
#define AUDIO_GLOB_NOTE_ENDED6       (1U << 6U)
#define AUDIO_GLOB_NOTE_ENDED7       (1U << 7U)
#define AUDIO_GLOB_NOTE_ENDED8       (1U << 8U)
#define AUDIO_GLOB_NOTE_ENDED_MASK   (0x1ffU)
#define AUDIO_GLOB_BUSY0             (1U << 9U)
#define AUDIO_GLOB_BUSY1             (1U << 10U)
#define AUDIO_GLOB_BUSY2             (1U << 11U)
#define AUDIO_GLOB_BUSY3             (1U << 12U)
#define AUDIO_GLOB_BUSY4             (1U << 13U)
#define AUDIO_GLOB_BUSY5             (1U << 14U)
#define AUDIO_GLOB_BUSY6             (1U << 15U)
#define AUDIO_GLOB_BUSY7             (1U << 16U)
#define AUDIO_GLOB_BUSY8             (1U << 17U)
#define AUDIO_GLOB_BUSY_MASK         (0x3fE00)
#define AUDIO_GLOB_PCM_FIFO_FULL     (1U << 20U)
#define AUDIO_GLOB_PCM_LOW_WATERMARK (1U << 21U)

enum AudioCsr {
    AUDIO_CSR_TRIGGER    = 1UL << 0, // Starts the note
    AUDIO_CSR_IE         = 1UL << 1, // interrupt enable
    AUDIO_CSR_LOOP       = 1UL << 2, // loops the note
    AUDIO_CSR_BUSY       = 1UL << 3,
    AUDIO_CSR_STOP       = 1UL << 4,
    AUDIO_CSR_NOTE_ENDED = 1UL << 5,
    AUDIO_CSR_GATE       = 1UL << 6, // changes mode to release note inmediately
};

#define AKAI_PROCESS_BASE      (0x080000U)
#define AKAI_PROCESS_END       (0xF5E000U)
#define AKAI_PROCESS_STACK_END (0xF60000U)
#define AKAI_PROCESS_STACK_TOP (0xF70000U)
#define AKAI_IPC_INBOX            (0xF7F000U)
#define AKAI_IPC_OUTBOX           (0xF80000U)
#define AKAI_IPC_SYSTEM_DASHBOARD (0xF81000U)
#define AKAI_TEXTBUFFER     (0xF84000U)
#define AKAI_FRAMEBUFFER    (0xFA5000U)
#define AKAI_ADDR_SPACE_END (0xFF0000U)

#define PDEV_TYPE_HW   0x1000
#define PDEV_TYPE_FILE 0x2000
#define PDEV_TYPE_VDEV 0x4000

#define PDEV_GET_RES_TYPE(r)           \
    ((r) & PDEV_TYPE_HW   ? RES_HW :   \
     (r) & PDEV_TYPE_FILE ? RES_FILE : \
     (r) & PDEV_TYPE_VDEV ? RES_VDEV : \
                            RES_NODEV)

#define PDEV_GET_RES(r) ((r) & ~(PDEV_TYPE_HW | PDEV_TYPE_FILE | PDEV_TYPE_VDEV))

#define BAD_RES 0xffffffff

enum HwDevices {
    DEV_FRAMEBUFFER,
    DEV_TEXTBUFFER,
    DEV_SYNTH,
    DEV_PCM,
    DEV_SERIAL,
    DEV_KEYBOARD,
    _DEV_NUM,
};

enum VDevices {
    _VDEV_START = 128,
    VDEV_ZERO   = 128,
    _VDEV_NUM,
};

enum DevProxy {
    PDEV_STDIN = 255,
    PDEV_STDOUT,
    PDEV_STDERR,
};

enum CanonIMode {
    IN_CANON = 1U << 0U,
    IN_ECHO  = 1U << 1U,
    IN_CRNL  = 1U << 2U,
    IN_BLOCK = 1U << 3U,
};

enum CanonOMode {
    OUT_NLCR = 1U << 0U,
};

// Common CTL commands
enum CtlCommand {
    DEV_RESET = 100,
    /* Proxy stream operations */
    PX_ATTACH = 1000,
    PX_READ,
    PX_WRITE,
    PX_POLL,
    PX_FLUSH,
    PX_SETCANON,
    PX_GETCANON,
    PX_GET_DEV,
};

enum KbCtl {
    KCTL_GETKEY,      // Non blocking key read
    KCTL_WAITKEY,     // Blocking key read
    KCTL_PEEK,        // Peek item from keyboard queue
    KCTL_QUEUE_COUNT, // Get no of items in queue
};

enum TxtCtl {
    TCTL_GET_INFO,
    TCTL_GET_CURSOR,
    TCTL_SET_CURSOR,
    TCTL_SET_ATTR,
    TCTL_GET_ATTR,
};

enum { RES_NODEV = 0, RES_HW, RES_VDEV, RES_FILE };

#define LINE_BUFFER_LEN 1023
#define	FA_READ				0x01
#define	FA_WRITE			0x02
#define	FA_OPEN_EXISTING	0x00
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define	FA_OPEN_APPEND		0x30
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

enum SeekFlags {
    SEEK_SET,
    SEEK_CUR,
    SEEK_END,
};

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

#define MAX_PROCESS     255
#define PROCESS_NAMELEN 10
#define MAX_OPEN_FILES  8

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

#define PRIORITY_KEYBOARD_INTERRUPT 4
#define PRIORITY_SERIAL_INTERRUPT   5
#define PRIORITY_STORAGE_INTERRUPT  5
#define PRIORITY_AUDIO_INTERRUPT    2
#define PRIORITY_TIMEOUT_INTERRUPT  6
#define PRIORITY_INTERVAL_INTERRUPT 6
#define PRIORITY_VBLANK_INTERRUPT   6
/* taken from raylib */
typedef enum {
    KEY_NULL = 0, // Key: NULL, used for no key pressed
    // Alphanumeric keys
    KEY_APOSTROPHE    = 39, // Key: '
    KEY_COMMA         = 44, // Key: ,
    KEY_MINUS         = 45, // Key: -
    KEY_PERIOD        = 46, // Key: .
    KEY_SLASH         = 47, // Key: /
    KEY_ZERO          = 48, // Key: 0
    KEY_ONE           = 49, // Key: 1
    KEY_TWO           = 50, // Key: 2
    KEY_THREE         = 51, // Key: 3
    KEY_FOUR          = 52, // Key: 4
    KEY_FIVE          = 53, // Key: 5
    KEY_SIX           = 54, // Key: 6
    KEY_SEVEN         = 55, // Key: 7
    KEY_EIGHT         = 56, // Key: 8
    KEY_NINE          = 57, // Key: 9
    KEY_SEMICOLON     = 59, // Key: ;
    KEY_EQUAL         = 61, // Key: =
    KEY_A             = 65, // Key: A | a
    KEY_B             = 66, // Key: B | b
    KEY_C             = 67, // Key: C | c
    KEY_D             = 68, // Key: D | d
    KEY_E             = 69, // Key: E | e
    KEY_F             = 70, // Key: F | f
    KEY_G             = 71, // Key: G | g
    KEY_H             = 72, // Key: H | h
    KEY_I             = 73, // Key: I | i
    KEY_J             = 74, // Key: J | j
    KEY_K             = 75, // Key: K | k
    KEY_L             = 76, // Key: L | l
    KEY_M             = 77, // Key: M | m
    KEY_N             = 78, // Key: N | n
    KEY_O             = 79, // Key: O | o
    KEY_P             = 80, // Key: P | p
    KEY_Q             = 81, // Key: Q | q
    KEY_R             = 82, // Key: R | r
    KEY_S             = 83, // Key: S | s
    KEY_T             = 84, // Key: T | t
    KEY_U             = 85, // Key: U | u
    KEY_V             = 86, // Key: V | v
    KEY_W             = 87, // Key: W | w
    KEY_X             = 88, // Key: X | x
    KEY_Y             = 89, // Key: Y | y
    KEY_Z             = 90, // Key: Z | z
    KEY_LEFT_BRACKET  = 91, // Key: [
    KEY_BACKSLASH     = 92, // Key: '\'
    KEY_RIGHT_BRACKET = 93, // Key: ]
    KEY_GRAVE         = 96, // Key: `
    // Function keys
    KEY_SPACE         = 32,  // Key: Space
    KEY_ESCAPE        = 256, // Key: Esc
    KEY_ENTER         = 257, // Key: Enter
    KEY_TAB           = 258, // Key: Tab
    KEY_BACKSPACE     = 259, // Key: Backspace
    KEY_INSERT        = 260, // Key: Ins
    KEY_DELETE        = 261, // Key: Del
    KEY_RIGHT         = 262, // Key: Cursor right
    KEY_LEFT          = 263, // Key: Cursor left
    KEY_DOWN          = 264, // Key: Cursor down
    KEY_UP            = 265, // Key: Cursor up
    KEY_PAGE_UP       = 266, // Key: Page up
    KEY_PAGE_DOWN     = 267, // Key: Page down
    KEY_HOME          = 268, // Key: Home
    KEY_END           = 269, // Key: End
    KEY_CAPS_LOCK     = 280, // Key: Caps lock
    KEY_SCROLL_LOCK   = 281, // Key: Scroll down
    KEY_NUM_LOCK      = 282, // Key: Num lock
    KEY_PRINT_SCREEN  = 283, // Key: Print screen
    KEY_PAUSE         = 284, // Key: Pause
    KEY_F1            = 290, // Key: F1
    KEY_F2            = 291, // Key: F2
    KEY_F3            = 292, // Key: F3
    KEY_F4            = 293, // Key: F4
    KEY_F5            = 294, // Key: F5
    KEY_F6            = 295, // Key: F6
    KEY_F7            = 296, // Key: F7
    KEY_F8            = 297, // Key: F8
    KEY_F9            = 298, // Key: F9
    KEY_F10           = 299, // Key: F10
    KEY_F11           = 300, // Key: F11
    KEY_F12           = 301, // Key: F12
    KEY_LEFT_SHIFT    = 340, // Key: Shift left
    KEY_LEFT_CONTROL  = 341, // Key: Control left
    KEY_LEFT_ALT      = 342, // Key: Alt left
    KEY_LEFT_SUPER    = 343, // Key: Super left
    KEY_RIGHT_SHIFT   = 344, // Key: Shift right
    KEY_RIGHT_CONTROL = 345, // Key: Control right
    KEY_RIGHT_ALT     = 346, // Key: Alt right
    KEY_RIGHT_SUPER   = 347, // Key: Super right
    KEY_KB_MENU       = 348, // Key: KB menu
    // Keypad keys
    KEY_KP_0        = 320, // Key: Keypad 0
    KEY_KP_1        = 321, // Key: Keypad 1
    KEY_KP_2        = 322, // Key: Keypad 2
    KEY_KP_3        = 323, // Key: Keypad 3
    KEY_KP_4        = 324, // Key: Keypad 4
    KEY_KP_5        = 325, // Key: Keypad 5
    KEY_KP_6        = 326, // Key: Keypad 6
    KEY_KP_7        = 327, // Key: Keypad 7
    KEY_KP_8        = 328, // Key: Keypad 8
    KEY_KP_9        = 329, // Key: Keypad 9
    KEY_KP_DECIMAL  = 330, // Key: Keypad .
    KEY_KP_DIVIDE   = 331, // Key: Keypad /
    KEY_KP_MULTIPLY = 332, // Key: Keypad *
    KEY_KP_SUBTRACT = 333, // Key: Keypad -
    KEY_KP_ADD      = 334, // Key: Keypad +
    KEY_KP_ENTER    = 335, // Key: Keypad Enter
    KEY_KP_EQUAL    = 336  // Key: Keypad =
} kbd_scancode;

#ifdef KEYS_INCLUDE_KEYNAME_MAP

struct KeyMapping {
    int   code;
    char *name;
};

static const struct KeyMapping Keys_keynames[] = {
    { KEY_NULL, "KEY_NULL" }, // Key: NULL, used for no key pressed
    // Alphanumeric keys
    { KEY_APOSTROPHE, "KEY_APOSTROPHE" },       // Key: '
    { KEY_COMMA, "KEY_COMMA" },                 // Key: ,
    { KEY_MINUS, "KEY_MINUS" },                 // Key: -
    { KEY_PERIOD, "KEY_PERIOD" },               // Key: .
    { KEY_SLASH, "KEY_SLASH" },                 // Key: /
    { KEY_ZERO, "KEY_ZERO" },                   // Key: 0
    { KEY_ONE, "KEY_ONE" },                     // Key: 1
    { KEY_TWO, "KEY_TWO" },                     // Key: 2
    { KEY_THREE, "KEY_THREE" },                 // Key: 3
    { KEY_FOUR, "KEY_FOUR" },                   // Key: 4
    { KEY_FIVE, "KEY_FIVE" },                   // Key: 5
    { KEY_SIX, "KEY_SIX" },                     // Key: 6
    { KEY_SEVEN, "KEY_SEVEN" },                 // Key: 7
    { KEY_EIGHT, "KEY_EIGHT" },                 // Key: 8
    { KEY_NINE, "KEY_NINE" },                   // Key: 9
    { KEY_SEMICOLON, "KEY_SEMICOLON" },         // Key: ;
    { KEY_EQUAL, "KEY_EQUAL" },                 // Key: =
    { KEY_A, "KEY_A" },                         // Key: A | a
    { KEY_B, "KEY_B" },                         // Key: B | b
    { KEY_C, "KEY_C" },                         // Key: C | c
    { KEY_D, "KEY_D" },                         // Key: D | d
    { KEY_E, "KEY_E" },                         // Key: E | e
    { KEY_F, "KEY_F" },                         // Key: F | f
    { KEY_G, "KEY_G" },                         // Key: G | g
    { KEY_H, "KEY_H" },                         // Key: H | h
    { KEY_I, "KEY_I" },                         // Key: I | i
    { KEY_J, "KEY_J" },                         // Key: J | j
    { KEY_K, "KEY_K" },                         // Key: K | k
    { KEY_L, "KEY_L" },                         // Key: L | l
    { KEY_M, "KEY_M" },                         // Key: M | m
    { KEY_N, "KEY_N" },                         // Key: N | n
    { KEY_O, "KEY_O" },                         // Key: O | o
    { KEY_P, "KEY_P" },                         // Key: P | p
    { KEY_Q, "KEY_Q" },                         // Key: Q | q
    { KEY_R, "KEY_R" },                         // Key: R | r
    { KEY_S, "KEY_S" },                         // Key: S | s
    { KEY_T, "KEY_T" },                         // Key: T | t
    { KEY_U, "KEY_U" },                         // Key: U | u
    { KEY_V, "KEY_V" },                         // Key: V | v
    { KEY_W, "KEY_W" },                         // Key: W | w
    { KEY_X, "KEY_X" },                         // Key: X | x
    { KEY_Y, "KEY_Y" },                         // Key: Y | y
    { KEY_Z, "KEY_Z" },                         // Key: Z | z
    { KEY_LEFT_BRACKET, "KEY_LEFT_BRACKET" },   // Key: [
    { KEY_BACKSLASH, "KEY_BACKSLASH" },         // Key: '\'
    { KEY_RIGHT_BRACKET, "KEY_RIGHT_BRACKET" }, // Key: ]
    { KEY_GRAVE, "KEY_GRAVE" },                 // Key: `
    // Function keys
    { KEY_SPACE, "KEY_SPACE" },                 // Key: Space
    { KEY_ESCAPE, "KEY_ESCAPE" },               // Key: Esc
    { KEY_ENTER, "KEY_ENTER" },                 // Key: Enter
    { KEY_TAB, "KEY_TAB" },                     // Key: Tab
    { KEY_BACKSPACE, "KEY_BACKSPACE" },         // Key: Backspace
    { KEY_INSERT, "KEY_INSERT" },               // Key: Ins
    { KEY_DELETE, "KEY_DELETE" },               // Key: Del
    { KEY_RIGHT, "KEY_RIGHT" },                 // Key: Cursor right
    { KEY_LEFT, "KEY_LEFT" },                   // Key: Cursor left
    { KEY_DOWN, "KEY_DOWN" },                   // Key: Cursor down
    { KEY_UP, "KEY_UP" },                       // Key: Cursor up
    { KEY_PAGE_UP, "KEY_PAGE_UP" },             // Key: Page up
    { KEY_PAGE_DOWN, "KEY_PAGE_DOWN" },         // Key: Page down
    { KEY_HOME, "KEY_HOME" },                   // Key: Home
    { KEY_END, "KEY_END" },                     // Key: End
    { KEY_CAPS_LOCK, "KEY_CAPS_LOCK" },         // Key: Caps lock
    { KEY_SCROLL_LOCK, "KEY_SCROLL_LOCK" },     // Key: Scroll down
    { KEY_NUM_LOCK, "KEY_NUM_LOCK" },           // Key: Num lock
    { KEY_PRINT_SCREEN, "KEY_PRINT_SCREEN" },   // Key: Print screen
    { KEY_PAUSE, "KEY_PAUSE" },                 // Key: Pause
    { KEY_F1, "KEY_F1" },                       // Key: F1
    { KEY_F2, "KEY_F2" },                       // Key: F2
    { KEY_F3, "KEY_F3" },                       // Key: F3
    { KEY_F4, "KEY_F4" },                       // Key: F4
    { KEY_F5, "KEY_F5" },                       // Key: F5
    { KEY_F6, "KEY_F6" },                       // Key: F6
    { KEY_F7, "KEY_F7" },                       // Key: F7
    { KEY_F8, "KEY_F8" },                       // Key: F8
    { KEY_F9, "KEY_F9" },                       // Key: F9
    { KEY_F10, "KEY_F10" },                     // Key: F10
    { KEY_F11, "KEY_F11" },                     // Key: F11
    { KEY_F12, "KEY_F12" },                     // Key: F12
    { KEY_LEFT_SHIFT, "KEY_LEFT_SHIFT" },       // Key: Shift left
    { KEY_LEFT_CONTROL, "KEY_LEFT_CONTROL" },   // Key: Control left
    { KEY_LEFT_ALT, "KEY_LEFT_ALT" },           // Key: Alt left
    { KEY_LEFT_SUPER, "KEY_LEFT_SUPER" },       // Key: Super left
    { KEY_RIGHT_SHIFT, "KEY_RIGHT_SHIFT" },     // Key: Shift right
    { KEY_RIGHT_CONTROL, "KEY_RIGHT_CONTROL" }, // Key: Control right
    { KEY_RIGHT_ALT, "KEY_RIGHT_ALT" },         // Key: Alt right
    { KEY_RIGHT_SUPER, "KEY_RIGHT_SUPER" },     // Key: Super right
    { KEY_KB_MENU, "KEY_KB_MENU" },             // Key: KB menu
    // Keypad keys
    { KEY_KP_0, "KEY_KP_0" },               // Key: Keypad 0
    { KEY_KP_1, "KEY_KP_1" },               // Key: Keypad 1
    { KEY_KP_2, "KEY_KP_2" },               // Key: Keypad 2
    { KEY_KP_3, "KEY_KP_3" },               // Key: Keypad 3
    { KEY_KP_4, "KEY_KP_4" },               // Key: Keypad 4
    { KEY_KP_5, "KEY_KP_5" },               // Key: Keypad 5
    { KEY_KP_6, "KEY_KP_6" },               // Key: Keypad 6
    { KEY_KP_7, "KEY_KP_7" },               // Key: Keypad 7
    { KEY_KP_8, "KEY_KP_8" },               // Key: Keypad 8
    { KEY_KP_9, "KEY_KP_9" },               // Key: Keypad 9
    { KEY_KP_DECIMAL, "KEY_KP_DECIMAL" },   // Key: Keypad .
    { KEY_KP_DIVIDE, "KEY_KP_DIVIDE" },     // Key: Keypad /
    { KEY_KP_MULTIPLY, "KEY_KP_MULTIPLY" }, // Key: Keypad *
    { KEY_KP_SUBTRACT, "KEY_KP_SUBTRACT" }, // Key: Keypad -
    { KEY_KP_ADD, "KEY_KP_ADD" },           // Key: Keypad +
    { KEY_KP_ENTER, "KEY_KP_ENTER" },       // Key: Keypad Enter
    { KEY_KP_EQUAL, "KEY_KP_EQUAL" },       // Key: Keypad =
    { -1, NULL }
};

static const char *keys_kname_KEY_UNKNOW = "KEY_UNKNOWN";

static const char *keys_getkeyname(int scancode)
{
    struct KeyMapping k;
    usize             i;

    for (i = 0; i < 107; i++) {
        if (Keys_keynames[i].code == scancode) return Keys_keynames[i].name;
        if (Keys_keynames[i].code == -1) break;
    }

    return "KEY_UNKNONW";
}

#endif


#ifdef INCLUDE_ANSI_IN
static const char *ANSI_UP     = "\x1B[A";
static const char *ANSI_DOWN   = "\x1B[B";
static const char *ANSI_LEFT   = "\x1B[D";
static const char *ANSI_RIGHT  = "\x1B[C";
static const char *ANSI_HOME   = "\x1B[H";
static const char *ANSI_END    = "\x1B[F";
static const char *ANSI_DELETE = "\x1B[3~";
static const char *ANSI_PGUP   = "\x1B[5~";
static const char *ANSI_PGDOWN = "\x1B[6~";

static const char *ANSI_F1 = "\x1BOP";
static const char *ANSI_F2 = "\x1BOQ";
static const char *ANSI_F3 = "\x1BOR";
static const char *ANSI_F4 = "\x1BOS";
#endif

enum AnsiCmd {
    ANSI_NONE = 0,
    ANSI_CHAR = 1,

    /* Standard CSI */
    ANSI_CUU = 'A',
    ANSI_CUD = 'B',
    ANSI_CUF = 'C',
    ANSI_CUB = 'D',
    ANSI_CNL = 'E',
    ANSI_CPL = 'F',
    ANSI_CHA = 'G',
    ANSI_CUP = 'H',
    ANSI_ED  = 'J',
    ANSI_EL  = 'K',
    ANSI_SGR = 'm',
    ANSI_SU  = 'S',
    ANSI_SD  = 'T',

    /* Private SCO modes */
    ANSI_SCP = 's',
    ANSI_RCP = 'u',
    /* Private modes use a bitmask (e.g., 0x100) to stay unique */
    ANSI_PRIV_H = 'h' | 0x100,
    ANSI_PRIV_L = 'l' | 0x100
};

enum AnsiSGR {
    ANSI_SGR_RESET            = 0,
    ANSI_SGR_BOLD             = 1,
    ANSI_SGR_DIM              = 2,
    ANSI_SGR_ITALIC           = 3,
    ANSI_SGR_UNDERLINE        = 4,
    ANSI_SGR_BLINK            = 5,
    ANSI_SGR_REVERSE          = 7,
    ANSI_SGR_TRANSPARENT      = 8,
    ANSI_SGR_FONT_0           = 10,
    ANSI_SGR_FONT_1           = 11,
    ANSI_SGR_FONT_2           = 12,
    ANSI_SGR_FONT_3           = 13,
    ANSI_SGR_NORMAL_INTENSITY = 22,
    ANSI_SGR_NOT_ITALIC       = 23,
    ANSI_SGR_NOT_UNDERLINE    = 24,
    ANSI_SGR_NOT_BLINK        = 25,
    ANSI_SGR_NOT_REVERSE      = 27,
    ANSI_SGR_NOT_TRANSPARENT  = 28,
    ANSI_SGR_SET_FG           = 30,
    ANSI_SGR_SET_FG_256       = 38,
    ANSI_SGR_DEFAULT_FG       = 39,
    ANSI_SGR_SET_BG           = 40,
    ANSI_SGR_SET_BG_256       = 48,
    ANSI_SGR_DEFAULT_BG       = 49,

};

#define ANSI_MAX_PARAMS 16

void ak_exit(int exit_code);
void ak_yield(void);
int ak_rfork(u32 flags,u32 heirloom);
int ak_exec(const char* path,int argc,char** argv,int flags);
int ak_wait(int pid,int* status,int options);




int ak_ipc_init(void);


int ak_dev_in(u32 devnum,u8 port);
int ak_dev_out(u32 devnum,u8 port,u8 value);
int ak_dev_ctl(u32 devnum,u32 cmd,void* buf,u32 len);
int ak_dev_claim(u32 devnum);
int ak_open(const char* path,int open_mode);

int ak_close(int fd);
int ak_unlink(const char* path);
int ak_rename(const char* old_name,const char* new_name);
int ak_mkdir(const char* path);


int ak_read(int fd,void* buf,u32 count);
int ak_write(int fd,void * buf,u32 count);
int ak_seek(int fd,i32 offset,int whence);

int ak_stat(const char* path,AkaiDirEntry* out_entry);

int ak_opendir(AkaiDir* dir,const char* path,int flags);
int ak_closedir(AkaiDir* dir,int flags);
int ak_readdir(AkaiDir* dir,AkaiDirEntry* out_entry,int flags);
int ak_chdir(const char* path);
int ak_getcwd(void* buf,u32 len);



void* ak_sbrk(u32 brk_increment);


int ak_error(ProcessPID pid);
int ak_getpid(void);
void ak_abort(void);
int ak_time(void);
int ak_clock(u32* user,u32* system,ProcessPID pid);
int ak_calendar(u32* calendar_1,u32* calendar_2);
int ak_getppid(void);
int ak_set_preempt(u32 mode);
int ak_asm(const char* line,usize len,usize curr_address,u8* out,usize out_size);
#endif /* AKAI_H */
