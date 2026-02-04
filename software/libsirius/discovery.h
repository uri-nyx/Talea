#ifndef DISCOVERY_H
#define DISCOVERY_H

struct SystemInfo {
    u8    version;     // [7:4] Major [3:0] Minor
    usize memory_size; // memory size in bytes

    // devices
    u8 installed_devices; // number of installed devices
    u8 terminal;          // terminal device base address
    u8 video;             // video device base address
    u8 storage;           // storage device base address
    u8 audio;             // audio device base address
    u8 mouse;             // mouse device base address

    // Video buffers
    struct FrameBuffer {
        u32 addr; // Base address of framebuffer
        u16 w, h; // size of framebuffer (in pixels)
    } framebuffer;

    struct TextBuffer {
        u32 addr; // Base address of text buffer
        u8  w, h; // size of text buffer (in characters)
        u8  bpc;  // bytes per character
    } textbuffer;

    u8 video_mode;

    // Boot
    u8 boot_device; // 0: TPS A, 1: TPS B, 2: HCS. Also, this is the boot order

    u32 kernel_load_addr;
    u32 kernel_sectors;
    u32 kernel_entry;
};

#endif /* DISCOVERY_H */
