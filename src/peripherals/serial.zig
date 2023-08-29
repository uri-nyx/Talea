//! A simple tty serial device
const std = @import("std");
const arch = @import("arch");
const cpu = @import("../cpu.zig");
const memory = @import("memory");
const network = @import("network");

pub const Serial = struct {
    pub const Self = @This();
    pub const Len: u4 = 6;
    pub const TX: u4 = 0;
    pub const RX: u4 = 2;
    pub const STAT: u4 = 4;
    pub const CTRL: u4 = 5;
    pub const Mode = enum { Lifo, Fifo };

    comptime base: u4 = 0,
    use: bool,
    frequency: u64,
    mode: Mode,
    interrupt_controller: *cpu.InterruptController,
    tx: std.ArrayList(u8),
    rx: std.ArrayList(u8),
    socket: *network.Socket,

    pub fn init(frequency: u64, irq: *cpu.InterruptController, allocator: std.mem.Allocator, use: bool) !Serial {
        return Serial{
            .use = use,
            .frequency = frequency,
            .mode = Serial.Mode.Lifo,
            .interrupt_controller = irq,
            .tx = try std.ArrayList(u8).initCapacity(allocator, arch.SerialBufferSize),
            .rx = try std.ArrayList(u8).initCapacity(allocator, arch.SerialBufferSize),
            .socket = undefined,
        };
    }

    pub fn deinit(self: *Self) void {
        self.tx.deinit();
        self.rx.deinit();
    }

    pub fn write(self: *Self, address: u4, data: u8) void {
        if (self.use) {
        switch (address) {
            RX => {
                const d = [_]u8{data};
                //std.debug.print("Sending char {x} ({c}) to Serial Port\n", .{data, data});
                _ = self.socket.send(&d) catch |err| std.debug.panic("Send Error: {s} ", .{@errorName(err)});
            },
            TX => {
                self.tx.append(data) catch |err| std.debug.panic("Allocator Error: {s} ", .{@errorName(err)});
                if (data == '\n') {
                    self.interrupt_controller.set(true, 4, @intFromEnum(arch.Interrupt.SerialIncoming));
                }
            },
            CTRL => {
                if (data == 1) self.mode = Mode.Fifo else if (data == 0) self.mode = Mode.Lifo else return;
                std.debug.print("Serial is now in mode {any}\n", .{self.mode});
            },
            else => return,
        }
        } else {
            return;
        }
    }

    pub fn read(self: *Self, address: u4) u8 {
        if (self.use) {
        return switch (address) {
            TX => {
                var b: u8 = undefined;
                switch (self.mode) {
                    Mode.Fifo => {
                        if (self.tx.items.len > 0) b = self.tx.orderedRemove(0) else b = 0;
                    },
                    Mode.Lifo => {
                        b = self.tx.popOrNull() orelse 0;
                    },
                }
                return b;
            },
            TX + 1 => @as(u8, @truncate(self.tx.items.len)),
            CTRL => if (self.mode == Mode.Fifo) 0 else 1, //TODO: update docs with new mode
            else => 0,
        };
        } else {
            return 0;
        }
    }

    pub fn receiveOne(self: *Self, socket: network.Socket) !void {
        const byte = socket.reader().readByte() catch 255;
        self.write(TX, byte);
    }

    pub fn transmitOne(self: *Self, socket: network.Socket) !void {
        if (self.rx.items.len != 0) {
            const byte = self.rx.swapRemove(0);
            const b = [_]u8{byte};
            _ = try socket.send(&b);
        }
    }

    pub fn cycleOne(self: *Self, socket: network.Socket) !void {
        if (self.use) {
        try self.receiveOne(socket);
        try self.transmitOne(socket);
        }
    }
};
