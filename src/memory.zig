///Error! The bus manages reads and writes to memory
const std = @import("std");
const arch = @import("root").arch;

pub const Sys = enum(u4) {
    memSize,
    clock,
    err,
    power,
    year,
    month,
    day,
    hour,
    minute,
    second,
    reservedA,
    reservedB,
    deviceNum,
    archID,
    vendorID,
    reservedF,
};

pub const MainBus = struct {
    pub const Error = error{BusError};
    pub const Self = @This();
    mem: *[arch.MAIN_SIZE]u8,

    pub fn write(self: *Self, address: u24, data: []const u8) Error!usize {
        if (address + data.len > self.mem.len) {
            return error.BusError;
        }
        std.mem.copy(u8, self.mem[address .. address + data.len], data);
        return data.len;
    }

    pub fn read(self: *Self, address: u24, buff: []u8) Error!usize {
        if (address + buff.len > self.mem.len) {
            return error.BusError;
        }
        const data = self.mem[address .. address + buff.len];
        std.mem.copy(u8, buff, data);
        return data.len;
    }

    pub inline fn readBeu32(self: *Self, address: u24) Error!u32 {
        var buff = [4]u8{ 0, 0, 0, 0 };
        _ = try self.read(address, buff[0..]);
        const data =
            @as(u32, buff[0]) << 24 |
            @as(u32, buff[1]) << 16 |
            @as(u32, buff[2]) << 8 |
            @as(u32, buff[3]);

        return data;
    }

    pub inline fn readBeu16(self: *Self, address: u24) Error!u16 {
        var buff = [2]u8{ 0, 0 };
        _ = try self.read(address, buff[0..]);
        return @as(u16, buff[0]) << 8 |
            @as(u16, buff[1]);
    }

    pub inline fn readu8(self: *Self, address: u24) Error!u8 {
        var buff = [1]u8{0};
        _ = try self.read(address, buff[0..]);
        return buff[0];
    }

    pub inline fn writeBeu32(self: *Self, address: u24, data: u32) Error!usize {
        const word = [4]u8{
            @as(u8, @truncate(data >> 24)),
            @as(u8, @truncate(data >> 16)),
            @as(u8, @truncate(data >> 8)),
            @as(u8, @truncate(data & 0xff)),
        };
        return self.write(address, word[0..]);
    }

    pub inline fn writeBeu16(self: *Self, address: u24, data: u16) Error!usize {
        const word = [2]u8{
            @as(u8, @truncate(data >> 8)),
            @as(u8, @truncate(data)),
        };
        return self.write(address, word[0..]);
    }

    pub inline fn writeu8(self: *Self, address: u24, data: u8) Error!usize {
        const word = [1]u8{data};
        return self.write(address, word[0..]);
    }
};

pub const Device = struct {
    write: *const fn (*Device, u4, u8) void,
    read: *const fn (*Device, u4) u8,

    pub fn write(dev: *Device, address: u4, data: u8) void {
        return dev.write(address, data);
    }

    pub fn read(dev: *Device, address: u4) u8 {
        return dev.read(address);
    }
};

pub const MemoryDevice = struct {
    const Self = @This();
    mem: *[arch.DeviceSize]u8,
    device: Device,

    pub fn init(allocator: std.mem.Allocator) !MemoryDevice {
        var mem = try allocator.create([arch.DeviceSize]u8);
        @memset(mem, 0);
        return MemoryDevice{
            .mem = mem,
            .device = Device{
                .write = write,
                .read = read,
            },
        };
    }

    pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
        allocator.destroy(self.mem);
    }

    fn write(dev: *Device, address: u4, data: u8) void {
        const self = @fieldParentPtr(MemoryDevice, "device", dev);
        self.mem[address] = data;
    }

    fn read(dev: *Device, address: u4) u8 {
        const self = @fieldParentPtr(MemoryDevice, "device", dev);
        return self.mem[address];
    }
};

pub const DataBus = struct {
    pub const Self = @This();
    pub const Error = error{BusError};
    devices: []*Device, // The devices in data memory shall be divided in 16 byte chunks

    pub fn write(self: *Self, address: u16, data: []const u8) Error!usize {
        if (address + data.len > self.devices.len * 16) {
            return error.BusError;
        }

        var i: u16 = 0;
        while (i < data.len) : (i += 1) {
            const byte_address = address + i;
            var dev = self.devices[@as(usize, byte_address >> 4)];
            dev.write(dev, @as(u4, @truncate(byte_address)), data[i]);
        }
        return data.len;
    }

    pub fn read(self: *Self, address: u16, buff: []u8) Error!usize {
        if (address + buff.len > self.devices.len * 16) {
            return error.BusError;
        }
        var i: u16 = 0;
        while (i < buff.len) : (i += 1) {
            const byte_address = address + i;
            var dev = self.devices[@as(usize, byte_address >> 4)];
            buff[i] = dev.read(dev, @as(u4, @truncate(byte_address)));
        }
        return buff.len;
    }

    pub inline fn readBeu32(self: *Self, address: u16) Error!u32 {
        var buff = [4]u8{ 0, 0, 0, 0 };
        _ = try self.read(address, buff[0..]);
        const v = @as(u32, buff[0]) << 24 |
            @as(u32, buff[1]) << 16 |
            @as(u32, buff[2]) << 8 |
            @as(u32, buff[3]);
        return v;
    }

    pub inline fn readBeu16(self: *Self, address: u16) Error!u16 {
        var buff = [2]u8{ 0, 0 };
        _ = try self.read(address, buff[0..]);
        return @as(u16, buff[0]) << 8 |
            @as(u16, buff[1]);
    }

    pub inline fn readu8(self: *Self, address: u16) Error!u8 {
        var buff = [1]u8{0};
        _ = try self.read(address, buff[0..]);
        return buff[0];
    }

    pub inline fn writeBeu32(self: *Self, address: u16, data: u32) Error!usize {
        const word = [4]u8{ @as(u8, @truncate(data >> 24)), @as(u8, @truncate(data >> 16)), @as(u8, @truncate(data >> 8)), @as(u8, @truncate(data)) };
        return self.write(address, word[0..]);
    }
    pub inline fn writeBeu16(self: *Self, address: u16, data: u16) Error!usize {
        const word = [2]u8{ @as(u8, @truncate(data >> 8)), @as(u8, @truncate(data)) };
        return self.write(address, word[0..]);
    }

    pub inline fn writeu8(self: *Self, address: u16, data: u8) Error!usize {
        const word = [1]u8{data};
        return self.write(address, &word);
    }
};

pub const PageDirectoryEntry = packed struct { physical_addr: u12, reserved: u4 };

pub const PageTableEntry = packed struct { 
    physical_addr: u24, 
    w: bool, 
    x: bool, 
    dirty: bool, 
    present: bool,
    mapped: bool,
};

pub const PhysicalDescriptor = struct { addr: u16, w: bool, x: bool };

pub const PhysicalAddr = struct { addr: u24, w: bool, x: bool}; //TODO: maybe readable bit too?

pub const Mmu = struct {
    pub const Self = @This();
    page_table: [4096]PageTableEntry,
    bus_main: *MainBus,
    bus_data: *DataBus,

    pub fn init(bus_main: *MainBus, bus_data: *DataBus) Mmu {
        return Mmu{
            .page_table = [_]PageTableEntry{.{.physical_addr = 0, .w = false, .x = false, .dirty = false, .present = false, .mapped = false}} ** 4096,
            .bus_main = bus_main,
            .bus_data = bus_data,
        };
    }

    pub fn set_page_table(self: *Self, pointer: u24, length: u12) !void {
        std.debug.print("Set Page table from 0x{x}\n", .{pointer});
        for (0..length) |i| {
            const raw = try self.bus_main.readBeu16(pointer +% @as(u24, @truncate(i * 2)));
            const physical: u24 = (raw & 0xfff0) << 8;
            const w = if (raw & 0x8 != 0) true else false;
            const x = if (raw & 0x4 != 0) true else false;
            self.map(physical, @truncate(i), w, x);
        }
    }

    pub fn map(self: *Self, physical: u24, linear: u24, w: bool, x: bool) void {
        self.page_table[linear >> 12] = PageTableEntry{
            .physical_addr = physical & 0xfff000,
            .w = w, .x = x, .dirty = false, .present = true, .mapped = true
        };
        std.debug.print("Mapped Real 0x{x} to Linear 0x{x} (index: 0x{x}) (w:{}, x:{})\n", .{physical, linear, linear >> 12, w, x});
    }

    pub fn unmap(self: *Self, linear: u24) void {
        self.page_table[linear >> 12] = PageTableEntry{
            .physical_addr = 0,
            .w = false, .x = false, .dirty = false, .present = false, .mapped = false
        };
    }

    pub fn update(self: *Self, linear: u24, dirty: bool, present: bool) void {
        self.page_table[linear >> 12].dirty = dirty;
        self.page_table[linear >> 12].present = present;
    }

    pub fn status(self: *Self, linear: u24) u32 { 
        const entry = self.page_table[linear >> 12];
        if (!entry.mapped) 
            return 0xff_ff_ff_ff;
        const stat: u32 = @as(u32, @intFromBool(entry.x)) << 3 | 
                          @as(u32, @intFromBool(entry.w)) << 2 | 
                          @as(u32, @intFromBool(entry.dirty)) << 1 | 
                          @as(u32, @intFromBool(entry.present));
        return stat;
    }

    pub fn translate(self: *Self, linear: u24) !PhysicalAddr {
        const offset = linear & 0xfff;
        const index =  linear >> 12;
        const table_entry = self.page_table[index];
        if (!table_entry.mapped) {
            std.debug.print("Page 0x{} not mapped (from linear: 0x{x})\n", .{index, linear});
            return error.PageFault;
        } else if (!table_entry.present) {
            std.debug.print("Page 0x{} not present (from linear: 0x{x})\n", .{index, linear});
            return error.PageFault;
        }
        return PhysicalAddr{.addr = table_entry.physical_addr + offset, .w = table_entry.w, .x = table_entry.x};
    }
};
