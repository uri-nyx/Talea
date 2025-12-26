#ifndef CPU_H
#define CPU_H

typedef u32 Register;
enum TaleaRegisters {
    x0,
    x1,
    x2,
    x3,
    x4,
    x5,
    x6,
    x7,
    x8,
    x9,
    x10,
    x11,
    x12,
    x13,
    x14,
    x15,
    x16,
    x17,
    x18,
    x19,
    x20,
    x21,
    x22,
    x23,
    x24,
    x25,
    x26,
    x27,
    x28,
    x29,
    x30,
    x31,
    REGISTER_COUNT
};

typedef u32 TaleaStatusRegister;

// GETTERS
#define SR_GET_SUPERVISOR(tsr) ((tsr) & 0x80000000)
#define SR_GET_INTERRUPT(tsr)  ((tsr) & 0x40000000)
#define SR_GET_MMU(tsr)        ((tsr) & 0x20000000)
#define SR_GET_PRIORITY(tsr)   ((((tsr) & 0x1C000000) >> 26) & 0x7)
#define SR_GET_IVT(tsr)        ((((tsr) & 0x03F00000) >> 20) & 0x3f)
#define SR_GET_PDT(tsr)        ((((tsr) & 0x000FF000) >> 12) & 0xff)
#define SR_GET_RESERVED(tsr)   ((tsr) & 0x000000FF)

// SETTERS

#define SR_CLEAR_SUPERVISOR(tsr) ((tsr) &= ~((1) << (31)))
#define SR_CLEAR_INTERRUPT(tsr)  ((tsr) &= ~((1) << (30)))

#define SR_SET_SUPERVISOR(tsr, x) ((tsr) |= ((x) << 31))
#define SR_SET_INTERRUPT(tsr, x)  ((tsr) |= ((x) << 30))
#define SR_SET_MMU(tsr, x)        ((tsr) |= ((x) << 29))
#define SR_SET_PRIORITY(tsr, x)   ((tsr) |= ((x) << 26))
#define SR_SET_IVT(tsr, x)        ((tsr) |= ((x) << 20))
#define SR_SET_PDT(tsr, x)        ((tsr) |= ((x) << 12))

enum InstructionFields {
    GROUP_SHIFHT = 29,
    OPCODE_MASK  = 0x1E000000,
    OPCODE_SHIFT = 25,
    R1_MASK      = 0x01F00000,
    R1_SHIFT     = 20,
    R2_MASK      = 0x000F8000,
    R2_SHIFT     = 15,
    R3_MASK      = 0x00007C00,
    R3_SHIFT     = 10,
    R4_MASK      = 0x000003E0,
    R4_SHIFT     = 5,
    VECTOR_MASK  = 0x000000FF,
    IMM15_MASK   = 0x00007FFF,
    IMM20_MASK   = 0x000FFFFF,
};

enum InstructionGroup {
    SYS    = 0b000,
    MEM    = 0b001,
    JU     = 0b010,
    BRANCH = 0b011,
    LOAD   = 0b100,
    ALUI   = 0b101,
    ALUR   = 0b110,
    STORE  = 0b111,
};

enum InstructionCycles {
    CYCLES_SYS       = 20,
    CYCLES_MEM       = 5,
    CYCLES_JU        = 5,
    CYCLES_BRANCH    = 5,
    CYCLES_LOAD      = 5,
    CYCLES_ALUI      = 1,
    CYCLES_ALUR      = 1,
    CYCLES_STORE     = 5,
    CYCLES_EXCEPTION = 20,
};

enum GroupSYS {
    CODE_Syscall     = 0x2,
    CODE_GsReg       = 0x3,
    CODE_SsReg       = 0x4,
    CODE_Sysret      = 0x6,
    CODE_Trace       = 0x5,
    CODE_MmuToggle   = 0x7,
    CODE_MmuMap      = 0x8,
    CODE_MmuUnmap    = 0x9,
    CODE_MmuStat     = 0xa,
    CODE_MmuSetPT    = 0xb,
    CODE_MmuUpdate   = 0xc,
    CODE_UmodeToggle = 0xd,
    CODE_MmuSwitch   = 0xe,
    CODE_MmuGetPT    = 0xf,
};

enum GroupMEM {
    CODE_Copy,
    CODE_Swap,
    CODE_Fill,
    CODE_Through,
    CODE_From,
    CODE_Popb,
    CODE_Poph,
    CODE_Pop,
    CODE_Pushb,
    CODE_Pushh,
    CODE_Push,
    CODE_Save,
    CODE_Restore,
    CODE_Exch,
    CODE_Slt,
    CODE_Sltu,
};

enum GroupJU { CODE_Jal, CODE_Lui, CODE_Auipc };

enum GroupBRANCH {
    CODE_Beq,
    CODE_Bne,
    CODE_Blt,
    CODE_Bge,
    CODE_Bltu,
    CODE_Bgeu,
};

enum GroupLOAD {
    CODE_Jalr = 1,
    CODE_Lb,
    CODE_Lbu,
    CODE_Lbd,
    CODE_Lbud,
    CODE_Lh,
    CODE_Lhu,
    CODE_Lhd,
    CODE_Lhud,
    CODE_Lw,
    CODE_Lwd,
};

enum GroupALUI {
    CODE_Muli,
    CODE_Mulih,
    CODE_Idivi,
    CODE_Addi,
    CODE_Subi,
    CODE_Ori,
    CODE_Andi,
    CODE_Xori,
    CODE_ShiRa,
    CODE_ShiRl,
    CODE_ShiLl,
    CODE_Slti,
    CODE_Sltiu,
};

enum GroupALUR {
    CODE_Add,
    CODE_Sub,
    CODE_Idiv,
    CODE_Mul,
    CODE_Or,
    CODE_And,
    CODE_Xor,
    CODE_Not,
    CODE_Ctz,
    CODE_Clz,
    CODE_Popcount,
    CODE_ShRa,
    CODE_ShRl,
    CODE_ShLl,
    CODE_Ror,
    CODE_Rol,
};

enum GroupSTORE {
    CODE_Sb,
    CODE_Sbd,
    CODE_Sh,
    CODE_Shd,
    CODE_Sw,
    CODE_Swd,
};

enum Instruction {
    Syscall     = (SYS << 4) | CODE_Syscall,
    GsReg       = (SYS << 4) | CODE_GsReg,
    SsReg       = (SYS << 4) | CODE_SsReg,
    Sysret      = (SYS << 4) | CODE_Sysret,
    Trace       = (SYS << 4) | CODE_Trace,
    MmuToggle   = (SYS << 4) | CODE_MmuToggle,
    MmuMap      = (SYS << 4) | CODE_MmuMap,
    MmuUnmap    = (SYS << 4) | CODE_MmuUnmap,
    MmuStat     = (SYS << 4) | CODE_MmuStat,
    MmuUpdate   = (SYS << 4) | CODE_MmuUpdate,
    MmuSetPT    = (SYS << 4) | CODE_MmuSetPT,
    UmodeToggle = (SYS << 4) | CODE_UmodeToggle,
    MmuSwitch   = (SYS << 4) | CODE_MmuSwitch,
    MmuGetPT    = (SYS << 4) | CODE_MmuGetPT,

    Copy    = (MEM << 4) | CODE_Copy,
    Swap    = (MEM << 4) | CODE_Swap,
    Fill    = (MEM << 4) | CODE_Fill,
    Through = (MEM << 4) | CODE_Through,
    From    = (MEM << 4) | CODE_From,
    Popb    = (MEM << 4) | CODE_Popb,
    Poph    = (MEM << 4) | CODE_Poph,
    Pop     = (MEM << 4) | CODE_Pop,
    Pushb   = (MEM << 4) | CODE_Pushb,
    Pushh   = (MEM << 4) | CODE_Pushh,
    Push    = (MEM << 4) | CODE_Push,
    Save    = (MEM << 4) | CODE_Save,
    Restore = (MEM << 4) | CODE_Restore,
    Exch    = (MEM << 4) | CODE_Exch,
    Slt     = (MEM << 4) | CODE_Slt,
    Sltu    = (MEM << 4) | CODE_Sltu,

    Jal   = (JU << 4) | CODE_Jal,
    Lui   = (JU << 4) | CODE_Lui,
    Auipc = (JU << 4) | CODE_Auipc,

    Beq  = (BRANCH << 4) | CODE_Beq,
    Bne  = (BRANCH << 4) | CODE_Bne,
    Blt  = (BRANCH << 4) | CODE_Blt,
    Bge  = (BRANCH << 4) | CODE_Bge,
    Bltu = (BRANCH << 4) | CODE_Bltu,
    Bgeu = (BRANCH << 4) | CODE_Bgeu,

    Jalr = (LOAD << 4) | CODE_Jalr,
    Lb   = (LOAD << 4) | CODE_Lb,
    Lbu  = (LOAD << 4) | CODE_Lbu,
    Lbd  = (LOAD << 4) | CODE_Lbd,
    Lbud = (LOAD << 4) | CODE_Lbud,
    Lh   = (LOAD << 4) | CODE_Lh,
    Lhu  = (LOAD << 4) | CODE_Lhu,
    Lhd  = (LOAD << 4) | CODE_Lhd,
    Lhud = (LOAD << 4) | CODE_Lhud,
    Lw   = (LOAD << 4) | CODE_Lw,
    Lwd  = (LOAD << 4) | CODE_Lwd,

    Muli  = (ALUI << 4) | CODE_Muli,
    Mulih = (ALUI << 4) | CODE_Mulih,
    Idivi = (ALUI << 4) | CODE_Idivi,
    Addi  = (ALUI << 4) | CODE_Addi,
    Subi  = (ALUI << 4) | CODE_Subi,
    Ori   = (ALUI << 4) | CODE_Ori,
    Andi  = (ALUI << 4) | CODE_Andi,
    Xori  = (ALUI << 4) | CODE_Xori,
    ShiRa = (ALUI << 4) | CODE_ShiRa,
    ShiRl = (ALUI << 4) | CODE_ShiRl,
    ShiLl = (ALUI << 4) | CODE_ShiLl,
    Slti  = (ALUI << 4) | CODE_Slti,
    Sltiu = (ALUI << 4) | CODE_Sltiu,

    Add      = (ALUR << 4) | CODE_Add,
    Sub      = (ALUR << 4) | CODE_Sub,
    Idiv     = (ALUR << 4) | CODE_Idiv,
    Mul      = (ALUR << 4) | CODE_Mul,
    Or       = (ALUR << 4) | CODE_Or,
    And      = (ALUR << 4) | CODE_And,
    Xor      = (ALUR << 4) | CODE_Xor,
    Not      = (ALUR << 4) | CODE_Not,
    Ctz      = (ALUR << 4) | CODE_Ctz,
    Clz      = (ALUR << 4) | CODE_Clz,
    Popcount = (ALUR << 4) | CODE_Popcount,
    ShRa     = (ALUR << 4) | CODE_ShRa,
    ShRl     = (ALUR << 4) | CODE_ShRl,
    ShLl     = (ALUR << 4) | CODE_ShLl,
    Ror      = (ALUR << 4) | CODE_Ror,
    Rol      = (ALUR << 4) | CODE_Rol,

    Sb  = (STORE << 4) | CODE_Sb,
    Sbd = (STORE << 4) | CODE_Sbd,
    Sh  = (STORE << 4) | CODE_Sh,
    Shd = (STORE << 4) | CODE_Shd,
    Sw  = (STORE << 4) | CODE_Sw,
    Swd = (STORE << 4) | CODE_Swd,
};

#endif /* CPU_H */
