/* talea.c */
/* implementation includes */
#include <stdio.h>
#include <time.h>
#include "talea.h"
#include "isa.h"
#include "include/inprint/SDL2_inprint.h"
#include "include/inprint/inline_font.h"

#define IV_KBINCHAR 0xfe // TODO: Remove this plz


/* helpers */

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
	cpu->InstructionPointer = 0;
    cpu->Memory = malloc(sizeof(uint8_t) * MAX_MEMORY_SIZE);
    memset(cpu->General, 0, sizeof(cpu->General));
    memset(cpu->Segment, 0, sizeof(cpu->Segment));
    memset(cpu->Cache, 0, sizeof(cpu->Cache));
    memset(cpu->Memory, 0, sizeof(uint8_t) * MAX_MEMORY_SIZE);
}

void Cpu_Reset(cpu_t *cpu)
{
	cpu->InstructionPointer = 0;
    memset(cpu->General, 0, sizeof(cpu->General));
    memset(cpu->Segment, 0, sizeof(cpu->Segment));
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
uint32_t Cpu_Fetch(cpu_t *cpu)
{
    uint32_t instruction = 0;
    instruction |= (uint32_t)cpu->Memory[trimAddr(cpu->InstructionPointer + 3)] << 24;
    instruction |= (uint32_t)cpu->Memory[trimAddr(cpu->InstructionPointer + 2)] << 16;
    instruction |= (uint32_t)cpu->Memory[trimAddr(cpu->InstructionPointer + 1)] << 8;
    instruction |= (uint32_t)cpu->Memory[trimAddr(cpu->InstructionPointer)];
    return instruction;
}
error_t Cpu_Execute(cpu_t *cpu, uint32_t instruction)
{

	/* isa fields */
	    int16_t opcode = get_opcode(instruction);
	
	    int16_t rd = get_rd(instruction);
	    int16_t rs1 = get_rs1(instruction);
	    int16_t rs2 = get_rs2(instruction);
	    
	    int16_t imm_i = get_imm_i(instruction);
	    int16_t imm_b = make_imm_b(instruction);
	
	    int32_t imm_j = make_imm_j(instruction);
	
	    int16_t imm_u = get_imm_u(instruction);
	
	    int16_t imm_s = make_imm_s(instruction);
	
	    int16_t shamt = get_shamt(instruction);
	    int16_t mod = get_mod(instruction);
	    int16_t funct3 = get_funct3(instruction);
	    int16_t funct7 = get_funct7(instruction);

	
    switch (opcode)
    {
    /* isa lui */
        case Lui:
            Cpu_SetSegRegister(cpu, rd, imm_u); // loads high bytes of s:rd with imm_u
            Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
            break;

	/* isa auipc */
	    case Auipc:
	        Cpu_SetSegRegister(cpu, rd, Cpu_GetIp(cpu) + imm_u); // loads high bytes of s:rd with imm_u + ip
	        Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
	        break;

    /* isa jal */
        case Jal:
            Cpu_SetSegRegister(cpu, rd, Cpu_GetIp(cpu) + 4); // Gets return destination
            Cpu_SetIp(cpu, Cpu_GetIp(cpu) + imm_j); // increment jump to offset
            break;

    /* isa jalr */
        case Jalr:
            Cpu_SetSegRegister(cpu, rd, Cpu_GetIp(cpu) + 4); // Gets return destination
            Cpu_SetIp(cpu, (Cpu_GetSegRegister(cpu, rs1) + imm_i) & ~1); // increment ip by imm_i
            break;


	/* isa branches */
	    case Branch:
	        switch (funct3)
	        { 
	            case Beq: // BEQ
	                Cpu_SetIp(cpu, Cpu_GetIp(cpu) + ((Cpu_GetRegister(cpu, rs1) == Cpu_GetRegister(cpu, rs2)) ? imm_b : 4));
	                break; // increment ip by imm_b if rs1 == rs2
	            case Bne: // BNE
	                Cpu_SetIp(cpu, Cpu_GetIp(cpu) + ((Cpu_GetRegister(cpu, rs1) != Cpu_GetRegister(cpu, rs2)) ? imm_b : 4));
	                break; // increment ip by imm_b if rs1 != rs2
	            case Blt: // BLT
	                Cpu_SetIp(cpu, Cpu_GetIp(cpu) + (((int16_t)Cpu_GetRegister(cpu, rs1) < (int16_t)Cpu_GetRegister(cpu, rs2)) ? imm_b : 4));
	                break; // increment ip by imm_b if rs1 < rs2
	            case Bge: // BGE
	                Cpu_SetIp(cpu, Cpu_GetIp(cpu) + (((int16_t)Cpu_GetRegister(cpu, rs1) >= (int16_t)Cpu_GetRegister(cpu, rs2)) ? imm_b : 4));
	                break; // increment ip by imm_b if rs1 == rs2
	            case Bltu: // BLTU
	                Cpu_SetIp(cpu, Cpu_GetIp(cpu) + ((Cpu_GetRegister(cpu, rs1) < Cpu_GetRegister(cpu, rs2)) ? imm_b : 4));
	                break; // increment ip by imm_b if rs1 < rs2
	            case Bgeu: // BGEU
	                Cpu_SetIp(cpu, Cpu_GetIp(cpu) + ((Cpu_GetRegister(cpu, rs1) >= Cpu_GetRegister(cpu, rs2)) ? imm_b : 4));
	                break;
	        }
	        break;

    /* isa loads */
        case Load:
            switch (funct3)
            {
                case Lb: 
                  { // LB
                    uint8_t byte = Cpu_GetMemory8(cpu, Cpu_GetSegRegister(cpu, rs1) + imm_i);
                    Cpu_SetRegister(cpu, rd, (byte & 0x80) ? 0xff00 | byte : 0x0000 | byte); // load byte from memory at rs1 + imm_i into rd
                    break;// load sign extended byte from memory at rs1 + imm_i into rd  
                  }
                case Lh: // LH
                    Cpu_SetRegister(cpu, rd, Cpu_GetMemory16(cpu, Cpu_GetSegRegister(cpu, rs1) + imm_i)); // load halfword from memory at rs1 + imm_i into rd
                    break; // load halfword from memory at rs1 + imm_i into rd
                case Lbu: // LBU
                    Cpu_SetRegister(cpu, rd, 0x0000 | Cpu_GetMemory8(cpu, Cpu_GetSegRegister(cpu, rs1) + imm_i)); // load byte from memory at rs1 + imm_i into rd
                    break; // load unsigned byte from memory at rs1 + imm_i into rd
                case Lhc: // ADDAPT LW to load from cache
                	if (cpu->Psr & 0x8000) { /* TODO: Raise Privilege Exception*/ break;}
                	Cpu_SetRegister(cpu, rd, Cpu_GetCache16(cpu, Cpu_GetRegister(cpu, rs1) + imm_i));
                	break;
                case Lbc: // ADDAPT LHU to load from cache //TODO: Study if it would be useful to add load byte (signed) from cache
                	if (cpu->Psr & 0x8000) { /* TODO: Raise Privilege Exception*/ break;}
                	Cpu_SetRegister(cpu, rd, Cpu_GetCache8(cpu, Cpu_GetRegister(cpu, rs1) + imm_i));
                	break;
            }
            Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
            break;

    /* isa stores */
     case Store:
            switch (funct3)
            {
                case Sb: // SB
                    Cpu_SetMemory8(cpu, Cpu_GetSegRegister(cpu, rs1) + imm_s, (uint8_t)Cpu_GetRegister(cpu, rs2)); // store byte from rd to memory at rs1 + imm_i
                    break; // store byte from rd to memory at rs1 + imm_i
                case Sh: // SH
                    Cpu_SetMemory16(cpu, Cpu_GetSegRegister(cpu, rs1) + imm_s, Cpu_GetRegister(cpu, rs2)); // store halfword from rd to memory at rs1 + imm_i
                    break; // store halfword from rd to memory at rs1 + imm_i
                case Shc: // Addapt SW to store to cache
                	if (cpu->Psr & 0x8000) { /* TODO: Raise Privilege Exception*/ break;}
                	Cpu_SetCache16(cpu, Cpu_GetRegister(cpu, rs1) + imm_s, Cpu_GetRegister(cpu, rs2));
                	break;
                case Sbc: // NEW OPCODE TO Store to CACHE
                	if (cpu->Psr & 0x8000) { /* TODO: Raise Privilege Exception*/ break;}
                	Cpu_SetCache8(cpu, Cpu_GetRegister(cpu, rs1) + imm_s, Cpu_GetRegister(cpu, rs2));
                	break;
            }
            Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
            break;

    /* isa mathi */
        case MathI:
            switch (funct3)
            {
                case Addi: // ADDI
                    Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) + imm_i); // add imm_i to rs1 and store in rd
                    break; // add imm_i to rs1 and store in rd
                case Slti: // SLTI
                    Cpu_SetRegister(cpu, rd, ((int16_t)Cpu_GetRegister(cpu, rs1) < imm_i) ? 1 : 0); // set rd to 1 if rs1 < imm_i, else 0
                    break; // set rd to 1 if rs1 < imm_i, else 0
                case Sltiu: // SLTIU
                    Cpu_SetRegister(cpu, rd, (Cpu_GetRegister(cpu, rs1) < (uint16_t)imm_i) ? 1 : 0); // set rd to 1 if rs1 < imm_i, else 0
                    break; // set rd to 1 if rs1 < imm_i, else 0
                case Xori: // XORI
                    Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) ^ imm_i); // xor imm_i to rs1 and store in rd
                    break; // xor imm_i to rs1 and store in rd
                case Ori: // ORI
                    Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) | imm_i); // or imm_i to rs1 and store in rd
                    break; // or imm_i to rs1 and store in rd
                case Andi: // ANDI
                    Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) & imm_i); // and imm_i to rs1 and store in rd
                    break; // and imm_i to rs1 and store in rd
                case Slli: // SLLI
                    Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) << shamt); // shift left imm_i to rs1 and store in rd
                    break; // shift left imm_i to rs1 and store in rd
                case Srli_Srai: // SRLI/SRAI				ARITHMETIC												LOGICAL
                    Cpu_SetRegister(cpu, rd, (mod) ? (int16_t)Cpu_GetRegister(cpu, rs1) >> shamt : Cpu_GetRegister(cpu, rs1) >> shamt); // shift right imm_i to rs1 and store in rd
                    break; // shift right imm_i to rs1 and store in rd
            }
            Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
            break;

    /* isa mathr */
        case MathR:
            switch (funct3)
            {
                case Add_Sub: // ADD/SUB
                    Cpu_SetRegister(cpu, rd, (mod) ? Cpu_GetRegister(cpu, rs1) - Cpu_GetRegister(cpu, rs2) : Cpu_GetRegister(cpu, rs1) + Cpu_GetRegister(cpu, rs2)); // add rs2 to rs1 and store in rd
                    break; // add rs2 to rs1 and store in rd
                case Sll: // SLL
                    Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) << Cpu_GetRegister(cpu, rs2)); // shift left rs2 to rs1 and store in rd
                    break; // shift left rs2 to rs1 and store in rd
                case Slt: // SLT
                    Cpu_SetRegister(cpu, rd, ((int16_t)Cpu_GetRegister(cpu, rs1) < (int16_t)Cpu_GetRegister(cpu, rs2)) ? 1 : 0); // set rd to 1 if rs1 < rs2, else 0
                    break; // set rd to 1 if rs1 < rs2, else 0
                case Sltu: // SLTU
                    Cpu_SetRegister(cpu, rd, (Cpu_GetRegister(cpu, rs1) < Cpu_GetRegister(cpu, rs2)) ? 1 : 0); // set rd to 1 if rs1 < rs2, else 0
                    break; // set rd to 1 if rs1 < rs2, else 0
                case Xor: // XOR
                    Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) ^ Cpu_GetRegister(cpu, rs2)); // xor rs2 to rs1 and store in rd
                    break; // xor rs2 to rs1 and store in rd
                case Srl_Sra: // SRL/SRAI							ARITMETHIC												LOGICAL
                    Cpu_SetRegister(cpu, rd, (mod) ? (int16_t)Cpu_GetRegister(cpu, rs1) >> Cpu_GetRegister(cpu, rs2) : Cpu_GetRegister(cpu, rs1) >> Cpu_GetRegister(cpu, rs2)); // shift right rs2 to rs1 and store in rd
                    break; // shift right rs2 to rs1 and store in rd
                case Or: // OR
                    Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) | Cpu_GetRegister(cpu, rs2)); // or rs2 to rs1 and store in rd
                    break; // or rs2 to rs1 and store in rd
                case And: // AND
                    Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) + Cpu_GetRegister(cpu, rs2)); // add rs2 to rs1 and store in rd
                    break; // add rs2 to rs1 and store in rd
            }
            Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
            break;

    
    /* isa e */
    case System:
            switch (funct3)
                {
                    uint8_t imm_8 = (instruction & 0x0ff00000) >> 20; // get 8-bit immediate
                    case Trap: // Call trap routine
                        //TODO: Tidy up please
                        
                        if (cpu->Psr & 0x8000) {
                            // Swap stack pointer and supervisor stack pointer
                            cpu->Usp = Cpu_GetSegRegister(cpu, x2);
                            Cpu_SetSegRegister(cpu, x2, cpu->Ssp);
                        }
                        // Push Psr into Supervisor Stack
                        Cpu_SetMemory16(cpu, Cpu_GetSegRegister(cpu, x2) - 2, cpu->Psr);
                        // Load ip+4 into rd and push into Supervisor stack
                        Cpu_SetSegRegister(cpu, rd, Cpu_GetIp(cpu) + 4);
                        Cpu_SetMemory16(cpu, Cpu_GetSegRegister(cpu, x2) - 4, Cpu_GetIp(cpu) + 4);
                        
                        // Update Ssp
                        Cpu_SetSegRegister(cpu, x2, Cpu_GetSegRegister(cpu, x2) - 4);
    
                        // Set CPU in Supervisor mode
                        cpu->Psr &= 0x7fff;
                        // Load ip with the address stored in the trap vector at imm_8 << 1
                        Cpu_SetIp(cpu, Cpu_GetMemory16(cpu, 0x00ff & (imm_8 << 1)));
                        break;
                    case Ssr: // Set segment register
                        cpu->Segment[rd] = Cpu_GetRegister(cpu, rs1) + imm_8; // set segment register to rs1 + imm_8
                        Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
                        break;
                    case Gsr: // Get segment register
                        Cpu_SetRegister(cpu, rd, (rs1) ? cpu->Segment[rs1] : 0); // set rd to segment regiter of rs1 
                        Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
                        break;
                    case Gpsr:
                        Cpu_SetRegister(cpu, rd, cpu->Psr); // set rd to psr
                        Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
                        break;
                }
            break;
        case Supervisor:
            if (cpu->Psr & 0x8000) { // Supervisor mode: PSR[15] == 0
                // TODO: Raise exception if supervisor mode is not enabled
                break;
            }
    
            switch (funct3)
                {
                    case Rti: // Retrurn from  interrupt
                        // Pop return address from Supervisor Stack into ip
                        Cpu_SetIp(cpu, Cpu_GetMemory16(cpu, Cpu_GetSegRegister(cpu, x2)));
                        // Pop PSR from Supervisor Stack into PSR
                        cpu->Psr = Cpu_GetMemory16(cpu, Cpu_GetSegRegister(cpu, x2) + 2);
                        // Update Ssp
                        Cpu_SetSegRegister(cpu, x2, Cpu_GetSegRegister(cpu, x2) + 4);
                        // Swap stack pointer and supervisor stack pointer
    
                         if (cpu->Psr & 0x8000) { // Supervisor mode: PSR[15] == 0
                            // If caller was in user mode, return to user mode, else continue in supervisor mode
                            cpu->Ssp = Cpu_GetSegRegister(cpu, x2);
                            Cpu_SetSegRegister(cpu, x2, cpu->Usp);
                        }
    
    
                        break;
                    case Spsr: // Set Processor Status Register
                    {
                    	uint16_t imm_16 = (instruction & 0xffff0000) >> 16; // get 16-bit immediate
                        cpu->Psr = Cpu_GetRegister(cpu, rs1) + imm_16; // set segment register to rs1 + imm_8
                        Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
                        break;
                    		
                    }
                }
            break;

    
    default:
        //TODO: Raise exception if opcode is not valid
        printf("Unknown opcode: %x. Instruction: %x\n", opcode, instruction);
        return ERROR_UNKNOWN_OPCODE;
    }
    //printf("executed instruction: %x, opcode: %x, func3, %b\n", instruction, opcode, funct3);
    return ERROR_NONE;
}

/* memory access */
static inline uint32_t Cpu_GetIp(cpu_t *cpu) 
{
    return trimAddr(cpu->InstructionPointer);
}

static inline void Cpu_SetIp(cpu_t *cpu, uint32_t value)
{
    cpu->InstructionPointer = trimAddr(value);
}
static inline uint16_t Cpu_GetRegister(cpu_t *cpu, enum RegisterIndex index)
{
    return (index) ? cpu->General[index] : 0;
}

static inline void Cpu_SetRegister(cpu_t *cpu,  enum RegisterIndex index, uint16_t value)
{
    cpu->General[index] = value;
}

static inline uint32_t Cpu_GetSegRegister(cpu_t *cpu, enum RegisterIndex index)
{
    return (index) ? (uint32_t)cpu->Segment[index] << 16 | cpu->General[index] : 0;
}
static inline void Cpu_SetSegRegister(cpu_t *cpu, enum RegisterIndex index, uint32_t value)
{
    cpu->Segment[index] = (value >> 16);
    cpu->General[index] = (uint16_t)value;
}
inline uint8_t Cpu_GetCache8(cpu_t *cpu, uint8_t addr)
{
    return cpu->Cache[addr];
}

inline uint16_t Cpu_GetCache16(cpu_t *cpu, uint8_t addr)
{
    return (uint16_t)cpu->Cache[addr + 1] << 8 | cpu->Cache[addr];
}

inline void Cpu_SetCache8(cpu_t *cpu, uint8_t addr, uint8_t value)
{
    cpu->Cache[addr] = value;
}

inline void Cpu_SetCache16(cpu_t *cpu, uint8_t addr, uint16_t value)
{
    cpu->Cache[addr] = value & 0xFF;
    cpu->Cache[addr + 1] = value >> 8;
}
uint8_t Cpu_GetMemory8(cpu_t *cpu, uint32_t addr)
{
    return cpu->Memory[trimAddr(addr)];
}

uint16_t Cpu_GetMemory16(cpu_t *cpu, uint32_t addr)
{
    return (uint16_t)Cpu_GetMemory8(cpu, addr + 1) << 8 | Cpu_GetMemory8(cpu, addr);
}

void Cpu_SetMemory8(cpu_t *cpu, uint32_t addr, uint8_t value)
{
    cpu->Memory[trimAddr(addr)] = value;
}

void Cpu_SetMemory16(cpu_t *cpu, uint32_t addr, uint16_t value)
{
    Cpu_SetMemory8(cpu, addr, value & 0xFF);
    Cpu_SetMemory8(cpu, addr + 1, value >> 8);
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

void Kb_Execute(cpu_t *cpu, kb_t *kb, SDL_Event *event, int *quit)
{

    while (SDL_PollEvent(event))
    {
        switch (event->type)
        {
        case SDL_QUIT:
            *quit = SDL_TRUE;
            break;
        case SDL_KEYDOWN:
            Cpu_SetCache8(cpu, kb->port, event->key.keysym.sym);
            kb->interrupt.ready = 1;
            kb->interrupt.priority = 4;
            kb->interrupt.vector = IV_KBINCHAR;
            break;
        }
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
    video->pixels = calloc(GRAPHIC_MODE_WIDTH * GRAPHIC_MODE_HEIGHT, sizeof(uint8_t));
    video->charbuffer = calloc(TEXT_MODE_WIDTH * TEXT_MODE_HEIGHT, sizeof(char));
    video->line = calloc(TEXT_MODE_WIDTH, sizeof(char));

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
        index = Cpu_GetSegRegister(cpu, x5);
        Video_SetPixelAbsolute(video, index, data);
        break;
    case Video_Command_SetChar:
        x = Cpu_GetRegister(cpu, x5) & 0x0f;
        y = Cpu_GetRegister(cpu, x5) >> 8;
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
    uint8_t rr = (data >> 6) + x28;
    uint8_t ss = ((data & 0x30) >> 4) + x28;
    drive->current_disk = &drive->disk_list[data & 0x0f]; 

    uint16_t starting_point = Cpu_GetRegister(cpu, rr) * 512;
    uint16_t sector_number = Cpu_GetRegister(cpu, ss);

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

/* addenda implementation */
// Custom devices functions


/* interrupt handler */
void interrupt_handler(cpu_t *cpu, struct interrupt_interface *interrupt)
{
    if ((interrupt->enable && interrupt->ready) && (interrupt->priority > (cpu->Psr & 0x7)))
    {
        // TODO: Tidy up and refactor to bitfields maybe?
            interrupt->ready = 0;
            if (cpu->Psr & 0x8000) {
            // Swap stack pointer and supervisor stack pointer to enter supervisor mode
                cpu->Usp = Cpu_GetSegRegister(cpu, x2);
                Cpu_SetSegRegister(cpu, x2, cpu->Ssp);
            }
            // Push Psr into Supervisor Stack
            Cpu_SetMemory16(cpu, Cpu_GetSegRegister(cpu, x2) - 2, cpu->Psr);
            // Push ip into Supervisor stack (ip was updated by instruction executed before interrupt)
            Cpu_SetMemory16(cpu, Cpu_GetSegRegister(cpu, x2) - 4, Cpu_GetIp(cpu));
            
            // Update Ssp
            Cpu_SetSegRegister(cpu, x2, Cpu_GetSegRegister(cpu, x2) - 4);

            // Set CPU in Supervisor mode (or leave it in it)
            cpu->Psr &= 0x7fff;
            // Load ip with the address stored in the interrupt vector
            Cpu_SetIp(cpu, Cpu_GetMemory16(cpu, 0x0100 | (interrupt->vector << 1)));
            printf("triggered interrupt: %x\n", interrupt->vector);
    }
}
void handle_interrupts(cpu_t *cpu, video_t *video, drive_t *drive, kb_t *kb)
{
    if (Cpu_GetCache8(cpu, VIDEO_PORT_CMD)) Video_Execute(cpu, video);
    if (Cpu_GetCache8(cpu, DISK_PORT_CMD)) Disk_Execute(cpu, drive);
    interrupt_handler(cpu, &kb->interrupt);
    interrupt_handler(cpu, &video->interrupt);
    interrupt_handler(cpu, &drive->interrupt);
    /* addenda handle interrupts */
    // Custom devices interrupt handling

}


/* emulator initialization */
void TaleaSystem_Init(cpu_t *cpu, video_t *video, tty_t *tty, drive_t *drive, kb_t *kb, mmu_t * mmu)
{
	// Disable interrupts for Video and Drive Modules
	video->interrupt.enable = 0;
	drive->interrupt.enable = 0;

	// Enable interrupts for Kb module
	kb->interrupt.enable = 1;
	kb->interrupt.ready = 0;
	kb->interrupt.vector = 0;
	
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
    
	Video_Render(video);
    Kb_Execute(cpu, kb, &event, &quit);

    while (!quit)
    {
        Clock_FrameStart(&clock_fps);

        // Before the render, perform 166666 cycles
        for (size_t cycles = 0; cycles < CYCLES_PER_FRAME; cycles++)
        {
            Cpu_Cycle(cpu);
            handle_interrupts(cpu, video, drive, kb);
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

	if (argc == 3)
    {
        if (strcmp(argv[1], "load") == 0)
        {
        	FILE * program = fopen(argv[2], "rb");
			fseek(program, 0, SEEK_END);
			long psize = ftell(program);
			fseek(program, 0, SEEK_SET);

			uint8_t *hex = malloc(psize + 1);
			fread(hex, psize, 1, program);
			fclose(program);

			for (int i = 0; i<psize; i++)
			{
				Cpu_SetMemory8(&cpu, i, hex[i]);
			}

			free(hex);
		}
    }	    	    

    TaleaSystem_Run(&cpu, &video, &tty, &drive, &kb);

    TaleaSystem_Destroy(&cpu, &video, &drive);

    return 0;
}


