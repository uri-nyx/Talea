//! This module implements a simple bare emulatro for the Taleä CPU
const std = @import("std");
const clap = @import("clap");
const network = @import("network");

const cpu = @import("cpu.zig");
pub const arch = @import("arch");
const memory = @import("memory");
const serial = @import("peripherals/serial.zig");
const storage = @import("peripherals/storage.zig");
const timers = @import("peripherals/timer.zig");
const video = @import("peripherals/video.zig");
const keyboard = @import("peripherals/keyboard.zig");

// DEFAULTS
var firmware_path: []const u8 = "utils/firmware/default.bin";
var with_serial: bool = false;
var serial_port: u16 = 65432;
var master_frequency: usize = 10_000_000; // in hz

const Device0 = struct {
    pub const Self = @This();

    ser: *serial.Serial,
    timer: *timers.Timer16,
    kb: *keyboard.Keyboard,
    dev: memory.Device,

    pub fn init(ser: *serial.Serial, timer: *timers.Timer16, kb: *keyboard.Keyboard) Device0 {
        return Device0{ .ser = ser, .timer = timer, .kb = kb, .dev = memory.Device{
            .write = write,
            .read = read,
        } };
    }

    pub fn write(dev: *memory.Device, address: u4, data: u8) void {
        const self = @fieldParentPtr(Device0, "dev", dev);
        if (address < (self.timer.base)) {
            self.ser.write(address, data);
        } else if (address < self.kb.base) {
            self.timer.write(address, data);
        } else {
            // writing to Keyboard is a NOP
        }
    }

    pub fn read(dev: *memory.Device, address: u4) u8 {
        const self = @fieldParentPtr(Device0, "dev", dev);
        if (address < (self.timer.base)) {
            return self.ser.read(address);
        } else if (address < self.kb.base) {
            return self.timer.read(address);
        } else {
            return self.kb.read(address);
        }
    }
};

const Device1 = struct {
    pub const Self = @This();

    gpu: *video.VideoDevice,
    dev: memory.Device,

    pub fn init(gpu: *video.VideoDevice) Device1 {
        return Device1{ .gpu = gpu, .dev = memory.Device{
            .write = write,
            .read = read,
        } };
    }

    pub fn write(dev: *memory.Device, address: u4, data: u8) void {
        const self = @fieldParentPtr(Device1, "dev", dev);
        self.gpu.write(address, data);
    }

    pub fn read(dev: *memory.Device, address: u4) u8 {
        const self = @fieldParentPtr(Device1, "dev", dev);
        return self.gpu.read(address);
    }
};

const Device2 = struct {
    pub const Self = @This();

    disk: *storage.DiskController,
    tps: *storage.TpsController,
    dev: memory.Device,

    pub fn init(disk: *storage.DiskController, tps: *storage.TpsController) Device2 {
        return Device2{ .disk = disk, .tps = tps, .dev = memory.Device{
            .write = write,
            .read = read,
        } };
    }

    pub fn write(dev: *memory.Device, address: u4, data: u8) void {
        const self = @fieldParentPtr(Device2, "dev", dev);
        if (address < (self.disk.base)) {
            self.tps.write(address, data);
        } else {
            self.disk.write(address, data);
        }
    }

    pub fn read(dev: *memory.Device, address: u4) u8 {
        const self = @fieldParentPtr(Device2, "dev", dev);
        if (address < (self.disk.base)) {
            return self.tps.read(address);
        } else {
            return self.disk.read(address);
        }
    }
};

fn teletype_receive(port: *serial.Serial, client: network.Socket) !void {
    while (true) {
        try port.receiveOne(client);
        std.time.sleep(100);
    }
}

pub fn main() !void {
    // ALLOCATOR ==============================================================
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer {
        const deinit_status = gpa.deinit();
        if (deinit_status == .leak) std.debug.print("Memory leaked", .{});
    }
    var arena = std.heap.ArenaAllocator.init(gpa.allocator());
    defer arena.deinit();
    const allocator = arena.allocator();

    // Command line arguments
    const params = comptime clap.parseParamsComptime(
        \\-h, --help                Display this help and exit.
        \\-f, --firmware <str>      Path to a firmware binary. 
        \\-s, --serial <u16>        Open the serial line on the specidfied port (default 65432).
        \\-z, --frequency <usize>   Frequency of the system (in Mhz, default 10)
        \\
    );

    // Initialize our diagnostics, which can be used for reporting useful errors.
    // This is optional. You can also pass `.{}` to `clap.parse` if you don't
    // care about the extra information `Diagnostics` provides.
    var diag = clap.Diagnostic{};
    var res = clap.parse(clap.Help, &params, clap.parsers.default, .{
        .diagnostic = &diag,
    }) catch |err| {
        // Report useful error and exit
        diag.report(std.io.getStdErr().writer(), err) catch {};
        return err;
    };
    defer res.deinit();

    if (res.args.help != 0)
        return clap.help(std.io.getStdErr().writer(), clap.Help, &params, .{});
    if (res.args.firmware) |f|
        firmware_path = f;
    if (res.args.serial) |port| {
        with_serial = true;
        serial_port = port;
    }
    if (res.args.frequency) |mhz| {
        master_frequency = mhz * 1_000_000;
    }

    // CPU ====================================================================

    // registers
    const register_file = try allocator.create([cpu.Register.count]u32);
    const main_memory = try allocator.create([arch.MAIN_SIZE]u8);
    @memset(register_file, 0);
    @memset(main_memory, 0);
    defer allocator.destroy(register_file);
    defer allocator.destroy(main_memory);

    // irq
    var irq = cpu.InterruptController.init();

    // MAIN BUS ===============================================================
    var main_bus = memory.MainBus{
        .mem = main_memory,
    };

    // SERIAL =================================================================
    var serial_controller = try serial.Serial.init(master_frequency, &irq, allocator, with_serial);
    defer serial_controller.deinit();

    // STORAGE ================================================================
    var disk_drive = storage.DiskDrive.init();
    defer disk_drive.deinit();
    var tps_drive = storage.TpsDrive.init();
    var disk_controller = storage.DiskController.init(&disk_drive, &main_bus, 6, master_frequency, allocator);
    var tps_controller = storage.TpsController.init(&tps_drive, &main_bus, 0, master_frequency, allocator);
    defer disk_controller.deinit(allocator);
    defer tps_controller.deinit(allocator);

    // TIMER ==================================================================
    var timer = timers.Timer16.init(master_frequency, comptime 6, &irq);

    // KEYBOARD ===============================================================
    var kb = keyboard.Keyboard.init(comptime 0xc, &irq);

    // GPU ====================================================================
    var gpu = try video.VideoDevice.init(master_frequency, &irq, &main_bus, &kb);

    // DEVICE INSTALLATION ====================================================
    const numdevices = 3;
    var devices = try allocator.alloc(*memory.Device, arch.DATA_SIZE / arch.DeviceSize);
    defer allocator.free(devices);

    var blocks = try allocator.alloc(memory.MemoryDevice, (arch.DATA_SIZE / arch.DeviceSize) - numdevices);
    for (blocks, 0..) |_, i| {
        blocks[i] = try memory.MemoryDevice.init(allocator);
    }
    defer allocator.free(blocks);

    for (devices[numdevices..], 0..) |_, i| {
        devices[i] = &blocks[i].device;
    }

    var device0 = Device0.init(&serial_controller, &timer, &kb);
    var device1 = Device1.init(&gpu);
    var device2 = Device2.init(&disk_controller, &tps_controller);

    devices[0] = &device0.dev;
    devices[1] = &device1.dev;
    devices[2] = &device2.dev;
    var data_bus = memory.DataBus{
        .devices = devices,
    };

    // MMU ====================================================================
    var mmu = memory.Mmu.init(allocator, &main_bus, &data_bus);
    defer mmu.deinit();

    // FIRMWARE LOADING ========================================================
    const file = try std.fs.cwd().openFile(firmware_path, .{ .mode = .read_only });
    var program = try allocator.alloc(u8, 131_072);
    defer allocator.free(program);
    const flen = try file.readAll(program);

    _ = try main_bus.write(0, program[0..flen]);

    // INSTANTIATE CPU ========================================================
    var sirius = cpu.Sirius.init(master_frequency, register_file, &irq, &main_bus, &data_bus, &mmu, allocator);
    irq.cpu = &sirius;

    // OPEN SERIAL PORT ========================================================
    if (with_serial) {
        var sock = try network.Socket.create(.ipv4, .tcp);
        defer sock.close();
        try sock.bindToPort(serial_port);
        try sock.listen();
        var client = try sock.accept();

        serial_controller.socket = &client;
        var handle_receive = try std.Thread.spawn(.{ .stack_size = 1000 }, teletype_receive, .{ &serial_controller, client }); // TODO: This is suuuper hacky but the only workaround I can find for now without async
        handle_receive.detach();
    }

    // MAIN LOOP and FRAME TIMING =============================================
    const frame = (16 * std.time.ns_per_ms / (std.time.ns_per_s / sirius.frequency));
    const timer_cycles_per_tick = sirius.frequency / timer.frequency;

    var alive = true;
    while (alive) {
        var target = sirius.cycles + frame;

        while (sirius.cycles < target) {
            try sirius.checkExceptions();
            if (sirius.cycles % timer_cycles_per_tick == 0) {
                timer.cycleOne();
            }
        }
        alive = try gpu.update();
        gpu.sync(alive);
    }
}
