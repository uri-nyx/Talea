//! This module implements the video device for the Taleä System
const std = @import("std");
const arch = @import("arch");
const cpu = @import("../cpu.zig");
const memory = @import("memory");
const keyboard = @import("keyboard.zig");
const c = @import("../c.zig");
// We use miniFB for now, because it does exactly what we we want
// we may consider using Mach, SDL, or Rayib in the future

pub const Screen = struct {
    // A simple wrapper over the pixel buffer
    pub const Self = @This();
    width: c_uint = arch.VideoConfig.MaxW,
    height: c_uint = arch.VideoConfig.MaxH,
    window: ?*c.c.mfb_window,
    framebuffer: c.c.Olivec_Canvas,

    pub fn init(w: c_uint, h: c_uint, flags: c_uint) Self {
        var window = c.c.mfb_open_ex(arch.VideoConfig.WindowName, arch.VideoConfig.MaxW, arch.VideoConfig.MaxH, flags);
        return Self{
            .width = w,
            .height = h,
            .window = window,
            .framebuffer = c.c.OLIVEC_CANVAS_NULL,
        };
    }
};

inline fn colorRGB(r: u8, g: u8, b: u8) u32 {
    return ((@as(u32, r) << 16) | (@as(u32, g) << 8) | @as(u32, b));
}

pub const Command = enum(u8) {
    nop = 0,
    clear,
    setMode,
    setFont,
    blit,
    setFB,
    setVBlank,
    loadFont,
    loadPalette,
    setBGColor,
    setFGColor,
    clearRegs,
};

pub const VideoDevice = struct {
    pub const Self = @This();
    pub const COMMAND: u4 = 0;
    pub const DATAH: u4 = 1;
    pub const DATAM: u4 = 2;
    pub const DATAL: u4 = 3;
    pub const GPU0: u4 = 4;
    pub const GPU1: u4 = 5;
    pub const GPU2: u4 = 6;
    pub const GPU3: u4 = 7;
    pub const GPU4: u4 = 8;
    pub const GPU5: u4 = 9;
    pub const GPU6: u4 = 10;
    pub const GPU7: u4 = 11;
    pub const STATUS0: u4 = 12;
    pub const STATUS1: u4 = 13;
    pub const STATUS2: u4 = 14;
    pub const STATUS3: u4 = 15;
    registers: [16]u8,
    mode: arch.VideoMode,
    font: arch.VideoFont,
    fonts: [256]c.c.Olivec_Font,
    next_font: u8,
    vblank_enable: bool,
    blink: u128,
    palette: [16]c_uint,

    hw_charbuffer_addr: u24,
    hw_charbuffer_len: usize,

    hw_framebuffer_addr: u24,
    hw_framebuffer_w: usize,
    hw_framebuffer_h: usize,
    hw_background_color: u32,
    hw_foreground_color: u32,

    kb: *keyboard.Keyboard,
    bus: *memory.MainBus,
    frequency: u64,
    cycles: u128,
    screen: Screen,
    interrupt_controller: *cpu.InterruptController,

    pub fn init(frequency: u64, irq: *cpu.InterruptController, bus: *memory.MainBus, kb: *keyboard.Keyboard) !Self {
        var screen = Screen.init(arch.VideoConfig.DefaultW, arch.VideoConfig.DefaultH, c.c.WF_RESIZABLE);
        screen.framebuffer =
            c.c.olivec_canvas(@as(*c_uint, @ptrCast(@alignCast(&bus.mem[@as(usize, arch.VideoConfig.FramebufferDefaultAddr)]))), @as(c_uint, @truncate(arch.VideoConfig.FramebufferDefaultW)), @as(c_uint, @truncate(arch.VideoConfig.FramebufferDefaultH)), @as(c_uint, @truncate(arch.VideoConfig.FramebufferDefaultW)));
        const ibm_vga_font = c.c.wrapper_load_font("src/include/ibm.png", 8, 16, 32);
        var fonts: [256]c.c.Olivec_Font = undefined;
        fonts[0] = ibm_vga_font;

        c.c.mfb_set_user_data(screen.window, kb);
        c.c.mfb_set_keyboard_callback(screen.window, keyboard_callback);
        c.c.mfb_set_char_input_callback(screen.window, char_input_callback);
        return Self{
            .registers = [_]u8{0} ** 16,
            .vblank_enable = false,
            .blink = 0,
            .frequency = frequency,
            .interrupt_controller = irq,
            .bus = bus,
            .cycles = 0,
            .screen = screen,
            .palette = arch.VideoDefaultPalette,
            .kb = kb,

            .hw_framebuffer_addr = arch.VideoConfig.FramebufferDefaultAddr,
            .hw_framebuffer_w = arch.VideoConfig.FramebufferDefaultW,
            .hw_framebuffer_h = arch.VideoConfig.FramebufferDefaultH,
            .hw_background_color = 0,
            .hw_foreground_color = 0xff_ff_ff_ff,
            .mode = arch.VideoMode.monochromeText,
            .font = arch.VideoFont.default,
            .fonts = fonts,
            .hw_charbuffer_addr = arch.VideoConfig.CharbufferDefaultAddr,
            .hw_charbuffer_len = 80 * 30,
            .next_font = 1,
        };
    }

    fn keyboard_callback(window: ?*c.c.mfb_window, key: c.c.mfb_key, mod: c.c.mfb_key_mod, pressed: c.c.bool) callconv(.C) void {
        const kb = @as(*keyboard.Keyboard, @ptrCast(@alignCast(c.c.mfb_get_user_data(window).?)));
        //std.debug.print("Pressed key {x}\n", .{key});
        kb.keyboard_in(@as(u16, @truncate(@as(u32, @bitCast(key)))), @as(u16, @truncate(@as(u32, @bitCast(mod)))));
        _ = pressed;
    }

    fn char_input_callback(window: ?*c.c.mfb_window, char: c_uint) callconv(.C) void {
        const kb = @as(*keyboard.Keyboard, @ptrCast(@alignCast(c.c.mfb_get_user_data(window).?)));
        //std.debug.print("Character {x}\n", .{char});
        kb.character_in(@as(u8, @truncate(char)));
    }

    pub fn write(self: *Self, address: u4, data: u8) void {
        switch (address) {
            COMMAND => {
                self.execute(data);
            },
            // Writing to the Status Reg is a NOP
            STATUS0 => {},
            STATUS1 => {},
            STATUS2 => {},
            STATUS3 => {},
            else => {
                self.registers[address] = data;
            },
        }
    }

    pub fn read(self: *Self, address: u4) u8 {
        return self.registers[address];
    }

    pub fn update(self: *Self) !bool {
        self.blink += 1;
        try self.render();
        const state = c.c.mfb_update(self.screen.window, self.screen.framebuffer.pixels);
        if (state < 0) {
            // This means probably that the user closed the window. We shall terminate the program
            std.debug.print("\nTerminating Emulation\n", .{});
            return false;
        }

        return true;
    }

    pub fn sync(self: *Self, alive: bool) void {
        if (alive) {
            _ = c.c.mfb_wait_sync(self.screen.window);
        }
    }

    fn execute(self: *Self, command: u8) void {
        switch (command) {
            @intFromEnum(Command.nop) => {},
            @intFromEnum(Command.clear) => {
                c.c.olivec_fill(self.screen.framebuffer, self.hw_background_color);
            },
            @intFromEnum(Command.clearRegs) => {
                for (self.registers, 0..) |_, i| {
                    self.registers[i] = 0;
                }
            },
            @intFromEnum(Command.setMode) => {
                self.mode = @as(arch.VideoMode, @enumFromInt(self.registers[DATAH]));
                std.debug.print("Set video mode {}\n", .{self.mode});
            },
            @intFromEnum(Command.setFont) => {
                self.font = @as(arch.VideoFont, @enumFromInt(self.registers[DATAH]));
            },
            @intFromEnum(Command.setFB) => {
                self.hw_framebuffer_addr =
                    @as(u24, self.registers[DATAH]) << 16 |
                    @as(u24, self.registers[DATAM]) << 8 |
                    @as(u24, self.registers[DATAL]);

                self.screen.framebuffer =
                    c.c.olivec_canvas(@as(*c_uint, @ptrCast(@alignCast(&self.bus.mem[@as(usize, self.hw_framebuffer_addr)]))), @as(c_uint, @truncate(self.hw_framebuffer_w)), @as(c_uint, @truncate(self.hw_framebuffer_h)), @as(c_uint, @truncate(self.hw_framebuffer_w)));
            },
            @intFromEnum(Command.setVBlank) => {
                if (self.registers[DATAH] == 0)
                    self.vblank_enable = false
                else
                    self.vblank_enable = true;
            },
            @intFromEnum(Command.setBGColor) => {
                self.hw_background_color =
                    @as(u32, self.registers[GPU0]) << 24 |
                    @as(u32, self.registers[GPU1]) << 16 |
                    @as(u32, self.registers[GPU2]) << 8 |
                    @as(u32, self.registers[GPU3]);
            },
            @intFromEnum(Command.setFGColor) => {
                self.hw_foreground_color =
                    @as(u32, self.registers[GPU0]) << 24 |
                    @as(u32, self.registers[GPU1]) << 16 |
                    @as(u32, self.registers[GPU2]) << 8 |
                    @as(u32, self.registers[GPU3]);
            },
            @intFromEnum(Command.loadFont) => {
                const font_addr: u24 =
                    @as(u24, self.registers[DATAH]) << 16 |
                    @as(u24, self.registers[DATAM]) << 8 |
                    @as(u24, self.registers[DATAL]);

                const font_len: u16 =
                    @as(u16, self.registers[GPU0]) << 8 |
                    @as(u16, self.registers[GPU1]);

                self.loadFont(font_addr, font_len);
            },
            @intFromEnum(Command.loadPalette) => {
                const palette_addr: u24 =
                    @as(u24, self.registers[DATAH]) << 16 |
                    @as(u24, self.registers[DATAM]) << 8 |
                    @as(u24, self.registers[DATAL]);

                self.loadPalette(palette_addr);
            },
            @intFromEnum(Command.blit) => {
                // BLIT SPRITE
            },
            else => {
                self.registers[STATUS0] = 0xde;
                self.registers[STATUS1] = 0xad;
                self.registers[STATUS2] = 0xbe;
                self.registers[STATUS3] = 0xef;
            },
        }
    }

    fn loadFont(self: *Self, addr: u24, len: u16) void {
        _ = self;
        _ = addr;
        _ = len;
        std.debug.print("Dynamically loading a font from Taleä's end is not implemented yet\n", .{});
    }

    fn loadPalette(self: *Self, addr: u24) void {
        var new_palette = [_]c_uint{arch.ErrorColor} ** 16;

        var i: u24 = 0;
        while (i < 16) : (i += 1) {
            new_palette[i] = self.bus.readBeu32(addr + (i * 4)) catch unreachable; //FIXME: handle this properly
        }

        self.palette = new_palette;
    }

    fn renderColorText(self: *Self) !void {
        c.c.olivec_fill(self.screen.framebuffer, self.hw_background_color);
        const font = self.fonts[@intFromEnum(self.font)];
        const glyphs_per_row = self.screen.width / font.width;
        const glyphs_per_col = self.screen.height / font.height;

        var row: c_uint = 0;
        while (row < glyphs_per_col) : (row += 1) {
            var col: c_uint = 0;
            while (col < glyphs_per_row) : (col += 1) {
                const addr = self.hw_charbuffer_addr + @as(u24, @truncate(row * (glyphs_per_row * 2))) + @as(u24, @truncate((col * 2)));
                const character = [2]u8{ try self.bus.readu8(addr), 0 };
                const attributes = try self.bus.readu8(addr + 1);
                const blink = if (attributes & 0x80 == 0x80) true else false;
                // Change colors to a palette
                const bg: c_uint = self.palette[(attributes & 0x70) >> 4];
                const fg: c_uint = self.palette[(attributes & 0x0f)];

                c.c.olivec_rect(self.screen.framebuffer, @as(c_int, @truncate(@as(isize, @bitCast((col) * font.width)))), @as(c_int, @truncate(@as(isize, @bitCast(row * font.height)))), @as(c_int, @truncate(@as(isize, @bitCast(font.width)))), @as(c_int, @truncate(@as(isize, @bitCast(font.height)))), bg);
                if (blink and self.blink % 60 <= 15) {
                    c.c.olivec_text(self.screen.framebuffer, &character, @as(c_int, @truncate(@as(isize, @bitCast((col) * font.width)))), @as(c_int, @truncate(@as(isize, @bitCast(row * font.height)))), font, 1, bg);
                } else {
                    c.c.olivec_text(self.screen.framebuffer, &character, @as(c_int, @truncate(@as(isize, @bitCast((col) * font.width)))), @as(c_int, @truncate(@as(isize, @bitCast(row * font.height)))), font, 1, fg);
                }
            }
        }
    }

    fn render(self: *Self) !void {
        switch (self.mode) {
            arch.VideoMode.monochromeText => {
                // TODO: add font sizes
                c.c.olivec_fill(self.screen.framebuffer, self.hw_background_color);
                const font = self.fonts[@intFromEnum(self.font)];
                const glyphs_per_row = self.screen.width / font.width;
                const glyphs_per_col = self.screen.height / font.height;

                var row: c_uint = 0;
                while (row < glyphs_per_col) : (row += 1) {
                    var col: c_uint = 0;
                    var line = [_]u8{0} ** 300;
                    while (col < glyphs_per_row) : (col += 1) {
                        line[col] = try self.bus.readu8(self.hw_charbuffer_addr + @as(u24, @truncate(row * glyphs_per_row)) + @as(u24, @truncate(col)));
                    }
                    line[glyphs_per_row] = 0;
                    c.c.olivec_text(self.screen.framebuffer, &line, 0, @as(c_int, @truncate(@as(isize, @bitCast(row * font.height)))), font, 1, self.hw_foreground_color);
                }
            },
            arch.VideoMode.colorText => {
                // TODO: add font sizes
                try self.renderColorText();
            },
            arch.VideoMode.graphic332 => {
                std.debug.print("graphic332 is not implemented yet\n", .{});
            },
            arch.VideoMode.graphic565 => {
                std.debug.print("graphic565 is not implemented yet\n", .{});
            },
            arch.VideoMode.graphicRGBA => {
                std.debug.print("graphic565 is not implemented yet\n", .{});
            },
            arch.VideoMode.combinedRGBA => {
                self.screen.framebuffer =
                    c.c.olivec_canvas(@as(*c_uint, @ptrCast(@alignCast(&self.bus.mem[@as(usize, self.hw_framebuffer_addr)]))), @as(c_uint, @truncate(arch.VideoConfig.FramebufferDefaultW)), @as(c_uint, @truncate(arch.VideoConfig.FramebufferDefaultH)), @as(c_uint, @truncate(arch.VideoConfig.FramebufferDefaultW)));
                try self.renderColorText();
            },
        }
    }
};
