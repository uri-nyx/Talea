/**
 * A very simple DOS-style kernel for the Taleä Computer System
 */

#include "libsirius/devices.h"
#include "libsirius/types.h"

extern void  _sbd(u16 addr, u8 value);
extern void  _shd(u16 addr, u16 value);
extern void  _swd(u16 addr, u32 value);
extern u8    _lbud(u16 addr);
extern u16   _lhud(u16 addr);
extern u32   _lwd(u16 addr);
extern usize _copydm(u16 data_addr_src, void *buff_dest, usize sz);
extern usize _copymd(void *buff_src, u16 data_addr_dest, usize sz);
extern void *memcpy(void *dest, const void *src, usize n);

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

static struct SystemInfo sys;

void puts(struct SystemInfo *s, const char *str)
{
    static u32 pos = 0;
    char      *p   = (char*)str;
    char       c;
    while (c = *p++) {
        *((u32 *)(s->textbuffer.addr) + pos++) = ((u32)c << 24) | 0x000f0000UL;
    }
}

void kmain(u16 sys_info_data_addr)
{
    // Copy the struct handed down by the firmware
    _copydm(sys_info_data_addr, &sys, sizeof(sys));

    // Print somewing quick
    puts(&sys, "Hello from the Kernel!");
    l: goto l;
}