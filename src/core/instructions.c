#ifndef INSTRUCTIONS_C
#define INSTRUCTIONS_C

#include "core/bus.h"
#include "core/cpu.h"
#include "machine_description.h"
#include "talea.h"

// #define DEBUG_LOG_INSTRUCTION_EXEC

#ifdef DEBUG_LOG_INSTRUCTION_EXEC
#define EXEC_LOG(s)                                                                                \
    do {                                                                                           \
        if ((strcmp(s, "Restore") == 0) || (strcmp(s, "Save") == 0) || (strcmp(s, "Sysret") == 0)) \
            TALEA_LOG_TRACE("[EXEC LOG]: " s "\t\t at 0x%x, retired: %d, ra: 0x%08x\n",            \
                            cpu->pc - 4, cpu->instructionsRetired, GPR_GET(m, x1));                \
                                                                                                   \
    } while (0);
#else
#define EXEC_LOG(s) (void)(s)
#endif

#define REQUIRE_SUPERVISOR                              \
    if (!SR_GET_SUPERVISOR(cpu->status)) {              \
        cpu->exception = EXCEPTION_PRIVILEGE_VIOLATION; \
        return;                                         \
    }

static void instr_Syscall(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                          u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Syscall");
    if (r1 == x0) {
        Exception(m, vector, 0);
    } else {
        Exception(m, GPR_GET(m, r1), 0);
    }
}
static void instr_GsReg(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                        u16 imm_15, u32 imm_20)
{
    EXEC_LOG("GsReg");
    GPR_SET(m, r1, m->cpu.status);
}

static void instr_SsReg(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                        u16 imm_15, u32 imm_20)
{
    EXEC_LOG("SsReg");
    REQUIRE_SUPERVISOR;
    m->cpu.status = GPR_GET(m, r1);
    SetUsermode(m);
}

static void instr_Sysret(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                         u16 imm_15, u32 imm_20)
{
    REQUIRE_SUPERVISOR;
    if (r1 == 1) {
        // CLEAR INTERRUPT
        EXEC_LOG("CLI");
        SR_CLEAR_INTERRUPT(m->cpu.status);
    } else if (r1 == 2) {
        // SET INTERRUPT
        EXEC_LOG("STI");
        SR_SET_INTERRUPT(m->cpu.status, 1);
    } else {
        EXEC_LOG("Sysret");
        m->cpu.status = PopW(m);
        u32 pc        = PopW(m);
        SET_PC(m, pc);
        SetUsermode(m);
        m->cpu.isProcessingException = false;
        // TALEA_LOG_TRACE("\tTo: %06x\n", pc);
    };
}

static void instr_Trace(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                        u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Trace");
    trace(m, r1, r2, r3, r4);
}

static void instr_Copy(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Copy");

    u32 src_addr  = GPR_GET(m, r1);
    u32 dest_addr = GPR_GET(m, r2);
    u32 size      = GPR_GET(m, r3) & 0xFFFFFF;

    bool src_is_data = (imm_15 & 1);
    bool dst_is_data = (imm_15 & 2);
    bool is_backward = (imm_15 & 4); // TODO: docimument this ( and put in the assembler )

    if (dst_is_data || src_is_data) {
        REQUIRE_SUPERVISOR
    }

    if (!src_is_data && SR_GET_MMU(cpu->status)) {
        if (!MMU_ValidateReadAccessRange(m, src_addr, size)) return;
    }

    if (!dst_is_data && SR_GET_MMU(cpu->status)) {
        if (!MMU_ValidateWriteAccessRange(m, dest_addr, size)) return;
    }

    if (is_backward) {
        src_addr += size;
        dest_addr += size;
    }

    while (size >= 4) {
        u32 v;

        u32 read_addr  = is_backward ? (src_addr - 4) : src_addr;
        u32 write_addr = is_backward ? (dest_addr - 4) : dest_addr;

        if (src_is_data)
            v = Machine_ReadData32(m, read_addr);
        else
            v = Machine_ReadMain32(m, read_addr);
        ON_FAULT_RETURN

        if (dst_is_data)
            Machine_WriteData32(m, write_addr, v);
        else
            Machine_WriteMain32(m, write_addr, v);
        ON_FAULT_RETURN

        if (is_backward) {
            src_addr -= 4;
            dest_addr -= 4;
        } else {
            src_addr += 4;
            dest_addr += 4;
        }

        size -= 4;
        GPR_SET(m, r1, src_addr);
        GPR_SET(m, r2, dest_addr);
        GPR_SET(m, r3, size);
    }

    /* 2. Byte Copy (Tail Path) */
    while (size > 0) {
        u32 read_addr  = is_backward ? (src_addr - 1) : src_addr;
        u32 write_addr = is_backward ? (dest_addr - 1) : dest_addr;

        u8 v;
        if (src_is_data)
            v = Machine_ReadData8(m, read_addr);
        else
            v = Machine_ReadMain8(m, read_addr);
        ON_FAULT_RETURN

        if (dst_is_data)
            Machine_WriteData8(m, write_addr, v);
        else
            Machine_WriteMain8(m, write_addr, v);
        ON_FAULT_RETURN

        if (is_backward) {
            src_addr -= 1;
            dest_addr -= 1;
        } else {
            src_addr += 1;
            dest_addr += 1;
        }

        size -= 1;
        GPR_SET(m, r1, src_addr);
        GPR_SET(m, r2, dest_addr);
        GPR_SET(m, r3, size);
    }
}

static void instr_Swap(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    {
        EXEC_LOG("Swap");

        size_t size = GPR_GET(m, r3) & 0xffffff;

        u32 addr_a = GPR_GET(m, r1);
        u32 addr_b = GPR_GET(m, r2);

        if (SR_GET_MMU(m->cpu.status)) {
            if (!MMU_ValidateWriteAccessRange(m, addr_a, size)) {
                return;
            }

            if (!MMU_ValidateWriteAccessRange(m, addr_b, size)) {
                return;
            }
        }

        while (size >= 4) {
            u32 a = Machine_ReadMain32(m, addr_a);
            ON_FAULT_RETURN
            u32 b = Machine_ReadMain32(m, addr_b);
            ON_FAULT_RETURN

            Machine_WriteMain32(m, addr_a, b);
            ON_FAULT_RETURN
            Machine_WriteMain32(m, addr_b, a);
            ON_FAULT_RETURN

            addr_a += 4;
            addr_b += 4;
            size -= 4;
            GPR_SET(m, r1, addr_a);
            GPR_SET(m, r2, addr_b);
            GPR_SET(m, r3, size);
        }

        while (size > 0) {
            u8 a = Machine_ReadMain8(m, addr_a);
            ON_FAULT_RETURN
            u8 b = Machine_ReadMain8(m, addr_b);
            ON_FAULT_RETURN

            Machine_WriteMain8(m, addr_a, b);
            ON_FAULT_RETURN
            Machine_WriteMain8(m, addr_b, a);
            ON_FAULT_RETURN

            addr_a++;
            addr_b++;
            size--;
            GPR_SET(m, r1, addr_a);
            GPR_SET(m, r2, addr_b);
            GPR_SET(m, r3, size);
        }
    }
}

static void instr_Fill(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Fill");
    u32 addr  = GPR_GET(m, r1);
    u32 count = GPR_GET(m, r2);
    u32 val   = GPR_GET(m, r3);
    u32 i;

    if (SR_GET_MMU(cpu->status)) {
        size_t total_bytes = count;
        if (imm_15 & 4)
            total_bytes *= 4; /* Word fill */
        else if (imm_15 & 2)
            total_bytes *= 3; /* 24-bit fill */
        else if (imm_15 & 1)
            total_bytes *= 2; /* Half-word fill */

        if (!MMU_ValidateWriteAccessRange(m, addr, total_bytes)) return;
    }

    for (i = 0; i < count; i++) {
        if (imm_15 & 4) { /* 32-bit */
            Machine_WriteMain32(m, addr + (i * 4), val);
        } else if (imm_15 & 1) { /* 16-bit */
            Machine_WriteMain16(m, addr + (i * 2), (u16)val);
        } else if (imm_15 & 2) { /* 24-bit */
            Machine_WriteMain16(m, addr + (i * 3), (u32)val >> 8);
            ON_FAULT_RETURN
            Machine_WriteMain8(m, addr + (i * 3) + 2, (u32)val);
        } else { /* 8-bit */
            Machine_WriteMain8(m, addr + i, (u8)val);
        }
        ON_FAULT_RETURN
    }
}

static void instr_Through(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                          u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Through");
    u32 a = Machine_ReadMain32(m, GPR_GET(m, r2));
    ON_FAULT_RETURN
    Machine_WriteMain32(m, a, GPR_GET(m, r1));
    ON_FAULT_RETURN
}

static void instr_From(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("From");
    u32 a = Machine_ReadMain32(m, GPR_GET(m, r2));
    ON_FAULT_RETURN
    u32 b = Machine_ReadMain32(m, a);
    ON_FAULT_RETURN
    GPR_SET(m, r1, b);
}

static void instr_Popb(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Popb");
    u8 a = Machine_ReadMain8(m, GPR_GET(m, r2));
    ON_FAULT_RETURN
    GPR_SET(m, r1, a);
    GPR_SET(m, r2, GPR_GET(m, r2) + 1);
}
static void instr_Poph(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Poph");
    u16 a = Machine_ReadMain16(m, GPR_GET(m, r2));
    ON_FAULT_RETURN
    GPR_SET(m, r1, a);
    GPR_SET(m, r2, GPR_GET(m, r2) + 2);
}
static void instr_Pop(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Pop");
    u32 a = Machine_ReadMain32(m, GPR_GET(m, r2));
    ON_FAULT_RETURN
    GPR_SET(m, r1, a);
    GPR_SET(m, r2, GPR_GET(m, r2) + 4);
}
static void instr_Pushb(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                        u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Pushb");
    GPR_SET(m, r2, GPR_GET(m, r2) - 1);
    Machine_WriteMain8(m, GPR_GET(m, r2), GPR_GET(m, r1));
    ON_FAULT_RETURN
}
static void instr_Pushh(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                        u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Pushh");
    GPR_SET(m, r2, GPR_GET(m, r2) - 2);
    Machine_WriteMain16(m, GPR_GET(m, r2), GPR_GET(m, r1));
    ON_FAULT_RETURN
}
static void instr_Push(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Push");
    GPR_SET(m, r2, GPR_GET(m, r2) - 4);
    Machine_WriteMain32(m, GPR_GET(m, r2), GPR_GET(m, r1));
    ON_FAULT_RETURN
}

static void instr_Save(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Save");
    REQUIRE_SUPERVISOR

    // TODO: document new functionality. And add four register, because the instruction encoding
    // allows it
    u32 reg1 = GPR_GET(m, r1);
    u32 reg2 = GPR_GET(m, r2);
    u32 reg3 = GPR_GET(m, r3);

    // TALEA_LOG_TRACE("Saving, cwp: %d, spill: %d\n", cpu->cwp, cpu->spilledWindows);

    if (m->cpu.cwp < 7) {
        m->cpu.cwp++;
    } else if (m->cpu.spilledWindows < 7) {
        u32 spill_base = TALEA_DATA_FIRMWARE_RES + (m->cpu.spilledWindows * 32 * 4);

        for (size_t i = 0; i < 32; i++) {
            u32 reg = GPR_GET(m, i);
            Machine_WriteData32(m, spill_base + (i * 4), reg);
        }

        m->cpu.spilledWindows++;
    } else {
        m->cpu.exception = EXCEPTION_OVERSPILL;
        m->cpu.faultAddr = GET_PC(m);
        return;
    }

    GPR_SET(m, r1, reg1);
    GPR_SET(m, r2, reg2);
    GPR_SET(m, r3, reg3);
}
static void instr_Restore(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                          u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Restore");
    REQUIRE_SUPERVISOR

    // TODO: document new functionality AND THAT IS DANGEROUS TO OVERWRITE THE WINDOW. And add four
    // register, because the instruction encoding allows it
    u32 reg1 = GPR_GET(m, r1);
    u32 reg2 = GPR_GET(m, r2);
    u32 reg3 = GPR_GET(m, r3);

    // TALEA_LOG_TRACE("Restoring, cwp: %d, spill: %d\n", cpu->cwp, cpu->spilledWindows);

    if (m->cpu.spilledWindows > 0) {
        m->cpu.spilledWindows--;
        u32 spill_base = TALEA_DATA_FIRMWARE_RES + (m->cpu.spilledWindows * 32 * 4);

        for (size_t i = 0; i < 32; i++) {
            u32 reg = Machine_ReadData32(m, spill_base + (i * 4));
            GPR_SET(m, i, reg);
        }
    } else if (m->cpu.cwp > 0) {
        m->cpu.cwp--;
    } else {
        m->cpu.exception = EXCEPTION_UNDERSPILL;
        m->cpu.faultAddr = GET_PC(m);
        return;
    }

    GPR_SET(m, r1, reg1);
    GPR_SET(m, r2, reg2);
    GPR_SET(m, r3, reg3);
}

static void instr_Exch(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Exch");
    u32 temp = GPR_GET(m, r1);
    GPR_SET(m, r1, GPR_GET(m, r2));
    GPR_SET(m, r2, temp);
}

static void instr_Slt(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Slt");
    GPR_SET(m, r1, (int32_t)GPR_GET(m, r2) < (int32_t)GPR_GET(m, r3));
}
static void instr_Sltu(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Sltu");
    GPR_SET(m, r1, GPR_GET(m, r2) < GPR_GET(m, r3));
}
static void instr_Jal(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Jal");
    const u32 pc = GET_PC(m);
    GPR_SET(m, r1, pc);
    const u32 offset = sext(imm_20 << 2, 22);
    SET_PC(m, (pc - 4) + offset);
}
static void instr_Lui(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Lui");
    GPR_SET(m, r1, imm_20 << 12);
}
static void instr_Auipc(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                        u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Auipc");
    GPR_SET(m, r1, (imm_20 << 12) + (GET_PC(m) - 4));
}
static void instr_Beq(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Beq");
    if (GPR_GET(m, r1) == GPR_GET(m, r2)) {
        SET_PC(m, (GET_PC(m) - 4) + sext((u32)imm_15 << 2, 17));
    }
}
static void instr_Bne(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Bne");
    if (GPR_GET(m, r1) != GPR_GET(m, r2)) {
        SET_PC(m, (GET_PC(m) - 4) + sext((u32)imm_15 << 2, 17));
    }
}
static void instr_Blt(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Blt");
    if ((int32_t)GPR_GET(m, r1) < (int32_t)GPR_GET(m, r2)) {
        SET_PC(m, (GET_PC(m) - 4) + sext((u32)imm_15 << 2, 17));
    }
}
static void instr_Bge(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Bge");
    if ((int32_t)GPR_GET(m, r1) >= (int32_t)GPR_GET(m, r2)) {
        SET_PC(m, (GET_PC(m) - 4) + sext((u32)imm_15 << 2, 17));
    }
}
static void instr_Bltu(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Bltu");
    if (GPR_GET(m, r1) < GPR_GET(m, r2)) {
        SET_PC(m, (GET_PC(m) - 4) + sext((u32)imm_15 << 2, 17));
    }
}
static void instr_Bgeu(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Bgeu");
    if (GPR_GET(m, r1) >= GPR_GET(m, r2)) {
        SET_PC(m, (GET_PC(m) - 4) + sext((u32)imm_15 << 2, 17));
    }
}
static void instr_Jalr(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Jalr");
    u32 temp = GPR_GET(m, r2) + sext((u32)imm_15 << 2, 17);
    GPR_SET(m, r1, GET_PC(m));
    SET_PC(m, temp);
}
static void instr_Lb(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                     u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Lb");
    u8 a = Machine_ReadMain8(m, GPR_GET(m, r2) + sext(imm_15, 15));
    ON_FAULT_RETURN
    GPR_SET(m, r1, sext(a, 8));
}
static void instr_Lbu(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Lbu");
    u8 a = Machine_ReadMain8(m, GPR_GET(m, r2) + sext(imm_15, 15));
    ON_FAULT_RETURN
    GPR_SET(m, r1, a);
}
static void instr_Lbd(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Lbd");
    REQUIRE_SUPERVISOR
    u8 a = Machine_ReadData8(m, GPR_GET(m, r2) + sext(imm_15, 15));
    ON_FAULT_RETURN
    GPR_SET(m, r1, sext(a, 8));
}
static void instr_Lbud(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Lbud");
    REQUIRE_SUPERVISOR
    u8 a = Machine_ReadData8(m, GPR_GET(m, r2) + sext(imm_15, 15));
    ON_FAULT_RETURN
    GPR_SET(m, r1, a);
}
static void instr_Lh(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                     u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Lh");
    u16 a = Machine_ReadMain16(m, GPR_GET(m, r2) + sext(imm_15, 15));
    ON_FAULT_RETURN
    GPR_SET(m, r1, sext(a, 16));
}
static void instr_Lhu(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Lhu");
    u16 a = Machine_ReadMain16(m, GPR_GET(m, r2) + sext(imm_15, 15));
    ON_FAULT_RETURN
    GPR_SET(m, r1, a);
}
static void instr_Lhd(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Lhd");
    REQUIRE_SUPERVISOR
    u16 a = Machine_ReadData16(m, GPR_GET(m, r2) + sext(imm_15, 15));
    ON_FAULT_RETURN
    GPR_SET(m, r1, sext(a, 16));
}
static void instr_Lhud(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Lhud");
    REQUIRE_SUPERVISOR
    u16 a = Machine_ReadData16(m, GPR_GET(m, r2) + sext(imm_15, 15));
    ON_FAULT_RETURN
    GPR_SET(m, r1, a);
}
static void instr_Lw(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                     u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Lw");
    u32 a = Machine_ReadMain32(m, GPR_GET(m, r2) + sext(imm_15, 15));
    ON_FAULT_RETURN
    GPR_SET(m, r1, a);
}
static void instr_Lwd(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Lwd");
    REQUIRE_SUPERVISOR
    u32 a = Machine_ReadData32(m, GPR_GET(m, r2) + sext(imm_15, 15));
    ON_FAULT_RETURN
    GPR_SET(m, r1, a);
}
static void instr_Muli(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Muli");
    GPR_SET(m, r1, GPR_GET(m, r2) * sext(imm_15, 15));
}
static void instr_Mulih(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                        u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Mulih");
    GPR_SET(m, r1, (uint64_t)(GPR_GET(m, r2) * sext(imm_15, 15)) >> 32);
}
static void instr_Idivi(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                        u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Idivi");

    u32 dividend = GPR_GET(m, r2);

    if (imm_15 == 0 || (dividend == INT32_MIN && imm_15 == -1)) {
        m->cpu.exception = EXCEPTION_DIVISION_ZERO;

        return;
    }

    GPR_SET(m, r1, GPR_GET(m, r2) / sext(imm_15, 15));
}
static void instr_Addi(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Addi");
    GPR_SET(m, r1, GPR_GET(m, r2) + sext(imm_15, 15));
}
static void instr_Subi(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Subi");
    GPR_SET(m, r1, GPR_GET(m, r2) - sext(imm_15, 15));
}
static void instr_Ori(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Ori");
    GPR_SET(m, r1, GPR_GET(m, r2) | imm_15);
}
static void instr_Andi(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Andi");
    GPR_SET(m, r1, GPR_GET(m, r2) & imm_15);
}
static void instr_Xori(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Xori");
    GPR_SET(m, r1, GPR_GET(m, r2) ^ imm_15);
}
static void instr_ShiRa(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                        u16 imm_15, u32 imm_20)
{
    EXEC_LOG("ShiRa");
    GPR_SET(m, r1, (int32_t)GPR_GET(m, r2) >> (imm_15 % 32));
}
static void instr_ShiRl(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                        u16 imm_15, u32 imm_20)
{
    EXEC_LOG("ShiRl");
    GPR_SET(m, r1, GPR_GET(m, r2) >> (imm_15 % 32));
}
static void instr_ShiLl(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                        u16 imm_15, u32 imm_20)
{
    EXEC_LOG("ShiLl");
    GPR_SET(m, r1, GPR_GET(m, r2) << (imm_15 % 32));
}
static void instr_Slti(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Slti");
    GPR_SET(m, r1, (int32_t)GPR_GET(m, r2) < (int32_t)sext(imm_15, 15));
}
static void instr_Sltiu(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                        u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Sltiu");
    GPR_SET(m, r1, GPR_GET(m, r2) < imm_15);
}
static void instr_Add(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Add");
    GPR_SET(m, r1, GPR_GET(m, r2) + GPR_GET(m, r3));
}
static void instr_Sub(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Sub");
    GPR_SET(m, r1, GPR_GET(m, r2) - GPR_GET(m, r3));
}
static void instr_Idiv(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Idiv");

    u32  divisor     = GPR_GET(m, r4);
    u32  dividend    = GPR_GET(m, r3);
    bool signed_mode = (vector & 1) == 0;

    if (divisor == 0 || (signed_mode && dividend == INT32_MIN && divisor == -1)) {
        m->cpu.exception = EXCEPTION_DIVISION_ZERO;
        return;
    }

    if (signed_mode) {
        GPR_SET(m, r1, (int32_t)dividend / (int32_t)divisor);
        GPR_SET(m, r2, (int32_t)dividend % (int32_t)divisor);
    } else {
        GPR_SET(m, r1, dividend / divisor);
        GPR_SET(m, r2, dividend % divisor);
    }
}
static void instr_Mul(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    {
        EXEC_LOG("Mul");
        if ((vector & 1) == 0) {
            // IMULU
            const u64 value = (u64)GPR_GET(m, r3) * (u64)GPR_GET(m, r4);
            // TODO:Document this implementation
            GPR_SET(m, r1, value >> 32);
            GPR_SET(m, r2, value);
        } else {
            const int64_t value = (int64_t)GPR_GET(m, r3) * (int64_t)GPR_GET(m, r4);
            // TODO: Document this implementation
            GPR_SET(m, r1, value >> 32);
            GPR_SET(m, r2, value);
        }
    }
}
static void instr_Or(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                     u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Or");
    GPR_SET(m, r1, GPR_GET(m, r2) | GPR_GET(m, r3));
}
static void instr_And(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("And");
    GPR_SET(m, r1, GPR_GET(m, r2) & GPR_GET(m, r3));
}
static void instr_Xor(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Xor");
    GPR_SET(m, r1, GPR_GET(m, r2) ^ GPR_GET(m, r3));
}
static void instr_Not(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Not");
    GPR_SET(m, r1, ~GPR_GET(m, r2));
}
#ifdef __GNUC__
static void instr_Ctz(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Ctz");
    GPR_SET(m, r1, __builtin_ctz(GPR_GET(m, r2)));
}
static void instr_Clz(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Clz");
    GPR_SET(m, r1, __builtin_clz(GPR_GET(m, r2)));
}
static void instr_Popcount(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                           u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Popcount");
    GPR_SET(m, r1, __builtin_popcount(GPR_GET(m, r2)));
}
#else
static void instr_Ctz(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Ctz");
    u32 v = GPR_GET(m, r2); // 32-bit word input to count zero bits on right
    u32 c = 32;             // c will be the number of zero bits on the right
    v &= -(signed)(v);
    if (v) c--;
    if (v & 0x0000FFFF) c -= 16;
    if (v & 0x00FF00FF) c -= 8;
    if (v & 0x0F0F0F0F) c -= 4;
    if (v & 0x33333333) c -= 2;
    if (v & 0x55555555) c -= 1;
    GPR_SET(m, r1, c);
}
static void instr_Clz(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Clz");
    GPR_SET(m, r1, __lzcnt(GPR_GET(m, r2)));
}
static void instr_Popcount(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                           u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Popcount");
    GPR_SET(m, r1, __popcnt(GPR_GET(m, r2)));
}
#endif
static void instr_ShRa(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("ShRa");
    GPR_SET(m, r1, (int32_t)GPR_GET(m, r2) >> (GPR_GET(m, r3) % 32));
}
static void instr_ShRl(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("ShRl");
    GPR_SET(m, r1, GPR_GET(m, r2) >> (GPR_GET(m, r3) % 32));
}
static void instr_ShLl(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                       u16 imm_15, u32 imm_20)
{
    EXEC_LOG("ShLl");
    GPR_SET(m, r1, GPR_GET(m, r2) << (GPR_GET(m, r3) % 32));
}
static void instr_Ror(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Ror");
    u32 val = GPR_GET(m, r2);
    u32 amt = GPR_GET(m, r3) & 31;
    u32 res = (amt == 0) ? val : (val >> amt) | (val << (32 - amt));

    GPR_SET(m, r1, res);
}
static void instr_Rol(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Rol");
    u32 val = GPR_GET(m, r2);
    u32 amt = GPR_GET(m, r3) & 31;
    u32 res = (amt == 0) ? val : (val << amt) | (val >> (32 - amt));

    GPR_SET(m, r1, res);
}
static void instr_Sb(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                     u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Sb");
    Machine_WriteMain8(m, GPR_GET(m, r2) + sext(imm_15, 15), GPR_GET(m, r1));
    ON_FAULT_RETURN
}
static void instr_Sbd(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Sbd");
    REQUIRE_SUPERVISOR;
    Machine_WriteData8(m, GPR_GET(m, r2) + sext(imm_15, 15), GPR_GET(m, r1));
    ON_FAULT_RETURN
}
static void instr_Sh(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                     u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Sh");
    Machine_WriteMain16(m, GPR_GET(m, r2) + sext(imm_15, 15), GPR_GET(m, r1));
    ON_FAULT_RETURN
}
static void instr_Shd(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Shd");
    REQUIRE_SUPERVISOR;
    Machine_WriteData16(m, GPR_GET(m, r2) + sext(imm_15, 15), GPR_GET(m, r1));
    ON_FAULT_RETURN
}
static void instr_Sw(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                     u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Sw");
    Machine_WriteMain32(m, GPR_GET(m, r2) + sext(imm_15, 15), GPR_GET(m, r1));
    ON_FAULT_RETURN
}
static void instr_Swd(TaleaMachine *m, CpuState *cpu, u8 r1, u8 r2, u8 r3, u8 r4, u8 vector,
                      u16 imm_15, u32 imm_20)
{
    EXEC_LOG("Swd");
    REQUIRE_SUPERVISOR;
    Machine_WriteData32(m, GPR_GET(m, r2) + sext(imm_15, 15), GPR_GET(m, r1));
    ON_FAULT_RETURN
}

#endif