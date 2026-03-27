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

static const char *ExceptionNames[] = {
    [EXCEPTION_RESET]                     = "EXCEPTION_RESET",
    [EXCEPTION_BUS_ERROR]                 = "EXCEPTION_BUS_ERROR",
    [EXCEPTION_ADDRESS_ERROR]             = "EXCEPTION_ADDRESS_ERROR",
    [EXCEPTION_ILLEGAL_INSTRUCTION_TALEA] = "EXCEPTION_ILLEGAL_INSTRUCTION",
    [EXCEPTION_DIVISION_ZERO]             = "EXCEPTION_DIVISION_ZERO",
    [EXCEPTION_PRIVILEGE_VIOLATION]       = "EXCEPTION_PRIVILEGE_VIOLATION",
    [EXCEPTION_PAGE_FAULT]                = "EXCEPTION_PAGE_FAULT",
    [EXCEPTION_ACCESS_VIOLATION_TALEA]    = "EXCEPTION_ACCESS_VIOLATION",
    [EXCEPTION_DEBUG_STEP]                = "EXCEPTION_DEBUG_STEP",
    [EXCEPTION_OVERSPILL]                 = "EXCEPTION_OVERSPILL",
    [EXCEPTION_UNDERSPILL]                = "EXCEPTION_UNDERSPILL",

    [INT_SER_RX]         = "INT_SER_RX",
    [INT_KBD_CHAR]       = "INT_KBD_CHAR",
    [INT_KBD_SCAN]       = "INT_KBD_SCAN",
    [INT_TPS_FINISH]     = "INT_TPS_FINISH",
    [INT_HCS_FINISH]     = "INT_HCS_FINISH",
    [INT_TIMER_TIMEOUT]  = "INT_TIMER_TIMEOUT",
    [INT_TIMER_INTERVAL] = "INT_TIMER_INTERVAL",
    [INT_VIDEO_VBLANK]   = "INT_VIDEO_VBLANK",
    [INT_MOUSE_PRESSED]  = "INT_MOUSE_PRESSED",
    [INT_TPS_EJECTED]    = "INT_TPS_EJECTED",
    [INT_TPS_INSERTED]   = "INT_TPS_INSERTED",
    [INT_AUDIO_NOTE_END] = "INT_AUDIO_NOTE_END",
};

static inline const char *getExceptionName(u8 vector)
{
    if (vector < EXCEPTION_RESET || (vector > EXCEPTION_UNDERSPILL && vector < INT_SER_RX) ||
        vector > INT_AUDIO_NOTE_END || vector == 2) {
        return "Unknown";
    } else {
        return ExceptionNames[vector];
    }
}

static inline u32 sext(u32 n, u8 bits)
{
    int mask = 1U << (bits - 1);
    return (n ^ mask) - mask;
}

static inline Register GPR_GET(TaleaMachine *m, u8 reg)
{
    return (reg == 0) ? 0 : m->cpu.gpr[(m->cpu.cwp * 32) + reg];
}

static inline void GPR_SET(TaleaMachine *m, u8 reg, u32 value)
{
    m->cpu.gpr[(m->cpu.cwp * 32) + reg] = (reg == 0) ? 0 : value;
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


#define NOTRACE
static inline void trace(TaleaMachine *m, u8 r1, u8 r2, u8 r3, u8 r4)
{
    #ifndef NOTRACE
    TALEA_LOG_TRACE(
        "[TRACE] R%d: %08x | R%d: %08x | R%d: %08x | R%d: %08x --- at %08x (ra: %08x) (sp: %08x) cwp: %d\n",
        r1, GPR_GET(m, r1), r2, GPR_GET(m, r2), r3, GPR_GET(m, r3), r4, GPR_GET(m, r4), GET_PC(m),
        GPR_GET(m, x1), GPR_GET(m, x2), m->cpu.cwp);
    #endif
}

static inline void SetSupervisor(TaleaMachine *m)
{
    if (SR_GET_SUPERVISOR(m->cpu.status)) return;

    SR_SET_SUPERVISOR(m->cpu.status, 1U);
    m->cpu.usp = GPR_GET(m, x2);
    GPR_SET(m, x2, m->cpu.ssp);
}

static inline void PushW(TaleaMachine *m, u32 word)
{
    u32 sp = GPR_GET(m, x2);
    GPR_SET(m, x2, sp - 4);
    Machine_WriteMain32(m, sp - 4, word);
    ON_FAULT_RETURN_M
}

static inline u32 PopW(TaleaMachine *m)
{
    CpuState *cpu  = &m->cpu;
    u32       sp   = GPR_GET(m, x2);
    u32       word = Machine_ReadMain32(m, sp);
    ON_FAULT_RETURN0
    GPR_SET(m, x2, sp + 4);
    return word;
}

#define DEBUG_LOG_EXCEPTIONS
//#define DEBUG_LOG_SYSCALLS
//#define DEBUG_LOG_INTERRUPTS

static void Exception(TaleaMachine *m, u8 vector, int trap_type)
{
    CpuState *cpu = &m->cpu;

    // double fault
    if (cpu->isProcessingException && trap_type == TRAP_TYPE_EXCEPTION) {
        TALEA_LOG_ERROR("CPU Halted: double fault\n");
        m->cpu.poweroff = true;
        return;
    }

    // if (trap_type == TRAP_TYPE_INTERRUPT && !SR_GET_INTERRUPT(m->cpu.status)) return;
    //  m->cpu.isProcessingException = !is_interrupt;

    // normal exception

    int exception_cached = m->cpu.exception;
    if (trap_type == TRAP_TYPE_EXCEPTION) {
        m->cpu.exception = EXCEPTION_NONE;

    } else if (trap_type == TRAP_TYPE_INTERRUPT) {
        m->cpu.interrupt = vector;
    }

    u32 pc         = GET_PC(m);
    pc             = trap_type != TRAP_TYPE_EXCEPTION ? pc : pc - 4;
    const u32 stat = cpu->status;
    m->cpu.status &= ~CPU_STATUS_DEBUG_STEP;

    if (trap_type != TRAP_TYPE_INTERRUPT) SR_CLEAR_INTERRUPT(m->cpu.status);

    SetSupervisor(m);
    PushW(m, pc);
    PushW(m, stat);
    if (cpu->exception != EXCEPTION_NONE) {
        TALEA_LOG_TRACE("Exception occured while pushing pc and status!, %s (0x%x)\n",
                        getExceptionName(cpu->exception), cpu->exception);
        m->cpu.poweroff = true;
        return;
    }

    if (trap_type == TRAP_TYPE_INTERRUPT) {
        SR_SET_PRIORITY(m->cpu.status, m->cpu.currentIpl);
    }

    const u32 handler_addr = Machine_ReadData32(m, TALEA_DATA_IVT_BASE + ((u16)vector << 2));
    SET_PC(m, handler_addr); // This has to set the virtual pc if mmu enabled

#ifdef DEBUG_LOG_EXCEPTIONS
    #ifndef DEBUG_LOG_SYSCALLS
    if (trap_type == TRAP_TYPE_EXCEPTION) {
        TALEA_LOG_TRACE("%s %s (0x%x), jumping from 0x%06x to 0x%06x, fault at: 0x%06x, cwp: %d, spilled: %d, ra: 0x%08x\n",
        trap_type == TRAP_TYPE_INTERRUPT ? "Interrupt" :
        trap_type == TRAP_TYPE_SYSCALL   ? "Syscall" :
                                           "Exception",
        getExceptionName(vector), vector, trap_type != TRAP_TYPE_EXCEPTION ? pc - 4 : pc,
        handler_addr, cpu->faultAddr, cpu->cwp, cpu->spilledWindows, GPR_GET(m, x1));
    }
    #else
    TALEA_LOG_TRACE(
        "%s %s (0x%x), jumping from 0x%06x to 0x%06x, fault at: 0x%06x, cwp: %d, spilled: %d, ra: 0x%08x\n",
        trap_type == TRAP_TYPE_INTERRUPT ? "Interrupt" :
        trap_type == TRAP_TYPE_SYSCALL   ? "Syscall" :
                                           "Exception",
        getExceptionName(vector), vector, trap_type != TRAP_TYPE_EXCEPTION ? pc - 4 : pc,
        handler_addr, cpu->faultAddr, cpu->cwp, cpu->spilledWindows, GPR_GET(m, x1));
    #endif
#endif

    m->cpu.exception     = EXCEPTION_NONE;
    m->cpu.LastException = exception_cached;
    m->cpu.cycles += CYCLES_EXCEPTION;
}

void Machine_RaiseInterrupt(TaleaMachine *m, u8 vector, u8 priority)
{
    m->cpu.pendingInterrupts[priority] = vector;

#ifdef DEBUG_LOG_INTERRUPTS
    TALEA_LOG_TRACE("Raised interrupt: %s (0x%x), priority: %u, pending: %u, cpu: %u, eabled: %d\n",
                    ExceptionNames[vector], vector, priority, m->cpu.highestPendingInterrupt,
                    SR_GET_PRIORITY(m->cpu.status), SR_GET_INTERRUPT(m->cpu.status));
#endif

    if (priority > m->cpu.highestPendingInterrupt) m->cpu.highestPendingInterrupt = priority;
}

static bool CheckInterrupts(TaleaMachine *m)
{
    if (!SR_GET_INTERRUPT(m->cpu.status)) return false;

    // It is ugly, but naive, to check this here in this way
    if (m->storage.currentTps->justInserted) {
        m->storage.currentTps->justInserted = false;
        Machine_RaiseInterrupt(m, INT_TPS_INSERTED, PRIORITY_STORAGE_INTERRUPT);
    }

    if (m->storage.currentTps->justEjected) {
        m->storage.currentTps->justEjected = false;
        Machine_RaiseInterrupt(m, INT_TPS_EJECTED, PRIORITY_STORAGE_INTERRUPT);
    }

    u8 old_tps_status = atomic_fetch_and(&m->storage.currentTps->status, ~STORAGE_STATUS_DONE);
    if (old_tps_status & STORAGE_STATUS_DONE) {
        Machine_RaiseInterrupt(m, INT_TPS_FINISH, PRIORITY_STORAGE_INTERRUPT);
    }

    u8 old_hcs_status = atomic_fetch_and(&m->storage.hcs.status, ~STORAGE_STATUS_DONE);
    if (old_hcs_status & STORAGE_STATUS_DONE) {
        Machine_RaiseInterrupt(m, INT_HCS_FINISH, PRIORITY_STORAGE_INTERRUPT);
    }

    m->cpu.pendingIpl = m->cpu.highestPendingInterrupt;
    u8 pending        = m->cpu.pendingIpl;
    u8 current        = m->cpu.currentIpl;

    if ((m->cpu.pendingIpl != 0) &&
        (pending > SR_GET_PRIORITY(m->cpu.status) || ((pending == 7) && (pending >= current)))) {
        m->cpu.currentIpl = m->cpu.pendingIpl;
        // acknowledge interrupt, maybe fail here
        u8 vector                                   = m->cpu.pendingInterrupts[m->cpu.currentIpl];
        m->cpu.pendingInterrupts[m->cpu.currentIpl] = 0;
        while (m->cpu.highestPendingInterrupt > 0 &&
               !m->cpu.pendingInterrupts[m->cpu.highestPendingInterrupt]) {
            m->cpu.highestPendingInterrupt -= 1;
        }

        Exception(m, vector, TRAP_TYPE_INTERRUPT);
        return true;
    }

    if (pending < current) m->cpu.currentIpl = m->cpu.pendingIpl;
    return false;
}

/* fetch, decode, execute cycle */

#if TALEA_WITH_MMU

static u32 MMU_WalkPageTable(TaleaMachine *m, u32 vaddr, enum MemAccessType access_type);

void MMU_FlushTLB(TaleaMachine *m)
{
    memset(m->cpu.tlb, 0, sizeof(m->cpu.tlb));
}

static inline bool MMU_VerifyPerms(u16 perm, enum MemAccessType access_type, bool is_supervisor)
{
    if (!is_supervisor && !(perm & PTE_U)) {
        TALEA_LOG_TRACE("Required supervisor (%x)\n", perm);
        return false;
    }

    switch (access_type) {
    case ACCESS_READ: return perm & PTE_R;
    case ACCESS_WRITE: return perm & PTE_W;
    case ACCESS_EXEC: return perm & PTE_X;

    default: return false;
    }
}

static bool enabled = false;
u32         MMU_TranslateAddr(TaleaMachine *m, u32 vaddr, enum MemAccessType access_type)
{
    CpuState *cpu    = &m->cpu;
    u32       vpn    = (vaddr & 0xffffff) >> 12;
    TLBEntry *cached = &cpu->tlb[vpn];

    if (!enabled) {
        TALEA_LOG_TRACE("MMU Switched ON. Translating 0x%08x virtual ", vaddr);
    }

    if (cached->valid) {
        if (!MMU_VerifyPerms(cached->perm, access_type, SR_GET_SUPERVISOR(cpu->status))) {
            m->cpu.faultAddr  = vaddr;
            m->cpu.faultCause = CAUSE_PERM | access_type;
            m->cpu.exception  = EXCEPTION_ACCESS_VIOLATION_TALEA;
            TALEA_LOG_TRACE("error no perm to access 0x%08x, access type: %s (%d)\n", vaddr,
                            access_type == ACCESS_EXEC  ? "X" :
                            access_type == ACCESS_WRITE ? "W" :
                                                          "R",
                            access_type);
            return 0;
        }

        if (access_type != ACCESS_WRITE || (cached->perm & PTE_D)) {
            if (!enabled) {
                TALEA_LOG_TRACE("(cached) 0x%08x\n", cached->phys | (vaddr & 0xfff));
                enabled = true;
            }
            return cached->phys | (vaddr & 0xFFF);
        }
    }

    u32 phys = MMU_WalkPageTable(m, vaddr, access_type);
    if (m->cpu.exception != EXCEPTION_NONE) {
        if (!enabled) {
            TALEA_LOG_TRACE("Exception %d\n", m->cpu.exception);
            enabled = true;
        }
        return 0;
    }
    cached->phys  = phys & 0xFFF000;
    cached->perm  = phys & 0xFF;
    cached->valid = true;

    if (!enabled) {
        TALEA_LOG_TRACE("-> 0x%08x (phys: %x, vaddr: %x)\n", (phys & 0xFFF000) | (vaddr & 0xFFF),
                        phys, vaddr & 0xFFF);
        enabled = true;
    }

    return (phys & 0xFFF000) | (vaddr & 0xFFF);
}

static u32 MMU_WalkPageTable(TaleaMachine *m, u32 vaddr, enum MemAccessType access_type)
{
    u32 vpn1 = (vaddr >> 22) & 0x3FF;
    u32 vpn0 = (vaddr >> 12) & 0x3FF;

    u16 root = SR_GET_PDT(m->cpu.status) << 4;
    u32 pte1 = Machine_ReadData32(m, root + (vpn1 * 4));

    if (!enabled)
        TALEA_LOG_TRACE("\n\tPTE1: (root: 0x%04x) 0x%08x (valid: %d)\n", root, pte1, pte1 & PTE_V);

    if (!(pte1 & PTE_V)) {
        m->cpu.faultAddr  = vaddr;
        m->cpu.faultCause = CAUSE_UNMAPPED | access_type;
        m->cpu.exception  = EXCEPTION_PAGE_FAULT;
        TALEA_LOG_TRACE("Page Fault cause: root mapping not valid\n");
        TALEA_LOG_TRACE("\tPTE1: (root: 0x%04x, vpn1: 0x%x) 0x%08x (valid: %d) accessing: 0x%08x\n",
                        root, vpn1, pte1, pte1 & PTE_V, vaddr);
        return 0;
    }

    u32 leaf      = pte1 & 0xFFF000;
    u32 pte0_addr = leaf + (vpn0 * 4);
    u32 pte0      = Machine_ReadMain32Physical(m, pte0_addr);

    if (!enabled)
        TALEA_LOG_TRACE("\n\tPTE0: (leaf: 0x%04x) 0x%08x (valid: %d)\n", pte0_addr, pte0,
                        pte0 & PTE_V);

    if (!(pte0 & PTE_V)) {
        m->cpu.faultAddr  = vaddr;
        m->cpu.faultCause = CAUSE_UNMAPPED | CAUSE_LEAF | access_type;
        m->cpu.exception  = EXCEPTION_PAGE_FAULT;
        TALEA_LOG_TRACE("Page Fault cause: leaf mapping not valid\n");
        TALEA_LOG_TRACE("\tPTE1: (root: 0x%04x, vpn1: 0x%x) 0x%08x (valid: %d) accessing: 0x%08x\n",
                        root, vpn1, pte1, pte1 & PTE_V, vaddr);
        TALEA_LOG_TRACE("\tPTE0: (leaf: 0x%04x, vpn0: 0x%x) 0x%08x (valid: %d) accessing 0x%08x\n",
                        leaf, vpn0, pte0, pte0 & PTE_V, vaddr);
        return 0;
    } else if (!MMU_VerifyPerms(pte0 & 0xFF, access_type, SR_GET_SUPERVISOR(m->cpu.status))) {
        m->cpu.faultAddr  = vaddr;
        m->cpu.faultCause = CAUSE_PERM | access_type;
        m->cpu.exception  = EXCEPTION_ACCESS_VIOLATION_TALEA;
        TALEA_LOG_TRACE(
            "(not cached) error no perm to access 0x%08x, access type: %s (%d) (pte0: %08x)\n",
            vaddr,
            access_type == ACCESS_EXEC  ? "X" :
            access_type == ACCESS_WRITE ? "W" :
                                          "R",
            access_type, pte0);
        return 0;
    }

    u32 new_pte0 = pte0 | PTE_A;
    if (access_type == ACCESS_WRITE) {
        new_pte0 |= PTE_D;
    }

    if (new_pte0 != pte0) {
        Machine_WriteMain32Physical(m, pte0_addr, new_pte0);
    }

    return new_pte0;
}

static bool MMU_ValidateWriteAccessRange(TaleaMachine *m, u32 vaddr, size_t size)
{
    if (size > 0 && ((u64)vaddr > (0xFFFFFFFFULL - size + 1))) {
        m->cpu.faultCause = CAUSE_UNMAPPED | ACCESS_WRITE;
        m->cpu.exception  = EXCEPTION_ACCESS_VIOLATION_TALEA;
        m->cpu.faultAddr  = vaddr;
        TALEA_LOG_TRACE("Cannot validate write access range, out of address space\n");
        return false;
    }

    if (!SR_GET_MMU(m->cpu.status)) {
        return (vaddr + size) <= TALEA_MAIN_MEM_SZ;
    }

    size_t end_addr = vaddr + size;

    size_t current = vaddr;
    while (current < end_addr) {
        u32 paddr = MMU_TranslateAddr(m, current, ACCESS_WRITE);

        if (m->cpu.exception != EXCEPTION_NONE) {
            return false;
        }

        size_t next_page = (current & 0xFFFFF000) + 0x1000;

        if (next_page > end_addr) break;
        current = next_page;
    }

    return true;
}

static bool MMU_ValidateReadAccessRange(TaleaMachine *m, u32 vaddr, size_t size)
{
    if (size > 0 && ((u64)vaddr > (0xFFFFFFFFULL - size + 1))) {
        m->cpu.faultCause = CAUSE_UNMAPPED | ACCESS_READ;
        m->cpu.exception  = EXCEPTION_ACCESS_VIOLATION_TALEA;
        m->cpu.faultAddr  = vaddr;
        TALEA_LOG_TRACE("Cannot validate write access range, out of address space\n");
        return false;
    }

    if (!SR_GET_MMU(m->cpu.status)) {
        return (vaddr + size) <= TALEA_MAIN_MEM_SZ;
    }

    size_t end_addr = vaddr + size;

    size_t current = vaddr;
    while (current < end_addr) {
        u32 paddr = MMU_TranslateAddr(m, current, ACCESS_READ);

        if (m->cpu.exception != EXCEPTION_NONE) {
            return false;
        }

        size_t next_page = (current & 0xFFFFF000) + 0x1000;

        if (next_page > end_addr) break;
        current = next_page;
    }

    return true;
}
#endif

static u32        fetch_paddr;
static u32        fetch_pc;
static inline u32 Fetch(TaleaMachine *m)
{
    CpuState *cpu = &m->cpu;
    Register  pc  = GET_PC(m);

    if (pc % 4 != 0) {
        m->cpu.exception = EXCEPTION_ADDRESS_ERROR;
        return 0;
    }

    u32 paddr = pc;

    if (SR_GET_MMU(m->cpu.status)) {
        paddr = MMU_TranslateAddr(m, pc, ACCESS_EXEC);
        ON_FAULT_RETURN0
    }

    fetch_paddr = paddr;
    fetch_pc    = pc;

    const u32 instruction = Machine_ReadMain32Physical(m, paddr);
    SET_PC(m, pc + 4);
    return instruction;
}

static inline void SetUsermode(TaleaMachine *m)
{
    if (!SR_GET_SUPERVISOR(m->cpu.status)) {
        m->cpu.ssp = GPR_GET(m, x2);
        GPR_SET(m, x2, m->cpu.usp);
    }
}

// I know this is not othodox, but keeps the file less cluttered
#include "instructions.c"

static const InstructionHandler InstructionDispatchTable[8][16] = {

 [SYS] = {
    [0]                = NULL,
    [1]                = NULL,
    [CODE_Syscall]     = instr_Syscall,
    [CODE_GsReg]       = instr_GsReg,
    [CODE_SsReg]       = instr_SsReg,
    [CODE_Sysret]      = instr_Sysret,
    [CODE_Trace]       = instr_Trace,
    [CODE_MmuToggle]   = NULL, // MmuToggle,
    [CODE_MmuMap]      = NULL, // MmuMap,
    [CODE_MmuUnmap]    = NULL, // MmuUnmap,
    [CODE_MmuStat]     = NULL, // MmuStat,
    [CODE_MmuSetPT]    = NULL, // MmuSetPT,
    [CODE_MmuUpdate]   = NULL, // MmuUpdate,
    [CODE_UmodeToggle] = NULL, // UmodeToggle,
    [CODE_MmuSwitch]   = NULL, // MmuSwitch,
    [CODE_MmuGetPT]    = NULL, // MmuGetPT,
},

 [MEM] = {
    [CODE_Copy] = instr_Copy,       [CODE_Swap] = instr_Swap, [CODE_Fill] = instr_Fill,
    [CODE_Through] = instr_Through, [CODE_From] = instr_From, [CODE_Popb] = instr_Popb,
    [CODE_Poph] = instr_Poph,       [CODE_Pop] = instr_Pop,   [CODE_Pushb] = instr_Pushb,
    [CODE_Pushh] = instr_Pushh,     [CODE_Push] = instr_Push, [CODE_Save] = instr_Save,
    [CODE_Restore] = instr_Restore, [CODE_Exch] = instr_Exch, [CODE_Slt] = instr_Slt,
    [CODE_Sltu] = instr_Sltu,
},


[    JU] = { [CODE_Jal] = instr_Jal, [CODE_Lui] = instr_Lui, [CODE_Auipc] = instr_Auipc },

 [BRANCH] = {
    [CODE_Beq] = instr_Beq, [CODE_Bne] = instr_Bne,   [CODE_Blt] = instr_Blt,
    [CODE_Bge] = instr_Bge, [CODE_Bltu] = instr_Bltu, [CODE_Bgeu] = instr_Bgeu,
},

 [LOAD] = {
    [CODE_Jalr] = instr_Jalr, [CODE_Lb] = instr_Lb, [CODE_Lbu] = instr_Lbu, [CODE_Lbd] = instr_Lbd,
    [CODE_Lbud] = instr_Lbud, [CODE_Lh] = instr_Lh, [CODE_Lhu] = instr_Lhu, [CODE_Lhd] = instr_Lhd,
    [CODE_Lhud] = instr_Lhud, [CODE_Lw] = instr_Lw, [CODE_Lwd] = instr_Lwd,
},

 [ALUI] = {
    [CODE_Muli] = instr_Muli,   [CODE_Mulih] = instr_Mulih, [CODE_Idivi] = instr_Idivi,
    [CODE_Addi] = instr_Addi,   [CODE_Subi] = instr_Subi,   [CODE_Ori] = instr_Ori,
    [CODE_Andi] = instr_Andi,   [CODE_Xori] = instr_Xori,   [CODE_ShiRa] = instr_ShiRa,
    [CODE_ShiRl] = instr_ShiRl, [CODE_ShiLl] = instr_ShiLl, [CODE_Slti] = instr_Slti,
    [CODE_Sltiu] = instr_Sltiu,
},

 [ALUR] = {
    [CODE_Add]      = instr_Add,
    [CODE_Sub]      = instr_Sub,
    [CODE_Idiv]     = instr_Idiv,
    [CODE_Mul]      = instr_Mul,
    [CODE_Or]       = instr_Or,
    [CODE_And]      = instr_And,
    [CODE_Xor]      = instr_Xor,
    [CODE_Not]      = instr_Not,
    [CODE_Ctz]      = instr_Ctz,
    [CODE_Clz]      = instr_Clz,
    [CODE_Popcount] = instr_Popcount,
    [CODE_ShRa]     = instr_ShRa,
    [CODE_ShRl]     = instr_ShRl,
    [CODE_ShLl]     = instr_ShLl,
    [CODE_Ror]      = instr_Ror,
    [CODE_Rol]      = instr_Rol,
},

 [STORE] = {
    [CODE_Sb] = instr_Sb,   [CODE_Sbd] = instr_Sbd, [CODE_Sh] = instr_Sh,
    [CODE_Shd] = instr_Shd, [CODE_Sw] = instr_Sw,   [CODE_Swd] = instr_Swd,
},


};

static const u8 InstructionCost[8] = {
    [SYS] = CYCLES_SYS,   [MEM] = CYCLES_MEM,   [JU] = CYCLES_JU,     [BRANCH] = CYCLES_BRANCH,
    [LOAD] = CYCLES_LOAD, [ALUI] = CYCLES_ALUI, [ALUR] = CYCLES_ALUR, [STORE] = CYCLES_STORE,
};

static bool Execute(TaleaMachine *m)
{
    u32 instruction = Fetch(m);
    if (m->cpu.exception != EXCEPTION_NONE) return true;

    // clang-format off
    const u8  /*u3*/   group  = (instruction >> GROUP_SHIFT) & 0b111;
    const u8  /*u8*/   opcode = ((instruction & OPCODE_MASK) >> OPCODE_SHIFT) & 0xf;
    const u8  /*u5*/   r1     = ((instruction & R1_MASK) >> R1_SHIFT) & 0x1f;
    const u8  /*u5*/   r2     = ((instruction & R2_MASK) >> R2_SHIFT) & 0x1f;
    const u8  /*u5*/   r3     = ((instruction & R3_MASK) >> R3_SHIFT) & 0x1f;
    const u8  /*u5*/   r4     = ((instruction & R4_MASK) >> R4_SHIFT) & 0x1f;
    const u8  /*u8*/   vector = instruction & VECTOR_MASK;
    const u16 /*u15*/  imm_15 = (instruction & IMM15_MASK) & 0x7fff;
    const u32 /*u20*/  imm_20 = (instruction & IMM20_MASK) & 0xfffff;
    // clang-format on

    InstructionHandler *group_table = InstructionDispatchTable[group];

    if (group_table != NULL) {
        // This is guaranteed to be != NULL
        InstructionHandler op_func = group_table[opcode];

        if (op_func != NULL) {
            op_func(m, &m->cpu, r1, r2, r3, r4, vector, imm_15, imm_20);
        } else {
            TALEA_LOG_TRACE("NULL intstruction (valid group)\n");
            m->cpu.exception = EXCEPTION_ILLEGAL_INSTRUCTION_TALEA;
        }
    } else {
        TALEA_LOG_TRACE("NULL intstruction (invalid group)\n");
        m->cpu.exception = EXCEPTION_ILLEGAL_INSTRUCTION_TALEA;
    }

    extern u8 *Bus_DebugGetMain(TaleaMachine * m);
#ifdef DEBUG_LOG_EXCEPTIONS
    if (m->cpu.exception == EXCEPTION_ILLEGAL_INSTRUCTION_TALEA) {
        TALEA_LOG_TRACE("ILLEGAL INSTRUCTION REPORT: (0x%08x) at pc: 0x%08x (phys:0x%08x) \n",
                        instruction, fetch_pc, fetch_paddr);
        TALEA_LOG_TRACE("\tgroup: %01x, opcode: %02x\n", group, opcode);
        TALEA_LOG_TRACE("\tr1: %d, r2: %d, r3: %d, r4: %d\n", r1, r2, r3, r4);
        TALEA_LOG_TRACE("\tvector: %02x, imm15: %04x, imm20: %05x\n", vector, imm_15, imm_20);
        TALEA_LOG_TRACE("\t0x%08x : 0x%08x == 0x%08x \n", fetch_paddr,
                        Machine_ReadMain32Physical(m, fetch_paddr),
                        *(u32*)(Bus_DebugGetMain(m) + fetch_paddr));
    }
#endif

    if (m->cpu.exception != EXCEPTION_NONE) {
        m->cpu.cycles += CYCLES_EXCEPTION;
        return true;
    }

    m->cpu.cycles += InstructionCost[group];
    return false;
}

#define REQUIRE_SUPERVISOR                                \
    if (!SR_GET_SUPERVISOR(m->cpu.status)) {              \
        m->cpu.exception = EXCEPTION_PRIVILEGE_VIOLATION; \
        return;                                           \
    }

void Cpu_Reset(TaleaMachine *m, TaleaConfig *config, bool is_restart)
{
    m->cpu = (CpuState){
        // Register file
        .pc     = TALEA_FIRMWARE_ADDRESS,
        .status = 0,
        .gpr    = { 0 },
        .ssp    = 0,
        .usp    = 0,
        // Clock
        .frequency = (SIRIUS_BASE_FREQ_HZ * 10) * config->frequency,
        .cycles    = 0,
        .ticks     = 0,
        // Interrupts
        .exception             = EXCEPTION_NONE,
        .isProcessingException = false,
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

    while ((m->cpu.cycles < target_cycles) && !m->cpu.poweroff) {
        bool debug_step    = m->cpu.status & CPU_STATUS_DEBUG_STEP;
        u64  cycles_before = m->cpu.cycles;
        bool exception     = Execute(m);
        m->cpu.instructionsRetired++;

        if (exception) {
            Exception(m, m->cpu.exception, TRAP_TYPE_EXCEPTION);
            continue;
        }

        Timer_Update(m, m->cpu.cycles - cycles_before);
        bool interrupt = CheckInterrupts(m);

        if (debug_step && !interrupt) Exception(m, EXCEPTION_DEBUG_STEP, TRAP_TYPE_EXCEPTION);
    }
}
