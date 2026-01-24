/*******************************************************************************************
 *
 *   Taleä System cpu emulation
 *
 ********************************************************************************************/

#include "bus.h"
#include "cpu.h"
#include "talea.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static inline u32 sext(u32 n, u8 bits)
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
    TALEA_LOG_TRACE("[TRACE] R%d: %08x | R%d: %08x | R%d: %08x | R%d: %08x \n", r1, GPR_GET(m, r1),
                    r2, GPR_GET(m, r2), r3, GPR_GET(m, r3), r4, GPR_GET(m, r4));
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
    ON_FAULT_RETURN_M
}

static inline u32 PopW(TaleaMachine *m)
{
    CpuState *cpu  = &m->cpu;
    u32       word = Machine_ReadMain32(m, cpu->gpr[x2]);
    ON_FAULT_RETURN0
    cpu->gpr[x2] += 4;
    return word;
}

// #define DEBUG_LOG_EXCEPTIONS
// #define DEBUG_LOG_INTERRUPTS

static void Exception(TaleaMachine *m, u8 vector, bool is_interrupt)
{
    // TODO: handle double and  triple fault
    CpuState *cpu = &m->cpu;

    if (is_interrupt && !SR_GET_INTERRUPT(m->cpu.status)) return;

    // m->cpu.is_processing_exception = is_interrupt ? false : true;
    m->cpu.exception = vector;

    // normal exception

    const u32 pc   = GET_PC(m);
    const u32 stat = m->cpu.status;
    m->cpu.status &= ~CPU_STATUS_DEBUG_STEP;
    SetSupervisor(m);
    PushW(m, pc);
    PushW(m, stat);

    if (vector == EXCEPTION_BUS_ERROR || vector == EXCEPTION_ADDRESS_ERROR ||
        vector == EXCEPTION_PAGE_FAULT) {
        // FAULT
        u32 offending = Machine_ReadMain32(m, GET_PC(m) - 4);
        ON_FAULT_RETURN // andle double and triple fault
            PushW(m, offending);

#ifdef DEBUG_LOG_EXCEPTIONS
        TALEA_LOG_TRACE("FAULT: %u\n", vector);
#endif
    }

    if (is_interrupt) {
        SR_SET_PRIORITY(m->cpu.status, m->cpu.current_ipl);
    }

#if TALEA_IVT_FIXED
    const u32 handler_addr = Machine_ReadData32(m, TALEA_IVT_BASE + ((u16)vector << 2));
    SET_PC(m, handler_addr); // This has to set the virtual pc if mmu enabled

#ifdef DEBUG_LOG_EXCEPTIONS
    TALEA_LOG_TRACE("Exception 0x%x, jumping to 0x%08x\n", vector, handler_addr);
#endif

#else
#error "Use fixed IVT for now please"
#endif

    // m->cpu.is_processing_exception = false;
    m->cpu.exception = EXCEPTION_NONE;

    m->cpu.cycles += CYCLES_EXCEPTION;
}

void Machine_RaiseInterrupt(TaleaMachine *m, u8 vector, u8 priority)
{
    m->cpu.pending_interrupts[priority] = vector;

#ifdef DEBUG_LOG_INTERRUPTS
    TALEA_LOG_TRACE("Interrupt: 0x%x, priority: %u, pending: %u, cpu: %u\n", vector, priority,
                    m->cpu.highest_pending_interrupt, SR_GET_PRIORITY(m->cpu.status));
#endif

    if (priority > m->cpu.highest_pending_interrupt) m->cpu.highest_pending_interrupt = priority;
}

static bool CheckInterrupts(TaleaMachine *m)
{
    // It is ugly, but naive, to check this here in this way
    if (m->storage.current_tps->just_inserted) {
        puts("Raise TPS INSERTED \n");
        m->storage.current_tps->just_inserted = false;
        Machine_RaiseInterrupt(m, INT_TPS_INSERTED, PRIORITY_STORAGE_INTERRUPT);
    }

    if (m->storage.current_tps->just_ejected) {
        puts("Raise TPS Ejected \n");
        m->storage.current_tps->just_ejected = false;
        Machine_RaiseInterrupt(m, INT_TPS_EJECTED, PRIORITY_STORAGE_INTERRUPT);
    }

    u8 old_tps_status = atomic_fetch_and(&m->storage.current_tps->status, ~STOR_STATUS_DONE);
    if (old_tps_status & STOR_STATUS_DONE) {
        puts("Raise TPS FINISH\n");
        Machine_RaiseInterrupt(m, INT_TPS_FINISH, PRIORITY_STORAGE_INTERRUPT);
    }

    u8 old_hcs_status = atomic_fetch_and(&m->storage.hcs.status, ~STOR_STATUS_DONE);
    if (old_tps_status & STOR_STATUS_DONE) {
        Machine_RaiseInterrupt(m, INT_HCS_FINISH, PRIORITY_STORAGE_INTERRUPT);
    }

    m->cpu.pending_ipl = m->cpu.highest_pending_interrupt;
    u8 pending         = m->cpu.pending_ipl;
    u8 current         = m->cpu.current_ipl;

    if ((m->cpu.pending_ipl != 0) &&
        (pending > SR_GET_PRIORITY(m->cpu.status) || ((pending == 7) && (pending >= current)))) {
        m->cpu.current_ipl = m->cpu.pending_ipl;
        // acknowledge interrupt, maybe fail here
        u8 vector = m->cpu.pending_interrupts[m->cpu.current_ipl];
        m->cpu.pending_interrupts[m->cpu.current_ipl] = 0;
        while (m->cpu.highest_pending_interrupt > 0 &&
               !m->cpu.pending_interrupts[m->cpu.highest_pending_interrupt]) {
            m->cpu.highest_pending_interrupt -= 1;
        }

        Exception(m, vector, true);
        return true;
    }

    if (pending < current) m->cpu.current_ipl = m->cpu.pending_ipl;
    return false;
}

/* fetch, decode, execute cycle */
// TODO: we aren't setting dirty bits nor accounting for the x flag with MMU.
// It's too slow anyways, we have to change implementation

#if TALEA_WITH_MMU


static void MMU_FlushTLB(TaleaMachine *m)
{
    memset(m->cpu.tlb, 0, sizeof(m->cpu.tlb));
}

static inline bool MMU_VerifyPerms(u16 perm, enum MemAccessType access_type, bool is_supervisor)
{
    if (!is_supervisor && !(perm & PTE_U)) return false;

    switch (access_type) {
    case ACCESS_READ: return perm & PTE_R;
    case ACCESS_WRITE: return perm & PTE_W;
    case ACCESS_EXEC: return perm & PTE_X;

    default: return false;
    }
}

u32 MMU_TranslateAddr(TaleaMachine *m, u32 vaddr, enum MemAccessType access_type)
{
    CpuState *cpu    = &m->cpu;
    u32       vpn    = vaddr >> 12;
    TLBEntry *cached = &cpu->tlb[vpn];

    if (cached->valid) {
        if (!MMU_VerifyPerms(cached->perm, access_type, SR_GET_SUPERVISOR(cpu->status))) {
            m->cpu.fault_addr = vaddr;
            m->cpu.exception  = EXCEPTION_ACCESS_VIOLATION_TALEA;
            return 0;
        }

        if (access_type != ACCESS_WRITE || (cached->perm & PTE_D)) {
            return cached->phys | (vaddr & 0xFFF);
        }
    }

    u32 phys = MMU_WalkPageTable(m, vaddr, access_type);
    if (m->cpu.exception != EXCEPTION_NONE) return 0;

    cached->phys  = phys & 0xFFF000;
    cached->perm  = phys & 0xFF;
    cached->valid = true;

    return phys | (vaddr & 0xFFF);
}



static u32 MMU_WalkPageTable(TaleaMachine *m, u32 vaddr, enum MemAccessType access_type)
{
    u32 vpn1 = (vaddr >> 22) & 0x3FF;
    u32 vpn0 = (vaddr >> 12) & 0x3FF;

    u16 root = SR_GET_PDT(m->cpu.status);
    u32 pte1 = Machine_ReadData32(m, root + (vpn1 * 4));

    if (!(pte1 & PTE_V)) {
        m->cpu.fault_addr = vaddr;
        m->cpu.exception  = EXCEPTION_PAGE_FAULT;
        return 0;
    }

    u32 leaf      = pte1 & 0xFFF000;
    u32 pte0_addr = leaf + (vpn0 * 4);
    u32 pte0      = Machine_ReadMain32Physical(m, pte0_addr);

    if (!(pte0 & PTE_V)) {
        m->cpu.fault_addr = vaddr;
        m->cpu.exception  = EXCEPTION_PAGE_FAULT;
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
    if (size > 0 && (vaddr > (0xFFFFFFFFU - size + 1))) {
        m->cpu.exception  = EXCEPTION_ACCESS_VIOLATION_TALEA;
        m->cpu.fault_addr = vaddr;
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
    if (size > 0 && (vaddr > (0xFFFFFFFFU - size + 1))) {
        m->cpu.exception  = EXCEPTION_ACCESS_VIOLATION_TALEA;
        m->cpu.fault_addr = vaddr;
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

static inline u32 Fetch(TaleaMachine *m)
{
    CpuState *cpu         = &m->cpu;
    const u32 instruction = Machine_ReadMain32(m, GET_PC(m));
    ON_FAULT_RETURN0
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
            m->cpu.exception = EXCEPTION_ILLEGAL_INSTRUCTION_TALEA;
        }
    } else {
        m->cpu.exception = EXCEPTION_ILLEGAL_INSTRUCTION_TALEA;
    }

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
        .pc        = TALEA_FIRMWARE_ADDRESS, // TODO: really think about this approach...
        .virtualPc = 0,
        .status    = 0,
        .gpr       = { 0 },
        .ssp       = 0,
        .usp       = 0,
        // Clock
        .frequency = (HZ * 10) * config->frequency,
        .cycles    = 0,
        .ticks     = 0,
        // Interrupts
        .exception               = EXCEPTION_NONE,
        .is_processing_exception = false,
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
        bool debug_step    = m->cpu.status & CPU_STATUS_DEBUG_STEP;
        u64  cycles_before = m->cpu.cycles;
        bool exception     = Execute(m);

        if (exception) {
            Exception(m, m->cpu.exception, false);
            continue;
        }

        Timer_Update(m, m->cpu.cycles - cycles_before);
        bool interrupt = CheckInterrupts(m);

        if (debug_step && !interrupt) Exception(m, EXCEPTION_DEBUG_STEP, false);
    }
}
