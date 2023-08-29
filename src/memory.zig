///Error! The bus manages reads and writes to memory
const std = @import("std");
const arch = @import("root").arch;

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

pub const PageTableEntry = packed struct { physical_addr: u12, w: bool, x: bool, dirty: bool, present: bool };

pub const PhysicalDescriptor = struct { addr: u16, w: bool, x: bool };

pub const PhysicalAddr = struct { addr: u24, w: bool, x: bool };

const Tlb = struct {
    const Self = @This();
    tlb: std.AutoHashMap(u16, PhysicalDescriptor),

    pub fn init(allocator: std.mem.Allocator) Tlb {
        return Tlb{ .tlb = std.AutoHashMap(u16, PhysicalDescriptor).init(allocator) };
    }

    pub fn record(self: *Self, linear: u16, physical: PhysicalDescriptor) !void {
        try self.tlb.put(linear, physical);
    }

    pub fn invalidate(self: *Self, entry: u16) void {
        _ = self.tlb.remove(entry);
    }

    pub fn get(self: *Self, linear: u16) ?PhysicalDescriptor {
        return self.tlb.get(linear);
    }
};

pub const Mmu = struct {
    pub const Self = @This();
    const PT_SHIFT: u5 = @popCount(@as(u32, @truncate(arch.MmuSizes.PageTableEntries)));
    const PT_MASK: u24 = (arch.PageDirectoryEntries) << PT_SHIFT;
    const PD_SHIFT: u5 = @popCount(@as(u32, @truncate(arch.PageDirectoryEntries))) + PT_SHIFT;
    const PD_MASK: u24 = arch.PageDirectoryEntries - 1;
    const OFFSET_MASK: u24 = arch.MmuSizes.Page - 1;
    tlb: Tlb,
    bus_main: *MainBus,
    bus_data: *DataBus,

    pub fn init(allocator: std.mem.Allocator, bus_main: *MainBus, bus_data: *DataBus) Mmu {
        return Mmu{
            .tlb = Tlb.init(allocator),
            .bus_main = bus_main,
            .bus_data = bus_data,
        };
    }

    pub fn deinit(self: *Self) void {
        self.tlb.tlb.clearAndFree();
    }

    pub fn translate(self: *Self, linear: u24, directory: u16) !PhysicalAddr {
        const entry = self.tlb.get(@as(u16, @truncate(linear >> PT_SHIFT)));
        if (entry != null)
            return PhysicalAddr{ .addr = @as(u24, entry.?.addr) << PT_SHIFT | linear & OFFSET_MASK, .w = entry.?.w, .x = entry.?.x }
        else {
            const table_entry =
                @as(u24, @as(PageDirectoryEntry, @bitCast(try self.bus_data.readBeu16(directory + @as(u16, @truncate(linear >> PD_SHIFT)) & @as(u16, @truncate(PD_MASK))))).physical_addr) << PT_SHIFT;
            const page_offset = (linear >> PT_SHIFT) & PT_MASK;
            const page = @as(PageTableEntry, @bitCast(try self.bus_main.readBeu16(table_entry | page_offset)));
            const w = page.w;
            const x = page.x;
            if (!page.present) {
                std.debug.print("Page Fault: {any}", .{page});
                return error.PageFault;
            }
            const page_addr = @as(u24, page.physical_addr) << PT_SHIFT;
            const physical = page_addr | (linear & OFFSET_MASK);
            try self.tlb.record(@as(u16, @truncate(linear >> PT_MASK)), PhysicalDescriptor{ .addr = (@as(u16, @truncate(page_addr >> PT_MASK))), .w = w, .x = x });
            return PhysicalAddr{ .addr = physical, .w = w, .x = x };
            // TODO: account for the flags and those things Implement dirty bit
        }
    }
};
