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

#define get_imm_b(instruction) ((instruction & 0x80000000) >> 19) | ((instruction & 0x00000080) << 4) | ((instruction & 0x7F000000) >> 20) | ((instruction & 0x0000F00) >> 7)
#define make_imm_b(instruction) (instruction & 0x80000000) ? 0xe000 | get_imm_b(instruction) : get_imm_b(instruction)
	
#define get_imm_j(instruction) (instruction & 0x80000000) >> 11 | (instruction & 0x000FF000) | ((instruction & 0x00100000) >> 9) | ((instruction & 0x7FE00000) >> 20)
#define make_imm_j(instruction) (instruction & 0x80000000) ? 0xfff00000 | get_imm_j(instruction) : get_imm_j(instruction)
	
#define get_imm_u(instruction) (unsigned)(instruction & 0xFFFF0000) >> 4

#define get_imm_s(instruction) (instruction & 0xFF000000) >> 20 | ((instruction & 0x00000F80) >> 7)
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
#define Ldi 0b001 // Same as lh, the offset is 0

// Loads from Cache
#define Lbc 0b101
#define Lhc 0b010

// Stores to RAM
#define Sb 0b000
#define Sh 0b001
#define Sti 0b001 // Same as Sh, but the offset is 0

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

