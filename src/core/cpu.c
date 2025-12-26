/*******************************************************************************************
 *
 *   Taleä System cpu emulation
 *
 ********************************************************************************************/

#include "cpu.h"
#include "bus.h"
#include "talea.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static inline u32 sext(u32 n, u32 bits)
{
    int mask = 1U << (bits - 1);
    return (n ^ mask) - mask;
}

static inline Register GPR_GET(TaleaMachine *m, u8 reg)
{
    return (reg == 0) ? 0 : m->cpu.gpr[reg];
}

static inline void GPR_SET(TaleaMachine *m, u8 reg, u32 value)
{
    m->cpu.gpr[reg] = (reg == 0) ? 0 : value;
}

static inline void SET_PC(TaleaMachine *m, u32 value)
{
    if (value % 4 != 0) m->cpu.exception = EXCEPTION_ADDRESS_ERROR;
    m->cpu.pc = value;
}

static inline u32 GET_PC(TaleaMachine *m)
{
    return (m->cpu.pc);
}

static inline void trace(TaleaMachine *m, u8 r1, u8 r2, u8 r3, u8 r4)
{
    TALEA_LOG_TRACE("[TRACE] R%d: %08x | R%d: %08x | R%d: %08x | R%d: %08x \n",
                    r1, GPR_GET(m, r1), r2, GPR_GET(m, r2), r3, GPR_GET(m, r3),
                    r4, GPR_GET(m, r4));
}

static inline void SetSupervisor(TaleaMachine *m)
{
    if (SR_GET_SUPERVISOR(m->cpu.status)) return;

    SR_SET_SUPERVISOR(m->cpu.status, 1U);
    m->cpu.usp     = m->cpu.gpr[x2];
    m->cpu.gpr[x2] = m->cpu.ssp;
}

static inline void PushW(TaleaMachine *m, u32 word)
{
    m->cpu.gpr[x2] -= 4;
    Machine_WriteMain32(m, m->cpu.gpr[x2], word);
}

static inline u32 PopW(TaleaMachine *m)
{
    u32 word = Machine_ReadMain32(m, m->cpu.gpr[x2]);
    m->cpu.gpr[x2] += 4;
    return word;
}

#define DEBUG_LOG_EXCEPTIONS
#define DEBUG_LOG_INTERRUPTS

static void Exception(TaleaMachine *m, u8 vector, bool is_interrupt)
{
    if (is_interrupt && !SR_GET_INTERRUPT(m->cpu.status)) return;

    m->cpu.exception = vector;

    // normal exception

    const u32 pc   = GET_PC(m);
    const u32 stat = m->cpu.status;
    SetSupervisor(m);
    PushW(m, pc);
    PushW(m, stat);

    if (vector == EXCEPTION_BUS_ERROR || vector == EXCEPTION_ADDRESS_ERROR ||
        vector == EXCEPTION_PAGE_FAULT) {
        // FAULT
        PushW(m, Machine_ReadMain32(m, GET_PC(m) - 4));

#ifdef DEBUG_LOG_EXCEPTIONS
        TALEA_LOG_TRACE("FAULT: %u\n", vector);
#endif
    }

    if (is_interrupt) {
        SR_SET_PRIORITY(m->cpu.status, m->cpu.current_ipl);
    }

#if TALEA_IVT_FIXED
    const u32 handler_addr =
        Machine_ReadData32(m, TALEA_IVT_BASE + ((u16)vector << 2));
    SET_PC(m, handler_addr);

#ifdef DEBUG_LOG_EXCEPTIONS
    TALEA_LOG_TRACE("Exception 0x%x, jumping to 0x%08x\n", vector,
                    handler_addr);
#endif

#else
#error "Use fixed IVT for now please"
#endif

    m->cpu.cycles += CYCLES_EXCEPTION;
}

void Machine_RaiseInterrupt(TaleaMachine *m, u8 vector, u8 priority)
{
    m->cpu.pending_interrupts[priority] = vector;

#ifdef DEBUG_LOG_INTERRUPTS
    TALEA_LOG_TRACE("Interrupt: 0x%x, priority: %u, pending: %u, cpu: %u\n",
                    vector, priority, m->cpu.highest_pending_interrupt,
                    SR_GET_PRIORITY(m->cpu.status));
#endif

    if (priority > m->cpu.highest_pending_interrupt)
        m->cpu.highest_pending_interrupt = priority;
}

static void CheckInterrupts(TaleaMachine *m)
{
    // It is ugly, but naive, to check this here in this way
    if (m->storage.current_tps->just_inserted) {
        puts("Raise TPS INSERTED \n");
        m->storage.current_tps->just_inserted = false;
        Machine_RaiseInterrupt(m, INT_TPS_INSERTED, PRIORITY_STORAGE_INTERRUPT);
    }

    if (m->storage.current_tps->just_ejected) {
        puts("Raise TPS Ejected \n");
        m->storage.current_tps->just_ejected = true;
        Machine_RaiseInterrupt(m, INT_TPS_EJECTED, PRIORITY_STORAGE_INTERRUPT);
    }

    if (m->storage.current_tps->real_status & STOR_STATUS_DONE) {
        m->storage.current_tps->real_status &= ~STOR_STATUS_DONE;
        puts("Raise TPS FINISH\n");
        SaveFileData("resources/dump.bin", &m->main_memory[0],
                     TALEA_SECTOR_SIZE); // TODO: this is just testing
        Machine_RaiseInterrupt(m, INT_TPS_FINISH, PRIORITY_STORAGE_INTERRUPT);
    }

    if (m->storage.hcs.real_status & STOR_STATUS_DONE) {
        m->storage.hcs.real_status &= ~STOR_STATUS_DONE;
        Machine_RaiseInterrupt(m, INT_HCS_FINISH, PRIORITY_STORAGE_INTERRUPT);
    }

    m->cpu.pending_ipl = m->cpu.highest_pending_interrupt;
    u8 pending         = m->cpu.pending_ipl;
    u8 current         = m->cpu.current_ipl;

    if ((m->cpu.pending_ipl != 0) &&
        (pending > SR_GET_PRIORITY(m->cpu.status) ||
         ((pending == 7) && (pending >= current)))) {
        m->cpu.current_ipl = m->cpu.pending_ipl;
        // acknowledge interrupt, maybe fail here
        u8 vector = m->cpu.pending_interrupts[m->cpu.current_ipl];
        m->cpu.pending_interrupts[m->cpu.current_ipl] = 0;
        while (m->cpu.highest_pending_interrupt > 0 &&
               !m->cpu.pending_interrupts[m->cpu.highest_pending_interrupt]) {
            m->cpu.highest_pending_interrupt -= 1;
        }

        Exception(m, vector, 1);
    }

    if (pending < current) m->cpu.current_ipl = m->cpu.pending_ipl;
}

#ifdef DEBUG_LOG_INSTRUCTION_EXEC
#define EXEC_LOG(s)                                                \
    TALEA_LOG_TRACE("[EXEC LOG]: " s "\t\t (0x%8x: %u) at 0x%x\n", \
                    instruction, group_opcode, cpu.pc - 4)
#else
#define EXEC_LOG(s) (void)(s)
#endif

/* fetch, decode, execute cycle */
// TODO: we aren't setting dirty bits nor accounting for the x flag with MMU.
// It's too slow anyways, we have to change implementation

static inline u32 Fetch(TaleaMachine *m)
{
    const u32 instruction = Machine_ReadMain32(m, GET_PC(m));
    SET_PC(m, GET_PC(m) + 4);
    return instruction;
}

static inline void SetUsermode(TaleaMachine *m)
{
    if (!SR_GET_SUPERVISOR(m->cpu.status)) {
        m->cpu.ssp     = m->cpu.gpr[x2];
        m->cpu.gpr[x2] = m->cpu.usp;
    }
}

static int Execute(TaleaMachine *m)
{
/*-----------------*/
#define REQUIRE_SUPERVISOR                                \
    if (!SR_GET_SUPERVISOR(m->cpu.status)) {              \
        m->cpu.exception = EXCEPTION_PRIVILEGE_VIOLATION; \
        exception        = 1;                             \
        break;                                            \
    }                                                     \
    /*-----------------*/

    int      exception     = 0;
    u32      instruction   = Fetch(m);
    const u8 group /*u3*/  = (instruction >> GROUP_SHIFHT) & 0b111;
    const u8 opcode /*u8*/ = ((instruction & OPCODE_MASK) >> OPCODE_SHIFT) &
                             0xf;
    const u8  r1 /*u5*/      = ((instruction & R1_MASK) >> R1_SHIFT) & 0x1f;
    const u8  r2 /*u5*/      = ((instruction & R2_MASK) >> R2_SHIFT) & 0x1f;
    const u8  r3 /*u5*/      = ((instruction & R3_MASK) >> R3_SHIFT) & 0x1f;
    const u8  r4 /*u5*/      = ((instruction & R4_MASK) >> R4_SHIFT) & 0x1f;
    const u8  vector /*u8*/  = instruction & VECTOR_MASK;
    const u16 imm_15 /*u15*/ = (instruction & IMM15_MASK) & 0x7fff;
    const u32 imm_20 /*u20*/ = (instruction & IMM20_MASK) & 0xfffff;
    u32       temp           = { 0 };

    const u32 group_opcode /*u7*/ = ((group << 4) | opcode) & 0x7f;
    switch (group_opcode) {
    case Syscall:
        EXEC_LOG("Syscall");
        if (r1 == x0) {
            Exception(m, vector, 0);
        } else {
            Exception(m, GPR_GET(m, r1), 0);
        }
        break;
    case GsReg:
        EXEC_LOG("GsReg");
        GPR_SET(m, r1, m->cpu.status);
        break;
    case SsReg:
        EXEC_LOG("SsReg");
        REQUIRE_SUPERVISOR;
        m->cpu.status = GPR_GET(m, r1);
        SetUsermode(m);
        break;
    case Sysret: {
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
            m->cpu.status = PopW(m);
            SET_PC(m, PopW(m));
            SetUsermode(m);
            EXEC_LOG("Sysret");
        };
        break;
    }
    case Trace:
        EXEC_LOG("Trace");
        trace(m, r1, r2, r3, r4);
        break;
    case Copy:
        EXEC_LOG("Copy");
        {
            if (((imm_15 & 1) != 0) || ((imm_15 & 2) != 0)) {
                REQUIRE_SUPERVISOR;
            }
            // src in data or main
            u8 *src = ((imm_15 & 1) != 0) ?
                          &m->data_memory[GPR_GET(m, r1) & 0xffff] :
                          &m->main_memory[GPR_GET(m, r1) & 0xffffff];

            size_t srcSize = MIN(GPR_GET(m, r3) & 0xffffff,
                                 (((imm_15 & 1) != 0) ? TALEA_DATA_MEM_SZ :
                                                        TALEA_MAIN_MEM_SZ));

            // dst in data or main
            u8 *dst = ((imm_15 & 2) != 0) ?
                          &m->data_memory[GPR_GET(m, r2) & 0xffff] :
                          &m->main_memory[GPR_GET(m, r2) & 0xffffff];
            int dstSize =
                ((imm_15 & 2) != 0) ?
                    GPR_GET(m, r3) & 0xffffff + (GPR_GET(m, r2) & 0xffff) :
                    GPR_GET(m, r3) & 0xffffff + (GPR_GET(m, r2) & 0xffffff);

            if ((imm_15 & 2) != 0 && dstSize > TALEA_DATA_MEM_SZ) break;
            if ((imm_15 & 2) == 0 && dstSize > TALEA_MAIN_MEM_SZ) break;

            memcpy(dst, src, srcSize);
            break;
        }
    case Swap: {
        EXEC_LOG("Swap");
        u8 *buff = malloc(GPR_GET(m, r3) & 0xffffff);
        if (buff == NULL) break;

        size_t size = GPR_GET(m, r3) & 0xffffff;

        if ((size > TALEA_MAIN_MEM_SZ - (GPR_GET(m, r1) & 0xffffff)) ||
            (size > TALEA_MAIN_MEM_SZ - (GPR_GET(m, r2) & 0xffffff))) {
            free(buff);
            break;
        }

        memcpy(buff, &m->main_memory[GPR_GET(m, r1) & 0xffffff],
               GPR_GET(m, r3) & 0xffffff);
        memmove(&m->main_memory[GPR_GET(m, r1) & 0xffffff],
                &m->main_memory[GPR_GET(m, r2) & 0xffffff],
                GPR_GET(m, r3) & 0xffffff);
        memcpy(&m->main_memory[GPR_GET(m, r2) & 0xffffff], buff,
               GPR_GET(m, r3) & 0xffffff);
        free(buff);
        break;
    }
    case Fill: {
        EXEC_LOG("Fill");
        u8 *dest = &m->main_memory[GPR_GET(m, r1) & 0xffffff];
        u32 len  = GPR_GET(m, r2);

        if ((imm_15 & 4) != 0) {
            if (len * 4 > TALEA_MAIN_MEM_SZ - (GPR_GET(m, r1) & 0xffffff))
                break;
            for (u32 a = 0; a < len; a++) ((u32 *)dest)[a] = GPR_GET(m, r3);
        } else if ((imm_15 & 2) != 0) {
            if (len * 3 > TALEA_MAIN_MEM_SZ - (GPR_GET(m, r1) & 0xffffff))
                break;
            for (u32 b = 0; b < (len * 3); b += 3) {
                dest[b]     = GPR_GET(m, r3) >> 16;
                dest[b + 1] = GPR_GET(m, r3) >> 8;
                dest[b + 2] = GPR_GET(m, r3);
            }
        } else if ((imm_15 & 1) != 0) {
            if (len * 2 > TALEA_MAIN_MEM_SZ - (GPR_GET(m, r1) & 0xffffff))
                break;

            for (u16 c = 0; c < len; c++) ((u16 *)dest)[c] = GPR_GET(m, r3);
        } else {
            if (len > TALEA_MAIN_MEM_SZ - (GPR_GET(m, r1) & 0xffffff)) break;

            memset(dest, GPR_GET(m, r3), len);
        }
        break;
    }
    case Through:
        EXEC_LOG("Through");
        Machine_WriteMain32(m, Machine_ReadMain32(m, GPR_GET(m, r2)),
                            GPR_GET(m, r1));
        break;
    case From:
        EXEC_LOG("From");
        GPR_SET(m, r1,
                Machine_ReadMain32(m, Machine_ReadMain32(m, GPR_GET(m, r2))));
        break;
    case Popb:
        EXEC_LOG("Popb");
        GPR_SET(m, r1, Machine_ReadMain8(m, GPR_GET(m, r2)));
        GPR_SET(m, r2, GPR_GET(m, r2) + 1);
        break;
    case Poph:
        EXEC_LOG("Poph");
        GPR_SET(m, r1, Machine_ReadMain16(m, GPR_GET(m, r2)));
        GPR_SET(m, r2, GPR_GET(m, r2) + 2);
        break;
    case Pop:
        EXEC_LOG("Pop");
        GPR_SET(m, r1, Machine_ReadMain32(m, GPR_GET(m, r2)));
        GPR_SET(m, r2, GPR_GET(m, r2) + 4);
        break;
    case Pushb:
        EXEC_LOG("Pushb");
        GPR_SET(m, r2, GPR_GET(m, r2) - 1);
        Machine_WriteMain8(m, GPR_GET(m, r2), GPR_GET(m, r1));
        break;
    case Pushh:
        EXEC_LOG("Pushh");
        GPR_SET(m, r2, GPR_GET(m, r2) - 2);
        Machine_WriteMain16(m, GPR_GET(m, r2), GPR_GET(m, r1));
        break;
    case Push:
        EXEC_LOG("Push");
        GPR_SET(m, r2, GPR_GET(m, r2) - 4);
        Machine_WriteMain32(m, GPR_GET(m, r2), GPR_GET(m, r1));
        break;
    case Save:
        EXEC_LOG("Save");
        {
            u32 addr = GPR_GET(m, r3);

            for (size_t d = r1; d <= r2; d++) {
                addr -= 4; // Pushes them
                Machine_WriteMain32(m, addr, GPR_GET(m, d));
            }

            GPR_SET(m, r3, addr);
            break;
        }
    case Restore: {
        EXEC_LOG("Restore");
        u32 addr = GPR_GET(m, r3);
        for (u32 r = r2; r >= r1; r -= 1) {
            GPR_SET(m, r, Machine_ReadMain32(m, addr));
            addr += 4; // Pops them
        }
        GPR_SET(m, r3, addr);
        break;
    }
    case Exch:
        EXEC_LOG("Exch");
        temp = GPR_GET(m, r1);
        GPR_SET(m, r1, GPR_GET(m, r2));
        GPR_SET(m, r2, temp);
        break;
    case Slt:
        EXEC_LOG("Slt");
        GPR_SET(m, r1, (int32_t)GPR_GET(m, r2) < (int32_t)GPR_GET(m, r3));
        break;
    case Sltu:
        EXEC_LOG("Sltu");
        GPR_SET(m, r1, GPR_GET(m, r2) < GPR_GET(m, r3));
        break;
    case Jal:
        EXEC_LOG("Jal");
        const u32 pc = GET_PC(m);
        GPR_SET(m, r1, pc);
        const u32 offset = sext(imm_20 << 2, 22);
        SET_PC(m, (pc - 4) + offset);
        break;
    case Lui:
        EXEC_LOG("Lui");
        GPR_SET(m, r1, imm_20 << 12);
        break;
    case Auipc:
        EXEC_LOG("Auipc");
        GPR_SET(m, r1, (imm_20 << 12) + (GET_PC(m) - 4));
        break;
    case Beq:
        EXEC_LOG("Beq");
        if (GPR_GET(m, r1) == GPR_GET(m, r2)) {
            SET_PC(m, (GET_PC(m) - 4) + sext((u32)imm_15 << 2, 17));
        }
        break;
    case Bne:
        EXEC_LOG("Bne");
        if (GPR_GET(m, r1) != GPR_GET(m, r2)) {
            SET_PC(m, (GET_PC(m) - 4) + sext((u32)imm_15 << 2, 17));
        }
        break;
    case Blt:
        EXEC_LOG("Blt");
        if ((int32_t)GPR_GET(m, r1) < (int32_t)GPR_GET(m, r2)) {
            SET_PC(m, (GET_PC(m) - 4) + sext((u32)imm_15 << 2, 17));
        }
        break;
    case Bge:
        EXEC_LOG("Bge");
        if ((int32_t)GPR_GET(m, r1) >= (int32_t)GPR_GET(m, r2)) {
            SET_PC(m, (GET_PC(m) - 4) + sext((u32)imm_15 << 2, 17));
        }
        break;
    case Bltu:
        EXEC_LOG("Bltu");
        if (GPR_GET(m, r1) < GPR_GET(m, r2)) {
            SET_PC(m, (GET_PC(m) - 4) + sext((u32)imm_15 << 2, 17));
        }
        break;
    case Bgeu:
        EXEC_LOG("Bgeu");
        if (GPR_GET(m, r1) >= GPR_GET(m, r2)) {
            SET_PC(m, (GET_PC(m) - 4) + sext((u32)imm_15 << 2, 17));
        }
        break;
    case Jalr:
        EXEC_LOG("Jalr");
        temp = GPR_GET(m, r2) + sext((u32)imm_15 << 2, 17);
        GPR_SET(m, r1, GET_PC(m));
        SET_PC(m, temp);
        break;
    case Lb:
        EXEC_LOG("Lb");
        GPR_SET(m, r1,
                sext(Machine_ReadMain8(m, GPR_GET(m, r2) + sext(imm_15, 15)),
                     8));
        break;
    case Lbu:
        EXEC_LOG("Lbu");
        GPR_SET(m, r1, Machine_ReadMain8(m, GPR_GET(m, r2) + sext(imm_15, 15)));
        break;
    case Lbd:
        EXEC_LOG("Lbd");
        REQUIRE_SUPERVISOR
        GPR_SET(m, r1,
                sext(Machine_ReadData8(m, GPR_GET(m, r2) + sext(imm_15, 15)),
                     8));
        break;
    case Lbud:
        EXEC_LOG("Lbud");
        REQUIRE_SUPERVISOR
        GPR_SET(m, r1, Machine_ReadData8(m, GPR_GET(m, r2) + sext(imm_15, 15)));
        break;
    case Lh:
        EXEC_LOG("Lh");
        GPR_SET(m, r1,
                sext(Machine_ReadMain16(m, GPR_GET(m, r2) + sext(imm_15, 15)),
                     16));
        break;
    case Lhu:
        EXEC_LOG("Lhu");
        GPR_SET(m, r1,
                Machine_ReadMain16(m, GPR_GET(m, r2) + sext(imm_15, 15)));
        break;
    case Lhd:
        EXEC_LOG("Lhd");
        REQUIRE_SUPERVISOR
        GPR_SET(m, r1,
                sext(Machine_ReadData16(m, GPR_GET(m, r2) + sext(imm_15, 15)),
                     16));
        break;
    case Lhud:
        EXEC_LOG("Lhud");
        REQUIRE_SUPERVISOR
        GPR_SET(m, r1,
                Machine_ReadData16(m, GPR_GET(m, r2) + sext(imm_15, 15)));
        break;
    case Lw:
        EXEC_LOG("Lw");
        GPR_SET(m, r1,
                Machine_ReadMain32(m, GPR_GET(m, r2) + sext(imm_15, 15)));
        break;
    case Lwd:
        EXEC_LOG("Lwd");
        REQUIRE_SUPERVISOR
        GPR_SET(m, r1,
                Machine_ReadData32(m, GPR_GET(m, r2) + sext(imm_15, 15)));
        break;
    case Muli:
        EXEC_LOG("Muli");
        GPR_SET(m, r1, GPR_GET(m, r2) * sext(imm_15, 15));
        break;
    case Mulih:
        EXEC_LOG("Mulih");
        GPR_SET(m, r1, (uint64_t)(GPR_GET(m, r2) * sext(imm_15, 15)) >> 32);
        break;
    case Idivi:
        EXEC_LOG("Idivi");
        if (imm_15 == 0) {
            m->cpu.exception = EXCEPTION_DIVISION_ZERO;
            exception        = 1;
            break;
        }
        GPR_SET(m, r1, GPR_GET(m, r2) / sext(imm_15, 15));
        break;
    case Addi:
        EXEC_LOG("Addi");
        GPR_SET(m, r1, GPR_GET(m, r2) + sext(imm_15, 15));
        break;
    case Subi:
        EXEC_LOG("Subi");
        GPR_SET(m, r1, GPR_GET(m, r2) - sext(imm_15, 15));
        break;
    case Ori:
        EXEC_LOG("Ori");
        GPR_SET(m, r1, GPR_GET(m, r2) | imm_15);
        break;
    case Andi:
        EXEC_LOG("Andi");
        GPR_SET(m, r1, GPR_GET(m, r2) & imm_15);
        break;
    case Xori:
        EXEC_LOG("Xori");
        GPR_SET(m, r1, GPR_GET(m, r2) ^ imm_15);
        break;
    case ShiRa:
        EXEC_LOG("ShiRa");
        GPR_SET(m, r1, (int32_t)GPR_GET(m, r2) >> (imm_15 % 32));
        break;
    case ShiRl:
        EXEC_LOG("ShiRl");
        GPR_SET(m, r1, GPR_GET(m, r2) >> (imm_15 % 32));
        break;
    case ShiLl:
        EXEC_LOG("ShiLl");
        GPR_SET(m, r1, GPR_GET(m, r2) << (imm_15 % 32));
        break;
    case Slti:
        EXEC_LOG("Slti");
        GPR_SET(m, r1, (int32_t)GPR_GET(m, r2) < (int32_t)sext(imm_15, 15));
        break;
    case Sltiu:
        EXEC_LOG("Sltiu");
        GPR_SET(m, r1, GPR_GET(m, r2) < imm_15);
        break;
    case Add:
        EXEC_LOG("Add");
        GPR_SET(m, r1, GPR_GET(m, r2) + GPR_GET(m, r3));
        break;
    case Sub:
        EXEC_LOG("Sub");
        GPR_SET(m, r1, GPR_GET(m, r2) - GPR_GET(m, r3));
        break;
    case Idiv:
        EXEC_LOG("Idiv");
        if (GPR_GET(m, r4) == 0) {
            m->cpu.exception = EXCEPTION_DIVISION_ZERO;
            exception        = 1;
            break;
        }

        if ((vector & 1) == 0) {
            GPR_SET(m, r1, (int32_t)GPR_GET(m, r3) / (int32_t)GPR_GET(m, r4));
            GPR_SET(m, r2, (int32_t)GPR_GET(m, r3) % (int32_t)GPR_GET(m, r4));
        } else {
            GPR_SET(m, r1, GPR_GET(m, r3) / GPR_GET(m, r4));
            GPR_SET(m, r2, GPR_GET(m, r3) % GPR_GET(m, r4));
        }
        break;
    case Mul: {
        EXEC_LOG("Mul");
        if ((vector & 1) == 0) {
            // IMULU
            const u64 value = (u64)GPR_GET(m, r3) * (u64)GPR_GET(m, r4);
            // TODO:Document this implementation
            GPR_SET(m, r1, value >> 32);
            GPR_SET(m, r2, value);
        } else {
            const int64_t value =
                (int64_t)GPR_GET(m, r3) * (int64_t)GPR_GET(m, r4);
            // TODO: Document this implementation
            GPR_SET(m, r1, value >> 32);
            GPR_SET(m, r2, value);
        }
    } break;
    case Or:
        EXEC_LOG("Or");
        GPR_SET(m, r1, GPR_GET(m, r2) | GPR_GET(m, r3));
        break;
    case And:
        EXEC_LOG("And");
        GPR_SET(m, r1, GPR_GET(m, r2) & GPR_GET(m, r3));
        break;
    case Xor:
        EXEC_LOG("Xor");
        GPR_SET(m, r1, GPR_GET(m, r2) ^ GPR_GET(m, r3));
        break;
    case Not:
        EXEC_LOG("Not");
        GPR_SET(m, r1, ~GPR_GET(m, r2));
        break;
#ifdef __GNUC__
    case Ctz:
        EXEC_LOG("Ctz");
        GPR_SET(m, r1, __builtin_ctz(GPR_GET(m, r2)));
        break;
    case Clz:
        EXEC_LOG("Clz");
        GPR_SET(m, r1, __builtin_clz(GPR_GET(m, r2)));
        break;
    case Popcount:
        EXEC_LOG("Popcount");
        GPR_SET(m, r1, __builtin_popcount(GPR_GET(m, r2)));
        break;
#else
    case Ctz: {
        EXEC_LOG("Ctz");
        u32 v = GPR_GET(m, r2); // 32-bit word input to count zero bits on right
        u32 c = 32; // c will be the number of zero bits on the right
        v &= -(signed)(v);
        if (v) c--;
        if (v & 0x0000FFFF) c -= 16;
        if (v & 0x00FF00FF) c -= 8;
        if (v & 0x0F0F0F0F) c -= 4;
        if (v & 0x33333333) c -= 2;
        if (v & 0x55555555) c -= 1;
        GPR_SET(m, r1, c);
        break;
    }
    case Clz:
        EXEC_LOG("Clz");
        GPR_SET(m, r1, __lzcnt(GPR_GET(m, r2)));
        break;
    case Popcount:
        EXEC_LOG("Popcount");
        GPR_SET(m, r1, __popcnt(GPR_GET(m, r2)));
        break;
#endif
    case ShRa:
        EXEC_LOG("ShRa");
        GPR_SET(m, r1, (int32_t)GPR_GET(m, r2) >> (GPR_GET(m, r3) % 32));
        break;
    case ShRl:
        EXEC_LOG("ShRl");
        GPR_SET(m, r1, GPR_GET(m, r2) >> (GPR_GET(m, r3) % 32));
        break;
    case ShLl:
        EXEC_LOG("ShLl");
        GPR_SET(m, r1, GPR_GET(m, r2) << (GPR_GET(m, r3) % 32));
        break;
    case Ror:
        EXEC_LOG("Ror");
        GPR_SET(m, r1, GPR_GET(m, r2) * (GPR_GET(m, r3) % 32)); // FIXME not
                                                                // implemente)d
        printf("ROR: NOT IMPLEMENTED\n");
        break;
    case Rol:
        EXEC_LOG("Rol");
        GPR_SET(m, r1, GPR_GET(m, r2) * (GPR_GET(m, r3) % 32)); // FIXME: not
                                                                // implemente)d
        printf("ROL: NOT IMPLEMENTED\n");

        break;
    case Sb:
        EXEC_LOG("Sb");
        Machine_WriteMain8(m, GPR_GET(m, r2) + sext(imm_15, 15),
                           GPR_GET(m, r1));
        break;
    case Sbd:
        EXEC_LOG("Sbd");
        REQUIRE_SUPERVISOR;
        Machine_WriteData8(m, GPR_GET(m, r2) + sext(imm_15, 15),
                           GPR_GET(m, r1));
        break;
    case Sh:
        EXEC_LOG("Sh");
        Machine_WriteMain16(m, GPR_GET(m, r2) + sext(imm_15, 15),
                            GPR_GET(m, r1));
        break;
    case Shd:
        EXEC_LOG("Shd");
        REQUIRE_SUPERVISOR;
        Machine_WriteData16(m, GPR_GET(m, r2) + sext(imm_15, 15),
                            GPR_GET(m, r1));
        break;
    case Sw:
        EXEC_LOG("Sw");
        Machine_WriteMain32(m, GPR_GET(m, r2) + sext(imm_15, 15),
                            GPR_GET(m, r1));
        break;
    case Swd:
        EXEC_LOG("Swd");
        REQUIRE_SUPERVISOR;
        Machine_WriteData32(m, GPR_GET(m, r2) + sext(imm_15, 15),
                            GPR_GET(m, r1));
        break;
    default:
        EXEC_LOG("no instruction");
        break;
    }

    switch (group) {
    case SYS:
        m->cpu.cycles += (CYCLES_SYS);
        break;
    case MEM:
        m->cpu.cycles += (CYCLES_MEM);
        break;
    case JU:
        m->cpu.cycles += (CYCLES_JU);
        break;
    case BRANCH:
        m->cpu.cycles += (CYCLES_BRANCH);
        break;
    case LOAD:
        m->cpu.cycles += (CYCLES_LOAD);
        break;
    case ALUI:
        m->cpu.cycles += (CYCLES_ALUI);
        break;
    case ALUR:
        m->cpu.cycles += (CYCLES_ALUR);
        break;
    case STORE:
        m->cpu.cycles += (CYCLES_STORE);
        break;
    default:
        break;
    }

    return exception;
}

void Cpu_Reset(TaleaMachine *m, TaleaConfig *config, bool is_restart)
{
    m->cpu = (CpuState){
        // Register file
        .pc        = 0xFFE000, // TODO: really think about this approach...
        .virtualPc = 0,
        .status    = 0,
        .gpr       = { 0 },
        .ssp       = 0,
        .usp       = 0,
        // Clock
        .frequency = (HZ * 10) * config->frequency,
        .cycles    = 0,
        .ticks     = 0,
        // Control lines
        .poweroff = 0,
        .restart  = 0,
    };

    SR_SET_SUPERVISOR(m->cpu.status, 1U);
    SR_SET_INTERRUPT(m->cpu.status, 1U);
    SR_SET_MMU(m->cpu.status, 0);
    SR_SET_PRIORITY(m->cpu.status, 7U);
    SR_SET_IVT(m->cpu.status, 0);
    SR_SET_PDT(m->cpu.status, 0);
}

void Cpu_RunCycles(TaleaMachine *m, u32 cycles)
{
    u64 target_cycles = cycles + m->cpu.cycles;

    while (m->cpu.cycles < target_cycles) {
        if (Execute(m)) Exception(m, m->cpu.exception, 0);
        CheckInterrupts(m);
        // TimerTick();
        m->cpu.ticks++;
        m->cpu.cycles++;
    }
}
