/*

A simple BIOS, the firmware of your Taleä System (TM).

All fictional rights belong to The Patron of the House of Taleä.

All earthly and real rigths belong to its respective autors, find the licenses
in LICENSE-THIRD-PARTY. This project is licensed under the MIT License.

*/

#include "isr.h"
#include "libsirius/devices.h"
#include "libsirius/types.h"
#include "ushell.h"

/* Defined in entry.s */
extern u8   _lbud(u16 addr);
extern void _sbd(u16 addr, u8 value);

/* -------------------- GLOBALS ----------------------------------------------*/
static interrupt_handler_t interrupt_handlers[16];
struct device_map          Devices;
u32                        Memsize;
enum TpsId                 CurrentTps;

/* ---------------------------- ----------------------------------------------*/

static void find_devices(void)
{
    u8 i, dev_id, audio_channel = 0, audio_channels[4];
    u8 tty, video, storage, mouse;
    for (i = 0; i < 16; i++) {
        dev_id = _lbud(DEVICE_MAP + i);

        switch (dev_id) {
        case DEVICE_ID_TTY:
            tty = i * 0x10;
    
            break;
        case DEVICE_ID_VIDEO:
            video = i * 0x10;
    
            break;
        case DEVICE_ID_STORAGE:
            storage = i * 0x10;
    
            break;
        case DEVICE_ID_AUDIO:
            audio_channels[audio_channel++] = i * 0x10;
    
            break;
        case DEVICE_ID_MOUSE:
            mouse = i * 0x10;
    
            break;
        default:
            break;
        }
    }

    Devices.tty      = tty;
    Devices.timer    = tty + 6;
    Devices.keyboard = tty + 12;
    Devices.video    = video;
    Devices.tps      = storage;
    Devices.tps      = storage + 6;
    Devices.audio0   = audio_channels[0];
    Devices.audio1   = audio_channels[1];
    Devices.audio2   = audio_channels[2];
    Devices.audio3   = audio_channels[3];
    Devices.mouse    = mouse;
}

void kb_handler(u32 keypress)
{
    return;
}

void dummy_isr(u32 a)
{
    return;
}

static void bios_init(void)
{
    usize i;

    Memsize = (u32)_lbud(DEVICE_SYSTEM + PORTS_SYSTEM_MEMSIZE) * 0x100000;

    find_devices();

    for (i = 0; i < 16; i++) {
        interrupt_handlers[i] = dummy_isr;
    }

    CurrentTps = 0;
}

void ushell_putchar(char chr)
{
    _sbd(Devices.tty + SER_TX, chr);
}

bool serial_connected()
{
    u8 status = _lbud(Devices.tty + SER_STATUS);
    return status & SER_STATUS_CARRIER_DETECT;
}

bool serial_available()
{
    u8 status = _lbud(Devices.tty + SER_STATUS);
    return status & SER_STATUS_DATA_AVAILABLE;
}

void serial_print(const char *s) {
    char c;
    while (c = *s++) {
        _sbd(Devices.tty + SER_TX, c);
    }
}

void print_args(int argc, char *argv[])
{
  int i;
  for (i = 0; i < argc; ++i)
    serial_print(argv[i]);
}

static const ushell_command_t commands[] = {
    { "print_args", "prints all args", &print_args }
};

void bios_start(void)
{

    bios_init();


    while (!serial_connected()); /* wait for conection*/
    serial_print("Talea System BIOS v0.5-beta\n");

    ushell_init(commands, sizeof(commands) / sizeof(commands[0]));


wait_for_connection:
    while (!serial_connected()); /* wait for conection*/
    if (serial_available()) {
        static int i = 0;
        u8 chr = _lbud(Devices.tty + SER_RX);
        _trace(i++, chr);
        ushell_process(chr); /* Why the last char read is 0? is it being transmitted? or is it reading wrong*/
    }
    goto wait_for_connection;
}
