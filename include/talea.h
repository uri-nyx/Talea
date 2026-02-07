#ifndef TALEA_H
#define TALEA_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "devices/audio/audio.h"
#include "devices/mouse/mouse.h"
#include "devices/storage/storage.h"
#include "devices/terminal/terminal.h"
#include "devices/video/video.h"
#include "machine_description.h"
#include "types.h"

#ifndef DO_NOT_INCLUDE_RAYLIB
// Due to confilcts with the WinAPI
#include "raylib.h"
#else
// definitions we need from raylib
#include "need_from_raylib.h"
#endif

#include "emu2413.h"
#include "frontend/config.h"

/* EMULATOR DEFAULTS */

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
#define FONT_PATH "resources" PATH_SEPARATOR "fonts" PATH_SEPARATOR
#define SHADERS_PATH(name)                                                                       \
    TextFormat("resources" PATH_SEPARATOR "shaders" PATH_SEPARATOR "glsl%i" PATH_SEPARATOR "%s", \
               GLSL_VERSION, name)
#if GLSL_VERSION == 130
#define VERTEX_SHADER SHADERS_PATH("vertex.vs")
#else
#define VERTEX_SHADER 0
#endif

#define FPS                    60
#define FRAME_MS               ((float)1000 / (float)FPS)
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

#define CONFIG_FILE_PATH "resources/config.toml"
#define TALEA_FLASH_FILE "resources/emulated/firmware/flash.bin"

typedef struct TaleaMachine TaleaMachine; // Forward declaration

typedef struct TLBEntry {
    u32  phys; // high 12 bits of phisical RAM
    u16  perm;
    bool valid;
} TLBEntry;

enum MemAccessType {
    ACCESS_READ,
    ACCESS_WRITE,
    ACCESS_EXEC,
};

typedef struct CpuState {
    // Register file
    u32 pc;
    u32 status;
    u32 gpr[256];
    u32 ssp, usp;       // supervisor and user stack pointer.
    u8  cwp;            // Register window pointer
    u8  spilledWindows; // counter for how many windows were spilled to DATA

    // Clock
    u64 frequency, cycles, ticks;
    u64 instructionsRetired;

    // Interrupts
    enum TaleaInterrupt exception, LastException;
    enum TaleaInterrupt interrupt;
    bool                isProcessingException;
    u8                  currentIpl, pendingIpl;
    u8                  pendingInterrupts[8];
    u8                  highestPendingInterrupt;

    // Control lines
    bool poweroff, restart;

    // MMU
    TLBEntry tlb[4096];
    u32      faultAddr;

} CpuState;

void Cpu_Reset(TaleaMachine *m, TaleaConfig *config, bool is_restart);
void Cpu_RunCycles(TaleaMachine *m, u32 cycles);

#if TALEA_WITH_MMU
u32 MMU_TranslateAddr(TaleaMachine *m, u32 vaddr, enum MemAccessType access_type);
#endif

#define TALEA_MAGIC_ARM_SEQUENCE     0xA5
#define TALEA_MAGIC_TRIGGER_SEQUENCE 0x5A

typedef struct DeviceSystem {
    u64  frequency;
    bool unixtimeMode;
    bool calendarMode;
    bool millisMode;
    bool microsMode;
    bool instMode;
    u64  uptime;
    u8   winSel;
    u8   winOp;
    u8   winBuff[32 * 4];
    u32  seed;
} DeviceSystem;

void System_Write(TaleaMachine *m, u16 addr, u8 value);
u8   System_Read(TaleaMachine *m, u16 addr);

void Bus_RegisterDevices(TaleaMachine *m);

/* --- The Talea Machine Structure --- */

typedef struct TaleaBus TaleaBus;

typedef struct TaleaMachine {
    CpuState       cpu;
    DeviceVideo    video;
    DeviceStorage  storage;
    DeviceTerminal terminal;
    DeviceSystem   sys;
    DeviceAudio    audio;
    DeviceMouse    mouse;
    TaleaBus      *bus;
} TaleaMachine;

/* --- The system interface --- */

void Machine_Init(TaleaMachine *m, TaleaConfig *conf);
void Machine_LoadFirmware(TaleaMachine *m, const char *path);
void Machine_RunFrame(TaleaMachine *m, TaleaConfig *conf);
void Machine_RaiseInterrupt(TaleaMachine *m, u8 vector, u8 priority);
void Machine_Deinit(TaleaMachine *m, TaleaConfig *conf);

void Machine_Poweroff(TaleaMachine *m);

#endif /* TALEA_H */
