/* talea.h */
#pragma once

#include <stdint.h>
#include <SDL2/SDL.h>

/* implementation constants */
/* paths */
#define TTY_FILE_PATH "devices/tty.txt"
#define DISK_FILE_PATH "devices/disk"


/* renderer flags */
#define RENDERER_FLAGS SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC


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

const int BYTE_SIZE = 8 * Bit;
const int WORD_SIZE = 16 * Bit;

#define MAX_MEMORY_SIZE (1 << ADDRESS_BUS_SIZE)
#define CACHE_SIZE (1 << BYTE_SIZE)
#define Byte 1
#define Word 2

const int STATUS_REGISTER_SIZE = Byte;
const int CHAPTER_REGISTER_SIZE = Byte;
const int PAGINA_REGISTER_SIZE = Byte;
const int LITTERA_REGISTER_SIZE = Byte;

const int STACK_POINTER_SIZE = Word;
const int FRAME_POINTER_SIZE = Word;
const int REGISTER_SIZE = Word;

const int INSTRUCION_POINTER_SIZE = 3 * Byte;

#define GeneralPurposeRegisters 5
#define REGISTER_FILE_SIZE (STATUS_REGISTER_SIZE + CHAPTER_REGISTER_SIZE + PAGINA_REGISTER_SIZE + LITTERA_REGISTER_SIZE + STACK_POINTER_SIZE + FRAME_POINTER_SIZE + (REGISTER_SIZE * GeneralPurposeRegisters) + INSTRUCION_POINTER_SIZE)

/* register indexes */
enum RegisterIndex {
    LITTERA_REGISTER = 0,
    CHAPTER_REGISTER,
    PAGINA_REGISTER,
    STACK_POINTER = 4,
    FRAME_POINTER = 6,
    GPR0 = 8,
    GPR1 = 10,
    GPR2 = 12,
    GPR3 = 14,
    GPR4 = 16,
    INSTRUCTION_POINTER = 18,
} typedef RegisterIndex;

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

// #region Cpu
/* cpu */
typedef struct
{
    uint8_t RegisterFile[REGISTER_FILE_SIZE];
    uint8_t Cache[CACHE_SIZE];
    uint8_t *Memory;
} cpu_t;

uint8_t Cpu_Fetch(cpu_t *cpu);
error_t Cpu_Execute(cpu_t *cpu, uint8_t opcode);
void Cpu_Cycle(cpu_t *cpu);
static inline uint8_t Cpu_GetRegister8(cpu_t *cpu, enum RegisterIndex index);
static inline uint16_t Cpu_GetRegister16(cpu_t *cpu, enum RegisterIndex index);
static inline void Cpu_SetRegister8(cpu_t *cpu,  enum RegisterIndex index, uint8_t value);
static inline void Cpu_SetRegister16(cpu_t *cpu,  enum RegisterIndex index, uint16_t value);
static inline uint32_t Cpu_GetIp(cpu_t *cpu);
void Cpu_SetIp(cpu_t *cpu, uint32_t addr);
static inline uint8_t Cpu_GetCache8(cpu_t *cpu, uint8_t addr);
static inline uint16_t Cpu_GetCache16(cpu_t *cpu, uint8_t addr);
static inline void Cpu_SetCache8(cpu_t *cpu, uint8_t addr, uint8_t value);
static inline void Cpu_SetCache16(cpu_t *cpu, uint8_t addr, uint8_t value);
uint8_t Cpu_GetMemory8(cpu_t *cpu, uint32_t addr);
uint16_t Cpu_GetMemory16(cpu_t *cpu, uint32_t addr);
void Cpu_SetMemory8(cpu_t *cpu, uint32_t addr, uint8_t value);
void Cpu_SetMemory16(cpu_t *cpu, uint32_t addr, uint8_t value);

// #endregion

// #region Mmu
/* mmu */
// TODO: Implement mmu

// #endregion

// #region Tty
/* tty */
typedef struct
{
    char *filename;
    uint8_t c;
} tty_t;

error_t Tty_Write(tty_t *tty);

// #endregion

// #region Keyboard
/* kb */
typedef struct
{
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
typedef struct
{
    enum Video_Mode mode;
    uint8_t *pixels;
    uint8_t *charbuffer;
    uint8_t *line;

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
error_t Video_SetChar(video_t *video, uint8_t x, uint8_t y, char c);

/* graphic mode commands prototypes */
void Video_SetPixelAbsolute(video_t* video, uint32_t addr, uint8_t color);

// #endregion

// #region Disk
/* sector */
const int SECTOR_SIZE = 512;

struct sector
{
    uint8_t data[SECTOR_SIZE];
};

/* disk */
const int MAX_SECTOR_COUNT = 0xffff
typedef struct 
{
    const char *filename;
    FILE *fp;
    uint16_t sector_count;
} disk_t;

/* disk commands */
enum DiskCommands
{
    Disk_Nop = 0x0,
    Disk_StoreSector,
    Disk_LoadSector,
};

/* disk commands prototypes */
void Disk_LoadSector(disk_t *disk, uint16_t sector_number, struct sector *sector);
void Disk_StoreSector(disk_t *disk, uint16_t sector_number, struct sector *sector);

// #endregion

// #region Instruction Set 
/* isa */
// TODO: Design instruction set

// #endregion

