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
    u8 audio0, audio1, audio2, audio3;
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

/* KEYBOARD DEVICE PORTS */
enum PortsKeyboard {
    KBD_CSR  = 0x0, /* R/W */
    KBD_CHAR = 0x1, /* R */
    KBD_SCAN = 0x1  /* R */
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
    VIDEO_DATA    = 0x01,
    VIDEO_GPU0    = 0x04,
    VIDEO_GPU3    = 0x07,
    VIDEO_STATUS  = 0x0c
};

enum VideoCommand {
    VIDEO_COMMAND_NOP = 0,
    VIDEO_COMMAND_CLEAR,
    VIDEO_COMMAND_SETMODE,
    VIDEO_COMMAND_SETFONT,
    VIDEO_COMMAND_BLIT,
    VIDEO_COMMAND_SETFB,
    VIDEO_COMMAND_SETVBLANK,
    VIDEO_COMMAND_LOADFONT,
    VIDEO_COMMAND_LOADPALETTE,
    VIDEO_COMMAND_SETBGCOLOR,
    VIDEO_COMMAND_SETFGCOLOR,
    VIDEO_COMMAND_CLEARREGS,

    // TODO: document
    VIDEO_COMMAND_SETFB_SZ,
    VIDEO_COMMAND_SET_BPP,
    VIDEO_COMMAND_SETCB,
    VIDEO_COMMAND_SETCB_SZ,
    VIDEO_COMMAND_SET_BPC,
    VIDEO_COMMAND_FONTINFO,
    VIDEO_COMMAND_MODE_INFO,
    VIDEO_COMMAND_SCREEN_INFO
};

enum StorageMedium {
    NoMedia,
    Tps128K,
    Tps512K,
    Tps1M,
    Hcs32M,
    Hcs64M,
    Hcs128M
};

enum StorageStatus {
    STOR_STATUS_READY = 0x01,
    STOR_STATUS_ERROR = 0x02, // an error happened in the last operation
    STOR_STATUS_DONE  = 0x04, // thread is done processing, it is safe to raise
                              // interrupt
    STOR_STATUS_INSERTED = 0x08, // always high if inserted
    STOR_STATUS_WPROT    = 0x10, // always high if write protected
    STOR_STATUS_BOOT = 0x20, // always high if disk is bootable. Also marked in
                             // first sector
    STOR_STATUS_BUSY = 0x80, // currently doing something, locked
};

enum TpsId { TPS_ID_A, TPS_ID_B, TPS_TOTAL_DRIVES };

/* TPS DEVICE PORTS */
enum PortsTps {
    TPS_COMMAND = 0x00,
    TPS_DATA    = 0x01,
    TPS_POINT   = 0x02,
    TPS_STATUS  = 0x04
};

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
    STORAGE_COMMAND_MEDIUM,  /* Returns the medium type in STATUS_H. Can infer
                                size, */
                             /* sector count, etc based on a lookup table */
    STORAGE_COMMAND_BANK,    /* changes bank */
    STORAGE_COMMAND_GETBANK, /* Return the current selected bank on STATUS_H */
};

/* AUDIO DEVICE PORTS */
/* TODO: Support not yet documented */


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
