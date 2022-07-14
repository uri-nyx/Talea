#ifndef TALEA_H
#define TALEA_H

#include <SDL2/SDL.h>
#include <stdint.h>

// #region Constants
/*
constants.h - constants for the Taleä system

Defines system constants such as sizes of data structures,
pixel widths and heights, clock frequencies, character buffers,
fonts, paths within the program, etc.
*/

/*System specification constants*/
#define FPS 60
#define MHz 1000000
#define SYSTEM_CLOCK_FREQUENCY 10 * MHz
#define CYCLES_TO_RENDER 666
const int CYCLES_PER_FRAME = (SYSTEM_CLOCK_FREQUENCY / FPS) - CYCLES_TO_RENDER;
const int MILLISECONDS_PER_FRAME = 1000 / FPS;

#define RENDERER_FLAGS SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
#define TTY_FILE_PATH "devices/tty.txt"
#define DISK_FILE_PATH "devices/disk"
#define MAX_SECTOR_COUNT 0xffff

// PORTS
enum Port {
    KB_PORT = 0x00,
    TTY_PORT_CHAR,

    VIDEO_PORT_CMD,
    VIDEO_PORT_DATA,

    DISK_PORT_CMD,
    DISK_PORT_SECTORl,
    DISK_PORT_SECTORh,
};
// #endregion

// #region Sizes
#define Bit 1

const int BYTE_SIZE = 8 * Bit;
const int WORD_SIZE = 16 * Bit;

const int DATA_BUS_SIZE = 16 * Bit;
const int ADDRESS_BUS_SIZE = 24 * Bit;

// Byte sizes
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
#define MAX_MEMORY_SIZE (1 << ADDRESS_BUS_SIZE)
#define CACHE_SIZE (1 << BYTE_SIZE)
// #endregion

// #region Register File
/* REGISTER FILE STRUCTURE AND INDICES */
/***************************************/
/* INDEX *  SIZE *      RESGISTER NAME */
/***************************************/
/*     0 *     1 * STATUS REGISTER     */
/*     1 *     1 * LITTERA REGISTER    */
/*     2 *     1 * CHAPTER REGISTER    */
/*     3 *     1 * PAGINA REGISTER     */
/*     4 *     2 * STACK POINTER       */
/*     6 *     2 * FRAME POINTER       */
/*     8 *     2 * GPR 0 (Ach){R0h,R0l}*/
/*     10 *    2 * GPR 1 (Acl){R1h,R1l}*/
/*     12 *    2 * GPR 2 (Bch){R2h,R2l}*/
/*     14 *    2 * GPR 3 (Bcl){R3h,R3l}*/
/*     16 *    3 * INSTRUCTION POINTER */
/***************************************/
enum RegisterIndex {
    STATUS_REGISTER = 0,
    LITTERA_REGISTER,
    CHAPTER_REGISTER,
    PAGINA_REGISTER,
    STACK_POINTER = 4,
    FRAME_POINTER = 6,
    ACH = 8,
    ACL = 10,
    BCH = 12,
    BCL = 14,
    INSTRUCTION_POINTER = 16,
} typedef RegisterIndex;
// #endregion

// #region Error
enum Error {
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
} typedef error_t;

void Talea_Panic(error_t error);
// #endregion

// #region Cpu

/*
cpu.h - Cpu Module Header

Defines Cpu, intructions and Clock functions
*/

struct cpu
{
    uint8_t RegisterFile[REGISTER_FILE_SIZE];
    uint8_t Cache[CACHE_SIZE];
    uint8_t *Memory;
} typedef cpu_t;

struct clock
{
    clock_t start, end;
    double cpu_time_used;
    double frame_took;
};

// CLock functions are used to time a frame roughly to 16 ms
void Clock_FrameStart(struct clock *clk);
void Clock_FrameEnd(struct clock *clk);

// Helpers
uint8_t nextByte(cpu_t *cpu);
static inline uint32_t trimAddr(uint32_t addr);

void Cpu_Init(cpu_t *cpu);
void Cpu_Reset(cpu_t *cpu);
void Cpu_Destroy(cpu_t *cpu);

// Fetch, decode execute cycles
void Cpu_Cycle(cpu_t *cpu);
uint8_t Cpu_Fetch(cpu_t *cpu);
error_t Cpu_Execute(cpu_t *cpu, uint8_t opcode);

// Memory getters and setters:e
static inline uint8_t Cpu_GetRegister8(cpu_t *cpu, enum RegisterIndex index);
static inline uint16_t Cpu_GetRegister16(cpu_t *cpu, int index);
static inline void Cpu_SetRegister8(cpu_t *cpu, int index, uint8_t value);
static inline void Cpu_SetRegister16(cpu_t *cpu, int index, uint16_t value);

static inline uint8_t Cpu_GetCache8(cpu_t *cpu, uint8_t addr);
static inline uint16_t Cpu_GetCache16(cpu_t *cpu, uint8_t addr);
static inline void Cpu_SetCache8(cpu_t *cpu, uint8_t addr, uint8_t value);
static inline void Cpu_SetCache16(cpu_t *cpu, uint8_t addr, uint8_t value);

uint8_t Cpu_GetMemory8(cpu_t *cpu, uint32_t addr);
uint16_t Cpu_GetMemory16(cpu_t *cpu, uint32_t addr);
void Cpu_SetMemory8(cpu_t *cpu, uint32_t addr, uint8_t value);
void Cpu_SetMemory16(cpu_t *cpu, uint32_t addr, uint8_t value);

static inline uint32_t Cpu_GetIp(cpu_t *cpu);
void Cpu_SetIp(cpu_t *cpu, uint32_t addr);

    // #region Instructions

    // #region Opcodes
enum Opcodes {
    Nop,
    Hlt,

    // #region Arithmetic
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    // #endregion

    // #region Logic
    And,
    Or,
    Xor,
    Not,
    // #endregion

    // #region Comparison
    Cmp,
    // #endregion

    // #region Stack
    Push,
    Pop,
    // #endregion

    // #region Memory
    Ld,
    St,
    // #endregion

    // #region Jumps
    Jmp,
    Jz,
    Jnz,
    // #endregion

    // #region Calls
    Call,
    Ret,
    // #endregion
};
    // #endregion

static inline void Cpu_Instruction_Nop(cpu_t *cpu);
static inline void Cpu_Instruction_Hlt(cpu_t *cpu);

    // #region Arithmetic
static inline void Cpu_Instruction_Add(cpu_t *cpu);
static inline void Cpu_Instruction_Sub(cpu_t *cpu);
static inline void Cpu_Instruction_Mul(cpu_t *cpu);
static inline void Cpu_Instruction_Div(cpu_t *cpu);
static inline void Cpu_Instruction_Mod(cpu_t *cpu);
    // #endregion

    // #region Logic
static inline void Cpu_Instruction_And(cpu_t *cpu);
static inline void Cpu_Instruction_Or(cpu_t *cpu);
static inline void Cpu_Instruction_Xor(cpu_t *cpu);
static inline void Cpu_Instruction_Not(cpu_t *cpu);
    // #endregion

    // #region Comparison
static inline void Cpu_Instruction_Cmp(cpu_t *cpu);
    // #endregion

    // #region Stack
static inline void Cpu_Instruction_Push(cpu_t *cpu);
static inline void Cpu_Instruction_Pop(cpu_t *cpu);
    // #endregion

    // #region Memory
static inline void Cpu_Instruction_Ld(cpu_t *cpu);
static inline void Cpu_Instruction_St(cpu_t *cpu);
    // #endregion

    // #region Jumps
static inline void Cpu_Instruction_Jmp(cpu_t *cpu);
static inline void Cpu_Instruction_Jz(cpu_t *cpu);
static inline void Cpu_Instruction_Jnz(cpu_t *cpu);
    // #endregion

    // #region Calls
static inline void Cpu_Instruction_Call(cpu_t *cpu);
static inline void Cpu_Instruction_Ret(cpu_t *cpu);
    // #endregion

// #endregion
// #endregion

// #region Video
/*
video.h - Video Controller module headers

Defines video controller functions and interactions
*/

#define Pixel 1 * Byte
#define Character 1 * Byte

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 400

const int TEXT_MODE_WIDTH = 80 * Character;
const int TEXT_MODE_HEIGHT = 50 * Character;

const int GRAPHIC_MODE_WIDTH = 640 * Pixel;
const int GRAPHIC_MODE_HEIGHT = 400 * Pixel;

enum Video_Mode {
    TEXT_MODE = 0,
    GRAPHIC_MODE,
};

typedef struct
{
    enum Video_Mode mode;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint8_t *pixels;
    char *charbuffer;
    char *line;
} video_t;

error_t Video_Init(video_t *video);
error_t Video_Destroy(video_t *video);
error_t Video_Render(video_t *video);
Video_Mode Video_GetMode(video_t *video);
error_t Video_SetMode(video_t *video, int mode);
error_t Video_SetPixel(video_t *video, int x, int y, uint8_t color);
error_t Video_SetChar(video_t *video, int x, int y, char c);

enum Video_Commands {
    Video_Command_Nop = 0x00,
    Video_Command_Clear,
    Video_Command_SetMode_Text,
    Video_Command_SetMode_Graphic,
    Video_Command_SetPixel,
    Video_Command_SetChar
};
// #endregion

// #region Tty
/*
tty.h - Teletype module headers

Defines teletype functions and interactions
*/

struct tty
{
    char *filename;
    uint8_t c;
} typedef tty_t;

error_t Tty_Write(tty_t *tty);
// #endregion

// #region Keyboard
struct kb
{
    uint8_t port;
    const uint8_t *state;
} typedef kb_t;

void Kb_Handle(kb_t *kb, cpu_t *cpu);
// #endregion

// #region Disk
/* Disk Storage */
const int SECTOR_SIZE = 512;

struct sector
{
    uint8_t data[SECTOR_SIZE];
};

struct disk
{
    const char *filename;
    FILE *fp;
    uint16_t sector_count;
    struct sector *sector_pointer;
} typedef disk_t;

error_t Disk_Create(const char *path, disk_t *disk, uint16_t sector_count);
void Disk_LoadSector(disk_t *disk, uint16_t sector_number, struct sector *sector);
void Disk_StoreSector(disk_t *disk, uint16_t sector_number, struct sector *sector);
// void Disk_LoadContiguousSectors(disk_t* disk, uint16_t sector_number, struct sector* sector, uint16_t sector_count);
// void Disk_StoreContiguousSectors(disk_t* disk, uint16_t sector_number, struct sector* sector, uint16_t sector_count);
// #endregion

// #region Memory Mapper Unit

// Perhaps it would be useful to implement both a Real Mode resembling that of the x86 (1MB max memory), segment * 16 + offset.
// And a protected mode implementing paging (that i have no clue how to implement)
// It is worth to notice the RISC-V way of using a base register and an offset litteral to address a memory location. (Maybe CPL as base register)
enum Mmu_Mode{
    Mmu_Mode_Real = 0,
    Mmu_Mode_Protected
};

struct mmu
{
    enum Mmu_Mode mode;
    // TODO: Page tables
} typedef mmu_t;

void Mmu_SetMode(mmu_t *mmu, enum Mmu_Mode mode);
uint32_t Mmu_Translate(mmu_t *mmu, uint32_t addr);

// #endregion
#endif