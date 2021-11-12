/* Headers providing the architecture for Talea VM */

/* INCLUDES */
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

/* DEFINES */
#define ALPHA 1 
#define BETA 2
#define GAMMA 3
#define RA 4
#define RL 5
#define RR 6

/* TYPEDEF */
typedef uint8_t byte_t;
typedef uint16_t word_t;

struct instr
{
    int op;
    int type;
    byte_t reg;
    byte_t litt;
    byte_t addrH;
    byte_t addrL; 

};

typedef struct
{
    byte_t general[8];
    byte_t status;
    word_t pc;
    byte_t sp;
} reg_table_t;

struct stack {
    byte_t pointer;
    byte_t stack[UINT8_MAX];
};

/* HARDWARE */
byte_t memory[UINT16_MAX];
struct stack st;
reg_table_t regs;

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

    noargs_n
};

/* INSTRUCTIONS POINTERS IN VECTOR */
enum {
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
void and_o (struct instr ins);
void xor_o (struct instr ins);
void add_o (struct instr ins) ;
void adc_o (struct instr ins) ;
void subb_o (struct instr ins) ;
// STACK
void push_o (struct instr ins) ;
void pop_o (struct instr ins) ;
// SHIFTS
void shiftl_o (struct instr ins) ;
void shiftr_o (struct instr ins) ;
// LOAD STORE
void ldr_o (struct instr ins) ;
void str_o (struct instr ins) ;
void sti_o (struct instr ins) ;
void ldi_o (struct instr ins) ;
void lea_o (struct instr ins) ;
// BRANCHES
void bnz_o (struct instr ins) ;
void bez_o (struct instr ins) ;
void ben_o (struct instr ins) ;
void call_o (struct instr ins) ;
// NOARGS
void jmp_o (struct instr ins) ;
void ret_o (struct instr ins) ;
void swap_o (struct instr ins) ;
void psr_o (struct instr ins) ;
void ssr_o (struct instr ins) ;
void psp_o (struct instr ins) ;
void ssp_o (struct instr ins) ;
void not_o (struct instr ins) ;
void sec_o (struct instr ins) ;
void clc_o (struct instr ins) ;
void nop_o (struct instr ins) ;
void undefined_o (struct instr ins) ;

// OPCODES FUNCTIONS VECTOR
void (*opcodes_v[pointers_count]) (struct instr instruction) = {
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

    undefined_o    
};

int instruction_size(byte_t instruction);

byte_t fetch();
struct instr decode(byte_t instruction);
void inc_pc(int size);
int exec(struct instr instruction);
void set_status(byte_t value);

int cycle() {

    struct instr ins = decode(fetch());

    if (ins.op == ldr_p) {
        inc_pc(3);
    } else {
        inc_pc(ins.type);
    }

    return exec(ins);
}

// DEFINITION OF HARDWARE FETCH DECODE EXECUTE

byte_t fetch() { return memory[regs.pc]; }


struct instr decode(byte_t instruction) {

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
            decoded.reg = memory[regs.pc +2] & 0x7;
            break;

        case GAMMA:
            //0x7c
            decoded.type = RR;
            decoded.litt = memory[regs.pc + 1] & 0x7;
            decoded.reg = memory[regs.pc +2] & 0x7;
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
        decoded.addrL =  memory[regs.pc + 1];
        decoded.addrH = memory[regs.pc + 2];
        break;
    
    case lea_n:
        decoded.op = lea_p;
        decoded.type = GAMMA;
        decoded.addrL = memory[regs.pc + 1];
        decoded.addrH = memory[regs.pc + 2];
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
        decoded.addrL = decoded.type == GAMMA ? memory[regs.pc + 1] : 0 ;
        decoded.addrH = decoded.type == GAMMA ? memory[regs.pc + 2] : 0 ;
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

    default:
        decoded.op = undefined_p;
        break;
    }

    return decoded;

}

void inc_pc(int size) { regs.pc += size; }

int exec(struct instr instruction) {

    if (instruction.op == undefined_p)
    {
        return EBADMSG; // RETURN ERROR IF UNDEFINED
    }
    opcodes_v[instruction.op](instruction);

    return 0;
}

void set_status(byte_t value) {
    // STATUS S S S N V D Z C (C D V MUST BE SET BY THE ISNTRUCTION!!!)
    regs.status = (value == 0) ? regs.status | 0x2 : regs.status & ~(0x2);
    regs.status = (value & 0x80) ? regs.status | 0x10 : regs.status & ~(0x10) ; 
}

int instruction_size(byte_t instruction) {
    byte_t size_flag = (instruction & 0x0c) >> 2;
    int i_size = size_flag >> 1 ? (size_flag & 1 ? GAMMA : BETA) : ALPHA;
    return i_size;
}

void push(struct stack * s, byte_t val) {
    s->stack[s->pointer] = val;
    s->pointer ++;
}

byte_t pop(struct stack * s) {
    s->pointer--;
    byte_t val = s->stack[s->pointer];
    return val;
}

/*
*   Instruction Definition *
*/

// LOGIC ARITHMETIC

void and_o (struct instr ins) {
    regs.general[acc] &= (ins.type == ALPHA) ? regs.general[ins.reg] :
                (ins.type == BETA) ? ins.litt : 
                /* GAMMA */ memory[(ins.addrH << 8) | ins.addrL];
    
    set_status(regs.general[acc]);
}

void xor_o (struct instr ins) {
    regs.general[acc] ^= (ins.type == ALPHA) ? regs.general[ins.reg] :
                (ins.type == BETA) ? ins.litt : 
                /* GAMMA */ memory[(ins.addrH << 8) | ins.addrL];
    
    set_status(regs.general[acc]);
}

void add_o (struct instr ins) {
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
        arg = memory[((word_t)ins.addrH << 8) | ins.addrL ];
        break;
    }

    word_t result = regs.general[acc] + arg;
    byte_t carry = result > 255 ? 1 : 0;
    byte_t overflow = 0;


    if (carry) {
        regs.status |= 0x1;
        
    } else {
        regs.status &= ~(0x1);
    }

    if ((int)result < (-128) || (int)result > 127)
    {
        regs.status |= 0x8;
    } else {
        regs.status &= ~(0x8);
    }
    

    regs.general[acc] = result;
    set_status(regs.general[acc]);
}

void adc_o (struct instr ins) {
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
        arg = memory[((word_t)ins.addrH << 8) | ins.addrL ];
        break;
    }


    word_t result = regs.general[acc] + arg + (regs.status & 0x1);
    byte_t carry = result > 255 ? 1 : 0;
    if (carry) {
        regs.status |= 0x1;
        
    } else {
        regs.status &= ~(0x1);
    }

    if ((int)result < (-128) || (int)result > 127)
    {
        regs.status |= 0x8;
    } else {
        regs.status &= ~(0x8);
    }

    regs.general[acc] = result;
    set_status(regs.general[acc]);
}

void subb_o (struct instr ins) {
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
        arg = memory[((word_t)ins.addrH << 8) | ins.addrL ];
        break;
    }

    word_t result = regs.general[acc] + ~(arg) + (regs.status & 0x1);
    byte_t carry = regs.general[acc] < arg ? 0 : 1;

    if (carry) {
        regs.status |= 0x1;
        
    } else {
        regs.status &= ~(0x1);
    }

    if ((int)result < (-128) || (int)result > 127)
    {
        regs.status |= 0x8;
    } else {
        regs.status &= ~(0x8);
    }

    regs.general[acc] = result;

    set_status(regs.general[acc]);
}

// STACK

void push_o (struct instr ins) {
    push(&st, regs.general[ins.reg]);
}

void pop_o (struct instr ins) {
    regs.general[ins.reg] = pop(&st);
    set_status(regs.general[ins.reg]);
}

// SHIFTS

void shiftl_o (struct instr ins) {
    regs.general[ins.reg] = regs.general[ins.reg] << 1;
    set_status(regs.general[ins.reg]);
}

void shiftr_o (struct instr ins) {
    regs.general[ins.reg] = regs.general[ins.reg] >> 1;
    set_status(regs.general[ins.reg]);
}

// LOAD STORE

void ldr_o (struct instr ins) {
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
    }
    
    set_status(regs.general[ins.reg]);
}

void str_o (struct instr ins) {
    switch (ins.type)
    {
    case ALPHA:
        memory[(((word_t)regs.general[hx] << 8) | regs.general[lx])] = regs.general[ins.reg];
        break;
    
    case GAMMA:
        memory[((word_t)ins.addrH << 8) | ins.addrL] = regs.general[ins.reg];
        break;
    }
}

void sti_o (struct instr ins) {
    word_t addr;
    switch (ins.type)
    {
    case ALPHA:
        addr = ((word_t)regs.general[hx] << 8) | regs.general[lx];
        
        break;
    
    case GAMMA:
        addr = ((word_t)ins.addrH << 8) | ins.addrL;
        break;
    }
    memory[((word_t)memory[addr + 1] << 8) | memory[addr]] = regs.general[ins.reg];
}

void ldi_o (struct instr ins) {
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

void lea_o (struct instr ins) {
    regs.general[hx] = ins.addrH;
    regs.general[lx] = ins.addrL;
    // WETHER set_status MUST BE INVOKED, I DO NOT KNOW
}

// BRANCHES

void bnz_o (struct instr ins) {
    if (!(regs.status & 0x2)){ //Z FLAG
        switch (ins.type)
        {
        case ALPHA:
            regs.pc = ((word_t)regs.general[hx] << 8) | regs.general[lx];
            break;
        
        case GAMMA:
            regs.pc = ((word_t)ins.addrH << 8) | ins.addrL;
            break;
        }
    }
}

void bez_o (struct instr ins) {
        if (regs.status & 2){ //Z FLAG
        switch (ins.type)
        {
        case ALPHA:
            regs.pc = ((word_t)regs.general[hx] << 8) | regs.general[lx];
            break;
        
        case GAMMA:
            regs.pc = ((word_t)ins.addrH << 8) | ins.addrL;
            break;
        }
    }
}

void ben_o (struct instr ins) {
        if (regs.status & 0x10){ //N FLAG
        switch (ins.type)
        {
        case ALPHA:
            regs.pc = ((word_t)regs.general[hx] << 8) | regs.general[lx];
            break;
        
        case GAMMA:
            regs.pc = ((word_t)ins.addrH << 8) | ins.addrL;
            break;
        }
    }
}

void call_o (struct instr ins) {
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
        }
}

// NOARGS

void jmp_o (struct instr ins) {
    byte_t pcL = regs.pc & 0xf;
    byte_t pcH = regs.pc >> 8;

    push(&st, pcH);
    push(&st, pcL);

    regs.pc = memory[((word_t)regs.general[hx] << 8) | regs.general[lx]];
}    

void ret_o (struct instr ins) {
    byte_t pcL = pop(&st);
    byte_t pcH = pop(&st);

    regs.pc = (pcH << 8) | pcL;
}

void swap_o (struct instr ins) {
    byte_t temp;
    temp = regs.general[acc];

    regs.general[acc] = regs.general[bcc];
    regs.general[bcc] = temp;

    set_status(regs.general[acc]);
}

void psr_o (struct instr ins) {
    push(&st, regs.status);
}

void ssr_o (struct instr ins) {
    regs.status = pop(&st);
}

void psp_o (struct instr ins) {
    push(&st, st.pointer);
}

void ssp_o (struct instr ins) {
    st.pointer = pop(&st);
}

void not_o (struct instr ins) {
    byte_t val = ~(regs.general[acc]);
    regs.general[acc] = val;
    set_status(regs.general[acc]);
}

void sec_o (struct instr ins) {
    regs.status |= 0x1;
}

void clc_o (struct instr ins) {
    regs.status &= ~(0x1);
}

void nop_o (struct instr ins) {

}

void undefined_o (struct instr ins) {

}