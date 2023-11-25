const std = @import("std");
const arch = @import("arch");
const memory = @import("memory");

/// A register is a memory cell 32 bits wide that lives inside the Cpu
pub const Register = enum(u5) {
    pub const count = 32;
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
};

/// The Processor Status Register keeps track of the state of the Cpu
pub const ProcessorStatus = packed struct {
    const Self = @This();
    supervisor: bool = true,
    interrupt_enable: bool = false,
    mmu_enable: bool = false,
    priority: u3 = 7,
    ivt: u6 = 62,
    pdt: u8 = 255,
    reserved: u12 = 0,

    pub fn fromu32(value: u32) ProcessorStatus {
        return ProcessorStatus{
            .supervisor = (value & 0x80_00_00_00) == 0x80_00_00_00,
            .interrupt_enable = (value & 0x40_00_00_00) == 0x40_00_00_00,
            .mmu_enable = (value & 0x20_00_00_00) == 0x20_00_00_00,
            .priority = @as(u3, @truncate((value & 0x1c_00_00_00) >> 26)),
            .ivt = @as(u6, @truncate((value & 0x03_f0_00_00) >> 20)),
            .pdt = @as(u8, @truncate((value & 0x00_0f_f0_00) >> 12)),
            .reserved = @as(u12, @truncate(value & 0x00_00_0f_ff)),
        };
    }

    pub fn tou32(self: *Self) u32 {
        return @as(u32, @intFromBool(self.supervisor)) << 31 |
            @as(u32, @intFromBool(self.interrupt_enable)) << 30 |
            @as(u32, @intFromBool(self.mmu_enable)) << 29 |
            @as(u32, self.priority) << 26 |
            @as(u32, self.ivt) << 20 |
            @as(u32, self.pdt) << 12 |
            @as(u32, self.reserved);
    }
};

pub const Interrupt = struct {
    asserted: bool,
    vector: u8,
};

pub const AssertedPriority = struct {
    asserted: bool,
    priority: u3,
};

/// A device to send control signals to the processor
pub const InterruptController = struct {
    pub const Self = @This();
    cpu: *Sirius,
    interrupts: [7]Interrupt,
    highest: u3,
    last_exception: u8,

    pub fn init() InterruptController {
        return InterruptController{
            .cpu = undefined,
            .interrupts = [7]Interrupt{ Interrupt{ .asserted = false, .vector = 0 }, Interrupt{ .asserted = false, .vector = 0 }, Interrupt{ .asserted = false, .vector = 0 }, Interrupt{ .asserted = false, .vector = 0 }, Interrupt{ .asserted = false, .vector = 0 }, Interrupt{ .asserted = false, .vector = 0 }, Interrupt{ .asserted = false, .vector = 0 } },
            .highest = 0,
            .last_exception = 0,
        };
    }

    pub fn set(self: *Self, state: bool, priority: u3, vector: u8) void {
        self.interrupts[@as(usize, priority)] = .{ .asserted = state, .vector = vector };
        if (state and priority > self.highest) self.highest = priority;
    }

    pub fn check(self: *Self) AssertedPriority {
        return if (self.highest > 0) .{ .asserted = true, .priority = self.highest } else .{ .asserted = false, .priority = 0 };
    }

    pub fn acknowledge(self: *Self, priority: u3) !u8 {
        const ack = self.interrupts[@as(usize, priority)].vector;
        self.interrupts[@as(usize, priority)].asserted = false;
        while (self.highest > 0 and !self.interrupts[@as(usize, self.highest)].asserted) {
            self.highest -= 1;
        }
        return ack;
    }

    pub fn poweroff(self: *Self) void {
        self.cpu.poweroff = true;
    }
};

/// The State of the processor in any given moment in time
pub const State = struct {
    current_ipl: u3,
    pending_ipl: u3,

    pc: u24,
    virtual_pc: u24,
    psr: ProcessorStatus,
    reg: *[Register.count]u32,
    ssp: u32,
    usp: u32,

    pub fn init(registers: *[Register.count]u32) State {
        return State{
            .current_ipl = 7,
            .pending_ipl = 0,
            .pc = 0,
            .virtual_pc = 0,
            .psr = ProcessorStatus{ .supervisor = true, .interrupt_enable = false, .mmu_enable = false, .priority = 7, .ivt = 62, .pdt = 255, .reserved = 0 },
            .reg = registers,
            .ssp = 0,
            .usp = 0,
        };
    }
};

/// The Sirius Cpu
pub const Sirius = struct {
    pub const Self = @This();
    pub const Error = arch.InterruptRaised || arch.ProcessorError || std.mem.Allocator.Error;
    frequency: u64,
    cycles: u128,
    state: State,
    main_bus: *memory.MainBus,
    data_bus: *memory.DataBus,
    mmu: *memory.Mmu,
    interrupt_controller: *InterruptController,
    allocator: std.mem.Allocator,
    poweroff: bool,

    pub fn init(frequency: u64, registers: *[Register.count]u32, irq: *InterruptController, main_bus: *memory.MainBus, data_bus: *memory.DataBus, mmu: *memory.Mmu, allocator: std.mem.Allocator) Sirius {
        const state = State.init(registers);
        return Sirius{
            .frequency = frequency,
            .cycles = 0,
            .state = state,
            .main_bus = main_bus,
            .data_bus = data_bus,
            .mmu = mmu,
            .interrupt_controller = irq,
            .allocator = allocator,
            .poweroff = false,
        };
    }

    pub fn runFor(self: *Self, ns: u64) Error!void {
        const target = self.cycles + (ns / (std.time.ns_per_s / self.frequency));
        while (self.cycles < target) {
            try self.checkExceptions();
        }
    }

    fn translate(self: *Self, addr: u24) Error!memory.PhysicalAddr {
        if (self.state.psr.mmu_enable) {
            return try self.mmu.translate(addr);
        } else {
            return memory.PhysicalAddr{ .addr = addr, .x = true, .w = true };
        }
    }

    fn setDirty(self: *Self, addr: u24) void {
        if (self.state.psr.mmu_enable) {
            self.mmu.update(addr, true, true);
        }
    }

    pub fn checkExceptions(self: *Self) Error!bool {
        self.cycle() catch |err| {
            try switch (err) {
                error.AccessViolation => self.exception(@intFromEnum(arch.Exception.AccessViolation), false),
                error.AddressError => self.exception(@intFromEnum(arch.Exception.AddressError), false),
                error.BusError => self.exception(@intFromEnum(arch.Exception.BusError), false),
                error.DivisionZero => self.exception(@intFromEnum(arch.Exception.DivisionZero), false),
                error.IllegalInstruction => std.debug.panic("Illegal Instruction at pc: {x}\n", .{self.state.pc}), //self.exception(@enumToInt(arch.Exception.IllegalInstruction), false),
                error.PageFault => self.exception(@intFromEnum(arch.Exception.PageFault), false),
                error.PrivilegeViolation => self.exception(@intFromEnum(arch.Exception.PrivilegeViolation), false),
                error.Reset => self.exception(@intFromEnum(arch.Exception.Reset), false),

                error.SerialIncoming => self.exception(@intFromEnum(arch.Interrupt.SerialIncoming), true),
                error.KeyboardCharacter => self.exception(@intFromEnum(arch.Interrupt.KeyboardCharacter), true),
                error.KeyboardScancode => self.exception(@intFromEnum(arch.Interrupt.KeyboardScancode), true),
                error.TPSFinished => self.exception(@intFromEnum(arch.Interrupt.TPSFinished), true),
                error.DiskFinished => self.exception(@intFromEnum(arch.Interrupt.DiskFinished), true),
                error.TimerInterval => self.exception(@intFromEnum(arch.Interrupt.TimerInterval), true),
                error.TimerTimeout => self.exception(@intFromEnum(arch.Interrupt.TimerTimeout), true),
                error.VideoRefresh => self.exception(@intFromEnum(arch.Interrupt.VideoRefresh), true),

                else => std.debug.panic("Fatal Error: {any}", .{err}),
            };
        };
        return self.poweroff;
    }

    fn getPc(self: *Self) u24 {
        if (self.state.psr.mmu_enable) return self.state.virtual_pc;
        return self.state.pc;
    }

    fn setPc(self: *Self, value: u24) Error!void {
        if (value % 4 != 0) return error.AddressError;
        if (self.state.psr.mmu_enable) self.state.virtual_pc = value;
        self.state.pc = value;
    }

    fn getReg(self: *Self, reg: u5) u32 {
        return switch (reg) {
            @intFromEnum(Register.x0) => 0,
            @intFromEnum(Register.x2) => if (self.state.psr.supervisor) self.state.ssp else self.state.usp,
            else => self.state.reg[reg],
        };
    }

    fn setReg(self: *Self, reg: u5, value: u32) void {
        switch (reg) {
            @intFromEnum(Register.x0) => {},
            @intFromEnum(Register.x2) => {
                if (self.state.psr.supervisor)
                    self.state.ssp = value
                else
                    self.state.usp = value;
            },
            else => self.state.reg[reg] = value,
        }
    }

    fn syscall(self: *Self, vector: u8) Error!void {
        try self.setupNormalException(vector, false);
    }

    fn sysret(self: *Self) Error!void {
        std.debug.print("Sysret: a0->{x}\n", .{self.getReg(@intFromEnum(Register.x10))});
        const psr = ProcessorStatus.fromu32(try self.popWord());
        self.state.psr = psr;
        const pc = @as(u24, @truncate(try self.popWord()));
        try self.setPc(pc);
    }

    fn trace(
        self: *Self,
        r1: u5,
        r2: u5,
        r3: u5,
        r4: u5,
    ) void {
        std.debug.print("Trace -> {any}: {x}, {any}: {x}, {any}: {x}, {any}: {x}{c}\n", .{ @as(Register, @enumFromInt(r1)), self.getReg(r1), @as(Register, @enumFromInt(r2)), self.getReg(r2), @as(Register, @enumFromInt(r3)), self.getReg(r3), @as(Register, @enumFromInt(r4)), self.getReg(r4), 0x07 });
    }

    fn requireSupervisor(self: *Self) Error!void {
        if (!self.state.psr.supervisor) return Error.PrivilegeViolation;
    }

    fn setSupervisor(self: *Self) void {
        if (self.state.psr.supervisor) return;
        self.state.psr.supervisor = true;
        self.state.usp = self.getReg(@intFromEnum(Register.x2));
        self.setReg(@intFromEnum(Register.x2), self.state.ssp);
    }

    fn setUsermode(self: *Self) void {
        if (!self.state.psr.supervisor) return;
        self.state.psr.supervisor = false;
        self.state.ssp = self.getReg(@intFromEnum(Register.x2));
        self.setReg(@intFromEnum(Register.x2), self.state.usp);
    }

    fn pushWord(self: *Self, word: u32) Error!void {
        const sp = self.getReg(@intFromEnum(Register.x2)) -% 4;
        self.setReg(@intFromEnum(Register.x2), sp);
        _ = try self.writeMainBeu32(@as(u24, @truncate(sp)), word);
    }

    fn popWord(self: *Self) Error!u32 {
        const sp = self.getReg(@intFromEnum(Register.x2));
        const word = try self.readMainBeu32(@as(u24, @truncate(sp)));
        self.setReg(@intFromEnum(Register.x2), @as(u24, @truncate(sp)) +% 4);
        return word;
    }

    fn exception(self: *Self, vector: u8, is_interrupt: bool) Error!void {
        self.interrupt_controller.last_exception = vector;
        // The difference between Excetpion and fault is that the last
        // tries to fix the problem that caused it and **TRIES** to
        // execute again th instruction that triggered it
        switch (vector) {
            @intFromEnum(arch.Exception.BusError), @intFromEnum(arch.Exception.AddressError), @intFromEnum(arch.Exception.PageFault) => {
                self.setupFault(vector) catch |err| {
                    // A double fault Stops the execution inmediately
                    std.debug.print("Fatal Error: a double fault arised {any}.\n", .{err});
                    std.process.exit(2);
                };
            },
            //@enumToInt(arch.Exception.PrivilegeViolation) => {
            //    std.debug.panic("Privilege violation at pc {x}, dumping core.\n", .{self.state.pc});
            //},
            else => try self.setupNormalException(vector, is_interrupt),
        }
    }

    fn setupFault(self: *Self, vector: u8) Error!void {
        std.debug.print("Fault {any} at pc {x}\n", .{ @as(arch.Exception, @enumFromInt(vector)), self.getPc() });
        const instruction = try self.readMainBeu32(self.getPc() -% 4);
        try self.pushWord(@as(u32, self.getPc()));
        try self.pushWord(self.state.psr.tou32());
        try self.pushWord(instruction);
        self.setSupervisor();
        const ivt_base = @as(u16, self.state.psr.ivt) * @as(u16, @truncate(arch.IVTSize));
        const index = ivt_base + (@as(u16, vector) << 2);
        const handler_addr = @as(u24, @truncate(try self.data_bus.readBeu32(index)));
        try self.setPc(handler_addr);
    }

    fn setupNormalException(self: *Self, vector: u8, is_interrupt: bool) Error!void {
        const pc = @as(u32, self.getPc());
        try self.pushWord(pc);
        try self.pushWord(self.state.psr.tou32());
        self.setSupervisor();

        if (is_interrupt and self.state.psr.interrupt_enable) {
            self.state.psr.priority = self.state.current_ipl;
        }

        const ivt_base = @as(u16, self.state.psr.ivt) * @as(u16, @truncate(arch.IVTSize));
        const index = ivt_base + (@as(u16, vector) << 2);
        const handler_addr = @as(u24, @truncate(try self.data_bus.readBeu32(index)));
        try self.setPc(handler_addr);
    }

    fn checkPendingInterrupts(self: *Self) !void {
        const irq = self.interrupt_controller.check();
        self.state.pending_ipl = if (irq.asserted) irq.priority else 0;
        const current = self.state.current_ipl;
        const pending = self.state.pending_ipl;

        if (self.state.pending_ipl != 0) {
            if (pending > self.state.psr.priority or pending == 7 and pending >= current) {
                std.debug.print("Interrupt: priority {x} @ {d}\n", .{ pending, self.cycles });
                self.state.current_ipl = self.state.pending_ipl;
                const vector = try self.interrupt_controller.acknowledge(self.state.current_ipl);
                try self.exception(vector, true);
            }
        }

        if (pending < current) self.state.current_ipl = self.state.pending_ipl;
    }

    fn cycle(self: *Self) Error!void {
        try self.execute(try self.fetch());
        try self.checkPendingInterrupts();
        self.cycles +|= 1;
    }

    fn fetch(self: *Self) Error!u32 {
        const pc = self.getPc();
        try self.setPc(pc +% 4);
        //A misaligned program counter **should** be unreachable
        const instruction = self.readMainBeu32(pc) catch unreachable;
        self.setDirty(pc);
        return instruction;
    }

    fn execute(self: *Self, instruction: u32) Error!void {
        const Decoder = arch.Decoder;
        const group: u3 = @as(u3, @truncate(instruction >> Decoder.GROUP_SHIFHT));
        const opcode: u4 = @as(u4, @truncate((instruction & Decoder.OPCODE_MASK) >> Decoder.OPCODE_SHIFT));
        const r1: u5 = @as(u5, @truncate((instruction & Decoder.R1_MASK) >> Decoder.R1_SHIFT));
        const r2: u5 = @as(u5, @truncate((instruction & Decoder.R2_MASK) >> Decoder.R2_SHIFT));
        const r3: u5 = @as(u5, @truncate((instruction & Decoder.R3_MASK) >> Decoder.R3_SHIFT));
        const r4: u5 = @as(u5, @truncate((instruction & Decoder.R4_MASK) >> Decoder.R4_SHIFT));
        const vector: u8 = @as(u8, @truncate(instruction & Decoder.VECTOR_MASK));
        const imm_15: u15 = @as(u15, @truncate(instruction & Decoder.IMM15_MASK));
        const imm_20: u20 = @as(u20, @truncate(instruction & Decoder.IMM20_MASK));

        const group_opcode = @as(u7, group) << 4 | opcode;
        switch (group_opcode) {
            @intFromEnum(arch.Instruction.Syscall) => {
                if (r1 == @intFromEnum(Register.x0)) {
                    try self.syscall(vector);
                } else {
                    try self.syscall(@as(u8, @truncate(self.getReg(r1))));
                }
            },
            @intFromEnum(arch.Instruction.GsReg) => {
                self.setReg(r1, self.state.psr.tou32());
            },
            @intFromEnum(arch.Instruction.SsReg) => {
                try self.requireSupervisor();
                self.state.psr = ProcessorStatus.fromu32(self.getReg(r1));
                //std.debug.print("SREG changed ({any})\n", .{self.state.psr});
            },
            @intFromEnum(arch.Instruction.Sysret) => {
                try self.requireSupervisor();
                try self.sysret();
            },
            @intFromEnum(arch.Instruction.Trace) => {
                self.trace(r1, r2, r3, r4);
            },
            @intFromEnum(arch.Instruction.MmuToggle) => { // TODO: document this
                // Toggles mmu and jumps to address in r: mmu.toggle a0
                try self.requireSupervisor();
                self.state.psr.mmu_enable = !self.state.psr.mmu_enable;
                std.debug.print("Toggled MMU {s}\n", .{if (self.state.psr.mmu_enable) "on" else "off"});
                try self.setPc(@truncate(self.getReg(r1)));
            },
            @intFromEnum(arch.Instruction.MmuMap) => { // TODO: document this
                // Maps physical page in r1 to logical r2
                // setting the write and execute bits as per r3 or immediate:
                // mmu.map phy, log, r, (w, x)
                try self.requireSupervisor();
                var w = false;
                var x = false;
                if (r3 == @intFromEnum(Register.x0)) {
                    w = if (imm_15 & 0x2 != 0) true else false;
                    x = if (imm_15 & 0x1 != 0) true else false;
                } else {
                    const flags = self.getReg(r3);
                    w = if (flags & 0x2 != 0) true else false;
                    x = if (flags & 0x1 != 0) true else false;
                }
                self.mmu.map(@truncate(self.getReg(r1)), @truncate(self.getReg(r2)), w, x);
            },
            @intFromEnum(arch.Instruction.MmuUnmap) => { // TODO: document this
                // Unmaps logical page: mmu.unmap a0
                try self.requireSupervisor();
                self.mmu.unmap(@truncate(self.getReg(r1)));
            },
            @intFromEnum(arch.Instruction.MmuUpdate) => { // TODO: document this
                // Updates a maping in the page table with the flags in registers or immediate
                // mmu.update a0, r, (dirty, present)
                try self.requireSupervisor();
                var dirty = false;
                var present = false;
                if (r2 == @intFromEnum(Register.x0)) {
                    dirty = if (imm_15 & 0x2 != 0) true else false;
                    present = if (imm_15 & 0x1 != 0) true else false;
                } else {
                    const flags = self.getReg(r2);
                    dirty = if (flags & 0x2 != 0) true else false;
                    present = if (flags & 0x1 != 0) true else false;
                }
                self.mmu.update(@truncate(self.getReg(r1)), dirty, present);
            },
            @intFromEnum(arch.Instruction.MmuStat) => { // TODO: document this
                // returns in rd the status for the page in rs1 as a set of flags (x, w, dirty, present -- low byte, low nibble)
                // if not mapped, returns -1 (0xff_ff_ff_ff)
                // mmu.stat rd, rs1
                try self.requireSupervisor();
                self.setReg(r1, self.mmu.status(@truncate(self.getReg(r2))));
            },
            @intFromEnum(arch.Instruction.MmuSetPT) => { // TODO: document this
                // Loads a page table from memory. Entries must be 16 bits long and in this format physical:12 w:1 x:1 reserved:2
                // The page location is specified by a pointer in r1 and the length by r2 or immediate (not more than 12 bits)
                // mmu.setpt ptr, len, lenimm
                try self.requireSupervisor();
                var len: u12 = 0;
                if (r2 == @intFromEnum(Register.x0)) {
                    len = @truncate(imm_15);
                } else {
                    len = @truncate(self.getReg(r2));
                }
                try self.mmu.set_page_table(@truncate(self.getReg(r1)), len);
            },
            @intFromEnum(arch.Instruction.Copy) => {
                const src = @as(u24, @truncate(self.getReg(r1)));
                const dest = @as(u24, @truncate(self.getReg(r2)));
                const len = @as(usize, self.getReg(r3));
                var buff = try self.allocator.alloc(u8, len);
                defer self.allocator.free(buff);
                _ = try self.readMain(src, buff);
                _ = try self.writeMain(dest, buff);
            },
            @intFromEnum(arch.Instruction.Swap) => {
                const a = @as(u24, @truncate(self.getReg(r1)));
                const b = @as(u24, @truncate(self.getReg(r2)));
                const len = @as(usize, self.getReg(r3));
                var buff_a = try self.allocator.alloc(u8, len);
                var buff_b = try self.allocator.alloc(u8, len);
                defer self.allocator.free(buff_a);
                defer self.allocator.free(buff_b);
                _ = try self.readMain(a, buff_a);
                _ = try self.readMain(b, buff_b);
                _ = try self.writeMain(b, buff_a);
                _ = try self.writeMain(a, buff_b);
            },
            @intFromEnum(arch.Instruction.Fill) => {
                const dest = @as(u24, @truncate(self.getReg(r1)));
                const len = @as(usize, self.getReg(r2));
                const fill = @as(u8, @truncate(self.getReg(r3)));
                var buff = try self.allocator.alloc(u8, len);
                defer self.allocator.free(buff);
                @memset(buff, fill);
                _ = try self.writeMain(dest, buff);
            },
            @intFromEnum(arch.Instruction.Through) => {
                const pointer = @as(u24, @truncate(self.getReg(r2)));
                const effective_address = try self.readMainBeu32(pointer);
                const data = self.getReg(r1);
                _ = try self.writeMainBeu32(@as(u24, @truncate(effective_address)), data);
            },
            @intFromEnum(arch.Instruction.From) => {
                const pointer = @as(u24, @truncate(self.getReg(r2)));
                const effective_address = try self.readMainBeu32(pointer);
                //std.debug.print("From {x} -> {!x}\n", .{effective_address, self.readMainBeu32(@truncate(u24, effective_address))});
                self.setReg(r1, try self.readMainBeu32(@as(u24, @truncate(effective_address))));
            },
            @intFromEnum(arch.Instruction.Popb) => {
                self.setReg(r1, try self.readMainu8(@as(u24, @truncate(self.getReg(r2)))));
                const sp = self.getReg(r2) +% 1;
                self.setReg(r2, sp);
            },
            @intFromEnum(arch.Instruction.Poph) => {
                self.setReg(r1, try self.readMainBeu16(@as(u24, @truncate(self.getReg(r2)))));
                const sp = self.getReg(r2) +% 2;
                self.setReg(r2, sp);
            },
            @intFromEnum(arch.Instruction.Pop) => {
                self.setReg(r1, try self.readMainBeu32(@as(u24, @truncate(self.getReg(r2)))));
                const sp = self.getReg(r2) +% 4;
                self.setReg(r2, sp);
            },
            @intFromEnum(arch.Instruction.Pushb) => {
                const sp = self.state.reg[r2] -% 1;
                self.setReg(r2, sp);
                _ = try self.writeMainu8(@as(u24, @truncate(self.getReg(r2))), @as(u8, @truncate(self.getReg(r1))));
            },
            @intFromEnum(arch.Instruction.Pushh) => {
                const sp = self.getReg(r2) -% 2;
                self.setReg(r2, sp);
                _ = try self.writeMainBeu16(@as(u24, @truncate(self.getReg(r2))), @as(u16, @truncate(self.getReg(r1))));
            },
            @intFromEnum(arch.Instruction.Push) => {
                const sp = self.getReg(r2) -% 4;
                self.setReg(r2, sp);
                _ = try self.writeMainBeu32(@as(u24, @truncate(self.getReg(r2))), self.getReg(r1));
            },
            @intFromEnum(arch.Instruction.Save) => {
                var addr = @as(u24, @truncate(self.getReg(r3)));
                var i = r1;
                while (i <= r2) : (i += 1) {
                    // std.debug.print("Saved register {} to {x}\n" , .{i, addr });
                    addr -%= 4; // Pushes them
                    _ = try self.writeMainBeu32(addr, self.getReg(i));
                }
                self.setReg(r3, addr);
                // std.debug.print("Set register {} to {x}\n" , .{r3, addr});
            },
            @intFromEnum(arch.Instruction.Restore) => {
                var addr = @as(u24, @truncate(self.getReg(r3)));
                var r = r2;
                while (r >= r1) : (r -= 1) {
                    // std.debug.print("Restore register {} to {x}\n" , .{r, addr });
                    self.setReg(r, try self.readMainBeu32(addr));
                    addr +%= 4; // Pops them
                }
                self.setReg(r3, addr);
                // std.debug.print("Set register {} to {x}\n" , .{r3, addr});
            },
            @intFromEnum(arch.Instruction.Exch) => {
                var tmp = self.getReg(r1);
                self.setReg(r1, self.getReg(r2));
                self.setReg(r2, tmp);
            },
            @intFromEnum(arch.Instruction.Slt) => {
                self.setReg(r1, if (@as(i32, @bitCast(self.getReg(r2))) < @as(i32, @bitCast(self.getReg(r3)))) 1 else 0);
            },
            @intFromEnum(arch.Instruction.Sltu) => {
                self.setReg(r1, if (self.getReg(r2) < self.getReg(r3)) 1 else 0);
            },
            @intFromEnum(arch.Instruction.Jal) => {
                const pc = self.getPc();
                self.setReg(r1, pc);
                const offset: u24 = sext(u24, @as(u22, @intCast(imm_20)) << 2, 22);
                try self.setPc((pc -% 4) +% offset);
            },
            @intFromEnum(arch.Instruction.Lui) => {
                self.setReg(r1, @as(u32, imm_20) << 12);
            },
            @intFromEnum(arch.Instruction.Auipc) => {
                self.setReg(r1, (@as(u32, imm_20) << 12) +% (self.getPc() -% 4));
            },
            @intFromEnum(arch.Instruction.Beq) => {
                if (self.getReg(r1) == self.getReg(r2)) {
                    try self.setPc((self.getPc() -% 4) +% sext(u24, @as(u17, @intCast(imm_15)) << 2, 17));
                }
            },
            @intFromEnum(arch.Instruction.Bne) => {
                if (self.getReg(r1) != self.getReg(r2)) {
                    try self.setPc((self.getPc() -% 4) +% sext(u24, @as(u17, @intCast(imm_15)) << 2, 17));
                }
            },
            @intFromEnum(arch.Instruction.Blt) => {
                if (@as(i32, @bitCast(self.getReg(r1))) < @as(i32, @bitCast(self.getReg(r2)))) {
                    try self.setPc((self.getPc() -% 4) +% sext(u24, @as(u17, @intCast(imm_15)) << 2, 17));
                }
            },
            @intFromEnum(arch.Instruction.Bge) => {
                if (@as(i32, @bitCast(self.getReg(r1))) >= @as(i32, @bitCast(self.getReg(r2)))) {
                    try self.setPc((self.getPc() -% 4) +% sext(u24, @as(u17, @intCast(imm_15)) << 2, 17));
                }
            },
            @intFromEnum(arch.Instruction.Bltu) => {
                if (self.getReg(r1) < self.getReg(r2)) {
                    try self.setPc((self.getPc() -% 4) +% sext(u24, @as(u17, @intCast(imm_15)) << 2, 17));
                }
            },
            @intFromEnum(arch.Instruction.Bgeu) => {
                if (self.getReg(r1) >= self.getReg(r2)) {
                    try self.setPc((self.getPc() -% 4) +% sext(u24, @as(u17, @intCast(imm_15)) << 2, 17));
                }
            },
            @intFromEnum(arch.Instruction.Jalr) => {
                const jump = self.getReg(r2) +% sext(u32, @as(u17, @intCast(imm_15)) << 2, 17);
                self.setReg(r1, self.getPc());
                //std.debug.print("r2: {x}, offset: {x}, jump: {x}\n", .{self.getReg(r2), sext(u32, @intCast(u17, imm_15) << 2, 17), jump});
                try self.setPc(@as(u24, @truncate(jump)));
            },
            @intFromEnum(arch.Instruction.Lb) => {
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = sext(u32, try self.readMainu8(@as(u24, @truncate(addr))), 8);
                self.setReg(r1, value);
            },
            @intFromEnum(arch.Instruction.Lbu) => {
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = try self.readMainu8(@as(u24, @truncate(addr)));
                self.setReg(r1, value);
            },
            @intFromEnum(arch.Instruction.Lbd) => {
                try self.requireSupervisor();
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = sext(u32, try self.data_bus.readu8(@as(u16, @truncate(addr))), 8);
                self.setReg(r1, value);
            },
            @intFromEnum(arch.Instruction.Lbud) => {
                try self.requireSupervisor();
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = try self.data_bus.readu8(@as(u16, @truncate(addr)));
                self.setReg(r1, value);
            },
            @intFromEnum(arch.Instruction.Lh) => {
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = sext(u32, try self.readMainBeu16(@as(u24, @truncate(addr))), 16);
                self.setReg(r1, value);
            },
            @intFromEnum(arch.Instruction.Lhu) => {
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = try self.readMainBeu16(@as(u24, @truncate(addr)));
                self.setReg(r1, value);
            },
            @intFromEnum(arch.Instruction.Lhd) => {
                try self.requireSupervisor();
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = sext(u32, try self.data_bus.readBeu16(@as(u16, @truncate(addr))), 16);
                self.setReg(r1, value);
            },
            @intFromEnum(arch.Instruction.Lhud) => {
                try self.requireSupervisor();
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = try self.data_bus.readBeu16(@as(u16, @truncate(addr)));
                self.setReg(r1, value);
            },
            @intFromEnum(arch.Instruction.Lw) => {
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = try self.readMainBeu32(@as(u24, @truncate(addr)));
                self.setReg(r1, value);
            },
            @intFromEnum(arch.Instruction.Lwd) => {
                try self.requireSupervisor();
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = try self.data_bus.readBeu32(@as(u16, @truncate(addr)));
                self.setReg(r1, value);
            },
            @intFromEnum(arch.Instruction.Muli) => {
                const value: u64 = self.getReg(r2) *% sext(u64, imm_15, 15);
                self.setReg(r1, @as(u32, @truncate(value)));
            },
            @intFromEnum(arch.Instruction.Mulih) => {
                const value: u64 = self.getReg(r2) *% sext(u64, imm_15, 15);
                self.setReg(r1, @as(u32, @truncate(value >> 32)));
            },
            @intFromEnum(arch.Instruction.Idivi) => {
                if (imm_15 == 0) return Error.DivisionZero;
                self.setReg(r1, @as(u32, @bitCast(@divFloor(@as(i32, @bitCast(self.getReg(r2))), sext(i32, imm_15, 15)))));
            },
            @intFromEnum(arch.Instruction.Addi) => {
                const value = self.getReg(r2) +% sext(u32, imm_15, 15);
                self.setReg(r1, value);
            },
            @intFromEnum(arch.Instruction.Subi) => {
                self.setReg(r1, self.getReg(r2) -% sext(u32, imm_15, 15));
            },
            @intFromEnum(arch.Instruction.Ori) => {
                self.setReg(r1, self.getReg(r2) | imm_15);
            },
            @intFromEnum(arch.Instruction.Andi) => {
                self.setReg(r1, self.getReg(r2) & imm_15);
            },
            @intFromEnum(arch.Instruction.Xori) => {
                self.setReg(r1, self.getReg(r2) ^ imm_15);
            },
            @intFromEnum(arch.Instruction.ShiRa) => {
                self.setReg(r1, @as(u32, @bitCast(@as(i32, @bitCast(self.getReg(r2))) >> @as(u5, @truncate(imm_15 % 32)))));
            },
            @intFromEnum(arch.Instruction.ShiRl) => {
                self.setReg(r1, self.getReg(r2) >> @as(u5, @truncate(imm_15 % 32)));
            },
            @intFromEnum(arch.Instruction.ShiLl) => {
                self.setReg(r1, self.getReg(r2) << @as(u5, @truncate(imm_15 % 32)));
            },
            @intFromEnum(arch.Instruction.Slti) => {
                self.setReg(r1, if (@as(i32, @bitCast(self.getReg(r2))) < sext(i32, imm_15, 15)) 1 else 0);
            },
            @intFromEnum(arch.Instruction.Sltiu) => {
                self.setReg(r1, if (self.getReg(r2) < imm_15) 1 else 0);
            },
            @intFromEnum(arch.Instruction.Add) => {
                self.setReg(r1, self.getReg(r2) +% self.getReg(r3));
            },
            @intFromEnum(arch.Instruction.Sub) => {
                self.setReg(r1, self.getReg(r2) -% self.getReg(r3));
            },
            @intFromEnum(arch.Instruction.Idiv) => {
                if (self.getReg(r4) == 0) return Error.DivisionZero;
                const numerator: i32 = @as(i32, @bitCast(self.getReg(r3)));
                const denominator: i32 = @as(i32, @bitCast(self.getReg(r4)));
                self.setReg(r1, @as(u32, @bitCast(@divFloor(numerator, denominator))));
                self.setReg(r2, @as(u32, @bitCast(@mod(numerator, denominator))));
            },
            @intFromEnum(arch.Instruction.Mul) => {
                const value: u64 = self.getReg(r3) *% self.getReg(r4);
                // TODO: Implement signed multiplication and unsigned division
                self.setReg(r1, @as(u32, @truncate(value >> 32)));
                self.setReg(r2, @as(u32, @truncate(value)));
            },
            @intFromEnum(arch.Instruction.Or) => {
                self.setReg(r1, self.getReg(r2) | self.getReg(r3));
            },
            @intFromEnum(arch.Instruction.And) => {
                self.setReg(r1, self.getReg(r2) & self.getReg(r3));
            },
            @intFromEnum(arch.Instruction.Xor) => {
                self.setReg(r1, self.getReg(r2) ^ self.getReg(r3));
            },
            @intFromEnum(arch.Instruction.Not) => {
                self.setReg(r1, ~self.getReg(r2));
            },
            @intFromEnum(arch.Instruction.Ctz) => {
                self.setReg(r1, @ctz(self.getReg(r2)));
            },
            @intFromEnum(arch.Instruction.Clz) => {
                self.setReg(r1, @clz(self.getReg(r2)));
            },
            @intFromEnum(arch.Instruction.Popcount) => {
                self.setReg(r1, @popCount(self.getReg(r2)));
            },
            @intFromEnum(arch.Instruction.ShRa) => {
                self.setReg(r1, @as(u32, @bitCast(@as(i32, @bitCast(self.getReg(r2))) >> @as(u5, @truncate(self.getReg(r3) % 32)))));
            },
            @intFromEnum(arch.Instruction.ShRl) => {
                self.setReg(r1, self.getReg(r2) >> @as(u5, @truncate(self.getReg(r3) % 32)));
            },
            @intFromEnum(arch.Instruction.ShLl) => {
                self.setReg(r1, self.getReg(r2) << @as(u5, @truncate(self.getReg(r3) % 32)));
            },
            @intFromEnum(arch.Instruction.Ror) => {
                self.setReg(r1, std.math.rotr(u32, self.getReg(r2), @as(u5, @truncate(self.getReg(r3) % 32))));
            },
            @intFromEnum(arch.Instruction.Rol) => {
                self.setReg(r1, std.math.rotl(u32, self.getReg(r2), @as(u5, @truncate(self.getReg(r3) % 32))));
            },
            @intFromEnum(arch.Instruction.Sb) => {
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = @as(u8, @truncate(self.getReg(r1)));
                _ = try self.writeMainu8(@as(u24, @truncate(addr)), value);
            },
            @intFromEnum(arch.Instruction.Sbd) => {
                try self.requireSupervisor();
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = @as(u8, @truncate(self.getReg(r1)));
                _ = try self.data_bus.writeu8(@as(u16, @truncate(addr)), value);
            },
            @intFromEnum(arch.Instruction.Sh) => {
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = @as(u16, @truncate(self.getReg(r1)));
                _ = try self.writeMainBeu16(@as(u24, @truncate(addr)), value);
            },
            @intFromEnum(arch.Instruction.Shd) => {
                try self.requireSupervisor();
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = @as(u16, @truncate(self.getReg(r1)));
                _ = try self.data_bus.writeBeu16(@as(u16, @truncate(addr)), value);
            },
            @intFromEnum(arch.Instruction.Sw) => {
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = self.getReg(r1);
                _ = try self.writeMainBeu32(@as(u24, @truncate(addr)), value);
            },
            @intFromEnum(arch.Instruction.Swd) => {
                try self.requireSupervisor();
                const addr = self.getReg(r2) +% sext(u32, imm_15, 15);
                const value = self.getReg(r1);
                _ = try self.data_bus.writeBeu32(@as(u16, @truncate(addr)), value);
            },

            else => {
                std.debug.print("Illegal Instruction: {x}\n", .{instruction});
                return Error.IllegalInstruction;
            },
        }
    }

    pub fn writeMain(self: *Self, address: u24, data: []const u8) Error!usize {
        const entry = try self.translate(address);
        if (entry.w or self.state.psr.supervisor) {
            const r = self.main_bus.write(entry.addr, data);
            self.setDirty(address);
            return r;
        } else {
            std.debug.print("Attempted to write to WP page (addr: 0x{x})\n", .{address});
            return error.AccessViolation;
        }
    }

    pub fn readMain(self: *Self, address: u24, buff: []u8) Error!usize {
        const entry = try self.translate(address);
        return self.main_bus.read(entry.addr, buff);
    }

    pub inline fn readMainBeu32(self: *Self, address: u24) Error!u32 {
        var buff = [4]u8{ 0, 0, 0, 0 };
        _ = try self.readMain(address, buff[0..]);
        const data =
            @as(u32, buff[0]) << 24 |
            @as(u32, buff[1]) << 16 |
            @as(u32, buff[2]) << 8 |
            @as(u32, buff[3]);

        return data;
    }

    pub inline fn readMainBeu16(self: *Self, address: u24) Error!u16 {
        var buff = [2]u8{ 0, 0 };
        _ = try self.readMain(address, buff[0..]);
        return @as(u16, buff[0]) << 8 |
            @as(u16, buff[1]);
    }

    pub inline fn readMainu8(self: *Self, address: u24) Error!u8 {
        var buff = [1]u8{0};
        _ = try self.readMain(address, buff[0..]);
        return buff[0];
    }

    pub inline fn writeMainBeu32(self: *Self, address: u24, data: u32) Error!usize {
        const word = [4]u8{
            @as(u8, @truncate(data >> 24)),
            @as(u8, @truncate(data >> 16)),
            @as(u8, @truncate(data >> 8)),
            @as(u8, @truncate(data & 0xff)),
        };
        return self.writeMain(address, word[0..]);
    }

    pub inline fn writeMainBeu16(self: *Self, address: u24, data: u16) Error!usize {
        const word = [2]u8{
            @as(u8, @truncate(data >> 8)),
            @as(u8, @truncate(data)),
        };
        return self.writeMain(address, word[0..]);
    }

    pub inline fn writeMainu8(self: *Self, address: u24, data: u8) Error!usize {
        const word = [1]u8{data};
        return self.writeMain(address, word[0..]);
    }
};

inline fn sext(T: anytype, x: anytype, bits: T) T {
    const mask: T = (1 << bits) - 1;
    const sign_bit: T = 1 << (bits - 1);
    if (x & sign_bit == sign_bit) {
        return @as(T, @bitCast(~mask | x));
    } else {
        return @as(T, x);
    }
}
