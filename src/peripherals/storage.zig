const std = @import("std");
const memory = @import("memory");
const arch = @import("arch");

pub const Command = enum(u8) {
    nop,
    store,
    load,
    isBootable,
    open,
    close,
    setCurrent,
};

pub const Sector = struct { data: [arch.SectorSize]u8 };

pub const Disk = struct {
    pub var path_buf: [arch.DiskPath.len + 8]u8 = undefined;
    pub const Self = @This();
    pub const Sectors: u16 = 65535;
    file_name: []const u8,
    fd: std.fs.File,

    pub fn create(file_name: []const u8) Self {
        @memset(&path_buf, 0);
        const path = std.fmt.bufPrint(&path_buf, "{s}{s}", .{ arch.DiskPath, file_name }) catch |err| std.debug.panic("Failed to concatenate disk name: {s}\n", .{@errorName(err)});
        const fd = std.fs.cwd().createFile(path, std.fs.File.CreateFlags{
            .read = true,
            .truncate = false,
        }) catch |err| std.debug.panic("Error creating Disk File {s}\n", .{@errorName(err)});
        fd.writer().writeByteNTimes(0, @as(usize, Disk.Sectors) * arch.SectorSize) catch |err| std.debug.panic("Failed to initialize tps file: {s}\n", .{@errorName(err)});
        return Self{ .file_name = file_name, .fd = fd };
    }

    pub fn open(file_name: []const u8) !Self {
        @memset(&path_buf, 0);
        const path = std.fmt.bufPrint(&path_buf, "{s}{s}", .{ arch.DiskPath, file_name }) catch |err| std.debug.panic("Failed to concatenate disk name: {s}\n", .{@errorName(err)});
        const fd = std.fs.cwd().openFile(path, std.fs.File.OpenFlags{ .mode = std.fs.File.OpenMode.read_write }) catch return error.UnableToOpen;
        return Self{ .file_name = file_name, .fd = fd };
    }

    pub fn load(self: *Self, sector: u16, data: *Sector) void {
        self.fd.seekTo(@as(u64, sector) * arch.SectorSize) catch |err| std.debug.panic("Error seting stream pointer: {s}\n", .{@errorName(err)});
        _ = self.fd.read(&data.data) catch |err| std.debug.panic("Error reading file: {s}\n", .{@errorName(err)});
    }

    pub fn store(self: *Self, sector: u16, data: *Sector) void {
        self.fd.seekTo(@as(u64, sector) * arch.SectorSize) catch |err| std.debug.panic("Error seting stream pointer: {s}\n", .{@errorName(err)});
        _ = self.fd.write(&data.data) catch |err| std.debug.panic("Error writing file: {s}\n", .{@errorName(err)});
    }
};

pub const DiskDrive = struct {
    const Self = @This();
    disk: [4]Disk,
    current: u2,

    pub fn init() Self {
        var i: usize = 0;
        var disk: [4]Disk = undefined;
        while (i < 4) : (i += 1) {
            const id = [_]u8{@as(u8, @truncate(i)) + '0'};
            disk[i] =
                Disk.open(&id) catch Disk.create(&id);
            const stat = disk[i].fd.stat() catch |err| std.debug.panic("Error getting disk file stat(): {s}", .{@errorName(err)});
            if (stat.size != @as(u64, Disk.Sectors) * arch.SectorSize) {
                std.debug.print("Disk file for disk {s} is corrupted (actual size: {d}, expected: {d})\n", .{ id, stat.size, @as(u64, Disk.Sectors) * arch.SectorSize });
            }
        }
        return Self{
            .disk = disk,
            .current = 0,
        };
    }

    pub fn deinit(self: *Self) void {
        for (self.disk) |disk| {
            disk.fd.close();
        }
    }
};

pub const DiskController = struct {
    const Self = @This();
    pub const COMMAND: u4 = 0;
    pub const DATA: u4 = 1;
    pub const SECTORH: u4 = 2;
    pub const SECTORL: u4 = 3;
    pub const POINTH: u4 = 4;
    pub const POINTL: u4 = 5;
    pub const STATUSH: u4 = 6;
    pub const STATUSL: u4 = 7;
    comptime base: u4 = 6,
    registers: [8]u8,
    bus: *memory.MainBus,
    drive: *DiskDrive,
    input_sector: *Sector,
    output_sector: *Sector,
    frequency: u64,

    pub fn init(drive: *DiskDrive, bus: *memory.MainBus, comptime base_addr: u4, frequency: u64, allocator: std.mem.Allocator) Self {
        const input_sector = allocator.create(Sector) catch |err| std.debug.panic("Failed to allocate temp sector: {s}", .{@errorName(err)});
        const output_sector = allocator.create(Sector) catch |err| std.debug.panic("Failed to allocate temp sector: {s}", .{@errorName(err)});
        return Self{
            .base = base_addr,
            .registers = [8]u8{
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
            },
            .drive = drive,
            .frequency = frequency,
            .bus = bus,
            .input_sector = input_sector,
            .output_sector = output_sector,
        };
    }

    pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
        allocator.destroy(self.input_sector);
        allocator.destroy(self.output_sector);
    }

    pub fn write(self: *Self, address: u4, data: u8) void {
        switch (address) {
            self.base + COMMAND => {
                self.execute(data) catch |err| std.debug.panic("This should not panic in all cases, but raise an exception {s}\n", .{@errorName(err)});
            },
            else => {
                if (address - self.base < self.base + 8)
                    self.registers[address - self.base] = data
                else
                    return;
            },
        }
    }

    pub fn read(self: *Self, address: u4) u8 {
        return if (self.base - address < 8) self.registers[address - self.base] else 0;
    }

    fn execute(self: *Self, command: u8) !void {
        switch (command) {
            @intFromEnum(Command.nop) => {},
            @intFromEnum(Command.store) => {
                self.drive.current = @as(u2, @truncate(self.registers[DATA]));
                const sector = (@as(u16, self.registers[SECTORH]) << 8 | self.registers[SECTORL]);
                const point = (@as(u24, self.registers[POINTH]) << 8 | self.registers[POINTL]) * arch.SectorSize;
                _ = try self.bus.read(point, &self.input_sector.data);
                self.drive.disk[self.drive.current].store(sector, self.input_sector);
            },
            @intFromEnum(Command.load) => {
                std.debug.print("Loading Sector...", .{});
                self.drive.current = @as(u2, @truncate(self.registers[DATA]));
                const sector = (@as(u16, self.registers[SECTORH]) << 8 | self.registers[SECTORL]);
                const point = (@as(u24, self.registers[POINTH]) << 8 | self.registers[POINTL]) * arch.SectorSize;
                self.drive.disk[self.drive.current].load(sector, self.output_sector);
                std.debug.print("{x}\n", .{point});
                _ = try self.bus.write(point, &self.input_sector.data);
            },
            @intFromEnum(Command.isBootable) => {
                std.debug.print("Testing if disk {s} is bootable... ", .{self.drive.disk[self.drive.current].file_name});
                self.drive.current = @as(u2, @truncate(self.registers[DATA]));
                self.drive.disk[self.drive.current].load(0, self.output_sector);
                if (self.output_sector.data[arch.SectorSize - 2] == 0x55 and self.output_sector.data[arch.SectorSize - 1] == 0xAA) {
                    self.registers[STATUSL] = 1;
                    std.debug.print("Disk {s} is bootable!\n", .{self.drive.disk[self.drive.current].file_name});
                } else {
                    std.debug.print("Disk {s} isn't bootable!\n", .{self.drive.disk[self.drive.current].file_name});
                    self.registers[STATUSL] = 0;
                }
            },
            else => {
                self.registers[STATUSH] = 0xde;
                self.registers[STATUSL] = 0xad;
            },
        }
    }
};

pub const Tps = struct {
    pub const Self = @This();
    pub var path_buf: [arch.TpsPath.len + 8]u8 = undefined;
    pub const Sectors: u8 = 255;
    file_name: []const u8,
    fd: std.fs.File,

    pub fn create(file_name: []const u8) Self {
        @memset(&path_buf, 0);
        const path = std.fmt.bufPrint(&path_buf, "{s}{s}", .{ arch.TpsPath, file_name }) catch |err| std.debug.panic("Failed to concatenate tps name: {s}\n", .{@errorName(err)});
        const fd = std.fs.cwd().createFile(path, std.fs.File.CreateFlags{
            .read = true,
            .truncate = false,
        }) catch |err| std.debug.panic("Error creating Tps File {s}\n", .{@errorName(err)});

        fd.writer().writeByteNTimes(0, @as(usize, Tps.Sectors) * arch.SectorSize) catch |err| std.debug.panic("Failed to initialize tps file: {s}\n", .{@errorName(err)});
        return Self{ .file_name = file_name, .fd = fd };
    }

    pub fn open(file_name: []const u8) !Self {
        @memset(&path_buf, 0);
        const path = std.fmt.bufPrint(&path_buf, "{s}{s}", .{ arch.TpsPath, file_name }) catch |err| std.debug.panic("Failed to concatenate tps name: {s}\n", .{@errorName(err)});
        const fd = std.fs.cwd().openFile(path, std.fs.File.OpenFlags{ .mode = std.fs.File.OpenMode.read_write }) catch return error.UnableToOpen;
        return Self{ .file_name = file_name, .fd = fd };
    }

    pub fn load(self: *Self, sector: u8, data: *Sector) void {
        self.fd.seekTo(@as(u64, sector) * arch.SectorSize) catch |err| std.debug.panic("Error seting stream pointer: {s}\n", .{@errorName(err)});
        _ = self.fd.read(&data.data) catch |err| std.debug.panic("Error reading file: {s}\n", .{@errorName(err)});
    }

    pub fn store(self: *Self, sector: u8, data: *Sector) void {
        self.fd.seekTo(@as(u64, sector) * arch.SectorSize) catch |err| std.debug.panic("Error seting stream pointer: {s}\n", .{@errorName(err)});
        _ = self.fd.write(&data.data) catch |err| std.debug.panic("Error writing file: {s}\n", .{@errorName(err)});
    }
};

pub const TpsDrive = struct {
    const Self = @This();
    tps: [2]Tps,
    current: u1,

    pub fn init() Self {
        var tps: [2]Tps = undefined;
        tps[0] = Tps.open("A") catch Tps.create("A");
        tps[1] = Tps.open("B") catch Tps.create("B");

        check(&tps[0]);
        check(&tps[1]);

        return Self{
            .tps = tps,
            .current = 0,
        };
    }

    fn check(tps: *Tps) void {
        const stat = tps.fd.stat() catch |err| std.debug.panic("Error getting TPS file stat(): {s}", .{@errorName(err)});
        if (stat.size != @as(u64, Tps.Sectors) * arch.SectorSize) {
            std.debug.print("TPS file for tps {s} is corrupted (actual size: {d}, expected: {d})\n", .{ tps.file_name, stat.size, @as(u64, Tps.Sectors) * arch.SectorSize });
        }
    }
};

pub const TpsController = struct {
    const Self = @This();
    pub const COMMAND: u4 = 0;
    pub const DATA: u4 = 1;
    pub const POINTH: u4 = 2;
    pub const POINTL: u4 = 3;
    pub const STATUSH: u4 = 4;
    pub const STATUSL: u4 = 5;
    comptime base: u4 = 0,
    registers: [6]u8,
    drive: *TpsDrive,
    input_sector: *Sector,
    output_sector: *Sector,
    bus: *memory.MainBus,
    frequency: u64,

    pub fn init(drive: *TpsDrive, bus: *memory.MainBus, comptime base_addr: u4, frequency: u64, allocator: std.mem.Allocator) Self {
        return Self{
            .base = base_addr,
            .registers = [6]u8{
                0,
                0,
                0,
                0,
                0,
                0,
            },
            .drive = drive,
            .bus = bus,
            .frequency = frequency,
            .input_sector = allocator.create(Sector) catch |err| std.debug.panic("Failed to allocate temp sector: {s}", .{@errorName(err)}),
            .output_sector = allocator.create(Sector) catch |err| std.debug.panic("Failed to allocate temp sector: {s}", .{@errorName(err)}),
        };
    }

    pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
        allocator.destroy(self.input_sector);
        allocator.destroy(self.output_sector);
    }

    pub fn write(self: *Self, address: u4, data: u8) void {
        switch (address) {
            self.base + COMMAND => {
                self.execute(data) catch |err| std.debug.panic("This should not panic in all cases, but raise an exception {s}\n", .{@errorName(err)});
            },
            else => {
                if (address - self.base < 6)
                    self.registers[address - self.base] = data
                else
                    return;
            },
        }
    }

    pub fn read(self: *Self, address: u4) u8 {
        std.debug.print("Reading from tps: addr {}\n", .{address});
        return if (address - self.base < 6) self.registers[address - self.base] else 0;
    }

    fn execute(self: *Self, command: u8) !void {
        switch (command) {
            @intFromEnum(Command.nop) => {},
            @intFromEnum(Command.store) => {
                const sector = self.registers[DATA];
                const point = (@as(u24, self.registers[POINTH]) << 8 | self.registers[POINTL]) * arch.SectorSize;
                _ = try self.bus.read(point, &self.input_sector.data);
                self.drive.tps[self.drive.current].store(sector, self.input_sector);
            },
            @intFromEnum(Command.load) => {
                std.debug.print("Loading from TPS\n", .{});
                const sector = self.registers[DATA];
                const point = (@as(u24, self.registers[POINTH]) << 8 | self.registers[POINTL]) * arch.SectorSize;
                self.drive.tps[self.drive.current].load(sector, self.output_sector);
                _ = try self.bus.write(point, &self.input_sector.data);
            },
            @intFromEnum(Command.isBootable) => {
                std.debug.print("Testing if Tps {s} is bootable... ", .{self.drive.tps[self.drive.current].file_name});
                self.drive.tps[self.drive.current].load(0, self.output_sector);
                if (self.output_sector.data[arch.SectorSize - 2] == 0x55 and self.output_sector.data[arch.SectorSize - 1] == 0xAA) {
                    self.registers[STATUSL] = 1;
                    std.debug.print("Tps {s} is bootable!\n", .{self.drive.tps[self.drive.current].file_name});
                } else {
                    self.registers[STATUSL] = 0;
                    std.debug.print("Tps {s} isn't bootable!\n", .{self.drive.tps[self.drive.current].file_name});
                }
            },
            @intFromEnum(Command.open) => {
                std.debug.print("Opening TPS {s}\n", .{self.drive.tps[self.drive.current].file_name});
                self.drive.tps[self.drive.current] = Tps.open(self.drive.tps[self.drive.current].file_name) catch std.debug.panic("Failed to open TPS", .{});
            },
            @intFromEnum(Command.close) => {
                self.drive.tps[self.drive.current].fd.close();
            },
            @intFromEnum(Command.setCurrent) => {
                self.drive.current = @as(u1, @truncate(self.registers[DATA]));
            },
            else => {
                self.registers[STATUSH] = 0xde;
                self.registers[STATUSL] = 0xad;
            },
        }
    }
};
