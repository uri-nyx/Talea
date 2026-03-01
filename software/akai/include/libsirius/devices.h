#ifndef DEVICES_H
#define DEVICES_H

/*
    DEFINITIONS ABOUT KNOWN DEVICES
*/

#include "types.h"

#define TALEA_MAGIC_ARM_SEQUENCE     0xA5
#define TALEA_MAGIC_TRIGGER_SEQUENCE 0x5A

#define DEVICE_SYSTEM 0x110
#define DEVICE_MAP    0x100

/* Official IDs registered by the House of Talea*/
#define VENDOR_ID 'T'
#define ARCH_ID   'S'

/* @AKAI: 100_DEVICES */
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

/* @AKAI */

enum {
    TALEA_SYSTEM_CALENDAR_MODE,
    TALEA_SYSTEM_UNIXTIME_MODE,
    TALEA_SYSTEM_MICROS_MODE,
    TALEA_SYSTEM_MILLIS_MODE,
    TALEA_SYSTEM_INST_MODE
};

enum {
    TALEA_SYSTEM_WIN_OP_LOAD,
    TALEA_SYSTEM_WIN_OP_STORE,
};

/* SYSTEM DEVICE PORTS */
enum PortsSystem {
    /* Identity */
    REG_SYSTEM_ARCH_ID = DEVICE_SYSTEM,
    REG_SYSTEM_VENDOR_ID,
    REG_SYSTEM_VERSION,       // The system version as [7:4] Major [3:0] Minor
    REG_SYSTEM_MEMSIZE_FLASH, // The memory size as a multiple of 64KB - 1
    REG_SYSTEM_CLOCK,         // The clock frequency
    REG_SYSTEM_DEVICE_NUM,    // The number of device slots used

    /* Entropy */
    REG_SYSTEM_RNG, // A random byte

    /* Power */
    REG_SYSTEM_POWER, // Poweroff register

    /* Time */
    REG_SYSTEM_YEAR,
    REG_SYSTEM_MONTH,
    REG_SYSTEM_DAY,
    REG_SYSTEM_HOUR,
    REG_SYSTEM_MINUTE,
    REG_SYSTEM_SECOND,
    REG_SYSTEM_MILLIS,
    REG_SYSTEM_COUNTER,

    /* Exceptions */
    REG_SYSTEM_EXCEPTION,  // Last or current exception raised
    REG_SYSTEM_FAULT_ADDR, // Last address where an exception originated

    /* CPU State */
    REG_SYSTEM_CYCLES_INSTRET = REG_SYSTEM_FAULT_ADDR + 4, // number of cycñes/ instructions
                                                           // executed

    /* Context Windowing */
    REG_SYSTEM_CWP = REG_SYSTEM_CYCLES_INSTRET + 8,
    REG_SYSTEM_WIN_SPILLED,
    REG_SYSTEM_WIN_SEL,
    REG_SYSTEM_WIN_OP,
    REG_SYSTEM_WIN_BUFF,

    /* MMU registers */
    REG_SYSTEM_PDT = REG_SYSTEM_WIN_BUFF + 128, // Current pdt base in DATA memory
    REG_SYSTEM_TLB = REG_SYSTEM_PDT + 2,        // Writing flushes the TLB
    REG_SYSTEM_MMU,                             // enable/disable MMU

    /* Context switching */
    REG_SYSTEM_USP, // Saved user stack pointer // NOTE; 4 bytes
    /* Interrupts */
    REG_SYSTEM_INTERRUPT = REG_SYSTEM_USP + 4, // Last interrupt acknowledged
    REG_SYSTEM_FAULT_CAUSE,
};

enum PTEFlags {
    PTE_V   = (1UL << 0), // Valid
    PTE_R   = (1UL << 1), // Read
    PTE_W   = (1UL << 2), // Write
    PTE_X   = (1UL << 3), // Execute
    PTE_U   = (1UL << 4), // User
    PTE_A   = (1UL << 6), // Accessed (Hardware sets this)
    PTE_D   = (1UL << 7), // Dirty (Hardware sets this on write)
    PTE_RW  = PTE_V | PTE_R | PTE_W,
    PTE_RX  = PTE_V | PTE_R | PTE_X,
    PTE_RWX = PTE_RW | PTE_X,
};

enum MemAccessType {
    ACCESS_READ = 1,
    ACCESS_WRITE,
    ACCESS_EXEC,
    CAUSE_PERM     = 1 << 5,
    CAUSE_UNMAPPED = 1 << 6,
    CAUSE_LEAF     = 1 << 7,
};

#endif /* DEVICES_H */
