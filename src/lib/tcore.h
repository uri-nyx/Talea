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
typedef struct reg_table
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

/**
 * @brief Struct TCore represents the processor
 * 
 */

struct Tcore
{
    byte_t memory[UINT16_MAX];
    struct stack st;
    reg_table_t regs;
    
};


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