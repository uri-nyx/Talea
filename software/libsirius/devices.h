#ifndef DEVICES_H
#define DEVICES_H

/*
    DEFINITIONS ABOUT KNOWN DEVICES
*/

#include "types.h"

#define TALEA_MAGIC_ARM_SEQUENCE     0xA5
#define TALEA_MAGIC_TRIGGER_SEQUENCE 0x5A

#define DEVICE_SYSTEM 0xf0
#define DEVICE_MAP    0x100

/* Official IDs registered by the House of Talea*/
#define VENDOR_ID 'T'
#define ARCH_ID   'S'

/* STARNDARD TALEA DEVICE IDS */
#define DEVICE_ID_TTY     'K'
#define DEVICE_ID_VIDEO   'V'
#define DEVICE_ID_STORAGE 'D'
#define DEVICE_ID_AUDIO   'A'
#define DEVICE_ID_MOUSE   'M'

/* STANDARD TALEA DEVICE MAP */
struct device_map {
    u8 tty;
    u8 timer;
    u8 keyboard;
    u8 video;
    u8 tps;
    u8 hcs;
    u8 audio;
    u8 mouse;
    u8 custom0, custom1, custom2, custom3;
    u8 custom4, custom5, custom6, custom7;
};

enum SerialStatus {
    SER_STATUS_DATA_AVAILABLE = 0x1, // RX FIFO not empty
    SER_STATUS_CARRIER_DETECT = 0x2, // Socket is connected
};

enum SerialControl {
    SER_CONTROL_INT_EN       = 0x1,  // Enable interrupt upon receiving data
    SER_CONTROL_MASTER_RESET = 0x80, // Master reset (clear all buffers)
};

/* TTY DEVICE PORTS */
enum PortsSerial {
    SER_DATA    = 0X0, /* R/W */
    SER_TX      = 0x0, /* W */
    SER_RX      = 0x0, /* R */
    SER_STATUS  = 0x1, /* R */
    SER_CTRL    = 0x2, /* R/W */
    SER_RXCOUNT = 0x3  /* R */
};

enum TimerCSR {
    TIM_TIMEOUT_EN  = 0x01, // start/stop timeout
    TIM_INTERVAL_EN = 0x02, // start/stop interval
    TIM_GLOBAL_EN   = 0x80, // global enable
};

/* TIMER DEVICE PORTS */
enum PortsTimer {
    TIM_TIMEOUT   = 0x0, /* R/W */
    TIM_INTERVAL  = 0x2, /* R/W */
    TIM_PRESCALER = 0x4, /* R/W */
    TIM_CSR       = 0x5  /* R/W */
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

/* KEYBOARD DEVICE PORTS */
enum PortsKeyboard {
    KBD_CSR  = 0x0, /* R/W */
    KBD_CHAR = 0x1, /* R */
    KBD_SCAN = 0x2  /* R */
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
    COMMAND_NOP,
    COMMAND_END_DRAWING,
    COMMAND_BEGIN_DRAWING,

    // Immediate commands
    COMMAND_SYS_INFO,
    COMMAND_BUFFER_INFO,

    // Queued commands
    COMMAND_CLEAR,
    COMMAND_SET_MODE,
    COMMAND_SET_ADDR,
    COMMAND_LOAD,
    COMMAND_BLIT,
    COMMAND_STRETCH_BLIT,
    COMMAND_PATTERN_FILL,

    // GPU commands (queued)
    COMMAND_DRAW_RECT,
    COMMAND_DRAW_LINE,
    COMMAND_DRAW_CIRCLE,
    COMMAND_DRAW_TRI,
    COMMAND_DRAW_BATCH,
    COMMAND_FILL_SPAN,
    COMMAND_FILL_VSPAN,

    COMMAND_SET_CSR, // queued for use in blits
    COMMAND_BIND_CTX,
    COMMAND_GET_MARKER,
};

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

enum TpsId { TPS_ID_A, TPS_ID_B, TPS_TOTAL_DRIVES };

/* TPS DEVICE PORTS */
enum PortsTps { TPS_COMMAND = 0x00, TPS_DATA = 0x01, TPS_POINT = 0x02, TPS_STATUS = 0x04 };

/* HCS DEVICE PORTS */
enum PortsHCS {
    HCS_COMMAND = 0x00,
    HCS_DATA    = 0x01,
    HCS_SECTOR  = 0x02,
    HCS_POINT   = 0x04,
    HCS_STATUS  = 0x06
};

/* SOTRAGE DEVICE COMMANDS */
enum StorageCommand {
    STORAGE_COMMAND_NOP,        /* does nothing */
    STORAGE_COMMAND_STORE,      /* stores to sector DATA of current bank from
                                   POINT_H:POINT_L */
    STORAGE_COMMAND_LOAD,       /* loads from sector DATA of current bank in
                                   POINT_H:POINT_L */
    STORAGE_COMMAND_GET_STATUS, /* returns the status register on STATUS_L.
                                   Should */
                                /* always be there */
    STORAGE_COMMAND_SETCURRENT, /* Sets the drive (A or B) */
    STORAGE_COMMAND_GETCURRENT, /* returns the current selected drive on
                                   STATUS_H */
    STORAGE_COMMAND_MEDIUM,     /* Returns the medium type in STATUS_H. Can infer
                                   size, */
                                /* sector count, etc based on a lookup table */
    STORAGE_COMMAND_BANK,       /* changes bank */
    STORAGE_COMMAND_GETBANK,    /* Return the current selected bank on STATUS_H */
};

/* AUDIO DEVICE PORTS */
/* TODO: Support not yet documented */
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

enum AudioGlobalStatus {
    AUDIO_GLOB_NOTE_ENDED0     = (1U << 0U),
    AUDIO_GLOB_NOTE_ENDED1     = (1U << 1U),
    AUDIO_GLOB_NOTE_ENDED2     = (1U << 2U),
    AUDIO_GLOB_NOTE_ENDED3     = (1U << 3U),
    AUDIO_GLOB_NOTE_ENDED4     = (1U << 4U),
    AUDIO_GLOB_NOTE_ENDED5     = (1U << 5U),
    AUDIO_GLOB_NOTE_ENDED6     = (1U << 6U),
    AUDIO_GLOB_NOTE_ENDED7     = (1U << 7U),
    AUDIO_GLOB_NOTE_ENDED8     = (1U << 8U),
    AUDIO_GLOB_NOTE_ENDED_MASK = 0x1ff,
    AUDIO_GLOB_BUSY0           = (1U << 9U),
    AUDIO_GLOB_BUSY1           = (1U << 10U),
    AUDIO_GLOB_BUSY2           = (1U << 11U),
    AUDIO_GLOB_BUSY3           = (1U << 12U),
    AUDIO_GLOB_BUSY4           = (1U << 13U),
    AUDIO_GLOB_BUSY5           = (1U << 14U),
    AUDIO_GLOB_BUSY6           = (1U << 15U),
    AUDIO_GLOB_BUSY7           = (1U << 16U),
    AUDIO_GLOB_BUSY8           = (1U << 17U),
    AUDIO_GLOB_BUSY_MASK       = 0x3fE00,
    AUDIO_GLOB_PCM_FIFO_FULL   = (1U << 20U),
    AUDIO_GLOB_PCM_LOW_WATERMARK = (1U<<21U),
};

// TODO: DOCUMENT
enum AudioCsr {
    AUDIO_CSR_TRIGGER    = 1 << 0, // Starts the note
    AUDIO_CSR_IE         = 1 << 1, // interrupt enable
    AUDIO_CSR_LOOP       = 1 << 2, // loops the note
    AUDIO_CSR_BUSY       = 1 << 3,
    AUDIO_CSR_STOP       = 1 << 4,
    AUDIO_CSR_NOTE_ENDED = 1 << 5,
    AUDIO_CSR_GATE       = 1 << 6, // changes mode to release note inmediately
};

enum MouseButtons {
    MOUSE_BUTT_RIGHT = 0x01,
    MOUSE_BUTT_LEFT  = 0x02,
};

/* MOUSE DEVICE PORTS */
/* TODO: Support not yet documented */

/* SYSTEM DEVICE PORTS */
enum PortsSystem {
    PORTS_SYSTEM_MEMSIZE = 0x00,
    PORTS_SYSTEM_CLOCK   = 0x01,
    PORTS_SYSTEM_INT     = 0x02,
    PORTS_SYSTEM_POWER   = 0x03,
    PORTS_SYSTEM_YEAR    = 0x04,
    PORTS_SYSTEM_MONTH   = 0x05,
    PORTS_SYSTEM_DAY     = 0x06,
    PORTS_SYSTEM_HOUR    = 0x07,
    PORTS_SYSTEM_MINUTE  = 0x08,
    PORTS_SYSTEM_SECOND  = 0x09,
    PORTS_SYSTEM_MILLIS  = 0x0a,
    PORTS_SYSTEM_COUNTER = 0x0b,
    PORTS_SYSTEM_DEVNUM  = 0x0c,
    PORTS_SYSTEM_ARCHID  = 0x0d,
    PORTS_SYSTEM_VENDOR  = 0x0e
};

#endif /* DEVICES_H */
