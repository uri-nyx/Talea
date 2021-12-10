#include "tcore.h"

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

byte_t fetch(struct Tcore tcore ) { return tcore.memory[tcore.regs.pc]; }

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
            decoded.op = lea_p;
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
        case 0xb:
            decoded.op = str_p;
            decoded.type = ZP;
            decoded.reg = lx;
            decoded.addrL = memory[regs.pc + 1];
            decoded.addrH = 0;
            break;
        case 0xc:
        decoded.op = str_p;
            decoded.type = ZP;
            decoded.reg = hx;
            decoded.addrL = memory[regs.pc + 1];
            decoded.addrH = 0;
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
            decoded.op = subb_p;
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
        case 0xa:
            decoded.op = str_p;
            decoded.type = ZP;
            decoded.reg = acc;
            decoded.addrL = memory[regs.pc + 1];
            decoded.addrH = 0;
            break;
        case 0xb:
            decoded.op = str_p;
            decoded.type = ZP;
            decoded.reg = bcc;
            decoded.addrL = memory[regs.pc + 1];
            decoded.addrH = 0;
            break;
        case 0xc:
        decoded.op = str_p;
            decoded.type = ZP;
            decoded.reg = r1;
            decoded.addrL = memory[regs.pc + 1];
            decoded.addrH = 0;
            break;
        case 0xd:
        decoded.op = str_p;
            decoded.type = ZP;
            decoded.reg = r2;
            decoded.addrL = memory[regs.pc + 1];
            decoded.addrH = 0;
            break;
        case 0xe:
        decoded.op = str_p;
            decoded.type = ZP;
            decoded.reg = r3;
            decoded.addrL = memory[regs.pc + 1];
            decoded.addrH = 0;
            break;
        case 0xf:
            decoded.op = str_p;
            decoded.type = ZP;
            decoded.reg = r4;
            decoded.addrL = memory[regs.pc + 1];
            decoded.addrH = 0;
            break;

        default:
            decoded.op = undefined_p;
            break;
        }

        decoded.type = ZP;
        decoded.addrL = memory[regs.pc + 1];
        decoded.addrH = 0;
        break;
    
    case zldr_n:
        if ((instruction & 0x8) == 0)
        {
            decoded.op = ldr_p;
            decoded.type = ZP;
            decoded.reg = instruction & 0x7;
            decoded.addrL = memory[regs.pc + 1];
            decoded.addrH = 0;
        } 
        else {
            decoded.op = sti_p;
            decoded.type = ZP;
            decoded.reg = instruction & 0x7;
            decoded.addrL = memory[regs.pc + 1];
            decoded.addrH = 0;
        }
        break;

    default:
        decoded.op = undefined_p;
        break;
    }

    return decoded;
}

void inc_pc(struct reg_table tcore, int size) { tcore.pc += size; }

int exec(struct instr instruction)
{

    if (instruction.op == undefined_p)
    {
        return EBADMSG; // RETURN ERROR IF UNDEFINED
    }
    opcodes_v[instruction.op](instruction);

    return 0;
}

void set_status(struct reg_table tcore, byte_t value)
{
    /// STATUS N V S S S S Z C (C V MUST BE SET BY THE ISNTRUCTION!!!)
    tcore.status = (value == 0) ? tcore.status | Z_FLAG : tcore.status & ~(Z_FLAG);
    tcore.status = (value & 0x80) ? tcore.status | N_FLAG : tcore.status & ~(N_FLAG);
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
        regs.general[ins.reg] = memory[((word_t)regs.general[hx] << 8) | regs.general[lx]];
        break;

    case GAMMA:
        regs.general[ins.reg] = memory[memory[((word_t)ins.addrH << 8) | ins.addrL]];
        break;
    case ZP:
        regs.general[ins.reg] = memory[memory[ins.addrL]];

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