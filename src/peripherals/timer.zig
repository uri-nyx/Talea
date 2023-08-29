//! A simple timer device
const std = @import("std");
const arch = @import("arch");
const cpu = @import("../cpu.zig");
const memory = @import("memory");

pub const Timer16 = struct {
    pub const Self = @This();
    pub const Len: u4 = 6;
    pub const TIMEOUT: u4 = 0;
    pub const INTERVAL: u4 = 2;
    pub const FREQ_T: u4 = 4;
    pub const FREQ_I: u4 = 5;

    comptime base: u4 = 6,
    frequency: u64,
    cycles: u128,
    interrupt_controller: *cpu.InterruptController,
    timeout_enable: bool,
    interval_enable: bool,
    interval_counter: u16,
    interval: u16,
    timeout_counter: u16,
    freq_t: u8,
    freq_i: u8,

    pub fn init(frequency: u64, comptime base_addr: u4, irq: *cpu.InterruptController) Timer16 {
        return Timer16{
            .base = base_addr,
            .frequency = frequency,
            .cycles = 0,
            .interrupt_controller = irq,
            .timeout_enable = false,
            .interval_enable = false,
            .interval_counter = 0,
            .interval = 0,
            .timeout_counter = 0,
            .freq_t = 1,
            .freq_i = 1,
        };
    }

    pub fn write(self: *Self, address: u4, data: u8) void {
        switch (address) {
            self.base + TIMEOUT => {
                self.timeout_enable = true;
                self.timeout_counter |= @as(u16, data) << 8;
            },
            self.base + TIMEOUT + 1 => {
                self.timeout_enable = true;
                self.timeout_counter |= data;
            },
            self.base + INTERVAL => {
                self.interval_enable = true;
                self.interval |= @as(u16, data) << 8;
                self.interval_counter |= @as(u16, data) << 8;
            },
            self.base + INTERVAL + 1 => {
                self.interval_enable = true;
                self.interval |= data;
                self.interval_counter |= data;
            },
            self.base + FREQ_T => self.freq_t = data,
            self.base + FREQ_I => self.freq_i = data,
            else => return,
        }
    }

    pub fn read(self: *Self, address: u4) u8 {
        return switch (address) {
            (self.base + TIMEOUT) => @as(u8, @truncate(self.timeout_counter >> 8)),
            (self.base + TIMEOUT + 1) => @as(u8, @truncate(self.timeout_counter & 0xff)),
            (self.base + INTERVAL) => @as(u8, @truncate(self.interval >> 8)),
            (self.base + INTERVAL + 1) => @as(u8, @truncate(self.interval & 0xff)),
            (self.base + FREQ_T) => self.freq_t,
            (self.base + FREQ_I) => self.freq_i,
            else => 0,
        };
    }

    inline fn check_timeout(self: *Self) bool {
        return self.timeout_counter == 0;
    }

    inline fn check_interval(self: *Self) bool {
        return self.interval_counter == 0;
    }

    pub fn cycleOne(self: *Self) void {
        if (self.timeout_enable and self.freq_t != 0 and @rem(self.cycles, self.freq_t) == 0) {
            self.timeout_counter -%= 1;
            if (self.check_timeout()) {
                self.interrupt_controller.set(true, 6, @intFromEnum(arch.Interrupt.TimerTimeout));
                self.timeout_enable = false;
            }
        }

        if (self.interval_enable and self.freq_i != 0 and @rem(self.cycles, self.freq_i) == 0) {
            self.interval_counter -%= 1;
            if (self.check_interval()) {
                self.interrupt_controller.set(true, 6, @intFromEnum(arch.Interrupt.TimerInterval));
                self.interval_counter = self.interval;
            }
        }
    }
};
