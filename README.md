# The Taleä Computer System

## *A Fantasy Computer for a Fantasy World*


It is a fantasy computer heavily inspired in the first IBM PC and the workstations of the early 90s era of pc compatibles. It is used in a fantasy world, and was designed to serve every need that may arise
to any successful merchant business across the sea.

The Taleä System uses a 32-bit processor, clocked at a base frequency of 10Mhz, that can adress up to 16Mb of memory. The system features an 640x480 indexed color screen, a serial port and modem, a keyboard, a mouse, an audio syntesizer based on the OPLL, and 128 Mb persistent storage, and two TPS (*tiny portable storage*, 128Kb-1MB) drives for removable media.


## Hardware Specifications

| Component | Specification |
| -----------| ------------ |
| CPU | Sirius 24-bit Processor (16MB Address Space) |
| RAM | Up to 16MB Main Memory |
| ROM | 8KB System BIOS (Mapped at 0xFFE000) |
| Display | 640x480 Indexed Color (RGBA Buffer) |
| Storage | 128MB Fixed Storage (HCS) + Removable Media(TPS) |
| Terminal | Dual Timer, Keyboard, Serial Modem |

## Software Toolchain

See some basic utilities and tools to develop software for the system

- SAS (Sirius Assembler): located in /tools/sas. It assembles raw binaries.
- BIOS Firmware: Found in /software/bios, this provides the initial hardware initialization and storage bootstrapper.
- Disk Utilities: /tools/tpack is used to create and manage tps images.

4. Build Instructions

The emulator requires Raylib for the frontend and a standard C toolchain (GCC/Clang).
Bash

## Building the emulator and utilities

To build the emulator on Windows, I recommend using the provided Visual Sturio solution. You need at least version 17.8 with support for C11 threads.

To build for POSIX, simply:

```bash

$ make

``` 

The emulator depends on raylib (and everything raylib depends on -- OpenGL, Math, etc...), the threading library, and the network library for your system (Ws2_32.lib in windows).

### Build the utilities

For Windows, use `tcc` and the provided batch files. On POSIX, just:

```bash
cd software/bios && make

```

## Emulator configuration

You can find the configuration file for the emulator in `resources/config.toml`. Some configurations are accessible at run-time inside the emulator. These are the current options:

```toml

Screen = "windowed" # "windowed", "fullscreen", or "borderless"
Dynamic_Cycles = true
CRT_shader = true
Keep_TPS_loaded = true
Sync_TPS_when_ejected = true
Key_Repeat = true
Enable_Serial = false
V-Sync = true
HiDPI = true
Port = 1212
Speed = 0.200000
Firmware = "resources/emulated/firmware/hello.bin"
TPS_A = "resources/tps_images/akai.tps"
Hardware_font = "spleen8x16.fnt"

```

You can find some prepackaged fonts for the terminal in `resources/fonts`. Add new fonts in `fnt` format, and generate an atlas with [BMFont](https://www.angelcode.com/products/bmfont/) (name the atlas like `fontname_0.png`). Then write its file name in the config file at `Hardware_font`. 

You can also modify the look of the menu by editing terminal.rgs with raylib's utilities.

## Connecting to the serial port

Connecting to the serial port is a bit tricky if you want it to work as expected, because we don't use a real or emulated serial port, but a tcp loopback socket listening on the emulator's end. You can connect very easily with `nc`, `netcat`, `socat` or even `telnet`, but expect some issues, mainly with echoing, if the emulator is transmitting data in raw mode and the software (running in the emulator) does not implement some sort of protocol or buffering.

If you want or need to use the serial port as if it were so, follow this steps:

1. Create a emulated serial to serial port bridge. In windows you can use com0com, I haven't tested this on linux.
2. Create a bridge between one of the emulated ports and Talea's listening socket. In windows you can use com2tcp, or porticulus (this is what I used).
3. Use a serial terminal, and connect to the emulated serial port.

```bash
com0com install - -
porticulus --com "\\.\COMPORT" 300 n 1  --1on1 --tcp --host 127.0.0.1 4321
```

The serial port is opened by default at `localhost:4321`. Connect to it from a network utility like `nc`, `telnet`, or a program like `PuTTY` or `TeraTerm`, following the above instructions. You can even connect from another machine (if you know how to reach your localhost)!

## Contributing

Feel free to contribute and open issues if you need! Pull requests are also welcome!

I have been developing this project for a very long time, but it has been a solo effort so far... any help is welcome, let's build a little community!

If you want to reach me, you can do so at rserranof03@gmail.com
