#include <stdio.h>
#include "talea.h"
#include <time.h>
#include "include/inprint/SDL2_inprint.h"
#include "include/inprint/inline_font.h"

// #region Clock and Helpers 
void Clock_FrameStart(struct clock *clk)
{
    clk->start = clock();
}

void Clock_FrameEnd(struct clock *clk)
{
    clk->end = clock();
    clk->cpu_time_used = ((double)(clk->end - clk->start)) / CLOCKS_PER_SEC;
    clk->frame_took = 16 - (clk->cpu_time_used * 1000);
    if (clk->frame_took > 0)
        SDL_Delay(clk->frame_took);
}

uint8_t nextByte(cpu_t *cpu)
{
    uint32_t ip = Cpu_GetIp(cpu);
    uint8_t byte = Cpu_GetMemory8(cpu, ip);
    Cpu_SetIp(cpu, ip + Byte);
    return byte;
}

static inline uint32_t trimAddr(uint32_t addr)
{
    return addr & 0x00FFFFFF;
}

void Talea_Panic(error_t error)
{
    return;
}
// #endregion */

// #region Cpu Module
void Cpu_Init(cpu_t *cpu)
{
    memset(cpu->RegisterFile, 0, sizeof(cpu->RegisterFile));
    memset(cpu->Cache, 0, sizeof(cpu->Cache));
    cpu->Memory = malloc(sizeof(uint8_t) * MAX_MEMORY_SIZE);
    memset(cpu->Memory, 0, sizeof(uint8_t) * MAX_MEMORY_SIZE);
}

void Cpu_Reset(cpu_t *cpu)
{
    memset(cpu->RegisterFile, 0, sizeof(cpu->RegisterFile));
    memset(cpu->Cache, 0, sizeof(cpu->Cache));
    memset(cpu->Memory, 0, sizeof(uint8_t) * MAX_MEMORY_SIZE);
}

void Cpu_Destroy(cpu_t *cpu)
{
    free(cpu->Memory);
}

void Cpu_Cycle(cpu_t *cpu)
{
    error_t error = Cpu_Execute(cpu, Cpu_Fetch(cpu));

    if (error != ERROR_NONE)
    {
        Talea_Panic(error);
    }
}

uint8_t Cpu_Fetch(cpu_t *cpu)
{
    return nextByte(cpu);
}

error_t Cpu_Execute(cpu_t *cpu, uint8_t opcode)
{
    switch (opcode)
    {
    case Nop:
        return ERROR_NONE;
    default:
        return ERROR_UNKNOWN_OPCODE;
    }
}

    /************************************************************/
    /* Address Resolution:                                      */
    /*  Relative addresses are resolved following the common    */
    /* scheme of a base register and an added offset, as can    */
    /* be found in architectures like x86 or RISC-V:            */
    /*          0RRR oooo oooo oooo                             */
    /*  R - one of the following registers: c+p, sp, fp, gpr0-3 */
    /*  o - a 12-bit litteral offset                            */
    /*  Addresses in Taleä Tabula are 24-bit long. Thus, regis- */
    /*  ters hold the upper 12 bits of the address and the off- */
    /*  set the lower 12 bits.                                  */
    /************************************************************/
static inline uint32_t Cpu_ResolveAddr(cpu_t *cpu, uint16_t regPlusOffset)
{
    return Cpu_GetRegister16(cpu, regPlusOffset >> 12) << 12 | (regPlusOffset & 0x0FFF);
}

// #region Register Accessors
inline uint8_t Cpu_GetRegister8(cpu_t *cpu, int index)
{
    return cpu->RegisterFile[index];
}

inline uint16_t Cpu_GetRegister16(cpu_t *cpu, int index)
{
    return (uint16_t)cpu->RegisterFile[index] << 8 | cpu->RegisterFile[index + 1];
}

inline void Cpu_SetRegister8(cpu_t *cpu, int index, uint8_t value)
{
    cpu->RegisterFile[index] = value;
}

inline void Cpu_SetRegister16(cpu_t *cpu, int index, uint16_t value)
{
    cpu->RegisterFile[index] = value >> 8;
    cpu->RegisterFile[index + 1] = value & 0xFF;
}
// #endregion

// #region Cache Accessors
inline uint8_t Cpu_GetCache8(cpu_t *cpu, uint8_t addr)
{
    return cpu->Cache[addr];
}

inline uint16_t Cpu_GetCache16(cpu_t *cpu, uint8_t addr)
{
    return (uint16_t)cpu->Cache[addr] << 8 | cpu->Cache[addr + 1];
}

inline void Cpu_SetCache8(cpu_t *cpu, uint8_t addr, uint8_t value)
{
    cpu->Cache[addr] = value;
}

inline void Cpu_SetCache16(cpu_t *cpu, uint8_t addr, uint8_t value)
{
    cpu->Cache[addr] = value >> 8;
    cpu->Cache[addr + 1] = value & 0xFF;
}
// #endregion

// #region Memory Accessors
uint8_t Cpu_GetMemory8(cpu_t *cpu, uint32_t addr)
{
    return cpu->Memory[trimAddr(addr)];
}

uint16_t Cpu_GetMemory16(cpu_t *cpu, uint32_t addr)
{
    return (uint16_t)Cpu_GetMemory8(cpu, addr) << 8 | Cpu_GetMemory8(cpu, addr + 1);
}

void Cpu_SetMemory8(cpu_t *cpu, uint32_t addr, uint8_t value)
{
    cpu->Memory[trimAddr(addr)] = value;
}

void Cpu_SetMemory16(cpu_t *cpu, uint32_t addr, uint8_t value)
{
    Cpu_SetMemory8(cpu, addr, value >> 8);
    Cpu_SetMemory8(cpu, addr + 1, value & 0xFF);
}
// #endregion

// #region Specific Purpose Register Accessors
inline uint32_t Cpu_GetIp(cpu_t *cpu)
{
    return (uint32_t)cpu->RegisterFile[INSTRUCTION_POINTER] << 16 | (uint32_t)cpu->RegisterFile[INSTRUCTION_POINTER + 1] << 8 | cpu->RegisterFile[INSTRUCTION_POINTER + 2];
}

void Cpu_SetIp(cpu_t *cpu, uint32_t value)
{
    cpu->RegisterFile[INSTRUCTION_POINTER] = value >> 16;
    cpu->RegisterFile[INSTRUCTION_POINTER + 1] = value >> 8;
    cpu->RegisterFile[INSTRUCTION_POINTER + 2] = value;
}
// #endregion
// #endregion

// #region Video Module
error_t Video_Init(video_t *video)
{

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == -1)
    {
        return VIDEO_ERROR_INIT_FAILED;
    }

    video->window = SDL_CreateWindow("Taleä System",
                                     SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                     WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE);
    video->renderer = SDL_CreateRenderer(video->window, -1, RENDERER_FLAGS);
    video->texture = SDL_CreateTexture(video->renderer,
                                       SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STATIC, WINDOW_WIDTH, WINDOW_HEIGHT);
    video->pixels = malloc(GRAPHIC_MODE_WIDTH * GRAPHIC_MODE_HEIGHT * sizeof(uint8_t));
    video->charbuffer = malloc(TEXT_MODE_WIDTH * TEXT_MODE_HEIGHT * sizeof(char));
    video->line = malloc(TEXT_MODE_WIDTH * sizeof(char));

    inrenderer(video->renderer);
    prepare_inline_font();

    if (video->window == NULL || video->renderer == NULL || video->texture == NULL)
    {
        return VIDEO_ERROR_INIT_FAILED;
    }
    else
    {
        return ERROR_NONE;
    }
}

error_t Video_Destroy(video_t *video)
{
    SDL_DestroyTexture(video->texture);
    SDL_DestroyRenderer(video->renderer);
    SDL_DestroyWindow(video->window);
    free(video->pixels);
    free(video->charbuffer);
    free(video->line);
    SDL_Quit();
    return ERROR_NONE;
}

error_t Video_Render(video_t *video)
{

    switch (video->mode)
    {
    case GRAPHIC_MODE:
        SDL_UpdateTexture(video->texture, NULL,
                          video->pixels, GRAPHIC_MODE_WIDTH * sizeof(uint8_t));
        SDL_RenderClear(video->renderer);
        SDL_RenderCopy(video->renderer, video->texture, NULL, NULL);
        SDL_RenderPresent(video->renderer);
        break;
    case TEXT_MODE:
        SDL_RenderClear(video->renderer);
        for (int i = 0; i < TEXT_MODE_HEIGHT; i++)
        {
            memcpy(video->line, video->charbuffer + i * TEXT_MODE_WIDTH, TEXT_MODE_WIDTH);
            inprint(video->renderer, video->line, 0, i * 8);
        }
        SDL_RenderPresent(video->renderer);
        break;
    default:
        return VIDEO_ERROR_INVALID_MODE;
    }

    return ERROR_NONE;
}

int Video_GetMode(video_t *video)
{
    return video->mode;
}

error_t Video_SetMode(video_t *video, int mode)
{
    if (mode > GRAPHIC_MODE)
        return VIDEO_ERROR_INVALID_MODE;
    video->mode = mode;
    return ERROR_NONE;
};

error_t Video_SetPixel(video_t *video, int x, int y, uint8_t color)
{
    if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT)
        return VIDEO_ERROR_INVALID_COORDINATE;
    if (video->mode != GRAPHIC_MODE)
        return VIDEO_ERROR_INVALID_MODE;
    video->pixels[x + y * GRAPHIC_MODE_WIDTH] = color;
    return ERROR_NONE;
};

void Video_SetPixelAbsolute(video_t* video, uint32_t index, uint8_t color)
{
    index = index & 0x3ffff;
    video->pixels[index] = color;
}

error_t Video_SetChar(video_t *video, int x, int y, char c)
{
    if (x < 0 || x >= TEXT_MODE_WIDTH || y < 0 || y >= TEXT_MODE_HEIGHT)
        return VIDEO_ERROR_INVALID_COORDINATE;
    if (video->mode != TEXT_MODE)
        return VIDEO_ERROR_INVALID_MODE;
    video->charbuffer[x + y * TEXT_MODE_WIDTH] = c;
    return ERROR_NONE;
};

void Video_Execute(cpu_t *cpu, video_t *video)
{
    int x, y;
    uint32_t index;
    uint8_t data = Cpu_GetCache8(cpu, VIDEO_PORT_DATA);

    switch (Cpu_GetCache8(cpu, VIDEO_PORT_CMD))
    {
    case Video_Command_Nop:
        break;
    case Video_Command_Clear:
        memset(video->charbuffer, data, GRAPHIC_MODE_WIDTH * GRAPHIC_MODE_HEIGHT * sizeof(uint8_t));
    case Video_Command_SetMode_Text:
        Video_SetMode(video, TEXT_MODE);
        break;
    case Video_Command_SetMode_Graphic:
        Video_SetMode(video, GRAPHIC_MODE);
        break;
    case Video_Command_SetPixel:
        index = (uint32_t)Cpu_GetRegister16(cpu, CHAPTER_REGISTER) << 8 | Cpu_GetRegister8(cpu, LITTERA_REGISTER);
        Video_SetPixelAbsolute(video, index, data);
        break;
    case Video_Command_SetChar:
        x = Cpu_GetRegister8(cpu, LITTERA_REGISTER);
        y = Cpu_GetRegister8(cpu, PAGINA_REGISTER);
        Video_SetChar(video, x, y, data);
        break;
    }

    Cpu_SetCache8(cpu, VIDEO_PORT_CMD, 0x00);
    Cpu_SetCache8(cpu, VIDEO_PORT_DATA, 0x00);
}
// #endregion */

// #region Tty module 
error_t Tty_Write(tty_t *tty)
{

    FILE *file = fopen(tty->filename, "a");
    int errno = fputc(tty->c, file);
    if (errno == EOF)
        return TTY_ERROR_WRITE_FAILED;

    errno = fclose(file);
    if (errno == EOF)
        return TTY_ERROR_CLOSE_FAILED;

    return ERROR_NONE;
}

void Tty_Execute(cpu_t *cpu, tty_t *tty)
{
    tty->c = Cpu_GetCache8(cpu, TTY_PORT_CHAR);
    if (tty->c != 0x00)
    {
        Tty_Write(tty);
    }
    Cpu_SetCache8(cpu, TTY_PORT_CHAR, 0x00);
}
// #endregion */

// #region Keyboard module 
void Kb_Handle(kb_t *kb, cpu_t *cpu)
{
    char controlChar;

    if (kb->state[SDL_SCANCODE_RETURN])
    {
        Cpu_SetCache8(cpu, kb->port, 0x0D);
    }
    if (kb->state[SDL_SCANCODE_BACKSPACE])
    {
        Cpu_SetCache8(cpu, kb->port, 0x08);
    }
    if (kb->state[SDL_SCANCODE_ESCAPE])
    {
        Cpu_SetCache8(cpu, kb->port, 0x1B);
    }
    if (kb->state[SDL_SCANCODE_TAB])
    {
        Cpu_SetCache8(cpu, kb->port, '\t');
    }
    if (kb->state[SDL_SCANCODE_LCTRL])
    {
        // TODO: Handle control characters
    }

    // trigger interrupt if port or modifier != 0
    if (Cpu_GetCache8(cpu, kb->port) != '\0')
    {
        printf("Key Port: %d\n", Cpu_GetCache8(cpu, kb->port));
    }
    // Erase cache
    Cpu_SetCache8(cpu, kb->port, '\0');
}
// #endregion */

// #region Disk module 
error_t Disk_Create(const char *path, disk_t *disk, uint16_t sector_count)
{

    struct sector prototype_sector;
    memset(prototype_sector.data, 0, SECTOR_SIZE);

    disk->filename = path;
    disk->fp = fopen(path, "wb");
    disk->sector_count = sector_count;
    if (disk->fp == NULL)
        return DISK_ERROR_OPEN_FAILED;

    for (int i = 0; i < sector_count; i++)
    {
        fwrite(&prototype_sector, sizeof(struct sector), 1, disk->fp);
    }

    fclose(disk->fp);

    return ERROR_NONE;
}

void Disk_LoadSector(disk_t *disk, uint16_t sector_number, struct sector *sector)
{
    fseek(disk->fp, sector_number * sizeof(struct sector), SEEK_SET);
    fread(sector, sizeof(struct sector), 1, disk->fp);
    ;
}

void Disk_SaveSector(disk_t *disk, uint16_t sector_number, struct sector *sector)
{
    fseek(disk->fp, sector_number * sizeof(struct sector), SEEK_SET);
    fwrite(sector, sizeof(struct sector), 1, disk->fp);
}

void Disk_Execute(cpu_t *cpu, disk_t *disk)
{
    return;
}
// #endregion */

// #region Mmu module

void Mmu_SetMode(mmu_t *mmu, enum Mmu_Mode mode)
{
    mmu->mode = mode;
}

uint32_t Mmu_Translate(mmu_t *mmu, uint32_t address)
{
    switch (mmu->mode)
    {
    case Mmu_Mode_Real:
        return address;
    case Mmu_Mode_Protected:
        // TODO: Implement protected mode
    }
}

// #endregion

// #region Taleä System module 
void TaleaSystem_Init(cpu_t *cpu, video_t *video, tty_t *tty, disk_t *disk, kb_t *kb, mmu_t * mmu)
{
    // Initialize systems
    Cpu_Init(cpu);
    tty->filename = TTY_FILE_PATH;
    disk->filename = DISK_FILE_PATH;
    disk->sector_count = DISK_SECTOR_COUNT;
    disk->fp = fopen(disk->filename, "rb+");
    if (disk->fp == NULL)
    {
        printf("Failed to open disk file\n");
        exit(1);
    }
    kb->port = KB_PORT;
    Mmu_SetMode(mmu, Mmu_Mode_Real); // Start in real mode

    error_t error = Video_Init(video);
    if (error != ERROR_NONE)
    {
        printf("Video init failed, error code %d\n", error);
    }

    // Setup systems
    Video_SetMode(video, TEXT_MODE);
}

void TaleaSystem_Run(cpu_t *cpu, video_t *video, tty_t *tty, disk_t *disk, kb_t *kb)
{

    struct clock clock_fps;
    int quit = SDL_FALSE;
    SDL_Event event;
    struct timespec frame_start, frame_end;
    kb->state = SDL_GetKeyboardState(NULL);

    while (!quit)
    {
        Clock_FrameStart(&clock_fps);

        // Before the render, perform 166666 cycles
        for (size_t cycles = 0; cycles < CYCLES_PER_FRAME; cycles++)
        {
            Cpu_Cycle(cpu);
            Video_Execute(cpu, video);
            Disk_Execute(cpu, disk); // TODO: implement Disk_Execute
            Tty_Execute(cpu, tty);
        }

        // Every 16ms check for events such as keypresses (83333 cycles at 10Mhz) perhaps too fast?
        SDL_StartTextInput();

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                quit = SDL_TRUE;
                break;
            case SDL_TEXTINPUT:
                SDL_StopTextInput();
                Cpu_SetCache8(cpu, kb->port, event.text.text[0]);
                break;
            }

            Kb_Handle(kb, cpu);
        }

        // Every 16ms, render the screen (166666 cycles at 10Mhz)
        Video_Render(video);

        Clock_FrameEnd(&clock_fps);
    }
}

void TaleaSystem_Destroy(cpu_t *cpu, video_t *video, disk_t *disk)
{
    // Destroy systems
    fclose(disk->fp);
    Cpu_Destroy(cpu);
    Video_Destroy(video);
}
// #endregion */

int main(int argc, char const *argv[])
{

    cpu_t cpu;
    tty_t tty;
    video_t video;
    disk_t disk;
    kb_t kb;
    mmu_t mmu;

    if (argc > 1)
    {
        if (strcmp(argv[1], "createDisk") == 0)
        {
            if (Disk_Create(DISK_FILE_PATH, &disk, DISK_SECTOR_COUNT) == ERROR_NONE)
            {
                printf("Disk created succesfully\n");
                return 0;
            }

            printf("Error creating disk\n");
            return -1;
        }
    }

    TaleaSystem_Init(&cpu, &video, &tty, &disk, &kb, &mmu);

    TaleaSystem_Run(&cpu, &video, &tty, &disk, &kb);

    TaleaSystem_Destroy(&cpu, &video, &disk);

    return 0;
}


// #region Instructions
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
static inline void Cpu_Instruction_Ld(cpu_t *cpu) {
    uint8_t registers = nextByte(cpu);
    uint16_t offset = nextWord(cpu);

    RegisterIndex dest = (registers >> 4) & 0x3;
    RegisterIndex segment = registers & 0x3; 
};
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