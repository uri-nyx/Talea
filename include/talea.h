#ifndef TALEA_H
#define TALEA_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define HACK_HIDPI 3

#ifndef DO_NOT_INCLUDE_RAYLIB
// Due to confilcts with the WinAPI
#include "raylib.h"
#else
// definitions we need from raylib
#include "need-from-raylib.h"
#endif

#ifdef _WIN32
typedef unsigned long long talea_net_t;
#else
typedef int talea_net_t;
#endif

#include "emu2413.h"
#include "frontend/config.h"

// MOVE THIS
// #define DEBUG_LOG_MEMORY_ACCESS
#define TALEA_LOG_ERROR   printf
#define TALEA_LOG_WARNING printf
#define TALEA_LOG_TRACE   printf

/* TYPE SHORTHANDS */
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/* PATHS */
#ifdef _WIN32
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 130
#endif

// SHADERS
#define VIDEO_FONT_FILE_HACK                                                       \
    "resources" PATH_SEPARATOR "emulated" PATH_SEPARATOR "firmware" PATH_SEPARATOR \
    "font_from_talea.fnt"
#define FONT_PATH "resources" PATH_SEPARATOR "fonts" PATH_SEPARATOR
#define SHADERS_PATH(name)                                                                       \
    TextFormat("resources" PATH_SEPARATOR "shaders" PATH_SEPARATOR "glsl%i" PATH_SEPARATOR "%s", \
               GLSL_VERSION, name)
#if GLSL_VERSION == 130
// #define VERTEX_SHADER 0
#define VERTEX_SHADER SHADERS_PATH("vertex.vs")
#else
#define VERTEX_SHADER 0
#endif

#define FPS                    60
#define FRAME_MS               ((float)1000 / (float)FPS)
#define HZ                     10200000
#define CYCLES_PER_FRAME_QUOTA (HZ / FPS)

/* --- SYSTEM CONSTANTS --- */
#define TALEA_WINDOW_TITLE "The Taleä Sirius System"

// The OS window size //TODO: put in config file
#define TALEA_WINDOW_WIDTH  800
#define TALEA_WINDOW_HEIGHT 500

// The virtual screen size
#define TALEA_SCREEN_WIDTH  640
#define TALEA_SCREEN_HEIGHT 480

// TODO: put in config file
#define HCS_FILE_LOCATION \
    "resources" PATH_SEPARATOR "emulated" PATH_SEPARATOR "devices" PATH_SEPARATOR "hcs"
#define TPS_IMAGES_DEFAULT_DIR PATH_SEPARATOR "resources" PATH_SEPARATOR "tps_images"
#define HCS_DRIVES             4

#define CONFIG_FILE_PATH            "resources/config.toml"
#define TALEA_NUM_INSTALLED_DEVICES 8
#define VENDOR_ID                   'T'
#define ARCH_ID                     'S'
#define HZ                          10200000
#define FPS                         60
#define TALEA_MAIN_MEM_SZ           (1U << 24) // 16MB
#define TALEA_DATA_MEM_SZ           (1U << 16) // 64KB
#define TALEA_MAX_FIRMWARE_SIZE     (32 * 1024)
// TODO: This should be a fixed  address at the top of the  address space, no matter  how much
// memory we have
//  0xFF8000
#define TALEA_FIRMWARE_ADDRESS (TALEA_MAIN_MEM_SZ - TALEA_MAX_FIRMWARE_SIZE)

#define TALEA_MEM_SZ_MB 16

enum DeviceID {
    ID_TERMINAL = 'K',
    ID_VIDEO    = 'V',
    ID_STORAGE  = 'D',
    ID_AUDIO    = 'A',
    ID_MOUSE    = 'M',
};

/* Hardware Constraints */
#define TALEA_WITH_MMU  0       // No MMU
#define TALEA_IVT_FIXED 1       // set to 0 to enable runtime ivt config
#define TALEA_IVT_BASE  0xF800U // only used with fised ivt

/* --- INTERRUPTS AND CPU EXCEPTIONS --- */

#define PRIORITY_KEYBOARD_INTERRUPT 4
#define PRIORITY_SERIAL_INTERRUPT   5
#define PRIORITY_STORAGE_INTERRUPT  5
#define PRIORITY_AUDIO_INTERRUPT    2
#define PRIORITY_TIMEOUT_INTERRUPT  6
#define PRIORITY_INTERVAL_INTERRUPT 6
#define PRIORITY_VBLANK_INTERRUPT   6

enum TaleaInterrupt {
    EXCEPTION_RESET,
    EXCEPTION_BUS_ERROR = 0x2,
    EXCEPTION_ADDRESS_ERROR,
    EXCEPTION_ILLEGAL_INSTRUCTION_TALEA,
    EXCEPTION_DIVISION_ZERO,
    EXCEPTION_PRIVILEGE_VIOLATION,
    EXCEPTION_PAGE_FAULT,
    EXCEPTION_ACCESS_VIOLATION_TALEA,

    INT_TTY_TRANSMIT = 0xA,
    INT_KBD_CHAR,
    INT_KBD_SCAN,
    INT_TPS_FINISH,
    INT_HCS_FINISH,
    INT_TIMER_TIMEOUT,
    INT_TIMER_INTERVAL,
    INT_VIDEO_REFRESH,
    INT_MOUSE_PRESSED,
    INT_TPS_EJECTED,
    INT_TPS_INSERTED,
    INT_AUDIO0_NOTE_END,
    INT_AUDIO1_NOTE_END,
    INT_AUDIO2_NOTE_END,
    INT_AUDIO3_NOTE_END
};

/* --- DEVICE STRUCTURES --- */

typedef struct TaleaMachine TaleaMachine; // Forward declaration

typedef struct CpuState {
    // Register file
    u32 pc, virtualPc;
    u32 ssp, usp;
    u32 status;
    u32 gpr[32];

    // Clock
    u64 frequency, cycles, ticks;

    // Interrupts
    enum TaleaInterrupt exception;
    u8                  current_ipl, pending_ipl;
    u8                  pending_interrupts[8];
    u8                  highest_pending_interrupt;

    // Control lines
    bool poweroff, restart;
} CpuState;

void Cpu_Reset(TaleaMachine *m, TaleaConfig *config, bool is_restart);
void Cpu_RunCycles(TaleaMachine *m, u32 cycles);

#define CONTROL    0x01
#define SHIFT      0x02
#define LALT       0x04
#define RGUI       0x08
#define CAPSLOCK   0x10
#define SCROLLLOCK 0x20
#define RALT       0x40

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

typedef struct {
    u16  scancode;
    u8   character;
    u8   modifiers;
    bool is_down;
} KbdEvent;

#define KB_QUEUE_LEN 32
typedef struct {
    KbdEvent queue[KB_QUEUE_LEN];
    int      head, tail;

    u8  csr;
    u16 modifiers;

} TerminalKeyboard;

void Keyboard_ProcessKeypress(TaleaMachine *m, bool is_down, int key, u8 chr, u16 mod);

enum TimerCSR {
    TIM_TIMEOUT_EN  = 0x01, // start/stop timeout
    TIM_INTERVAL_EN = 0x02, // start/stop interval
    TIM_GLOBAL_EN   = 0x80, // global enable
};

typedef struct {
    u8  csr;
    u8  prescaler;
    u16 timeout_counter;
    u16 interval_counter;

    u64 cycles;
    u16 interval_reload;

    // Internal cycle accumulators
    u32 accum_t;
    u32 accum_i;
} TerminalTimer;

void Timer_Update(TaleaMachine *m, u32 cycles);

enum SerialStatus {
    SER_STATUS_DATA_AVAILABLE = 0x1, // RX FIFO not empty
    SER_STATUS_CARRIER_DETECT = 0x2, // Socket is connected
    SER_STATUS_BUFFER_OVERRUN = 0x4,
};

enum SerialControl {
    SER_CONTROL_INT_EN       = 0x1,  // Enable interrupt upon receiving data
    SER_CONTROL_MASTER_RESET = 0x80, // Master reset (clear all buffers)
};

enum HayesResponse { HAYES_OK, HAYES_CONNECT, HAYES_RING, HAYES_NO_CARRIER, HAYES_ERROR };
enum ModemState {
    MODEM_STATE_COMMAND,      /* Accepting AT commands */
    MODEM_STATE_DIALING,      /* TCP connect() in progress (Non-blocking) */
    MODEM_STATE_DATA,         /* Transparent pass-through mode */
    MODEM_STATE_DISCONNECTING /* Closing socket/Cleaning up */
};

#define MODEM_CMD_BUFFER_SIZE 65
#define GET_GUARD_TIME(m)     ((double)((m)->s_regs[12]) / 50.0)

typedef struct {
    // --- Internal State Machine ---
    enum ModemState state;

    char cmd_buffer[MODEM_CMD_BUFFER_SIZE];         // Stores AT commands
    char last_valid_command[MODEM_CMD_BUFFER_SIZE]; // Stores The last valid executed AT command
    int  cmd_pos;

    // Configuration
    bool echo_enabled;
    bool defer_response;
    bool last_was_A;
    bool verbose_mode;

    u8 s_regs[16]; // S registers
    u8 current_s_reg;

    // --- Escape Sequence (+++) ---
    double last_tx_time;  // For "Guard Time" checks
    int    plus_count;    // Counts '+' symbols
    bool   waiting_after; // waiting after escape

    // Networking
    double dial_start;

} HayesModem;

void Modem_CheckEscapeSequence(TaleaMachine *m, u8 byte);
void Modem_Update(TaleaMachine *m);
void Modem_ResetSregs(HayesModem *modem);
void Modem_SendResponse(TaleaMachine *m, enum HayesResponse code);

#define SERIAL_BAUD_RATE 9600
#define SERIAL_FIFO_SIZE 256
typedef struct {
    // --- Hardware Registers ---
    u8 rx_fifo[SERIAL_FIFO_SIZE];
    u8 head;    // Write pointer (for host)
    u8 tail;    // Read pointer (for guest CPU)
    u8 status;  // Status bits (Data Avail, Carrier)
    u8 control; // Interrupt enable flags

    HayesModem modem;

    // --- Throttling logic ---
    double last_update_time;
    double byte_credit;

    // --- Host/Networking Layer ---
    talea_net_t host_socket;  // The "Active Line" (Could be Telnet or BBS)
    talea_net_t server_fd;    // The "Wall Jack" (Always listening for incoming)
    bool        is_listening; // State for AT_LISTEN
} TerminalSerial;

void Serial_PushByte(TaleaMachine *m, u8 byte);
void Serial_PushString(TaleaMachine *m, const u8 *str);

typedef struct DeviceTerminal {
    TerminalKeyboard kb;
    TerminalTimer    timer;
    TerminalSerial   serial;
} DeviceTerminal;

void Terminal_Reset(TaleaMachine *m, TaleaConfig *conf, bool is_restart);

u8   Terminal_ReadHandler(TaleaMachine *m, u16 addr);
void Terminal_WriteHandler(TaleaMachine *m, u16 addr, u8 value);

#define TALEA_FRAMEBUFFER_ADDR 0xE60000
#define TALEA_CHARBUFFER_ADDR  0xE51000

enum TextModeAttrib {
    TEXTMODE_CHAR = 0xFF000000,
    TEXTMODE_FG   = 0X00FF0000,
    TEXTMODE_BG   = 0X0000FF00,
    TEXTMODE_ATT  = 0X000000FF,

    TEXTMODE_ATT_CODEPAGE    = 0x01,
    TEXTMODE_ATT_ALT_FONT    = 0x02,
    TEXTMODE_ATT_TRANSPARENT = 0x04,
    TEXTMODE_ATT_OBLIQUE     = 0x08,
    TEXTMODE_ATT_DIM         = 0x10,
    TEXTMODE_ATT_UNDERLINE   = 0x20,
    TEXTMODE_ATT_BOLD        = 0x40,
    TEXTMODE_ATT_BLINK       = 0x80,
};

enum VideoCSR {
    VIDEO_VBLANK_EN    = 0X01,
    VIDEO_RESET_REGS   = 0X02,
    VIDEO_CURSOR_EN    = 0X04,
    VIDEO_CURSOR_BLINK = 0X08,
    VIDEO_ROP0         = 0X10,
    VIDEO_ROP1         = 0X20,
    VIDEO_RESERVED     = 0X40,
    VIDEO_QUEUE_FULL   = 0X80,
};

enum VideoROP {
    VIDEO_CONFIG_ROP_COPY = 0,
    VIDEO_CONFIG_ROP_AND  = VIDEO_ROP0,
    VIDEO_CONFIG_ROP_OR   = VIDEO_ROP1,
    VIDEO_CONFIG_ROP_XOR  = VIDEO_ROP1 | VIDEO_ROP0,
};

enum VideoFontID {
    VIDEO_FONT_BASE_CP0,
    VIDEO_FONT_BASE_CP1,
    VIDEO_ALT_FONT_CP0,
    VIDEO_ALT_FONT_CP1,
    VIDEO_FONT_IDS
};

enum VideoMode {
    VIDEO_MODE_TEXT_MONO,
    VIDEO_MODE_TEXT_COLOR,
    VIDEO_MODE_GRAPHIC = 4,
    VIDEO_MODE_TEXT_AND_GRAPHIC,
    VIDEO_MODE_SMALL_FONT = 0x100,
};

enum VideoError {
    VIDEO_NO_ERROR = 0,
    VIDEO_NOT_ENOUGH_ARGS,
    VIDEO_TOO_MANY_ARGS,
    VIDEO_ERROR_QUEUE_FULL,
};

typedef struct Videorenderer {
    u8 font_translation_tables[VIDEO_FONT_IDS][256];

    Color background_color, foreground_color;

    RenderTexture2D *screen_texture;
    RenderTexture2D  characters_texture;
    RenderTexture2D  framebuffer;
    Color           *charsFake;

    u32 shaderPalette[256 * 4];

    Shader shader;
    int    chars_loc;
    int    font_locs[VIDEO_FONT_IDS];
    int    palette_loc;
    int    palettebg_loc;
    int    time_loc;
    int    char_size_loc;
    int    base_color_loc;
    int    cursor_cell_idx_loc;
    int    csr_loc;
    int    texture_size_loc;
} VideoRenderer;

struct Buff2D {
    u32 addr;
    u32 w, h;
    u32 size;
    u32 stride;
};

#define VIDEO_CMD_MAX_ARGS 8
struct VideoCmd {
    enum VideoCommand op;

    u64 args[VIDEO_CMD_MAX_ARGS];
    u8  argc;
};

#define VIDEO_CMD_QUEUE_SIZE 64
struct CommandQueue {
    struct VideoCmd cmd[VIDEO_CMD_QUEUE_SIZE];
    u8             head, tail;
};

typedef struct DeviceVideo {
    enum VideoMode    mode;
    enum VideoError   error;
    enum VideoROP     rop;
    enum VideoCommand last_executed_command;

    u8   csr;
    bool is_drawing;
    bool vblank_enable;
    bool cursor_enable;
    bool cursor_blink;
    bool queue_full;

    Font fonts[VIDEO_FONT_IDS];

    struct CommandQueue cmd_queue;

    struct VideoCmd current_cmd;

    struct Buff2D framebuffer;
    struct Buff2D textbuffer;
    struct Buff2D spritebuffer;

    u16 cursor_cell_index;

    VideoRenderer renderer;
} DeviceVideo;

void Video_Reset(TaleaMachine *m, TaleaConfig *config, bool is_restart);
void Video_Update(TaleaMachine *m);

u8   Video_ReadHandler(TaleaMachine *m, u16 addr);
void Video_WriteHandler(TaleaMachine *m, u16 addr, u8 value);

enum MouseButtons {
    MOUSE_BUTT_RIGHT = 0x01,
    MOUSE_BUTT_LEFT  = 0x02,
};

void Mouse_ProcessButtonPress(TaleaMachine *m, int buttons, int scaled_x, int scaled_y);
void Mouse_UpdateCoordinates(TaleaMachine *m, int buttons, int scaled_x, int scaled_y);

/*
ALL INTEGER MULTIBYTE INTEGER VALUES ARE STORED BIG ENDIAN
0x00	Magic Number	    4B	TPS! or HCS! (Identifies this as a Taleä disk).
0x04	Version	            1B	Hardware revision (e.g., 0x01).
0x05	Flags	            1B	Bit 0: Bootable, Bit 1: Write-Protected.
0x06	Medium Type	        1B	see StorageMedium.
0x07	Bank count          1B	number of 256 banks
0x08	Sector Count	    4B	Total LBA sectors (e.g., 2048 for 1MB).
0x0C	Sector Size	        2B	Usually 512.
0x0E	Creation Date	    8B	Unix Timestamp (64-bit)
0x16    Disk Name	        16B	ASCII label (e.g., "SYSTEM_DISK_01").
0x26 -- 0x200               Reserved.
*/

enum StorageMedium { NoMedia, Tps128K, Tps512K, Tps1M, Hcs32M, Hcs64M, Hcs128M };

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

#define STOR_HEADER_SIZE      512
#define STOR_HEADER_TPS_MAGIC "TPS!"
#define STOR_HEADER_HCS_MAGIC "HCS!"
struct StorageHeader {
    char               magic[4];
    u8                 version;
    bool               bootable, write_protected;
    enum StorageMedium medium_type;
    u8                 banks;
    u32                sector_count;
    u16                sector_size;
    u64                creation_date;
    char               label[16];
};

enum TpsId { TPS_ID_A, TPS_ID_B, TPS_TOTAL_DRIVES };
#define TALEA_SECTOR_SIZE 512
typedef struct Tps {
    struct StorageHeader header;
    FILE                *file;
    bool                 inserted;
    bool                 writeProtected;
    bool                 just_inserted;
    bool                 just_ejected;
    enum TpsId           id;

    // status
    u8 real_status; // to be sure to always keep the state

    // registers
    u8  data;
    u16 point;
    u8  statusH, statusL;
    // --
    u8 bank;
} Tps;

#define HCS_FILE_PATH "resources/emulated/devices/hcs/drive.hcs"
typedef struct {
    struct StorageHeader header;
    FILE                *file; // The persistent handle to the .hcs file

    u8 real_status;

    u8  data;
    u16 sector;
    u16 point;            // 16-bit register (to be shifted << 9)
    u8  statusH, statusL; // BUSY, ERROR, READY bits
    u32 total_sectors;    // Cached from the header for safety checks
} Hcs;

typedef struct DeviceStorage {
    Tps        tps_drives[TPS_TOTAL_DRIVES];
    Tps       *current_tps;
    enum TpsId current_tps_id;
    Hcs        hcs;
} DeviceStorage;

bool Storage_InsertTps(enum TpsId tps_id, const char *path);
void Storage_EjectTps(enum TpsId tps_id);

void Storage_Reset(TaleaMachine *m, TaleaConfig *config, bool is_restart);
void Storage_Deinit(TaleaMachine *m);

void Storage_WriteHandler(TaleaMachine *m, u16 addr, u8 value);
u8   Storage_ReadHandler(TaleaMachine *m, u16 addr);

typedef OPLL *DeviceSynth;
#define SYNTH_MSX_CLK 3579545

#define TALEA_MAGIC_ARM_SEQUENCE     0xA5
#define TALEA_MAGIC_TRIGGER_SEQUENCE 0x5A

typedef struct DeviceSystem {
    u64  frequency;
    bool unixtime_mode;
    bool counter_mode;
    u32  uptime;
} DeviceSystem;

void System_WriteHandler(TaleaMachine *m, u16 addr, u8 value);
u8   System_ReadHandler(TaleaMachine *m, u16 addr);

void Bus_RegisterDevices(TaleaMachine *m, const int *id_array, u8 start_index, u8 end_index);

/* --- The Talea Machine Structure --- */

typedef struct TaleaMachine {
    CpuState       cpu;
    DeviceVideo    video;
    DeviceStorage  storage;
    DeviceTerminal terminal;
    DeviceSystem   sys;
    DeviceSynth    synth;
    u8             main_memory[TALEA_MAIN_MEM_SZ];
    u8             data_memory[TALEA_DATA_MEM_SZ];
} TaleaMachine;

/* --- The system interface --- */

void Machine_Init(TaleaMachine *m, TaleaConfig *conf);
void Machine_LoadFirmware(TaleaMachine *m, const char *path);
void Machine_RunFrame(TaleaMachine *m, TaleaConfig *conf);
void Machine_RaiseInterrupt(TaleaMachine *m, u8 vector, u8 priority);
void Machine_Deinit(TaleaMachine *m, TaleaConfig *conf);

void Machine_Poweroff(TaleaMachine *m);

// Unified memory access
u8   Machine_ReadMain8(TaleaMachine *m, u32 addr);
void Machine_WriteMain8(TaleaMachine *m, u32 addr, u8 val);

u16  Machine_ReadMain16(TaleaMachine *m, u32 addr);
void Machine_WriteMain16(TaleaMachine *m, u32 addr, u16 val);

u32  Machine_ReadMain32(TaleaMachine *m, u32 addr);
void Machine_WriteMain32(TaleaMachine *m, u32 addr, u32 val);

u8   Machine_ReadData8(TaleaMachine *m, u16 addr);
void Machine_WriteData8(TaleaMachine *m, u16 addr, u8 val);

u16  Machine_ReadData16(TaleaMachine *m, u16 addr);
void Machine_WriteData16(TaleaMachine *m, u16 addr, u16 val);

u32  Machine_ReadData32(TaleaMachine *m, u16 addr);
void Machine_WriteData32(TaleaMachine *m, u16 addr, u32 val);

#endif /* TALEA_H */