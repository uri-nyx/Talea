/* talea.c */
/* implementation includes */
#include <stdio.h>
#include <time.h>
#include "talea.h"
#include "include/inprint/SDL2_inprint.h"
#include "include/inprint/inline_font.h"


/* helpers */
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


/* clock implementation */
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

/* cpu implementation */
// #region Cpu Module
/* cpu initialization and deinitialization */
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

/* FDE cycle */
void Cpu_Cycle(cpu_t *cpu)
{
    error_t error = Cpu_Execute(cpu, Cpu_Fetch(cpu));

    if (error != ERROR_NONE)
    {
        TaleaSystem_Panic(error);
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
    case 0: //TODO: ISA
        return ERROR_NONE;
    default:
        return ERROR_UNKNOWN_OPCODE;
    }
}

/* memory access */
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
inline uint8_t Cpu_GetRegister8(cpu_t *cpu, enum RegisterIndex index)
{
    return cpu->RegisterFile[index];
}

inline uint16_t Cpu_GetRegister16(cpu_t *cpu, enum RegisterIndex index)
{
    return (uint16_t)cpu->RegisterFile[index] << 8 | cpu->RegisterFile[index + 1];
}

inline void Cpu_SetRegister8(cpu_t *cpu, enum RegisterIndex index, uint8_t value)
{
    cpu->RegisterFile[index] = value;
}

inline void Cpu_SetRegister16(cpu_t *cpu, enum RegisterIndex index, uint16_t value)
{
    cpu->RegisterFile[index] = value >> 8;
    cpu->RegisterFile[index + 1] = value & 0xFF;
}
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

/* mmu implementation */
// TODO: Implement mmu

/* tty implementation */
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

/* kb implementation */
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

void Kb_Execute(cpu_t *cpu, kb_t *kb, SDL_Event *event, int *quit)
{
    SDL_StartTextInput();

    while (SDL_PollEvent(event))
    {
        switch (event->type)
        {
        case SDL_QUIT:
            *quit = SDL_TRUE;
            break;
        case SDL_TEXTINPUT:
            SDL_StopTextInput();
            Cpu_SetCache8(cpu, kb->port, event->text.text[0]);
            break;
        }

        Kb_Handle(kb, cpu);
    }
}

/* video implementation */
// #region Video Module Implementation
/* video initialization */
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
void Video_Destroy(video_t *video)
{
    SDL_DestroyTexture(video->texture);
    SDL_DestroyRenderer(video->renderer);
    SDL_DestroyWindow(video->window);
    free(video->pixels);
    free(video->charbuffer);
    free(video->line);
    SDL_Quit();
}

/* video modes implementation */
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

/* video render implementation */
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

/* video execute implementation */
void Video_Execute(cpu_t *cpu, video_t *video)
{
    int x, y;
    uint32_t index;
    uint8_t data = Cpu_GetCache8(cpu, VIDEO_PORT_DATA);

    switch (Cpu_GetCache8(cpu, VIDEO_PORT_CMD))
    {
    case Video_Command_Nop:
        return;
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

/* text mode commands implementation */
error_t Video_SetChar(video_t *video, uint8_t x, uint8_t y, uint8_t c)
{
    if (x < 0 || x >= TEXT_MODE_WIDTH || y < 0 || y >= TEXT_MODE_HEIGHT)
        return VIDEO_ERROR_INVALID_COORDINATE;
    if (video->mode != TEXT_MODE)
        return VIDEO_ERROR_INVALID_MODE;
    video->charbuffer[x + y * TEXT_MODE_WIDTH] = c;
    return ERROR_NONE;
};

/* graphic mode commands implementation */
void Video_SetPixelAbsolute(video_t* video, uint32_t index, uint8_t color)
{
    index = index & 0x3ffff;
    video->pixels[index] = color;
}

// #endregion

/* disk drive implementation */
// #region Disk Drive
/* disk create */
error_t Disk_Create(const char *path, disk_t *disk, uint16_t sector_count)
{

    struct sector prototype_sector;
    memset(prototype_sector.data, 0, SECTOR_SIZE);

    disk->filename = path;
    disk->fp = fopen(path, "wb");
    disk->sector_count = sector_count;
    if (disk->fp == NULL)
        return DISK_ERROR_OPEN_FAILED;

    for (int i = 0; i <= sector_count; i++)
    {
        fwrite(&prototype_sector, sizeof(struct sector), 1, disk->fp);
    }

    fclose(disk->fp);

    return ERROR_NONE;
}

error_t Disk_CreateDrive(char *path, drive_t *drive, uint8_t disk_count)
{
    error_t error;
    drive->disk_count = disk_count;
    for (int i = 0; i < disk_count; i++)
    {
        char filename[strlen(DISK_FILE_PATH) + 2]; //remember \0
        sprintf(filename, "%s%X", DISK_FILE_PATH, i);
        error = Disk_Create(filename, &drive->disk_list[i], MAX_SECTOR_COUNT);

        if (error != ERROR_NONE)
        {
              return error;
        }
    }
    return ERROR_NONE;
}

/* disk execute implementation */
void Disk_Execute(cpu_t *cpu, drive_t *drive)
{
    uint8_t data = Cpu_GetCache16(cpu, DISK_PORT_DATA);
    uint8_t rr = ((data >> 6) * 2) + GPR1;
    uint8_t ss = (((data & 0x30) >> 4) * 2) + GPR1;
    drive->current_disk = &drive->disk_list[data & 0x0f]; 

    uint16_t starting_point = Cpu_GetRegister16(cpu, rr) * 512;
    uint16_t sector_number = Cpu_GetRegister16(cpu, ss);

    struct sector tmp_sector;

    switch (Cpu_GetCache8(cpu, DISK_PORT_CMD))
    {
    case Disk_Command_Nop:
        return;
    
    case Disk_Command_LoadSector:
        Disk_LoadSector(drive->current_disk, sector_number, &tmp_sector);
        memcpy(cpu->Memory + starting_point, &tmp_sector, sizeof(struct sector));
        break;
    case Disk_Command_StoreSector:
        memcpy(&tmp_sector, cpu->Memory + starting_point, sizeof(struct sector));
        Disk_StoreSector(drive->current_disk, sector_number, &tmp_sector);
        break;
    }

    Cpu_SetCache8(cpu, DISK_PORT_CMD, 0x00);
    Cpu_SetCache8(cpu, DISK_PORT_DATA, 0x00);
}

/* disk commands implementation */
void Disk_LoadSector(disk_t *disk, uint16_t sector_number, struct sector *sector)
{
    fseek(disk->fp, sector_number * sizeof(struct sector), SEEK_SET);
    fread(sector, sizeof(struct sector), 1, disk->fp);
}
void Disk_StoreSector(disk_t *disk, uint16_t sector_number, struct sector *sector)
{
    fseek(disk->fp, sector_number * sizeof(struct sector), SEEK_SET);
    fwrite(sector, sizeof(struct sector), 1, disk->fp);
}

// #endregion


/* emulator initialization */
void TaleaSystem_Init(cpu_t *cpu, video_t *video, tty_t *tty, drive_t *drive, kb_t *kb, mmu_t * mmu)
{
    // Initialize systems
    Cpu_Init(cpu);

    tty->filename = TTY_FILE_PATH;

    for (int i = 0; i < MAX_DISK_COUNT; i++)
    {
        char filename[strlen(DISK_FILE_PATH) + 2]; //remember \0
        sprintf(filename, "%s%X", DISK_FILE_PATH, i);

        drive->disk_list[i].filename = filename;
        drive->disk_list[i].fp = fopen(filename, "wb");
        drive->disk_list[i].sector_count = MAX_SECTOR_COUNT;

        if (drive->disk_list[i].fp == NULL)
        {
            printf("Failed to open disk file: %s\n", filename);
            exit(1);
        }
    }
    
    kb->port = KB_PORT;

    // MMU_Init(mmu);

    error_t error = Video_Init(video);
    if (error != ERROR_NONE)
    {
        
        printf("Video init failed, error code %d\n", error);
        return;
    }

    // Setup systems
    Video_SetMode(video, TEXT_MODE);
    /* addenda init */
    // Custom devices initialization

}
void TaleaSystem_Run(cpu_t *cpu, video_t *video, tty_t *tty, drive_t *drive, kb_t *kb)
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
            Disk_Execute(cpu, drive);
            Tty_Execute(cpu, tty);
            /* addenda execute (after tty) */
            // Custom devices execution

        }

        // Every 16ms check for events such as keypresses (83333 cycles at 10Mhz) perhaps too fast?
        Kb_Execute(cpu, kb, &event, &quit);
        /* addenda execute (after kb) */
        // Custom devices execution


        // Every 16ms, render the screen (166666 cycles at 10Mhz)
        Video_Render(video);

        Clock_FrameEnd(&clock_fps);
    }
}
void TaleaSystem_Destroy(cpu_t *cpu, video_t *video, drive_t *drive)
{
    // Destroy systems
    /* addenda destroy */
    // Custom devices destruction

    for (int i = 0; i < MAX_DISK_COUNT; i++)
    {
        fclose(drive->disk_list[i].fp);
    }

    Cpu_Destroy(cpu);
    Video_Destroy(video);
}
void TaleaSystem_Panic(error_t error)
{
    printf("Error: %d", error); // TODO: make this better
}


/* main */
int main(int argc, char const *argv[])
{

    cpu_t cpu;
    tty_t tty;
    video_t video;
    drive_t drive;
    kb_t kb;
    mmu_t mmu;
    /* addenda creation */
    // Custom devices variable declaration on int main()


    if (argc > 1)
    {
        if (strcmp(argv[1], "createDrive") == 0)
        {
            if (Disk_CreateDrive(DISK_FILE_PATH, &drive, MAX_DISK_COUNT) == ERROR_NONE)
            {
                printf("Drive created succesfully\n");
                return 0;
            }

            printf("Error creating drive\n");
            return -1;
        }
    }

    TaleaSystem_Init(&cpu, &video, &tty, &drive, &kb, &mmu);

    TaleaSystem_Run(&cpu, &video, &tty, &drive, &kb);

    TaleaSystem_Destroy(&cpu, &video, &drive);

    return 0;
}


