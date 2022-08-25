/* talea.c */
/* implementation includes */
#include <stdio.h>
#include <time.h>
#include "talea_working.h"
#include "vectors.h"
#include "include/inprint/SDL2_inprint.h"
#include "include/inprint/inline_font.h"
#include <unistd.h>

int dbg;

/* helpers */
void setfont(SDL_Renderer* renderer,  const char * fname)
{
    if (fname == NULL) {infont(NULL); return;}
    char fontpath[200]; 
    strcpy(fontpath, "src/include/fonts/");
    strcat(fontpath, fname);
    SDL_Texture* bmpfont = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP(fontpath));
    if (bmpfont == NULL)
    {
        printf("Error Loading font, reverting to default: %s", SDL_GetError());
        getchar();
    }
    infont(bmpfont);
}

void initFont(video_t *video, char* font)
{
    // TODO: Implement rich text mode
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
    /* The processor starts execution with all memory flushed to zero, and priority 7 (highest) in Supervisor Mode */
    cpu->InstructionPointer = 0;
    cpu->Memory = malloc(sizeof(uint8_t) * MAX_MEMORY_SIZE);
    cpu->Cache = malloc(sizeof(uint8_t) * CACHE_SIZE);
    memset(cpu->General, 0, sizeof(cpu->General));
    memset(cpu->Segment, 0, sizeof(cpu->Segment));
    memset(cpu->Memory, 0, sizeof(uint8_t) * MAX_MEMORY_SIZE);

    cpu->Psr = 0x0007; // Set Supervisor Mode and Priority 7
    // The kernel should issue a las tailored rti instruction to transfer control to user mode and allocate its stack pointer as follows:
    // It should push first the Psr for the user mode in the supervisor stack, with the mode bit set and the pririty level (usually 0, 1, or 2)
    // Then it should push the Entry point for the user mode (aligned to a 4 byte boundary)
    // Last, invoke the rti instruction to transfer control to user mode
}

void Cpu_Reset(cpu_t *cpu)
{
    cpu->InstructionPointer = 0;
    memset(cpu->General, 0, sizeof(cpu->General));
    memset(cpu->Segment, 0, sizeof(cpu->Segment));
    memset(cpu->Cache, 0, sizeof(uint8_t) * CACHE_SIZE);
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
    {
        uint32_t bf = Cpu_GetSegRegister(cpu, rd);
        Cpu_SetSegRegister(cpu, rd, imm_u); // loads high bytes of s:rd with imm_u
        Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
        //printf("lui: rd:%d imm:%x, before: %x, after: %x\n", rd, imm_u, bf, Cpu_GetSegRegister(cpu, rd));
        break;
    }

        /* isa auipc */
    case Auipc:
        Cpu_SetSegRegister(cpu, rd, Cpu_GetIp(cpu) + imm_u); // loads high bytes of s:rd with imm_u + ip
        Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4);                  // increment ip by 4
        break;

        /* isa jal */
    case Jal:
        Cpu_SetSegRegister(cpu, rd, Cpu_GetIp(cpu) + 4); // Gets return destination
        Cpu_SetIp(cpu, Cpu_GetIp(cpu) + imm_j);          // increment jump to offset
        break;

        /* isa jalr */
    case Jalr:
        Cpu_SetSegRegister(cpu, rd, Cpu_GetIp(cpu) + 4);             // Gets return destination
        Cpu_SetIp(cpu, (Cpu_GetSegRegister(cpu, rs1) + imm_i) & ~1); // increment ip by imm_i
        break;

        /* isa jump */
    case Jump:
        { //TODO: Include this in lit. There is a bit unused ;) see what you can do
        // Jumps to an absolute 24 bit address. Optionally it links in register x2
        uint32_t jump_address = (instruction & 0xfffffc00) >> 8;
        Cpu_SetSegRegister(cpu, (rd & 0x01), Cpu_GetIp(cpu) + 4);  // Gets return destination
        Cpu_SetIp(cpu, jump_address); // increment ip by imm_i
        break;
        }

        /* isa branches */
    case Branch:
        switch (funct3)
        {
        case Beq: // BEQ
            Cpu_SetIp(cpu, Cpu_GetIp(cpu) + ((Cpu_GetRegister(cpu, rs1) == Cpu_GetRegister(cpu, rs2)) ? imm_b : 4));
            break; // increment ip by imm_b if rs1 == rs2
        case Bne:  // BNE
            Cpu_SetIp(cpu, Cpu_GetIp(cpu) + ((Cpu_GetRegister(cpu, rs1) != Cpu_GetRegister(cpu, rs2)) ? imm_b : 4));
            break; // increment ip by imm_b if rs1 != rs2
        case Blt:  // BLT
            Cpu_SetIp(cpu, Cpu_GetIp(cpu) + (((int16_t)Cpu_GetRegister(cpu, rs1) < (int16_t)Cpu_GetRegister(cpu, rs2)) ? imm_b : 4));
            break; // increment ip by imm_b if rs1 < rs2
        case Bge:  // BGE
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
            Cpu_SetRegister(cpu, rd, (uint16_t)(int16_t)(int8_t)byte); // load byte from memory at rs1 + imm_i into rd
            break;                                                                   // load sign extended byte from memory at rs1 + imm_i into rd
        }
        case Lh:                                                                                          // LH
            Cpu_SetRegister(cpu, rd, Cpu_GetMemory16(cpu, Cpu_GetSegRegister(cpu, rs1) + imm_i));         // load halfword from memory at rs1 + imm_i into rd
            break;                                                                                        // load halfword from memory at rs1 + imm_i into rd
        case Lbu:                                                                                         // LBU
            Cpu_SetRegister(cpu, rd, 0x0000 | Cpu_GetMemory8(cpu, Cpu_GetSegRegister(cpu, rs1) + imm_i)); // load byte from memory at rs1 + imm_i into rd
            break;                                                                                        // load unsigned byte from memory at rs1 + imm_i into rd
        case Lhc:                                                                                         // ADDAPT LW to load from cache
            if (cpu->Psr & 0x8000)
            {
                TaleaSystem_RaiseException(cpu, VECTOR_EXCEPTION_BAD_PRIVILEGE);
                return ERROR_BAD_PRIVILEGE;
            }
            Cpu_SetRegister(cpu, rd, Cpu_GetCache16(cpu, Cpu_GetRegister(cpu, rs1) + imm_i));
            break;
        case Lbc: //Load byte signed from cache
            if (cpu->Psr & 0x8000)
            {
                TaleaSystem_RaiseException(cpu, VECTOR_EXCEPTION_BAD_PRIVILEGE);
                return ERROR_BAD_PRIVILEGE;
            }
            Cpu_SetRegister(cpu, rd, (uint16_t)(int16_t)(int8_t)Cpu_GetCache8(cpu, Cpu_GetRegister(cpu, rs1) + imm_i));
            break;
        case Lbuc: //Load byte unsigned from cache
            if (cpu->Psr & 0x8000)
            {
                TaleaSystem_RaiseException(cpu, VECTOR_EXCEPTION_BAD_PRIVILEGE);
                return ERROR_BAD_PRIVILEGE;
            }
            Cpu_SetRegister(cpu, rd, Cpu_GetCache8(cpu, Cpu_GetRegister(cpu, rs1) + imm_i));
            break;
        }
        Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
        break;

        /* isa stores */
    case Store:
        switch (funct3)
        {
        case Sb:                                                                                           // SB
            Cpu_SetMemory8(cpu, Cpu_GetSegRegister(cpu, rs1) + imm_s, (uint8_t)Cpu_GetRegister(cpu, rs2)); // store byte from rd to memory at rs1 + imm_i
            //printf("sb: opcode: %x, rs1: %d, imm: %x, rs2:%d\n", opcode, rs1, imm_s, rs2);
            break;                                                                                         // store byte from rd to memory at rs1 + imm_i
        case Sh:                                                                                           // SH
            Cpu_SetMemory16(cpu, Cpu_GetSegRegister(cpu, rs1) + imm_s, Cpu_GetRegister(cpu, rs2));         // store halfword from rd to memory at rs1 + imm_i
            break;                                                                                         // store halfword from rd to memory at rs1 + imm_i
        case Shc:
            //printf("SHC: RS1: %d, IMM: %d, RS2 (VALUE): %d\n", rs1, imm_s, rs2);                                                                                       // Addapt SW to store to cache
            if (cpu->Psr & 0x8000)
            {
                TaleaSystem_RaiseException(cpu, VECTOR_EXCEPTION_BAD_PRIVILEGE);
                return ERROR_BAD_PRIVILEGE;
            }
            Cpu_SetCache16(cpu, Cpu_GetRegister(cpu, rs1) + imm_s, Cpu_GetRegister(cpu, rs2));
            break;
        case Sbc: // NEW OPCODE TO Store to CACHE
            //printf("SBC: RS1: %d, IMM: %d, RS2 (VALUE): %d\n", rs1, imm_s, rs2);      
            if (cpu->Psr & 0x8000)
            {
                TaleaSystem_RaiseException(cpu, VECTOR_EXCEPTION_BAD_PRIVILEGE);
                return ERROR_BAD_PRIVILEGE;
            }
            Cpu_SetCache8(cpu, Cpu_GetRegister(cpu, rs1) + imm_s, Cpu_GetRegister(cpu, rs2));
            break;
        }
        Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
        break;

        /* isa mathi */
    case MathI:
        switch (funct3)
        {
        case Addi:                                                                                                              // ADDI
            Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) + imm_i);                                                        // add imm_i to rs1 and store in rd
            break;                                                                                                              // add imm_i to rs1 and store in rd
        case Slti:                                                                                                              // SLTI
            Cpu_SetRegister(cpu, rd, ((int16_t)Cpu_GetRegister(cpu, rs1) < imm_i) ? 1 : 0);                                     // set rd to 1 if rs1 < imm_i, else 0
            break;                                                                                                              // set rd to 1 if rs1 < imm_i, else 0
        case Sltiu:                                                                                                             // SLTIU
            Cpu_SetRegister(cpu, rd, (Cpu_GetRegister(cpu, rs1) < (uint16_t)imm_i) ? 1 : 0);                                    // set rd to 1 if rs1 < imm_i, else 0
            break;                                                                                                              // set rd to 1 if rs1 < imm_i, else 0
        case Xori:                                                                                                              // XORI
            Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) ^ imm_i);                                                        // xor imm_i to rs1 and store in rd
            break;                                                                                                              // xor imm_i to rs1 and store in rd
        case Ori:                                                                                                               // ORI
            Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) | imm_i);                                                        // or imm_i to rs1 and store in rd
            break;                                                                                                              // or imm_i to rs1 and store in rd
        case Andi:                                                                                                              // ANDI
            Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) & imm_i);                                                        // and imm_i to rs1 and store in rd
            break;                                                                                                              // and imm_i to rs1 and store in rd
        case Slli:                                                                                                              // SLLI
            Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) << shamt);                                                       // shift left imm_i to rs1 and store in rd
            break;                                                                                                              // shift left imm_i to rs1 and store in rd
        case Srli_Srai:                                                                                                         // SRLI/SRAI				ARITHMETIC												LOGICAL
            Cpu_SetRegister(cpu, rd, (mod) ? (int16_t)Cpu_GetRegister(cpu, rs1) >> shamt : Cpu_GetRegister(cpu, rs1) >> shamt); // shift right imm_i to rs1 and store in rd
            break;                                                                                                              // shift right imm_i to rs1 and store in rd
        }
        Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
        break;

        /* isa mathr */
    case MathR:
        switch (funct3)
        {
        case Add_Sub:                                                                                                                                                   // ADD/SUB
            Cpu_SetRegister(cpu, rd, (mod) ? Cpu_GetRegister(cpu, rs1) - Cpu_GetRegister(cpu, rs2) : Cpu_GetRegister(cpu, rs1) + Cpu_GetRegister(cpu, rs2));            // add rs2 to rs1 and store in rd
            break;                                                                                                                                                      // add rs2 to rs1 and store in rd
        case Sll:                                                                                                                                                       // SLL
            Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) << Cpu_GetRegister(cpu, rs2));                                                                           // shift left rs2 to rs1 and store in rd
            break;                                                                                                                                                      // shift left rs2 to rs1 and store in rd
        case Slt:                                                                                                                                                       // SLT
            Cpu_SetRegister(cpu, rd, ((int16_t)Cpu_GetRegister(cpu, rs1) < (int16_t)Cpu_GetRegister(cpu, rs2)) ? 1 : 0);                                                // set rd to 1 if rs1 < rs2, else 0
            break;                                                                                                                                                      // set rd to 1 if rs1 < rs2, else 0
        case Sltu:                                                                                                                                                      // SLTU
            Cpu_SetRegister(cpu, rd, (Cpu_GetRegister(cpu, rs1) < Cpu_GetRegister(cpu, rs2)) ? 1 : 0);                                                                  // set rd to 1 if rs1 < rs2, else 0
            break;                                                                                                                                                      // set rd to 1 if rs1 < rs2, else 0
        case Xor:                                                                                                                                                       // XOR
            Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) ^ Cpu_GetRegister(cpu, rs2));                                                                            // xor rs2 to rs1 and store in rd
            break;                                                                                                                                                      // xor rs2 to rs1 and store in rd
        case Srl_Sra:                                                                                                                                                   // SRL/SRAI							ARITMETHIC												LOGICAL
            Cpu_SetRegister(cpu, rd, (mod) ? (int16_t)Cpu_GetRegister(cpu, rs1) >> Cpu_GetRegister(cpu, rs2) : Cpu_GetRegister(cpu, rs1) >> Cpu_GetRegister(cpu, rs2)); // shift right rs2 to rs1 and store in rd
            break;                                                                                                                                                      // shift right rs2 to rs1 and store in rd
        case Or:                                                                                                                                                        // OR
            Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) | Cpu_GetRegister(cpu, rs2));                                                                           // or rs2 to rs1 and store in rd
            break;                                                                                                                                                      // or rs2 to rs1 and store in rd
        case And:                                                                                                                                                       // AND
            Cpu_SetRegister(cpu, rd, Cpu_GetRegister(cpu, rs1) + Cpu_GetRegister(cpu, rs2));                                                                            // add rs2 to rs1 and store in rd
            break;                                                                                                                                                      // add rs2 to rs1 and store in rd
        }
        Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
        break;

    /* isa e */
    case System:
        switch (funct3)
        {
            uint8_t imm_8 = (instruction & 0x0ff00000) >> 20; // get 8-bit immediate
        case Trap:                                       // Call trap routine
            // TODO: Tidy up please
            imm_8 = (instruction & 0x0ff00000) >> 20;

            if (cpu->Psr & 0x8000)
            {
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
        case Ssr:                                                 // Set segment register
            imm_8 = (instruction & 0x0ff00000) >> 20;
            cpu->Segment[rd] = Cpu_GetRegister(cpu, rs1) + imm_8; // set segment register to rs1 + imm_8
            Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4);                   // increment ip by 4
            break;
        case Gsr:                                                    // Get segment register
            Cpu_SetRegister(cpu, rd, (rs1) ? cpu->Segment[rs1] : 0); // set rd to segment regiter of rs1
            Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4);                      // increment ip by 4
            //printf("gsr passed");
            break;
        case Gpsr:
            Cpu_SetRegister(cpu, rd, cpu->Psr); // set rd to psr
            Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4); // increment ip by 4
            break;
        }
        break;
    case Supervisor:
        if (cpu->Psr & 0x8000)
        {
            TaleaSystem_RaiseException(cpu, VECTOR_EXCEPTION_BAD_PRIVILEGE);
            return ERROR_BAD_PRIVILEGE;
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

            if (cpu->Psr & 0x8000)
            { // Supervisor mode: PSR[15] == 0
                // If caller was in user mode, return to user mode, else continue in supervisor mode
                cpu->Ssp = Cpu_GetSegRegister(cpu, x2);
                Cpu_SetSegRegister(cpu, x2, cpu->Usp);
            }

            break;
        case Spsr: // Set Processor Status Register
        {
            uint16_t imm_16 = (instruction & 0xffff0000) >> 16; // get 16-bit immediate
            cpu->Psr = Cpu_GetRegister(cpu, rd) + imm_16;      // set segment register to rs1 + imm_8
            Cpu_SetIp(cpu, Cpu_GetIp(cpu) + 4);                 // increment ip by 4
            break;
        }
        }
        break;

    default:
        {
        TaleaSystem_RaiseException(cpu, VECTOR_EXCEPTION_ILLEGAL_OPCODE);
        printf("Illegal opcode: %x. Instruction: %x at address@ip: %x\n", opcode, instruction, Cpu_GetIp(cpu));
        
        return ERROR_UNKNOWN_OPCODE;
        }
    }
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

static inline void Cpu_SetRegister(cpu_t *cpu, enum RegisterIndex index, uint16_t value)
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
static inline uint8_t Cpu_GetCache8(cpu_t *cpu, uint16_t addr)
{
    return cpu->Cache[addr];
}

static inline uint16_t Cpu_GetCache16(cpu_t *cpu, uint16_t addr)
{
    return (uint16_t)cpu->Cache[addr + 1] << 8 | cpu->Cache[addr];
}

static inline void Cpu_SetCache8(cpu_t *cpu, uint16_t addr, uint8_t value)
{
    cpu->Cache[addr] = value;
}

static inline void Cpu_SetCache16(cpu_t *cpu, uint16_t addr, uint16_t value)
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

void kb_send(cpu_t *cpu, kb_t *kb, uint16_t k)
{
    //printf("%x, %c\n", k, k);
    Cpu_SetCache16(cpu, kb->port, k);
    kb->interrupt.ready = 1;
    kb->interrupt.priority = 4;
    kb->interrupt.vector = VECTOR_INTERRUPT_KEYBOARD;
}

void Kb_Execute(cpu_t *cpu, kb_t *kb, SDL_Event *event, int *quit)
{
    SDL_StartTextInput();
    while (SDL_PollEvent(event))
    {
        uint16_t k;
        uint32_t l;
        switch (event->type)
        {
        case SDL_QUIT:
            *quit = SDL_TRUE;
            break;
        
        case SDL_KEYDOWN:
            switch (event->key.keysym.sym)
            {
            case SDLK_TAB:
                k = '\t';
                kb_send(cpu, kb, k);
                break;
            case SDLK_ESCAPE:
                k = '\x1b';
                kb_send(cpu, kb, k);
                break;
            case SDLK_BACKSPACE:
                k = '\b';
                kb_send(cpu, kb, k);
                break;
            case SDLK_RETURN:
                k = '\n';
                kb_send(cpu, kb, k);
                break;
            }
            break;

        case SDL_TEXTINPUT:
            memcpy(&k, event->text.text, 2);
            kb_send(cpu, kb, k);
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
                                     WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN /*| SDL_WINDOW_BORDERLESS*/ | SDL_WINDOW_RESIZABLE);
    video->renderer = SDL_CreateRenderer(video->window, -1, RENDERER_FLAGS);
    video->texture = SDL_CreateTexture(video->renderer,
                                       SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STATIC, WINDOW_WIDTH, WINDOW_HEIGHT);
    video->pixels = calloc(GRAPHIC_MODE_WIDTH * GRAPHIC_MODE_HEIGHT, sizeof(uint8_t));
    video->charbuffer = calloc(TEXT_MODE_WIDTH * TEXT_MODE_HEIGHT, sizeof(char));
    video->line = calloc(TEXT_MODE_WIDTH, sizeof(char));

    memset(video->charbuffer, ' ', TEXT_MODE_WIDTH * TEXT_MODE_HEIGHT);

    inrenderer(video->renderer);
    setfont(video->renderer, FONTDEFAULT);
    getFontwh(&video->fw, &video->fh);

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
    if (mode > RTEXT_MODE)
        return VIDEO_ERROR_INVALID_MODE;
    
    int w, h;
    switch (mode)
    {
    case GRAPHIC_MODE:
        /* set window to 640*400 */
        w = GRAPHIC_MODE_WIDTH;
        h = GRAPHIC_MODE_HEIGHT;
        break;
    
    case TEXT_MODE:
        /* set window to text widths */
        w = TEXT_MODE_WIDTH * video->fw;
        h = TEXT_MODE_HEIGHT * video->fh;
        break;
    case RTEXT_MODE: // TODO: implement rich text mode
        initFont(video, Video_RichFonts[RICHFONTDEFAULT]);
        /* set window to text widths */
        w = TEXT_MODE_WIDTH * video->fw;
        h = TEXT_MODE_HEIGHT * video->fh;
        break;
    }

    SDL_SetWindowSize(video->window, w, h);
    video->mode = mode;
    return ERROR_NONE;
}

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
            inprint(video->renderer, video->line, 0, i * video->fh);
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
    uint32_t index = 0;
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
    case Video_Command_SetMode_RText:
        Video_SetMode(video, RTEXT_MODE);
        break;
    case Video_Command_SetMode_Graphic:
        Video_SetMode(video, GRAPHIC_MODE);
        break;
    case Video_Command_SetPixel:
        index = Cpu_GetSegRegister(cpu, x5);
        Video_SetPixelAbsolute(video, index, data);
        break;
    case Video_Command_Blit:
        index = Cpu_GetSegRegister(cpu, x5);
        Video_Blit(video, cpu->Memory + index);
        break;
    case Video_Command_SetChar: // TODO: MAKE CHARBUFFER BIGGER, MAYBE TO 128 chars wide
        x = Cpu_GetRegister(cpu, x5) & 0x7f;
        y = Cpu_GetRegister(cpu, x5) >> 8;

        // scroll LOSING INFORMATION
        if (data == '\n')
        {
            Cpu_SetRegister(cpu, x5, (y << 8) | TEXT_MODE_WIDTH);
            break;
        } 

        if (x >= TEXT_MODE_WIDTH )
        {
            x = 0;
            y++;
            Cpu_SetRegister(cpu, x5, (y << 8) | 0);
        }

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
}

/* graphic mode commands implementation */
void Video_SetPixelAbsolute(video_t *video, uint32_t index, uint8_t color)
{
    index = index & 0x3ffff;
    video->pixels[index] = color;
}

void Video_Blit(video_t *video, uint8_t* index)
{
    memcpy(video->pixels, index, sizeof(uint8_t) * GRAPHIC_MODE_HEIGHT * GRAPHIC_MODE_WIDTH);
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
        char filename[strlen(DISK_FILE_PATH) + 6]; // remember \0
        sprintf(filename, "%s%X.hex", DISK_FILE_PATH, i);
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
    uint8_t data = Cpu_GetCache8(cpu, DISK_PORT_DATA);
    uint8_t rr = (data >> 6) + x28;
    uint8_t ss = ((data & 0x30) >> 4) + x28;
    drive->current_disk = &drive->disk_list[data & 0x0f];

    uint16_t starting_point = Cpu_GetRegister(cpu, rr) * 512;
    uint16_t sector_number = Cpu_GetRegister(cpu, ss);

    struct sector tmp_sector;
    uint8_t response;

    switch (Cpu_GetCache8(cpu, DISK_PORT_CMD))
    {
    case Disk_Command_Nop:
        return;

    case Disk_Command_LoadSector:
        Disk_LoadSector(drive->current_disk, sector_number, &tmp_sector);
        memmove(cpu->Memory + starting_point, &tmp_sector, sizeof(struct sector));
        break;
    case Disk_Command_StoreSector:
        memmove(&tmp_sector, cpu->Memory + starting_point, sizeof(struct sector));
        Disk_StoreSector(drive->current_disk, sector_number, &tmp_sector);
        break;
    case Disk_Command_Query:
        Disk_Query(drive,data, &response);
        Cpu_SetSegRegister(cpu, x6, response);
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

void Disk_Query(drive_t *drive, uint8_t query, uint8_t *response)
{
    struct sector tmp_sector;
    switch (query)
    {
    case BOOTABLE:
        Disk_LoadSector(&drive->disk_list[0], 0, &tmp_sector);
        *response = 0;
        if (tmp_sector.data[510] == 0xaa && tmp_sector.data[511] == 0x55) *response = 1;
        break;
    }
    
}


// #endregion

/* addenda implementation */
// Custom devices functions

void Tps_Execute(cpu_t *cpu, tps_drive_t *tps) 
{
    uint8_t sector_number = Cpu_GetCache8(cpu, TPS_PORT_DATA);
    
    uint16_t starting_point = Cpu_GetRegister(cpu, x6) * 512; //x6 will be the pointer to the starting mounting point

    struct sector tmp_sector;

    uint8_t cmd = Cpu_GetCache8(cpu, TPS_PORT_CMD);
    tps->current_tps = ((cmd & 0x80) == TA) ? &tps->tps_a : &tps->tps_b;

    uint8_t response;
    switch (cmd & 0x7f)
    {
    case Tps_Command_Nop:
        return;

    case Tps_Command_Query:

        TPS_Query(tps, sector_number, &response);
        Cpu_SetRegister(cpu, x6, response);
        break;

    case Tps_Command_Open:
        tps->current_tps->fp = fopen(tps->current_tps->filename, "r+b");
        break;
    
    case Tps_Command_Close:
        fclose(tps->current_tps->fp);
        break;

    case Tps_Command_LoadSector:
        //printf("starting point: %x\n", starting_point);
        TPS_LoadSector(tps->current_tps, sector_number, &tmp_sector);
        memmove(cpu->Memory + starting_point, &tmp_sector, sizeof(struct sector));
        break;
    case Tps_Command_StoreSector:
        memmove(&tmp_sector, cpu->Memory + starting_point, sizeof(struct sector));
        TPS_StoreSector(tps->current_tps, sector_number, &tmp_sector);
        break;
    }

    Cpu_SetCache8(cpu, TPS_PORT_CMD, 0x00);
    Cpu_SetCache8(cpu, TPS_PORT_DATA, 0x00);
}


void TPS_LoadSector(tps_t *tps, uint8_t sector_number, struct sector *sector)
{
    fseek(tps->fp, sector_number * sizeof(struct sector), SEEK_SET);
    fread(sector, sizeof(struct sector), 1, tps->fp);
}
void TPS_StoreSector(tps_t *tps, uint8_t sector_number, struct sector *sector)
{
    fseek(tps->fp, sector_number * sizeof(struct sector), SEEK_SET);
    fwrite(sector, sizeof(struct sector), 1, tps->fp);
}

void TPS_Query(tps_drive_t *tps, uint8_t query, uint8_t *response)
{
    struct sector tmp_sector;
    switch (query)
    {
    case BOOTABLE:
        tps->current_tps->fp = fopen(tps->current_tps->filename, "r+b");
        TPS_LoadSector(tps->current_tps, 0, &tmp_sector);
        *response = 0;
        if (tmp_sector.data[510] == 0xaa && tmp_sector.data[511] == 0x55) *response = 1;
        break;

    case PRESENT:
        *response = 0;
        if (access(tps->current_tps->filename, F_OK) == 0) *response = 1;
        break;
    }

}

/* interrupt handler */
void interrupt_handler(cpu_t *cpu, struct interrupt_interface *interrupt)
{
    if ((interrupt->enable && interrupt->ready) && (interrupt->priority > (cpu->Psr & 0x7)))
    {
        // TODO: Tidy up and refactor to bitfields maybe?
        interrupt->ready = 0;
        if (cpu->Psr & 0x8000)
        {
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
        Cpu_SetIp(cpu, Cpu_GetMemory16(cpu, (0x0100 + interrupt->vector) << 1));
        //printf("triggered interrupt: vector %x, in address %x\n", interrupt->vector, 0x0100 | (0x0100 + interrupt->vector) << 1);
        //printf("State after interrupt:\n");
        //printf("IP: %d\n", Cpu_GetIp(cpu));
    }
}

void exception_handler(cpu_t *cpu, struct interrupt_interface *exception)
{
    uint16_t ex_hand = Cpu_GetMemory16(cpu, (0x0100 + exception->vector) << 1);
    if (exception->enable && exception->ready) // Exceptions are allways triggered
    {
        // TODO: Tidy up and refactor to bitfields maybe?
        exception->ready = 0;
        if (cpu->Psr & 0x8000)
        {
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
        Cpu_SetIp(cpu, ex_hand);
    }
}

void handle_interrupts(cpu_t *cpu, video_t *video, drive_t *drive, tps_drive_t *tps, kb_t *kb, tty_t *tty)
{
    if (Cpu_GetCache8(cpu, VIDEO_PORT_CMD))
        Video_Execute(cpu, video);
    if (Cpu_GetCache8(cpu, DISK_PORT_CMD))
        Disk_Execute(cpu, drive);
    if (Cpu_GetCache8(cpu, TPS_PORT_CMD))
        Tps_Execute(cpu, tps);
    if (Cpu_GetCache8(cpu, TTY_PORT_CHAR))
        Tty_Execute(cpu, tty);
    interrupt_handler(cpu, &kb->interrupt);
    interrupt_handler(cpu, &video->interrupt);
    interrupt_handler(cpu, &drive->interrupt);
    /* addenda handle interrupts */
    // Custom devices interrupt handling
}

/* emulator initialization */
void TaleaSystem_Init(cpu_t *cpu, video_t *video, tty_t *tty, drive_t *drive, tps_drive_t *tps, kb_t *kb, mmu_t *mmu)
{
    // Disable interrupts for Video and Drive Modules
    video->interrupt.enable = 0;
    drive->interrupt.enable = 0;
    tps->interrupt.enable   = 0;


    // Enable interrupts for Kb module
    kb->interrupt.enable = 1;
    kb->interrupt.ready = 0;
    kb->interrupt.vector = 0;
    kb->interrupt.priority = 4;

    // Initialize systems
    Cpu_Init(cpu);

    tty->filename = TTY_FILE_PATH;

    // initialize disk
    for (int i = 0; i < MAX_DISK_COUNT; i++)
    {
        char filename[strlen(DISK_FILE_PATH) + 6]; // remember \0
        sprintf(filename, "%s%X.hex", DISK_FILE_PATH, i);

        drive->disk_list[i].filename = filename;
        drive->disk_list[i].fp = fopen(filename, "r+b");
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
void TaleaSystem_Run(cpu_t *cpu, video_t *video, tty_t *tty, drive_t *drive, tps_drive_t *tps, kb_t *kb)
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
        if (dbg)
        {
            printf("----------\nCycle: %lu\n", cycles);
            printf("IP: %d\n", Cpu_GetIp(cpu));
            printf("SP: %x\n", Cpu_GetSegRegister(cpu, x2));
            printf("PSR: %x\n", cpu->Psr);
            printf("\n");
            printf("Charbuffer: %x \n", Cpu_GetCache8(cpu, VIDEO_PORT_DATA));
            for (int i = 0; i < 16; i++)
            {
                printf("%c ", video->charbuffer[i]);
            }
            printf("\nCache:\n");
            for (int i = 0; i < 16; i++)
            {
                printf("%x ", Cpu_GetCache8(cpu, i));
            }
            printf("\nBUFFER:\n");
            for (int i = 1320; i < 1400; i++)
            {
                printf("%c", Cpu_GetMemory8(cpu, i));
            }
            printf("\nA0: %x A1: %x, X5: %x, T1: %x, T2:%x, T6: %x\n----------", Cpu_GetRegister(cpu, x10), Cpu_GetRegister(cpu, x11), Cpu_GetRegister(cpu, x5), Cpu_GetRegister(cpu, x6), Cpu_GetRegister(cpu, x7), Cpu_GetRegister(cpu, x31));
            printf("Memory at Illegal Opcode vector [%x]\n", Cpu_GetMemory16(cpu, (0x0100 + VECTOR_EXCEPTION_ILLEGAL_OPCODE) << 1));
            char c = getchar();
            if (c == 'q')
            {
                exit(1);
            }
            if (c == 'r') dbg = 0;
        }
            Cpu_Cycle(cpu);
            handle_interrupts(cpu, video, drive, tps, kb, tty);

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
    printf("Error %x:\t", error);
}

void TaleaSystem_RaiseException(cpu_t *cpu, enum Exception vector)
{
    struct interrupt_interface exception;
    exception.enable = 1;
    exception.ready = 1;
    exception.priority = 7;
    exception.vector = (uint8_t)vector;
    exception_handler(cpu, &exception);
}

/* main */
int main(int argc, char const *argv[])
{

    cpu_t cpu;
    tty_t tty;
    video_t video;
    drive_t drive;
    tps_drive_t tps;
    kb_t kb;
    mmu_t mmu;
    /* addenda creation */
    // Custom devices variable declaration on int main()

    // initialize tps

    tps.tps_a.filename = "devices/tps/tpsA.tps";
    tps.tps_b.filename = "devices/tps/tpsB.tps";
    tps.current_tps = &tps.tps_a;



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

    TaleaSystem_Init(&cpu, &video, &tty, &drive, &tps, &kb, &mmu);

    if (argc == 3 || argc == 4)
    {
        if (strcmp(argv[1], "load") == 0)
        {
            if (strcmp(argv[2], "bios") == 0) cpu.InstructionPointer = 0x200; // Set ip to entry point of bios or systems if it is loaded
            FILE *program = fopen(argv[argc - 1], "rb");
            fseek(program, 0, SEEK_END);
            long psize = ftell(program);
            fseek(program, 0, SEEK_SET);

            uint8_t *hex = malloc(psize + 1);
            fread(hex, psize, 1, program);
            fclose(program);

            // Raw programs start with a 3 byte header indicating the address to load in (BIG ENDIAN)

            uint32_t start_address = 0x00000000 | hex[0] << 16 | hex[1] << 8 | hex[2];

            for (int i = 0; i < psize - 3; i++)
            {
                Cpu_SetMemory8(&cpu, start_address + i, hex[i + 3]);
            }

            if (strcmp(argv[2], "dbg") == 0)
            {
                dbg = 1;
                printf("Dumping program loaded:\n");
                for (int i = 0; i < psize - 3; i++)
                {
                    if (i % 16 == 0) printf("\n%2x00: ", i/16);
                    printf("%02x ", cpu.Memory[start_address + i]);
                }
                printf("\n Program dumped. Press to continue...\n");
                getchar();
            }
            

            free(hex);
        }
    }

    TaleaSystem_Run(&cpu, &video, &tty, &drive, &tps, &kb);
    
    TaleaSystem_Destroy(&cpu, &video, &drive);

    return 0;
}
