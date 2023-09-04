# The Taleä Computer System

## *A Fantasy Computer for a Fantasy World*

This is the specification for one of the two families of computers that exist in the fantasy world of *Nar Naellan* (the other one is the [Machine of the Archive of Arkade](about:blank) *not yet even designed*).

It is a fantasy computer heavily inspired in the first IBM PC and its successors in the early era of pc compatibles. It is used in a fantasy world, and was designed to serve every need that may arise
to any successful merchant business across the sea.

It belongs to the family of Taleä architectures, and stands as the most powerful system
for consumers in its range. It shares it's architecture and instruction set with whe less
powerful Taleä Codex.

Taleä Tabula uses a 32-bit processor, clocked at a base frequency of 10Mhz (boost to 25Mhz), that can adress up to 16Mb of memory. The system features an 640x480 RGBA color screen, a printer style teletype, a keyboard, and a 128 Mb hard disk, and two TPS (*tiny portable storage*, 128Kb) drives.

It's main purpose is, opposed to that of the Machine of the Archive of Arkade, accounting and basic data processing, to be used in commerce and banking by merchants and companies of Talandel. It is cheaper and smaller than the one in the Academy, and a net of these standard machines, once tested, has begun spreading over the city and some of its colonies.

## About this Emulator

The final version of the emulator is implemented in Zig, and uses software rendering (MiniFB) for the screen module, this I intend to change. Currently the project only builds on mac, but it should not be difficult to tweak `build.zig`.

## Software available for the Taleä System

### Tools & Utilities

- Cross assembler: under `utils/asm` there are several files that implement the custom architecture, to use with [customasm](https://github.com/hlorenzi/customasm).
- Jack cross compiler: ~[JackCompiler](https://github.com/uri-nyx/JackCompiler)~ a simple semi-optimizing compiler for a superset of the Jack Programming Language from Nand2Tetris.
- TALLUM: a very simple and in development backend targeting among other Sirius and the Taleä System (in `utils/tallum`), the compiler uses this backend.
- Firmware: a very simple and minimal firmware called minimal (in `utils/firmware`), and a template runtime for freestanding Jack programs (`irs.s`).

### Native software

- Sirius Forth: a simple forth system inspired (translated in essence) by Itsy Forth, and amplied with vocabulary from CAMEL FORTH (the original one) and FORTH83. It's a bit buggy yet, and it runs over serial. Written in assembly as a freestanding program: ~[SiriusForth](https://github.com/uri-nyx/SiriusForth).~

## Changelog

I've been working on this project localy for over a year. First I decided to redesign the computer a bit and programmed the emulator in Rust, but it was too slow. This version is almost final. The architecture is frozen, as are all the peripherals except the video module. It lacks a bit of documentation yet, but everything in `docs` is accurate, except the MMU and video docs. v1.0 should follow in the coming month or two.
