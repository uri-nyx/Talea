/**
 * A very simple firmware for the Taleä Computer System.
 * Just prepare the system enough to transfer control to a loaded kernel:
 *  1. Check if hardware version matches this version
 *  2. Discover hardware.
 *  3. Set up the character buffer for rich text mode and the framebuffer.
 *  4. Look for bootable disks
 *  5. Initiate boot process and relinquish control.
 */

#include "libsirius/devices.h"
#include "libsirius/types.h"

#define FIRMWARE_VERSION_MAJOR 0
#define FIRMWARE_VERSION_MINOR 9

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

static void poweroff(void)
{
    _sbd(REG_SYSTEM_POWER, TALEA_MAGIC_ARM_SEQUENCE);
    _sbd(REG_SYSTEM_POWER, TALEA_MAGIC_TRIGGER_SEQUENCE);
}

static void beep(u8 times)
{
    // TODO: beeps with the opll (OR MAYBE PUT A 7 segment display in the UI)
    _trace(0xdead, times);
    return;
}

static void wait(void)
{
    usize i, wait;
    for (i = 0; i < 100000; i++) {
        wait += i;
    }
}

#define DATA_SECTOR 0x1000 // address in data memory of the buffer to load the sector

static bool tps_sector_io(int command, u8 bank, u8 sector, u8 *buff)
{
    u32 left;
    // copy from TAM to DATA IF COMMAND == STORE
    if (command == STORAGE_COMMAND_STORE) {
        if ((left = _copymd(buff, DATA_SECTOR, 512)) != 0) {
            return false;
        }
    };

    while (_lbud(sys.storage + TPS_STATUS) & STOR_STATUS_BUSY); // wait for ready

    _sbd(sys.storage + TPS_DATA, bank);
    _sbd(sys.storage + TPS_COMMAND, STORAGE_COMMAND_BANK);

    if (_lbud(sys.storage + TPS_STATUS) & STOR_STATUS_ERROR) {
        return false;
    }

    while (_lbud(sys.storage + TPS_STATUS) & STOR_STATUS_BUSY); // wait for ready

    _sbd(sys.storage + TPS_DATA, sector);
    // Assume buff is aligned, and if not, we'r sorry
    _shd(sys.storage + TPS_POINT, DATA_SECTOR >> 9);
    _sbd(sys.storage + TPS_COMMAND, command);

    while (_lbud(sys.storage + TPS_STATUS) & STOR_STATUS_BUSY); // wait for ready

    if (_lbud(sys.storage + TPS_STATUS) & STOR_STATUS_ERROR) {
        return false;
    }

    // copy from DATA to RAM IF COMMAND == LOAD
    if (command == STORAGE_COMMAND_LOAD) {
        if ((left = _copydm(DATA_SECTOR, buff, 512)) != 0) {
            return false;
        }
    }

    return true;
}

static bool hcs_sector_io(int command, u8 bank, u16 sector, u8 *buff)
{
    u32 left;
    // copy from TAM to DATA IF COMMAND == STORE
    if (command == STORAGE_COMMAND_STORE) {
        if ((left = _copymd(buff, DATA_SECTOR, 512)) != 0) {
            return false;
        }
    };

    while (_lbud(sys.storage + HCS_STATUS) & STOR_STATUS_BUSY); // wait for ready

    _sbd(sys.storage + HCS_DATA, bank);
    _shd(sys.storage + HCS_SECTOR, sector);
    // Assume buff is aligned, and if not, we'r sorry
    _shd(sys.storage + HCS_POINT, DATA_SECTOR >> 9);
    _sbd(sys.storage + HCS_COMMAND, command);

    if (_lbud(sys.storage + HCS_STATUS) & STOR_STATUS_ERROR) {
        return false;
    };

    while (_lbud(sys.storage + HCS_STATUS) & STOR_STATUS_BUSY); // wait for ready

    // return
    if (_lbud(sys.storage + HCS_STATUS) & STOR_STATUS_ERROR) {
        return false;
    }

    if (command == STORAGE_COMMAND_LOAD) {
        if ((left = _copydm(DATA_SECTOR, buff, 512)) != 0) {
            return false;
        }
    };

    return true;
}

void firmware_start(void)
{
    // 1. Power off if version mismatch
    if ((_lbud(REG_SYSTEM_VERSION) >> 4) != FIRMWARE_VERSION_MAJOR) poweroff();

    sys.version     = _lbud(REG_SYSTEM_VERSION);
    sys.memory_size = _lbud(REG_SYSTEM_MEMSIZE_FLASH) * (64 * 1024) + (64 * 1024);

    // 2. Discover hardware
    {
        u8 i, found = 0;
        for (i = 0; i < 16; i++) {
            u8 dev_id = _lbud(DEVICE_MAP + i);

            // clang-format off
            switch (dev_id) {
            case DEVICE_ID_TTY: sys.terminal = i * 0x10; found++; break;
            case DEVICE_ID_VIDEO: sys.video = i * 0x10; found++; break;
            case DEVICE_ID_STORAGE: sys.storage = i * 0x10; found++; break;
            case DEVICE_ID_AUDIO: sys.audio = i * 0x10; found++; break;
            case DEVICE_ID_MOUSE: sys.mouse = i * 0x10; found++; break;
            default: break;
            }
            // clang-format on
        }

        if (found != 5) poweroff();
    }

    // 3. set up the video buffers (at the top of memory)
    {
        // Set the video mode to 5
        _sbd(sys.video + VIDEO_COMMAND, COMMAND_BEGIN_DRAWING);
        _sbd(sys.video + VIDEO_GPU0, VIDEO_MODE_TEXT_AND_GRAPHIC);
        _sbd(sys.video + VIDEO_GPU7, 0); // latch
        _sbd(sys.video + VIDEO_COMMAND, COMMAND_SET_MODE);
        _sbd(sys.video + VIDEO_COMMAND, COMMAND_END_DRAWING);

        wait(); // wait for frame update

        _sbd(sys.video + VIDEO_COMMAND, COMMAND_SYS_INFO);
        sys.textbuffer.bpc = _lbud(sys.video + VIDEO_GPU3);
        sys.textbuffer.w   = _lbud(sys.video + VIDEO_GPU4);
        sys.textbuffer.h   = _lbud(sys.video + VIDEO_GPU5);

        sys.framebuffer.w = 640;
        sys.framebuffer.h = 480;

        // set buffers and clear
        if (sys.memory_size < sys.textbuffer.w * sys.textbuffer.h * sys.textbuffer.bpc * 2) {
            sys.framebuffer.addr = 0; // NO FRAMEBUFFER
            sys.textbuffer.addr  = 0; // NO TEXT BUFFER
            beep(1);
        } else if (sys.memory_size < sys.framebuffer.w * sys.framebuffer.h * 2) {
            sys.framebuffer.addr = 0; // NO FRAMEBUFFER
            // ONLY TEXT BUFFER
            sys.textbuffer.addr =
                sys.memory_size - (sys.textbuffer.w * sys.textbuffer.h * sys.textbuffer.bpc) - 512;
            beep(2);
        } else {
            sys.framebuffer.addr = sys.memory_size - (sys.framebuffer.w * sys.framebuffer.h) - 512;
            sys.textbuffer.addr  = (sys.framebuffer.addr - 512) -
                                  (sys.textbuffer.w * sys.textbuffer.h * sys.textbuffer.bpc * 2);
        }

        _sbd(sys.video + VIDEO_COMMAND, COMMAND_BEGIN_DRAWING);

        _swd(sys.video + VIDEO_GPU0, sys.framebuffer.addr);
        _swd(sys.video + VIDEO_GPU4, sys.textbuffer.addr);
        _sbd(sys.video + VIDEO_COMMAND, COMMAND_SET_ADDR);

        // clear black
        _swd(sys.video + VIDEO_GPU0, 0x20000000);
        _sbd(sys.video + VIDEO_GPU4, 0x0);
        _sbd(sys.video + VIDEO_GPU7, 0); // latch
        _sbd(sys.video + VIDEO_COMMAND, COMMAND_CLEAR);

        _sbd(sys.video + VIDEO_COMMAND, COMMAND_END_DRAWING);
    }

    // 4. look for bootable disks
    {
        u8 status;

        _sbd(sys.storage + TPS_DATA, TPS_ID_A);
        _sbd(sys.storage + TPS_COMMAND, STORAGE_COMMAND_SETCURRENT);
        status = _lbud(sys.storage + TPS_STATUS);
        if (status & (STOR_STATUS_BOOT | STOR_STATUS_INSERTED)) {
            sys.boot_device = 0;
            goto transfer;
        }

        _sbd(sys.storage + TPS_DATA, TPS_ID_B);
        _sbd(sys.storage + TPS_COMMAND, STORAGE_COMMAND_SETCURRENT);
        status = _lbud(sys.storage + TPS_STATUS);
        if (status & (STOR_STATUS_BOOT | STOR_STATUS_INSERTED)) {
            sys.boot_device = 1;
            goto transfer;
        }

        status = _lbud(sys.storage + HCS_STATUS);
        if (status & (STOR_STATUS_BOOT)) {
            sys.boot_device = 2;
            goto transfer;
        }

        beep(3);
        poweroff();
    }

    // 5. transfer control
transfer: {
    // Read sector 1. This header must be present at ofsset 0x100 to initiate the transfer
    // HEADER: (in BIG ENDIAN)
    // 0x100    4 bytes     MAGIC (TAL!)
    // 0x104    4 bytes     Entry point offset
    // 0x108    4 bytes     Load address (code+data+bss is assumed)
    // 0x10C    4 bytes     Image size, in sectors, of the kernel
    // 0x110    4 bytes     Offset, in sectors, of the kernel start in disk

    // The firmware will read from sector 1 till sector size, and jump to the
    // offset, pasing a pointer to sys in x12. If the offset is larger than
    // size * 512, boot will fail with poweroff. If the load address + (size * 512) is greater
    // than the memory size, boot will fail with poweroff. If the image size is
    // larger than 1 bank -1 sectors of the boot media, boot will fail with poweroff.
    // If the magic number is not present, boot will fail with poweroff. If loading any sector
    // fails, boot will fail with poweroff. If the offset is > than 1 bank -ofsset of the boot media
    // boot wil fail with poweroff. And pray that nothing faults.

    static u8 sector[512];
    u32      *header;
    u32       magic, entry, load_addr, size, offset;

    if (sys.boot_device < 2) {
        // BOOT from TPS. current drive is set already
        if (!tps_sector_io(STORAGE_COMMAND_LOAD, 0, 0, sector)) {
            beep(4);
            poweroff();
        }
    } else {
        // BOOT from HCS
        if (!hcs_sector_io(STORAGE_COMMAND_LOAD, 0, 0, sector)) {
            beep(4);
            poweroff();
        }
    }

    header    = (u32 *)(&sector[0x100]);
    magic     = header[0];
    entry     = header[1];
    load_addr = header[2];
    size      = header[3];
    offset    = header[4] ? header[4] - 1 : header[4];

    if (size > 32 * 1024) {
        // TODO: RELOCATE THE STACK TO THE TOP OF MEMORY SO IT DOES NOT GET BURNT
    }

    sys.kernel_load_addr = load_addr;
    sys.kernel_entry     = entry;
    sys.kernel_sectors   = size;

    if (magic != 0x54414C21) {
        beep(5);
        poweroff();
    }

    if (size > (sys.boot_device < 2 ? 255 : 65535)) {
        beep(6);
        poweroff();
    }

    if (entry > (size * 512)) {
        beep(7);
        poweroff();
    }

    if (load_addr + (size * 512) > sys.memory_size) {
        beep(8);
        poweroff();
    }

    if (offset > (sys.boot_device < 2 ? 255 : 65535) - 1) {
        beep(9);
        poweroff();
    }

    // copy the sys struct to DATA 0x2000, in case the kernel wipes it during load
    _copymd(&sys, 0x2000, sizeof(sys));

    // Load the kernel
    if (sys.boot_device < 2) {
        usize i;
        for (i = 0; i < size; i++) {
            if (!tps_sector_io(STORAGE_COMMAND_LOAD, 0, offset + i + 1,
                               (u8 *)load_addr + (i * 512))) {
                beep(9);
                poweroff();
            }
        }
    } else {
        usize i;
        for (i = 0; i < size; i++) {
            if (!hcs_sector_io(STORAGE_COMMAND_LOAD, 0, offset + i + 1,
                               (u8 *)load_addr + (i * 512))) {
                beep(9);
                poweroff();
            }
        }
    }

    // And jump to the kernel
    ((void (*)(struct SystemInfo *sys))(load_addr + entry))(&sys);
}
}