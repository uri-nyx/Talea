/* INCLUDES */
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

/* DEFINES */
/**
 * @brief This constants are used in decoding the instruction
 * They give information mainly over the length and the kind of arguments that it takes
 * 
 */
#define ALPHA 1
#define BETA 2
#define GAMMA 3
#define RA 4
#define RL 5
#define RR 6
#define ZP 7
#define HL 8

/**
 * @brief This constants convey the bit position fo the flags in the status register
 * 
 */
#define N_FLAG 0X80 /// NEGATIVE FLAG
#define V_FLAG 0x40 /// OVERFLOW FLAG
#define Z_FLAG 0x02 /// ZERO FLAG
#define C_FLAG 0x01 /// CARRY FLAG

/* TYPEDEF */
/**
 * @brief These types are self explanatory
 * 
 */
typedef uint8_t byte_t;
typedef uint16_t word_t;

/**
 * @brief Struct instr represents a unique instruction to the machine.
 * + op holds the index of the opcode vector for the opcode.
 * + type holds the length of the instruction.
 * + reg, litt and addrH/L hold the different arguments an instrucion can take 
 */
struct instr
{
    int op;
    int type;
    byte_t reg;
    byte_t litt;
    byte_t addrH;
    byte_t addrL;
};

/**
 * @brief reg_table_t represents the register table of Talea
 * 
 */
typedef struct
{
    byte_t general[8];
    byte_t status;
    word_t pc;
    byte_t sp;
} reg_table_t;

/**
 * @brief Struct stack represents the hadware stack of Talea
 * 
 */

struct stack
{
    byte_t pointer;
    byte_t stack[UINT8_MAX];
};

/* HARDWARE */
/**
 * @brief Talea directs 64Kb of memory
 * 
 */

byte_t memory[UINT16_MAX];

/**
 * @brief The Stack
 * 
 */

struct stack st;
reg_table_t regs;

/**
 * @brief These are the names for the different general purpose registers
 * 
 */
enum
{
    acc = 0,
    bcc,
    r1,
    r2,
    r3,
    r4,
    hx,
    lx
};

/* HIGH NIBBLES*/
/**
 * @brief These are the high nibbles for each group of instuctions
 * 
 */
enum
{
    and_n = 0,
    xor_n,
    add_n,
    adc_n,
    subb_n,

    push_pop_n,
    shifts_n,

    ldr_n,

    str_n,
    sti_n,
    ldi_n,
    lea_n,

    branch_n,

    noargs_n,

    zerop_n,
    zldr_n,
    zstr_n
};

/* INSTRUCTIONS POINTERS IN VECTOR */
/**
 * @brief These pointers represent the location of each individiual instruction in the opcodes vector
 * 
 */
enum
{
    and_p = 0,
    xor_p,
    add_p,
    adc_p,
    subb_p,

    push_p,
    pop_p,
    shiftl_p,
    shiftr_p,

    ldr_p,

    str_p,
    sti_p,

    ldi_p,
    lea_p,

    bnz_p,
    bez_p,
    ben_p,
    call_p,

    jmp_p,
    ret_p,
    swap_p,
    psr_p,
    ssr_p,
    psp_p,
    ssp_p,
    not_p,
    sec_p,
    clc_p,
    nop_p,

    undefined_p,

    pointers_count
};

// INSTURCTIONS DECLARATIONS
/**
 * @brief And: acc & arg
 * 
 * @param ins 
 */
void and_o(struct instr ins);
/**
 * @brief Xor: acc ^= arg
 * 
 * @param ins 
 */
void xor_o(struct instr ins);
/**
 * @brief Add: acc += arg (set carry and overflow if necessary)
 * 
 * @param ins 
 */
void add_o(struct instr ins);
/**
 * @brief Add with carry: acc += arg + carry (set carry and overflow if neccesary)
 * 
 * @param ins 
 */
void adc_o(struct instr ins);
/**
 * @brief Sub with borrow (actually, with carry): acc += ~(arg) + carry (set carry and overflow if neccesary)
 * 
 * @param ins 
 */
void subb_o(struct instr ins);
// STACK
void push_o(struct instr ins);
void pop_o(struct instr ins);
// SHIFTS
void shiftl_o(struct instr ins);
void shiftr_o(struct instr ins);
// LOAD STORE
void ldr_o(struct instr ins);
void str_o(struct instr ins);
void sti_o(struct instr ins);
void ldi_o(struct instr ins);
void lea_o(struct instr ins);
// BRANCHES
void bnz_o(struct instr ins);
void bez_o(struct instr ins);
void ben_o(struct instr ins);
void call_o(struct instr ins);
// NOARGS
void jmp_o(struct instr ins);
void ret_o(struct instr ins);
void swap_o(struct instr ins);
void psr_o(struct instr ins);
void ssr_o(struct instr ins);
void psp_o(struct instr ins);
void ssp_o(struct instr ins);
void not_o(struct instr ins);
void sec_o(struct instr ins);
void clc_o(struct instr ins);
void nop_o(struct instr ins);
void undefined_o(struct instr ins);

// OPCODES FUNCTIONS VECTOR
/**
 * @brief Opcodes Vector
 * 
 */
void (*opcodes_v[pointers_count])(struct instr instruction) = {
    and_o,
    xor_o,
    add_o,
    adc_o,
    subb_o,

    push_o,
    pop_o,
    shiftl_o,
    shiftr_o,

    ldr_o,
    str_o,
    sti_o,

    ldi_o,
    lea_o,

    bnz_o,
    bez_o,
    ben_o,
    call_o,

    jmp_o,
    ret_o,
    swap_o,
    psr_o,
    ssr_o,
    psp_o,
    ssp_o,
    not_o,
    sec_o,
    clc_o,
    nop_o,

    undefined_o};

/**
 * @brief Calculates the size of the instruction (based on bit 3)
 * 
 * @param instruction the first byte of an instruction
 * @return int the size of the instruction (1, 2 or 3)
 */
int instruction_size(byte_t instruction);

/**
 * @brief Retrieves the instruction at wich the program counter points
 * 
 * @return byte_t the head of the instruction to execute 
 */
byte_t fetch();

/**
 * @brief Decodes the opcode and the arguments for the instruction to execute
 * 
 * @param instruction the head byte of the instruction
 * @return struct instr the instruction, laid out by its arguments and length
 */

struct instr decode(byte_t instruction);

/**
 * @brief Increases the pc by the length of the intruction to execute
 * 
 * @param size the length of the instruction
 */
void inc_pc(int size);

/**
 * @brief Executes the instruction
 * 
 * @param instruction the instruction to execute
 * @return int wether the instruction failed to execute or not
 */
int exec(struct instr instruction);

/**
 * @brief Set the status register
 * 
 * @param value the value wich the status register attends for it actualization, usually a register
 */
void set_status(byte_t value);

/**
 * @brief Performs a fetch-decode-execute cycle
 * 
 * @return int wheter the cicle was succesfully completed
 */
int cycle()
{

    struct instr ins = decode(fetch());

    if (ins.op == ldr_p)
    {
        if (ins.type == ZP) {
            inc_pc(2);
        } else {
            inc_pc(3);
        }
    } else if (ins.type == ZP) {
        inc_pc(2);
    } 
    else
    {
        inc_pc(ins.type);
    }

    return exec(ins);
}

// DEFINITION OF HARDWARE FETCH DECODE EXECUTE

byte_t fetch() { return memory[regs.pc]; }

struct instr decode(byte_t instruction)
{

    struct instr decoded;
    int args = instruction_size(instruction);

    byte_t h_nibble = instruction >> 4;
    switch (h_nibble)
    {
    case and_n:
    case xor_n:
    case add_n:
    case adc_n:
    case subb_n:

        decoded.op = h_nibble;
        switch (args)
        {
        case ALPHA:
            decoded.type = ALPHA;
            decoded.reg = instruction & 0x07;
            break;

        case BETA:
            decoded.type = BETA;
            decoded.litt = memory[regs.pc + 1];
            break;

        case GAMMA:
            decoded.type = GAMMA;
            decoded.addrL = memory[regs.pc + 1];
            decoded.addrH = memory[regs.pc + 2];
            break;
        }

        break;

    case push_pop_n:
        decoded.op = instruction & 0x8 ? pop_p : push_p;
        decoded.type = ALPHA;
        decoded.reg = instruction & 0x7;
        break;

    case shifts_n:
        decoded.op = instruction & 0x8 ? shiftr_p : shiftl_p;
        decoded.type = ALPHA;
        decoded.reg = instruction & 0x7;
        break;

    case ldr_n:
        decoded.op = ldr_p;
        switch (args)
        {
            //TYPES ARE TEMPORAL, THIS INSTRUCTION ALWAYS SPANS · BYTES
        case ALPHA:
            //0x70
            decoded.type = RA;
            decoded.reg = instruction & 0x07;
            decoded.addrL = memory[regs.pc + 1];
            decoded.addrH = memory[regs.pc + 2];
            break;

        case BETA:
            //0x78
            decoded.type = RL;
            decoded.litt = memory[regs.pc + 1];
            decoded.reg = memory[regs.pc + 2] & 0x7;
            break;

        case GAMMA:
            //0x7c
            decoded.type = RR;
            decoded.litt = memory[regs.pc + 1] & 0x7;
            decoded.reg = memory[regs.pc + 2] & 0x7;
            break;
        }
        break;
    case str_n:
    case sti_n:
    case ldi_n:
        switch (h_nibble)
        {
        case str_n:
            decoded.op = str_p;
            break;

        case sti_n:
            decoded.op = sti_p;
            break;
        case ldi_n:
            decoded.op = ldi_p;
            break;
        }

        decoded.reg = instruction & 0x7;
        decoded.type = (instruction & 0x8) ? GAMMA : ALPHA;
        decoded.addrL = memory[regs.pc + 1];
        decoded.addrH = memory[regs.pc + 2];
        break;

    case lea_n:
        if ((instruction & 0x8) == 0)
        {
            decoded.type = GAMMA;
            decoded.addrL = memory[regs.pc + 1];
            decoded.addrH = memory[regs.pc + 2];
        } else {
            // LDI ZP
            decoded.op = ldi_p;
            decoded.type = ZP;
            decoded.reg = instruction & 0x7;
            decoded.addrL = memory[regs.pc + 1];
        }
        decoded.op = lea_p;
        break;
        
    case branch_n:
        switch (instruction & 0x3)
        {
        case 0x0:
            decoded.op = bnz_p;
            break;

        case 0x1:
            decoded.op = bez_p;
            break;
        case 0x2:
            decoded.op = ben_p;
            break;
        case 0x3:
            decoded.op = call_p;
            break;
        }

        decoded.type = instruction & 0x8 ? GAMMA : ALPHA;
        decoded.addrL = decoded.type == GAMMA ? memory[regs.pc + 1] : 0;
        decoded.addrH = decoded.type == GAMMA ? memory[regs.pc + 2] : 0;
        break;

    case noargs_n:
        switch (instruction & 0xf)
        {
        case 0x0:
            decoded.op = jmp_p;
            break;

        case 0x1:
            decoded.op = ret_p;
            break;
        case 0x2:
            decoded.op = swap_p;
            break;
        case 0x3:
            decoded.op = psr_p;
            break;
        case 0x4:
            decoded.op = ssr_p;
            break;
        case 0x5:
            decoded.op = psp_p;
            break;
        case 0x6:
            decoded.op = ssp_p;
            break;
        case 0x7:
            decoded.op = not_p;
            break;
        case 0x8:
            decoded.op = sec_p;
            break;
        case 0x9:
            decoded.op = clc_p;
            break;
        case 0xa:
            decoded.op = nop_p;
            break;
        default:
            decoded.op = undefined_p;
            break;
        }

        decoded.type = ALPHA;
        break;
    
    case zerop_n:
        switch (instruction & 0xf)
        {
        case 0x0:
            decoded.op = and_p;
            break;
        case 0x1:
            decoded.op = xor_p;
            break;
        case 0x2:
            decoded.op = add_p;
            break;
        case 0x3:
            decoded.op = adc_p;
            break;
        case 0x4:
            decoded.op = xor_p;
            break;
        case 0x5:
            decoded.op = bnz_p;
            break;
        case 0x6:
            decoded.op = bez_p;
            break;
        case 0x7:
            decoded.op = ben_p;
            break;
        case 0x8:
            decoded.op = call_p;
            break;
        case 0x9:
            decoded.op = lea_p;
            break;
        }

        decoded.type = ZP;
        decoded.addrL = memory[regs.pc + 1];
        decoded.addrH = 0;
        break;
    
    case zldr_n:
        decoded.op = ldr_p;
        if ((instruction & 0x8) == 0)
        {
            decoded.type = ZP;
            decoded.reg = instruction & 0x7;
            decoded.addrL = memory[regs.pc + 1];
            decoded.addrH = 0;   
        else {
            decoded.type = HL;
            decoded.reg = instruction & 0x7;
            decoded.addrL = regs.general[lx];
            decoded.addrH = regs.general[hx];
        }
        break;

    case zstr_n:
        
        if ((instruction & 0x8) == 0)
        {
            decoded.op = str_p;
        } else {
            decoded.op = sti_p;
        }
            decoded.type = ZP;
            decoded.reg = instruction & 0x7;
            decoded.addrL = memory[regs.pc + 1];
            decoded.addrH = 0;

        break;

    default:
        decoded.op = undefined_p;
        break;
    }

    return decoded;
}

void inc_pc(int size) { regs.pc += size; }

int exec(struct instr instruction)
{

    if (instruction.op == undefined_p)
    {
        return EBADMSG; // RETURN ERROR IF UNDEFINED
    }
    opcodes_v[instruction.op](instruction);

    return 0;
}

void set_status(byte_t value)
{
    /// STATUS N V S S S S Z C (C V MUST BE SET BY THE ISNTRUCTION!!!)
    regs.status = (value == 0) ? regs.status | Z_FLAG : regs.status & ~(Z_FLAG);
    regs.status = (value & 0x80) ? regs.status | N_FLAG : regs.status & ~(N_FLAG);
}

int instruction_size(byte_t instruction)
{
    byte_t size_flag = (instruction & 0x0c) >> 2;
    int i_size = size_flag >> 1 ? (size_flag & 1 ? GAMMA : BETA) : ALPHA;
    return i_size;
}

/**
 * @brief Pushes a byte to a stack
 * 
 * @param s the stack to push in
 * @param val the byte wich is pused
 */
void push(struct stack *s, byte_t val)
{
    s->stack[s->pointer] = val;
    s->pointer++;
}

/**
 * @brief Pops a byte from a stack
 * 
 * @param s the stack to pop from
 * @return byte_t the byte that is popped
 */
byte_t pop(struct stack *s)
{
    s->pointer--;
    byte_t val = s->stack[s->pointer];
    return val;
}

/*
*   Instruction Definition *
*/

// LOGIC ARITHMETIC

void and_o(struct instr ins)
{
    byte_t arg;
    switch (ins.type)
    {
    case ALPHA:
        arg = regs.general[ins.reg];
        break;
    case BETA:
        arg = ins.litt;
        break;
    case GAMMA:
        arg = memory[(ins.addrH << 8) | ins.addrL];
        break;
    case ZP:
        arg = memory[ins.addrL];
        break;
    default:
        break;
    }
    regs.general[acc] &= arg;

    set_status(regs.general[acc]);
}

void xor_o(struct instr ins)
{
    byte_t arg;
    switch (ins.type)
    {
    case ALPHA:
        arg = regs.general[ins.reg];
        break;
    case BETA:
        arg = ins.litt;
        break;
    case GAMMA:
        arg = memory[((word_t)ins.addrH << 8) | ins.addrL];
        break;
    case ZP:
        arg = memory[ins.addrL];
        break;
    default:
        break;
    }
    regs.general[acc] ^= arg;

    set_status(regs.general[acc]);
}

void add_o(struct instr ins)
{
    byte_t arg;

    switch (ins.type)
    {
    case ALPHA:
        arg = regs.general[ins.reg];
        break;

    case BETA:
        arg = ins.litt;
        break;

    case GAMMA:
        arg = memory[((word_t)ins.addrH << 8) | ins.addrL];
        break;
    case ZP:
        arg = memory[ins.addrL];
        break;
    }

    word_t result = regs.general[acc] + arg;
    byte_t carry = result > 255 ? 1 : 0;
    byte_t overflow = 0;

    if (carry)
    {
        regs.status |= C_FLAG;
    }
    else
    {
        regs.status &= ~(C_FLAG);
    }

    if ((int)result < (-128) || (int)result > 127)
    {
        regs.status |= V_FLAG;
    }
    else
    {
        regs.status &= ~(V_FLAG);
    }

    regs.general[acc] = result;
    set_status(regs.general[acc]);
}

void adc_o(struct instr ins)
{
    byte_t arg;

    switch (ins.type)
    {
    case ALPHA:
        arg = regs.general[ins.reg];
        break;

    case BETA:
        arg = ins.litt;
        break;

    case GAMMA:
        arg = memory[((word_t)ins.addrH << 8) | ins.addrL];
        break;
    case ZP:
        arg = memory[ins.addrL];
        break;
    }

    word_t result = regs.general[acc] + arg + (regs.status & 0x1);
    byte_t carry = result > 255 ? 1 : 0;
    if (carry)
    {
        regs.status |= C_FLAG;
    }
    else
    {
        regs.status &= ~(C_FLAG);
    }

    if ((int)result < (-128) || (int)result > 127)
    {
        regs.status |= V_FLAG;
    }
    else
    {
        regs.status &= ~(V_FLAG);
    }

    regs.general[acc] = result;
    set_status(regs.general[acc]);
}

void subb_o(struct instr ins)
{
    byte_t arg;

    switch (ins.type)
    {
    case ALPHA:
        arg = regs.general[ins.reg];
        break;

    case BETA:
        arg = ins.litt;
        break;

    case GAMMA:
        arg = memory[((word_t)ins.addrH << 8) | ins.addrL];
        break;
    case ZP:
        arg = memory[ins.addrL];
        break;
    }

    word_t result = regs.general[acc] + ~(arg) + (regs.status & 0x1);
    byte_t carry = regs.general[acc] < arg ? 0 : 1;

    if (carry)
    {
        regs.status |= C_FLAG;
    }
    else
    {
        regs.status &= ~(C_FLAG);
    }

    if ((int)result < (-128) || (int)result > 127)
    {
        regs.status |= V_FLAG;
    }
    else
    {
        regs.status &= ~(V_FLAG);
    }

    regs.general[acc] = result;

    set_status(regs.general[acc]);
}

// STACK

void push_o(struct instr ins)
{
    push(&st, regs.general[ins.reg]);
}

void pop_o(struct instr ins)
{
    regs.general[ins.reg] = pop(&st);
    set_status(regs.general[ins.reg]);
}

// SHIFTS

void shiftl_o(struct instr ins)
{
    regs.general[ins.reg] = regs.general[ins.reg] << 1;
    set_status(regs.general[ins.reg]);
}

void shiftr_o(struct instr ins)
{
    regs.general[ins.reg] = regs.general[ins.reg] >> 1;
    set_status(regs.general[ins.reg]);
}

// LOAD STORE

void ldr_o(struct instr ins)
{
    switch (ins.type)
    {
    case RR:
        regs.general[ins.reg] = regs.general[ins.litt];
        break;

    case RL:
        regs.general[ins.reg] = ins.litt;
        break;
    case RA:
        regs.general[ins.reg] = memory[((word_t)ins.addrH << 8) | ins.addrL];
        break;
    case ZP:
        regs.general[ins.reg] = memory[ins.addrL];
        break;
    }

    set_status(regs.general[ins.reg]);
}

void str_o(struct instr ins)
{
    switch (ins.type)
    {
    case ALPHA:
        memory[(((word_t)regs.general[hx] << 8) | regs.general[lx])] = regs.general[ins.reg];
        break;

    case GAMMA:
        memory[((word_t)ins.addrH << 8) | ins.addrL] = regs.general[ins.reg];
        break;
    case ZP:
        memory[ins.addrL] = regs.general[ins.reg];
        break;
    }
}

void sti_o(struct instr ins)
{
    word_t addr;
    switch (ins.type)
    {
    case ALPHA:
        addr = ((word_t)regs.general[hx] << 8) | regs.general[lx];

        break;

    case GAMMA:
        addr = ((word_t)ins.addrH << 8) | ins.addrL;
        break;
    case ZP:
        addr = ins.addrL;
        break;
    }
    memory[((word_t)memory[addr + 1] << 8) | memory[addr]] = regs.general[ins.reg];
}

void ldi_o(struct instr ins)
{
    switch (ins.type)
    {
    case ALPHA:
        regs.general[ins.reg] = memory[memory[((word_t)regs.general[hx] << 8) | regs.general[lx]]];
        break;

    case GAMMA:
        regs.general[ins.reg] = memory[memory[((word_t)ins.addrH << 8) | ins.addrL]];
        break;
    }
    set_status(regs.general[ins.reg]);
}

void lea_o(struct instr ins)
{
    switch (ins.type)
    {
    case GAMMA:
        regs.general[hx] = ins.addrH;
        regs.general[lx] = ins.addrL;
        break;
    
    case ZP:
        regs.general[lx] = ins.addrL;
        regs.general[hx] = 0;
        break;
    }
    // WETHER set_status MUST BE INVOKED, I DO NOT KNOW
}

// BRANCHES

void bnz_o(struct instr ins)
{
    if (!(regs.status & Z_FLAG))
    { //Z FLAG
        switch (ins.type)
        {
        case ALPHA:
            regs.pc = ((word_t)regs.general[hx] << 8) | regs.general[lx];
            break;

        case GAMMA:
            regs.pc = ((word_t)ins.addrH << 8) | ins.addrL;
            break;
        case ZP:
            regs.pc = ins.addrL;
            break;
        }
    }
}

void bez_o(struct instr ins)
{
    if (regs.status & Z_FLAG)
    { //Z FLAG
        switch (ins.type)
        {
        case ALPHA:
            regs.pc = ((word_t)regs.general[hx] << 8) | regs.general[lx];
            break;

        case GAMMA:
            regs.pc = ((word_t)ins.addrH << 8) | ins.addrL;
            break;
        case ZP:
            regs.pc = ins.addrL;
            break;
        }
    }
}

void ben_o(struct instr ins)
{
    if (regs.status & N_FLAG)
    { //N FLAG
        switch (ins.type)
        {
        case ALPHA:
            regs.pc = ((word_t)regs.general[hx] << 8) | regs.general[lx];
            break;

        case GAMMA:
            regs.pc = ((word_t)ins.addrH << 8) | ins.addrL;
            break;
        case ZP:
            regs.pc = ins.addrL;
            break;
        }
    }
}

void call_o(struct instr ins)
{
    byte_t pcL = regs.pc & 0xf;
    byte_t pcH = regs.pc >> 8;

    push(&st, pcH);
    push(&st, pcL);

    switch (ins.type)
    {
    case ALPHA:
        regs.pc = ((word_t)regs.general[hx] << 8) | regs.general[lx];
        break;

    case GAMMA:
        regs.pc = ((word_t)ins.addrH << 8) | ins.addrL;
        break;
    case ZP:
            regs.pc = ins.addrL;
            break;
    }
}

// NOARGS

void jmp_o(struct instr ins)
{
    byte_t pcL = regs.pc & 0xf;
    byte_t pcH = regs.pc >> 8;

    push(&st, pcH);
    push(&st, pcL);

    regs.pc = memory[((word_t)regs.general[hx] << 8) | regs.general[lx]];
}

void ret_o(struct instr ins)
{
    byte_t pcL = pop(&st);
    byte_t pcH = pop(&st);

    regs.pc = (pcH << 8) | pcL;
}

void swap_o(struct instr ins)
{
    byte_t temp;
    temp = regs.general[acc];

    regs.general[acc] = regs.general[bcc];
    regs.general[bcc] = temp;

    set_status(regs.general[acc]);
}

void psr_o(struct instr ins)
{
    push(&st, regs.status);
}

void ssr_o(struct instr ins)
{
    regs.status = pop(&st);
}

void psp_o(struct instr ins)
{
    push(&st, st.pointer);
}

void ssp_o(struct instr ins)
{
    st.pointer = pop(&st);
}

void not_o(struct instr ins)
{
    byte_t val = ~(regs.general[acc]);
    regs.general[acc] = val;
    set_status(regs.general[acc]);
}

void sec_o(struct instr ins)
{
    regs.status |= C_FLAG;
}

void clc_o(struct instr ins)
{
    regs.status &= ~(C_FLAG);
}

void nop_o(struct instr ins)
{
}

void undefined_o(struct instr ins)
{
}