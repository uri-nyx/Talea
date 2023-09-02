const std = @import("std");
const cpu = @import("../cpu.zig");
const arch = @import("arch");

pub const Sys = struct {
    pub const Self = @This();
    pub const MEMSIZE: u4 = 0;
    pub const CLOCK: u4 = 1;
    pub const INT: u4 = 2;
    pub const POWER: u4 = 3;
    pub const YEAR: u4 = 4;
    pub const MONTH: u4 = 5;
    pub const DAY: u4 = 6;
    pub const HOUR: u4 = 7;
    pub const MINUTE: u4 = 8;
    pub const SECOND: u4 = 9;
    pub const DEVICENUM: u4 = 12;
    pub const ARCHID: u4 = 13;
    pub const VENDORID: u4 = 14;

    interrupt_controller: *cpu.InterruptController,
    freq: u64,
    connected: u4,

    pub fn init(irq: *cpu.InterruptController, freq: u64, device_number: u4) Sys {
        return Sys{
            .interrupt_controller = irq,
            .freq = freq,
            .connected = device_number,
        };
    }

    pub fn write(self: *Self, address: u4, data: u8) void {
        _ = data;
        switch (address) {
            CLOCK => std.debug.print("Runtime change of clock speed not implemented yet\n", .{}),
            INT => self.interrupt_controller.last_exception = 1,
            POWER => {
                std.debug.print("Shutting down the machine\n", .{});
                self.interrupt_controller.poweroff();
            },
            else => return,
        }
        return;
    }

    pub fn read(self: *Self, address: u4) u8 {
        const now: std.time.epoch.EpochSeconds = .{.secs = @intCast(@as(i32, @truncate(std.time.timestamp())))};
        return switch (address) {
            MEMSIZE => (arch.MAIN_SIZE / (64 * 1024)) - 1, //TODO: make this configurable in some way
            CLOCK => @truncate(self.freq / 1000000),
            INT => {
                const le = self.interrupt_controller.last_exception;
                self.interrupt_controller.last_exception = 1;
                return le;
            },
            YEAR => @truncate(now.getEpochDay().calculateYearDay().year - 2000),
            MONTH => now.getEpochDay().calculateYearDay().calculateMonthDay().month.numeric(),
            DAY => now.getEpochDay().calculateYearDay().calculateMonthDay().day_index + 1,
            HOUR => now.getDaySeconds().getHoursIntoDay(),
            MINUTE => now.getDaySeconds().getMinutesIntoHour(),
            SECOND => now.getDaySeconds().getSecondsIntoMinute(),
            DEVICENUM => self.connected,
            ARCHID =>  arch.ID,
            VENDORID => arch.VENDOR,
            else => 0,
        };
    }
};
