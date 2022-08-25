/* talea.h */
#pragma once

#include <stdint.h>
#include <SDL2/SDL.h>

#ifdef _WIN64
#include <io.h>
#define F_OK 0
#define access _access
#endif

/* implementation constants */
/* paths */
#define TTY_FILE_PATH "devices/tty.txt"
#define DISK_FILE_PATH "devices/drive0/disk"
#define TPS_FILE_PATH "devices/tps/tps"
#define FONTDEFAULT "ConsoleFont.bmp"


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
#define CACHE_SIZE (1 << DATA_BUS_SIZE)
#define Byte 1
#define Word 2

#define REGISTER_SIZE Word
#define SREGISTER_SIZE Byte

#define INSTRUCION_POINTER_SIZE (3 * Byte)
#define PSR_SIZE Byte

#define REGISTER_COUNT 32

/* register indexes */
enum RegisterIndex {
	x0 = 0,
	x1,x2,x3,x4,x5,x6,x7,x8,x9,
	x10,x11,x12,x13,x14,x15,x16,
	x17,x18,x19,x20,x21,x22,x23,
	x24,x25,x26,x27,x28,x29,x30,
	x31,
};

/* ports */
enum Port {
    UNUSED = 0x00,
    TTY_PORT_CHAR,

    VIDEO_PORT_DATA,
    VIDEO_PORT_CMD,

    DISK_PORT_DATA,
    DISK_PORT_CMD,

    TPS_PORT_DATA,
    TPS_PORT_CMD,

    KB_PORT,
    KB_PORTH
};

// #endregion

/* interrupt interface */
struct interrupt_interface
{
    uint8_t enable: 1;
    uint8_t ready: 1;
    uint8_t priority: 3;
    uint8_t vector: 8;
};

typedef enum {
    ERROR_UNKNOWN = 0xff,
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
    ERROR_BAD_PRIVILEGE,
} error_t;

// #region Clock
/* clock prototypes */
struct clock
{
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
typedef struct
{
    uint16_t General[REGISTER_COUNT];
    uint8_t Segment[REGISTER_COUNT];
    uint32_t InstructionPointer;
    uint32_t Ssp;
    uint32_t Usp;
    uint8_t *Cache;
    uint16_t Psr;
    uint8_t *Memory;
} cpu_t;

uint32_t Cpu_Fetch(cpu_t *cpu);
error_t Cpu_Execute(cpu_t *cpu, uint32_t instruction);
void Cpu_Cycle(cpu_t *cpu);
static inline uint16_t Cpu_GetRegister(cpu_t *cpu, enum RegisterIndex index);
static inline void Cpu_SetRegister(cpu_t *cpu,  enum RegisterIndex index, uint16_t value);
static inline uint32_t Cpu_GetSegRegister(cpu_t *cpu, enum RegisterIndex index);
static inline void Cpu_SetSegRegister(cpu_t *cpu, enum RegisterIndex index, uint32_t value);
static inline uint32_t Cpu_GetIp(cpu_t *cpu);
static inline void Cpu_SetIp(cpu_t *cpu, uint32_t value);
static inline uint8_t Cpu_GetCache8(cpu_t *cpu, uint16_t addr);
static inline uint16_t Cpu_GetCache16(cpu_t *cpu, uint16_t addr);
static inline void Cpu_SetCache8(cpu_t *cpu, uint16_t addr, uint8_t value);
static inline void Cpu_SetCache16(cpu_t *cpu, uint16_t addr, uint16_t value);
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
    struct interrupt_interface interrupt;
} kb_t;

// #endregion

// #region Video
/* video modes */
#define Pixel 1 * Byte
#define Character 1 * Byte

enum Video_Mode {
    TEXT_MODE = 0,
    RTEXT_MODE,
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
    uint8_t  *pixels;
    char *charbuffer;
    uint16_t *rich_buffer;
    char *line;
    uint16_t fw;
    uint16_t fh;
    struct interrupt_interface interrupt;


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
    Video_Command_SetMode_RText,
    Video_Command_SetMode_Graphic,

    /* text mode commands */
    Video_Command_SetChar,

    /* RICH MODE COMMANDS */
    Video_Command_SetFont,
    
    /* graphic mode commands */
    Video_Command_SetPixel,
    Video_Command_Blit

};

/* text mode commands prototypes */
error_t Video_SetChar(video_t *video, uint8_t x, uint8_t y, uint8_t c);

/* graphic mode commands prototypes */
void Video_SetPixelAbsolute(video_t* video, uint32_t addr, uint8_t color);
void Video_Blit(video_t *video, uint8_t* index);

/* FONTS */
void initFont(video_t * video, char* font);
#define RICHFONTDEFAULT 0
char * Video_RichFonts[] = {
    "Willy.bmp"
};

// #endregion

// #region Disk
/* sector */
#define SECTOR_SIZE 512

struct sector
{
    uint8_t data[SECTOR_SIZE];
};

/* disk */
#define MAX_SECTOR_COUNT 0xffff

typedef struct 
{
    const char *filename;
    FILE *fp;
    uint16_t sector_count;
} disk_t;
#define MAX_DISK_COUNT 16

typedef struct
{
        uint8_t disk_count;
        disk_t disk_list[MAX_DISK_COUNT];
        disk_t *current_disk;

        struct interrupt_interface interrupt;
} drive_t;

/* disk commands */
enum DiskCommands
{
    Disk_Command_Nop = 0x0,
    Disk_Command_StoreSector,
    Disk_Command_LoadSector,
    Disk_Command_Query
};

/* disk commands prototypes */
void Disk_LoadSector(disk_t *disk, uint16_t sector_number, struct sector *sector);
void Disk_StoreSector(disk_t *disk, uint16_t sector_number, struct sector *sector);
void Disk_Query(drive_t *drive, uint8_t query, uint8_t *response);


// #endregion

// #region Instruction Set 
/* isa opcodes */
// isa.h defines constants for funct3 fields (pseudo opcodes) 
//      and macros to tidy field extraction.

// Macros
#define get_opcode(instruction) (instruction & 0x0000007F)
#define get_rd(instruction) (instruction & 0x00000F80) >> 7
#define get_rs1(instruction) (instruction & 0x000F8000) >> 15
#define get_rs2(instruction) (instruction & 0x01F00000) >> 20
#define get_shamt(instruction) (unsigned)(instruction & 0x00F00000) >> 20
#define get_mod(instruction) instruction & 0x40000000
#define get_funct3(instruction) (unsigned)(instruction & 0x00007000) >> 12
#define get_funct7(instruction) (unsigned)(instruction & 0xFE000000) >> 25

#define get_imm_i(instruction) (signed)(instruction & 0xFFF00000) >> 20

#define get_imm_b(instruction) ((instruction & 0xFE000000) >> 19) | (instruction & 0x00000f80) >> 6 // TODO: change this in lit. Made comprenhensible
#define make_imm_b(instruction) (instruction & 0x80000000) ? 0xe000 | get_imm_b(instruction) : get_imm_b(instruction)

// TODO: UPDATE LIT, PCREL21 CHANGED TO BE COMPRENHENSIBLE
#define get_imm_j(instruction) (instruction & 0xfffff000) >> 11
#define make_imm_j(instruction) (instruction & 0x80000000) ? 0xffe00000 | get_imm_j(instruction) : get_imm_j(instruction)
	
#define get_imm_u(instruction) (unsigned)(instruction & 0xFFFF0000) >> 8

#define get_imm_s(instruction) (instruction & 0xFE000000) >> 20 | ((instruction & 0x00000F80) >> 7)
#define make_imm_s(instruction) (instruction & 0x80000000) ? 0xf000 | get_imm_s(instruction): get_imm_s(instruction)


// Branches
#define Beq 0b000
#define Bne 0b001
#define Blt 0b100
#define Bge 0b101
#define Bltu 0b110
#define Bgeu 0b111

// Loads from RAM
#define Lb 0b000
#define Lh 0b001
#define Lbu 0b100

// Loads from Cache
#define Lbc 0b101
#define Lhc 0b010
#define Lbuc 0b110

// Stores to RAM
#define Sb 0b000
#define Sh 0b001

// Stores to Cache
#define Sbc 0b011
#define Shc 0b010

// Math Register Immediate
#define Addi 0b000
#define Slti 0b010
#define Sltiu 0b011
#define Xori 0b100
#define Ori 0b110
#define Andi 0b111
#define Slli 0b001
#define Srli_Srai 0b101

// Math Register Register
#define Add_Sub 0b000
#define Sll 0b001
#define Slt 0b010
#define Sltu 0b011
#define Xor 0b100
#define Srl_Sra 0b101
#define Or 0b110
#define And 0b111

// System Extension
#define Trap 0b000
#define Ssr 0b001
#define Gsr 0b010
#define Gpsr 0b011

// Supervisor Extension
#define Rti 0b000 
#define Spsr 0b001
			
enum Opcode {
  Lui = 0b0110111,
  Auipc = 0b0010111,
  Jal = 0b1101111,
  Jalr = 0b1100111,
  Jump = 0b1101011,

  Branch = 0b1100011,
  Load = 0b0000011,
  Store = 0b0100011,
  MathI = 0b0010011,
  MathR = 0b0110011,

  System = 0b1110011,
  Supervisor = 0b1110001,
};

// #endregion

// #region Addenda
// For future system ampliations or custom devices
/* addenda header */
// TPS is a tiny portable storage medium, up to 128KiB.
typedef struct tps
{
    const char *filename;
    FILE *fp;

} tps_t;

typedef struct tps_drive
{
    tps_t tps_a;
    tps_t tps_b;
    tps_t *current_tps;

    struct interrupt_interface interrupt;

} tps_drive_t;

enum TpsCommands
{
    Tps_Command_Nop = 0x0,
    Tps_Command_Query,
    Tps_Command_Open,
    Tps_Command_Close,
    Tps_Command_StoreSector,
    Tps_Command_LoadSector
};

enum TpsQueries
{
    BOOTABLE,
    PRESENT
};

enum TpsUnits
{
    TA = 0x00,
    TB = 0x80
};

/* disk commands prototypes */
void TPS_Query(tps_drive_t *tps, uint8_t query, uint8_t *response);
void TPS_LoadSector(tps_t *tps, uint8_t sector_number, struct sector *sector);
void TPS_StoreSector(tps_t *tps, uint8_t sector_number, struct sector *sector);

// Custom devices

// #endregion

// #region Error
/* error */

enum Exception {
	EXCEPTION_UNKNOWN = 0XFF,
	EXCEPTION_NONE = 0,

	EXCEPTION_ILLEGAL_OPCODE,
	EXCEPTION_BAD_PRIVILEGE,	
};

void TaleaSystem_RaiseException(cpu_t *cpu, enum Exception exception);
void TaleaSystem_Panic(error_t error);

// #endregion
