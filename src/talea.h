/* talea.h */
#pragma once

#include <SDL2/SDL.h>
#include <stdint.h>

/* implementation constants */
/* paths */
#define TTY_FILE_PATH "devices/tty.txt"
#define DISK_FILE_PATH "devices/drive0/disk"

/* renderer flags */
#define RENDERER_FLAGS SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 400

/* frequencies */
#define FPS 60
#define MHz 1000000
#define SYSTEM_CLOCK_FREQUENCY 10 * MHz
#define CYCLES_TO_RENDER 666
const int CYCLES_PER_FRAME = (SYSTEM_CLOCK_FREQUENCY / FPS) - CYCLES_TO_RENDER;
const int MILLISECONDS_PER_FRAME = 1000 / FPS;

// #region Architecture
/* sizes */
#define Bit 1

const int DATA_BUS_SIZE = 16 * Bit;
const int ADDRESS_BUS_SIZE = 24 * Bit;

#define BYTE_SIZE 8
#define WORD_SIZE 16

#define MAX_MEMORY_SIZE (1 << ADDRESS_BUS_SIZE)
#define CACHE_SIZE (1 << BYTE_SIZE)
#define Byte 1
#define Word 2

#define REGISTER_SIZE Word
#define SREGISTER_SIZE Byte

#define INSTRUCION_POINTER_SIZE (3 * Byte)

#define REGISTER_COUNT 32

/* register indexes */
enum RegisterIndex {
  x0 = 0,
  x1,
  x2,
  x3,
  x4,
  x5,
  x6,
  x7,
  x8,
  x9,
  x10,
  x11,
  x12,
  x13,
  x14,
  x15,
  x16,
  x17,
  x18,
  x19,
  x20,
  x21,
  x22,
  x23,
  x24,
  x25,
  x26,
  x27,
  x28,
  x29,
  x30,
  x31,
};

/* ports */
enum Port {
  KB_PORT = 0x00,
  TTY_PORT_CHAR,

  VIDEO_PORT_CMD,
  VIDEO_PORT_DATA,

  DISK_PORT_CMD,
  DISK_PORT_DATA,
};

// #endregion

// #region Error
/* error */
typedef enum {
  ERROR_UNKNOWN = -1,
  ERROR_NONE = 0,

  // Tty errors
  TTY_ERROR_WRITE_FAILED,
  TTY_ERROR_CLOSE_FAILED,

  // Video errors
  VIDEO_ERROR_INIT_FAILED,
  VIDEO_ERROR_INVALID_MODE,
  VIDEO_ERROR_INVALID_COORDINATE,

  DISK_ERROR_OPEN_FAILED,

  ERROR_UNKNOWN_OPCODE,
} error_t;

void TaleaSystem_Panic(error_t error);

// #endregion

// #region Clock
/* clock prototypes */
struct clock {
  clock_t start, end;
  double cpu_time_used;
  double frame_took;
};

// CLock functions are used to time a frame roughly to 16 ms
void Clock_FrameStart(struct clock *clk);
void Clock_FrameEnd(struct clock *clk);

// #endregion

// #region Cpu
/* cpu */
typedef struct {
  uint16_t General[REGISTER_COUNT];
  uint8_t Segment[REGISTER_COUNT];
  uint32_t InstructionPointer;
  uint8_t Cache[CACHE_SIZE];
  uint8_t *Memory;
} cpu_t;

uint32_t Cpu_Fetch(cpu_t *cpu);
error_t Cpu_Execute(cpu_t *cpu, uint32_t instruction);
void Cpu_Cycle(cpu_t *cpu);
static inline uint16_t Cpu_GetRegister(cpu_t *cpu, enum RegisterIndex index);
static inline void Cpu_SetRegister(cpu_t *cpu, enum RegisterIndex index,
                                   uint16_t value);
static inline uint32_t Cpu_GetSegRegister(cpu_t *cpu, enum RegisterIndex index);
static inline void Cpu_SetSegRegister(cpu_t *cpu, enum RegisterIndex index,
                                      uint32_t value);
static inline uint32_t Cpu_GetIp(cpu_t *cpu);
static inline void Cpu_SetIp(cpu_t *cpu, uint32_t value);
static inline uint8_t Cpu_GetCache8(cpu_t *cpu, uint8_t addr);
static inline uint16_t Cpu_GetCache16(cpu_t *cpu, uint8_t addr);
static inline void Cpu_SetCache8(cpu_t *cpu, uint8_t addr, uint8_t value);
static inline void Cpu_SetCache16(cpu_t *cpu, uint8_t addr, uint16_t value);
uint8_t Cpu_GetMemory8(cpu_t *cpu, uint32_t addr);
uint16_t Cpu_GetMemory16(cpu_t *cpu, uint32_t addr);
void Cpu_SetMemory8(cpu_t *cpu, uint32_t addr, uint8_t value);
void Cpu_SetMemory16(cpu_t *cpu, uint32_t addr, uint16_t value);

// #endregion

// #region Mmu
/* mmu */
typedef int mmu_t;
// TODO: Implement mmu

// #endregion

// #region Tty
/* tty */
typedef struct {
  char *filename;
  uint8_t c;
} tty_t;

error_t Tty_Write(tty_t *tty);

// #endregion

// #region Keyboard
/* kb */
typedef struct {
  uint8_t port;
  const uint8_t *state;
} kb_t;

void Kb_Handle(kb_t *kb, cpu_t *cpu);

// #endregion

// #region Video
/* video modes */
#define Pixel 1 * Byte
#define Character 1 * Byte

enum Video_Mode {
  TEXT_MODE = 0,
  GRAPHIC_MODE,
};
const int TEXT_MODE_WIDTH = 80 * Character;
const int TEXT_MODE_HEIGHT = 50 * Character;

const int GRAPHIC_MODE_WIDTH = 640 * Pixel;
const int GRAPHIC_MODE_HEIGHT = 400 * Pixel;

/* video controller */
typedef struct {
  enum Video_Mode mode;
  uint8_t *pixels;
  char *charbuffer;
  char *line;

  /* sdl video internals */
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;

} video_t;

/* video commands */
enum Video_Commands {
  Video_Command_Nop = 0x00,
  Video_Command_Clear,
  Video_Command_SetMode_Text,
  Video_Command_SetMode_Graphic,

  /* text mode commands */
  Video_Command_SetChar,

  /* graphic mode commands */
  Video_Command_SetPixel

};

/* text mode commands prototypes */
error_t Video_SetChar(video_t *video, uint8_t x, uint8_t y, uint8_t c);

/* graphic mode commands prototypes */
void Video_SetPixelAbsolute(video_t *video, uint32_t addr, uint8_t color);

// #endregion

// #region Disk
/* sector */
#define SECTOR_SIZE 512

struct sector {
  uint8_t data[SECTOR_SIZE];
};

/* disk */
#define MAX_SECTOR_COUNT 0xffff

typedef struct {
  const char *filename;
  FILE *fp;
  uint16_t sector_count;
} disk_t;
#define MAX_DISK_COUNT 16

typedef struct {
  uint8_t disk_count;
  disk_t disk_list[MAX_DISK_COUNT];
  disk_t *current_disk;
} drive_t;

/* disk commands */
enum DiskCommands {
  Disk_Command_Nop = 0x0,
  Disk_Command_StoreSector,
  Disk_Command_LoadSector,
};

/* disk commands prototypes */
void Disk_LoadSector(disk_t *disk, uint16_t sector_number,
                     struct sector *sector);
void Disk_StoreSector(disk_t *disk, uint16_t sector_number,
                      struct sector *sector);

// #endregion

// #region Instruction Set
/* isa */
/*
 * U-type
 * J-type
 * R-type
 * I-type
 * S-type
 * B-type
 */

enum Opcode {
  Lui = 0b0110111,
  Auipc = 0b0010111,
  Jal = 0b1101111,
  Jalr = 0b1100111,

  Branch = 0b1100011,
  Load = 0b0000011,
  Store = 0b0100011,
  MathI = 0b0010011,
  MathR = 0b0110011,

  E = 0b1110011,
  /* CSR Not Implemented */
};


static inline error_t I_ecall(void);
static inline error_t I_ebreak(void);

// #endregion

// #region Addenda
// For future system ampliations or custom devices
/* addenda header */
// Custom devices

// #endregion
