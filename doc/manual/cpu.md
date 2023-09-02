# The Central Processing Unit

This chapter discusses the nucleus of the Taleä Computer System. The Central Processing Unit is the component wich captains the machine, the piece that performs almost all calculations and manages the data flows of the system. I'ts understanding in depth is utterly *necessary* to achieve mastery over the computer.

- [The Central Processing Unit](#the-central-processing-unit)
  - [Registers](#registers)
  - [Memory](#memory)
  - [Instruction Set](#instruction-set)
    - [U instructions](#u-instructions)
    - [J instructions](#j-instructions)
    - [B instructions](#b-instructions)
    - [I instructions](#i-instructions)
      - [Jump And Link Register](#jump-and-link-register)
      - [Load operations](#load-operations)
      - [Artimetic operations with immediate values](#artimetic-operations-with-immediate-values)
    - [S instructions](#s-instructions)
    - [R instructions](#r-instructions)
    - [M instructions](#m-instructions)
    - [T instructions](#t-instructions)
  - [Interrupts and Exceptions](#interrupts-and-exceptions)
  - [Memory Mapping Unit](#memory-mapping-unit)

## Registers

The first step to understand Sirius (the Central Processing Unit) is to understand its basic component: the registers.

The registers are cells of quick access memory, and hold the data on wich the machine is operating in the moment. There are varius type of registers inside Sirius:

*General purpose registers*: there are 32 of these registers, labeled `x0` to `x31`, or by the names in the [Appendix A](appendix.md#A#registers). They hold 32-bit integer values, and are the source or the destination of the majority of the operations that the machine can perform. However, it's worth to notice that `x0` is special: it behaves as a *constant zero*, wirtes do not affect this register, which always reads as `0`.

*Program Counter*: it is a single 24-bit register, that keeps track of the memory location that Sirius will execute in the next cycle.

*Stack Pointer*: though the stack pointer corresponds to `x2`, it should be noticed that this register is different at different privilege modes.

*Processor Status Register*: this is a 32-bit register that keeps track of varius states of the processor and some system settings:

    ╭────┬───────┬───────┬─────┬───────┬───────┬────────╮
    │s: 1│ irq: 1│ mmu: 1│ p: 3│ ivt: 6│ pdt: 8│ res: 12│
    ╰────┴───────┴───────┴─────┴───────┴───────┴────────╯
    s: supervisor, irq: interrupt enable, mmu: mmu enable,
    p: priority, ivt: interrupt vector table (value*1024),
    pdt: page directory table (value*256), res: reserved

## Memory

To perform task and complex calculations, a machine needs to manage data. As the registers are few, the system provides *memory*, a file of 8-bit wide cells (bytes), that can be read and written at will, though slower than registers. Memory also serves another main purpose in this machine: it provides itself the instructions to execute.

The Taleä Computer System provides two different types of memory: *main* memory, that can be read, written, and executed; and *data* memory, that can *only* be read or written and is *only* accesible in *supervisor* or *privileged* mode. It is possible to address up to 24 bits of addresses in *main* memory (roughly sixteen million seven hundred and fifty thousand bytes, 16 Mib), and 16 bits of addresses in *data* (sixty five thousand five hundred thirty six bytes, 64 Kib). *Data* memory serves thus as a way of tidily managing the system: many important structures, such as the interrupt vector table, input buffers, paging structures and I/O ports sit in this memory. This is the **default** memory map for the system:

    ╭───────────────────╮
    │╭────────╮╭───────╮│
    ││Teletype││ 0x0000││ 6  bytes DEVICE 0x0
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││Timer   ││ 0x0006││ 6  bytes DEVICE 0x0
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││Keyboard││ 0x000c││ 4  bytes DEVICE 0x0
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││Video   ││ 0x0010││ 16 bytes DEVICE 0x1
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││TPS     ││ 0x0020││ 6  bytes DEVICE 0x2
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││Disk    ││ 0x0026││ 8  bytes DEVICE 0x2 
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││Custom  ││ 0x0030││
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││........││.......││
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││System  ││ 0x00f0││ DEVICE 0xF
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││Dev Map ││ 0x0100││ DEVICE 0x10 (16)
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││........││.......││
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││IVT     ││ 0xf800││
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││PDT     ││ 0xff00││
    │╰────────╯╰───────╯│
    ╰───────────────────╯

The Taleä System default configuration reserves data memory from `0x0000` to `0x00ff` for i/o devices, and provides three different coupled devices. Hardware devices contain up to 16 byte-size accessible registers for interface with the machine.

It's worth noticing that devides 15 and 16 correspond to pseudo devices that provide information about the system and it's configuration:

The system device is comprised by the following registers:

- [0x0] MEMSIZE: indicates the total available memory of the system (normal configurations include 1, 2, 4, 8 or 16 MB), encoded as a byte `(n * (64 * 1024) + (64 * 1024))`.
- [0x1] CLOCK: clock speed, in Mhz (turbo speed is around 20-25 Mhz).
- [0x2] INT: last interrupt raised (8 bit vector). Writing or reading to this register sets it to 1 (no exception).
- [0x3] POWER: if written to, the machine will halt and power off.
- [0x4..0x9] YEAR, MONTH, DAY, HOUR, MINUTE, SECOND.
- [0xA, 0xB] RESERVED.
- [0xC] DEVICES: number of connected devices.
- [0xD] ARCHITECTURE: architecture ID.
- [0xE] VENDOR: vendor ID.
- [0xF] RESERVED.

The device map (device 0x10) stores the device id for each device or 0 if it's not installed.

## Instruction Set

The instruction set for sirius is somewhat simple and reduced: it supports 70 instructions divided in 8 groups. All instructions are 32 bits long and encoded in big endian format. The binary representation of the instructions can be one of five forms:

    ╭────────┬──────────┬───────┬───────────────────────────────────┬──────────────╮
    │group: 3│ opcode: 4│ reg: 5│ imm: 20                           │ ENCODING   I │
    ├────────┼──────────┼───────┼───────┬───────────────────────────┼──────────────╮
    │group: 3│ opcode: 4│ reg: 5│ reg: 5│ imm: 15                   │ ENCODING  II │
    ├────────┼──────────┼───────┼───────┼───────┬───────────────────┼──────────────╮
    │group: 3│ opcode: 4│ reg: 5│ reg: 5│ reg: 5│zero: 10           │ ENCODING III │
    ├────────┼──────────┼───────┼───────┼───────┼────────┬──────────┼──────────────╮
    │group: 3│ opcode: 4│ reg: 5│ reg: 5│ reg: 5│ reg: 5 │ zero: 5  │ ENCODING  IV │
    ├────────┼──────────┼───────┼───────┴───────┴─────┬──┴──────────┼──────────────╮
    │group: 3│ opcode: 4│ reg: 5│ zero: 12            │ vector: 8   │ ENCODING   V │
    ╰────────┴──────────┴───────┴─────────────────────┴─────────────┴──────────────╯

### U instructions

Its `group code` is `0b010`, and take `ENCODING I`.

- *Load Upper Immediate*, `lui rd, imm`, `0x1`: `rd <- (imm << 12)`, it loads the immediate value on the upper 20 bits of the register.
- *Add Upper Immediate to PC*, `auipc rd, imm`, `0x2`: `rd <- (imm << 12) + pc`, loads the upper 20 bits of the register with the immediate value and adds the current program counter.

### J instructions

There is only a `J` instruction. Its group code is also `0b010`, and takes `ENCODING I`.

- *Jump And Link*, `jal rd, ±imm`, `0x0`: `rd <- pc; pc <- pc ± imm`, performs an inconditional jump to the relative offset specified. Takes a signed two's complement argument that *must* be 4 byte aligned, so the range is actually 22 bits.

### B instructions

B instructions are conditional branches. Its group code is `0b011`, and take `ENCODING II`. Its relative jump argument is also a signed two's complement integer, *must* be 4 byte aligned, and makes thus the range effectively 17 bits.

- *Branch if EQual*, `beq rs1, rs2, ±imm`, `0x0`: `pc <- (rs1 == rs2) ? pc ± imm : pc`.
- *Branch if Not Equal*, `bne rs1, rs2, ±imm`, `0x1`: `pc <- (rs1 != rs2) ? pc ± imm : pc`.
- *Branch if Less Than*, `beq rs1, rs2, ±imm`, `0x2`: `pc <- (±rs1 < ±rs2) ? pc ± imm : pc`, it is worth noticing that treates both registers as signed two's complement values.
- *Branch if Greater or Equal*, `bge rs1, rs2, ±imm`, `0x3`: `pc <- (±rs1 >= ±rs2) ? pc ± imm : pc`, it is worth noticing that treates both registers as signed two's complement values.
- *Branch if Less Than Unsigned*, `bltu rs1, rs2, ±imm`, `0x4`: `pc <- (rs1 < rs2) ? pc ± imm : pc`.
- *Branch if Greater or Equal Unsigned*, `bgeu rs1, rs2, ±imm`, `0x5`: `pc <- (rs1 <= rs2) ? pc ± imm : pc`.

### I instructions

I instructions are loads and arithmetic operations, they involve several groups and take `ECODING II`. Its immediate argument need not be aligned, so the range remains 15 bit.

#### Jump And Link Register

For this instruction, the relative offset *must* be 4 byte aligned. Group `0b100`.

- *Jump And Link Register*, `jalr rd, ±imm(rs2)`, `0x1`: `rd <- pc; pc <- rs1 ± imm`.

#### Load operations

Load operations use the group code `0b100`, and load the value pointed to `rs1 ± imm` into `rd`. Loads from *data* memory are restricted to *supervisor* mode.

- *Load Byte*, `lb rd, ±imm(rs1)`, `0x2`: `rd <- sext(m[rs1 ± imm])`, loads a byte and performs sign extension on it, thus treating it as a signed integer.
- *Load Byte Unsigned*, `lbu rd, ±imm(rs1)`, `0x3`: `rd <- m[rs1 ± imm]`, loads a byte without sign extending, thus unsigned.
- *Load Byte from Data*, `lbd rd, ±imm(rs1)`, `0x4`, **supervisor**: `rd <- sext(d[rs1 ± imm])`.
- *Load Byte Unsigned from Data*, `lbud rd, ±imm(rs1)`, `0x5`, **supervisor**: `rd <- d[rs1 ± imm]`.
- *Load Halfword*, `lh rd, ±imm(rs1)`, `0x6`: `rd <- sext(m[rs1 ± imm])`, loads a halfword (16 bit value).
- *Load Halfword Unsigned*, `lhu rd, ±imm(rs1)`, `0x7`: `rd <- m[rs1 ± imm]`.
- *Load Halfword from Data*, `lhd rd, ±imm(rs1)`, `0x8`, **supervisor**: `rd <- sext(d[rs1 ± imm])`.
- *Load Halfword Unsigned from Data*, `lhud rd, ±imm(rs1)`, `0x9`, **supervisor**: `rd <- d[rs1 ± imm]`.
- *Load Word*, `lw rd, ±imm(rs1)`, `0x9`: `rd <- m[rs1 ± imm]`, loads a word (32 bit value).
- *Load Word from Data*, `lwd rd, ±imm(rs1)`, `0xa`, **supervisor**: `rd <- d[rs1 ± imm]`.

#### Artimetic operations with immediate values

These operations use the group code `0b101`, and load the result of the operation between `rs1` and `imm` into `rd`. `imm` is taken here as a two's complement 15 bit integer, and thus, sign extended.

- *MULtiply by Immediate*, `muli rd, rs1, imm`. `0x0`: `rd <- (rs1 * ±imm)[0:31]`, if the result exceeds 32 bits, only the lower half is loaded into `rd`.
- *MULtiply by Immediate and take High word*, `mulih rd, rs1, imm`. `0x1`: `rd <- (rs1 * ±imm)[32:63]`, the upper half of the result is loaded into `rd`.
- *Integer DIVIsion*, `idivi rd, rs1, imm`. `0x2`: `rd <- rs1/±imm`, notice it will throw an exception if triying to divide by zero.
- *ADD Immediate*, `addi rd, rs1, imm`. `0x3`: `rd <- rs1 + ±imm`
- *SUBtract Immediate*, `Subi rd, rs1, imm`. `0x4`: `rd <- rs1 - ±imm`.
- *bitwise OR with Immediate*, `ori rd, rs1, imm`. `0x5`: `rd <- rs1 | ±imm`.
- *bitwise AND with Immediate*, `andi rd, rs1, imm`. `0x6`: `rd <- rs1 & ±imm`.
- *bitwise eXclusive OR with Immediate*, `xori rd, rs1, imm`. `0x7`: `rd <- rs1 ^ ±imm`.
- *SHift Immediate Rigth Arithmetic*, `shira rd, rs1, imm`. `0x8`: `rd <- rs1 >> ±imm`.
- *SHift Immediate Rigth Logical*, `shirl rd, rs1, imm`. `0x9`: `rd <- rs1 >>> ±imm`.
- *SHift Immediate Left Logical*, `shill rd, rs1, imm`. `0xa`: `rd <- rs1 << ±imm`.
- *Set Less Than Immediate*, `slti rd, rs1, imm`. `0xb`: `rd <- rs1 < ±imm ? 1 : 0`.
- *Set Less Than Unsigned Immediate*, `sltiu rd, rs1, imm`. `0xc`: `rd <- rs1 < +imm ? 1 : 0`, treats both the register and the immediate value as unsigned integers.

### S instructions

S instructions are store operations, their group code is `0b111` and take `ECODING II`. Its immediate argument need not be aligned, so the range remains 15 bit. They store values into memory taking a base location in `rs2` and an offset `±imm`.

- *Store Byte*, `sb rs1, ±imm(rs2)`, `0x0`: `m[rs2 ± imm] <- rs1[0:8]`.
- *Store Byte in Data*, `sbd rs1, ±imm(rs2)`, `0x1`, **supervisor**: `d[rs2 ± imm] <- rs1[0:8]`.
- *Store Halfword*, `sh rs1, ±imm(rs2)`, `0x2`, **supervisor**: `m[rs2 ± imm] <- rs1[0:16]`.
- *Store Halfword in Data*, `shd rs1, ±imm(rs2)`, `0x3`, **supervisor**: `d[rs2 ± imm] <- rs1[0:15]`.
- *Store Word*, `sw rs1, ±imm(rs2)`, `0x4`, **supervisor**: `m[rs2 ± imm] <- rs1`.
- *Store Word in Data*, `swd rs1, ±imm(rs2)`, `0x5`, **supervisor**: `d[rs2 ± imm] <- rs1`.

### R instructions

R instructions are analogous to the I ones: they perform arithmetic operations, but over registers. They take thus `ENCODING III` and `ENCODING IV`. Their group code is `0b110`.

- *ADD*, `add rd, rs1, rs2`, `0x0`: `rd <- rs1 + rs2`.
- *SUBtract*, `sub rd, rs1, rs2`, `0x1`: `rd <- rs1 - rs2`.
- (IV) *Integer DIVision*, `idiv rd, rd2, rs1, rs2`, `0x2`: `rd <- rs1 / rs2; rd2 <- rs1 % rs2`. the quotient is loaded in `rd`, and the remainder in `rd2`. Notice that **both** operands are treated as **signed two's complement** integers.
- (IV) *MULtiply*, `mul rd, rd2, rs1, rs2`, `0x3`: `rd, rd2 <- rs1 * rs2`. the high byte is loaded in `rd`, and the low one in `rd2`.
- *bitwise OR*, `or rd, rs1, rs2`, `0x4`: `rd <- rs1 | rs2`.
- *bitwise AND*, `and rd, rs1, rs2`, `0x5`: `rd <- rs1 & rs2`.
- *bitwise eXclusive OR*, `xor rd, rs1, rs2`, `0x6`: `rd <- rs1 ^ rs2`.
- *bitwise NOT*, `not rd, rs1`, `0x7`: `rd <- ~rs1`.
- *Count Trailing Zeroes*, `ctz rd, rs1`, `0x8`: `rd <- ctz(rs1)`, return the number of zero bits after the last bit set to one.
- *Count Leading Zeroes*, `clz rd, rs1`, `0x9`: `rd <- clz(rs1)`, returns the number of zero bits before the first bit set to one.
- *POPulation COUNT*, `popcount rd, rs1`, `0xa`: `rd <- rs1 + rs2`, returns the number of bits set to one.
- *SHift Rigth Arithmetic*, `shra rd, rs1, rs2`, `0xb`: `rd <- rs1 >> rs2`.
- *SHift Rigth Logical*, `shrl rd, rs1, rs2`, `0xc`: `rd <- rs1 >>> rs2`.
- *SHift Left Logical*, `shll rd, rs1, rs2`, `0xd`: `rd <- rs1 << rs2`.
- *ROtate Rigth*, `ror rd, rs1, rs2`, `0xe`: `rd <- rs1 ror rs2`.
- *ROtate Left*, `rol rd, rs1, rs2`, `0xf`: `rd <- rs1 rol rs2`.

### M instructions

M instructions deal with data in bulk and data structures like stacks or buffers. Their group code is `0b001`, and use `ENCODING II` and `ENCODING III`.

- *COPY buffer*, `copy src, dest, len`, `0x0`: Copies the buffer of length `len` staarting at location `src`, to location `dest`.
- *SWAP buffer*, `swap buff1, buff2, len`, `0x1`: Swaps two buffers of equal lenght.
- *FILL buffer*, `fill buff, n, fill`, `0x2`: Fills the buffer at `buff` with `n` bytes `fill` (the least significant byte will be taken).
- *store THROug pointer*, `thro rd, rs1`, `0x3`: Performs two levels of indirection, it will use the contents the location pointed by `rs1` as a pointer to the data to be stored.
- *load FROM pointer*, `from rd, rs1`, `0x4`: Performs two levels of indirection, it will use the contents the location pointed by `rs1` as a pointer to the data to be loaded.
- *POP Byte*, `popb rd, sp`, `0x5`: Pops a byte from the stack at `sp`.
- *POP Halfword*, `poph rd, sp`, `0x6`: Pops a halfword from the stack at `sp`.
- *POP word*, `pop rd, sp`, `0x7`: Pops a word from the stack at `sp`.
- *PUSH Byte*, `pushb rd, sp`, `0x8`: Pushes a byte to the stack at `sp`.
- *PUSH Halfword*, `pushh rd, sp`, `0x9`: Pushes a halfword to the stack at `sp`.
- *PUSH word*, `push rd, sp`, `0xa`: Pushes a word to the stack at `sp`.
- *SAVE registers*, `save start, end, dest`, `0xb`: Saves the values of the range of registers between `start` to `end` (including this last one) to location `dest`. The actual register number is used, **not its value**.
- *RESTORE registers*, `restore start, end, src`, `0xc`: Restores the values of the range of registers between `start` to `end` (including this last one) from location `src`. The actual register number is used, **not its value**.
- *EXCHANGE registers*, `exch rs1, rs2`, `0xd`: Exchanges the values of the registers.
- *Set Less Than*, `slt rd, rs1, rs2`, `0xe`: `rd <- rs1 < rs2 ? 1 : 0`.
- *Set Less Than Unsigned*, `sltu rd, rs1, rs2`, `0xe`: `rd <- rs1 < rs2 ? 1 : 0` treating both registers as unsigned integers.

### T instructions

T instructions are system management instructions. They use the group code `0b000` and take `ENCODING V`.

- *perform SYStem CALL* `syscall rd, vector`, `0x2`: Performs the system call specified either in `rd` or by `vector` if the former is the `zero` register. It effectively raises a *software interrupt*, and raises the systems privilege to **supervisor*.
- *Get Status REGister* `gsreg rd`, `0x3`: `rd <- psr`.
- *Set Status REGister* `ssreg rs1`, `0x4`, **supervisor**: `psr <- rs1`.
- *SYStem call RETurn* `sysret`, `0x6`, **supervisor**: Returns from a system call or from an interrupt/exception handler and lowers privileges to **unprivileged** mode.
- *TRACE registers* `trace rs1, rs2, rs3, rs4`, `0x5`: logs the values of up to four registers.

## Interrupts and Exceptions

*Interrupts* are a mechanism that allow certain events in the system to *interrupt* Sirius' normal processing in response to them. Exceptions arise from the program itself, be it as errors in it, wrong privilege level or other exceptional cause. Interrupts come from the outside and represent requests to the processor to respond to events.

The general workings are alike for both of them: a request is sent to Sirius with a *priority level*. If this *priority level* is strictly higher than the current's cp *priority* (stored in `psr`), Sirius will acknowledge the interrupt. In doing so, it  shall raise the privilege level to *supervisor*, save the state of the machine by pushing `psr` and `pc` onto the stack, equate the *priority level* of the request to its own, and proceed to process the handler requested (a byte that indexes into a vector table of 32-bit values, `index = request * 4`. The process for exceptios is the same, though they are **always** acknowledged. After an interrupt, Sirius shall restore the state of the machine as it was. After an exception, it will depend of the type: if it is a *trap*, it will continue after the instruction that caused it. If it is a *fault*, it will retry to execte the instruction that triggered it. If while processing a *fault* another fault occurs, a *double fault* shall arise, and most likely the system will shut down.

The *interrupt vector table* (IVT), sits in data memory, at location `0xf800` by default, and can be rearranged to 64 different points by writing the 6 bit `psr` field `ivt`. The hardware exception indexes cannot be rearranged, and are the following:

    ╭───────────────────┬─────╮
    │Reset              │ 0x00│
    ├───────────────────┼─────┤
    │Bus Error          │ 0x02│
    ├───────────────────┼─────┤
    │Address Error      │ 0x03│
    ├───────────────────┼─────┤
    │Illegal Instruction│ 0x04│
    ├───────────────────┼─────┤
    │Division by Zero   │ 0x05│
    ├───────────────────┼─────┤
    │Privilege Violation│ 0x06│
    ├───────────────────┼─────┤
    │Page Fault         │ 0x07│
    ├───────────────────┼─────┤
    │Access Violation   │ 0x08│
    ╰───────────────────┴─────╯

The different peripherals of the system define also their interrupts, their default indexes being:

    ╭──────────────────┬─────╮
    │TTY Transmit      │ 0x0a│
    ├──────────────────┼─────┤
    │KBD Character     │ 0x0b│
    ├──────────────────┼─────┤
    │KBD Scancode      │ 0x0c│
    ├──────────────────┼─────┤
    │TPS Load Finished │ 0x0d│
    ├──────────────────┼─────┤
    │DISK Load Finished│ 0x0e│
    ├──────────────────┼─────┤
    │TIMER Timeout     │ 0x0f│ 
    ├──────────────────┼─────┤     
    │TIMER Interval    │ 0x10│
    ├──────────────────┼─────┤ 
    │VIDEO Refresh     │ 0x11│
    ╰──────────────────┴─────╯

## Memory Mapping Unit

The *memory mamping unit* (MMU) is a special part of Sirius. It can be enabled by writing to `psr` `mmu_enable` field, and it's purpose is that of providing *virtual memory*: it provides a mapping between *virtual*, that is, arbitrary addresses, to *physical* ones, actual locations in memory. It only affects *main* memory, and its reason of existence is to provide a cleaner and more organized address space to programs.

When enabled, every read, write, or instruction fetch that Sirius executes, will undergo a translating process:

1. The *virtual* address will be decomposed in various parts: a prefix to a page table directory (PDT), that lies in Data memory, and whose address is configurable by writing to `psr` `pdt` field (`pdt_address = pdt * 256`), a *page table* index, and and offset within tat page.
2. The entry in the PDT shall contain a *page table* address, a *real* address pointing to another table in main memory.
3. This *page table* will contain entries that indicate the physical locations of pages and their status at the index indicated by the *virtual* address (`page_table[page_table_index] = page_real_address`).
4. The *physical* address shall then be the page's real address plus the offset supplied.

The actual sizes of PDT, page tables and pages can be configured **in hardware**, but Sirius accounts for some status bits: `w`, wether the page is writable or not, `x`, wether it is executable, `d`, wether it has been written, and `p`, wether it is present and mapped to a frame in main memory. Trying to perform operations on pages that are not configured accordinly may raise exceptions, though none except `p` apply in supervisor mode.

Entries in the page table must be 2 bytes long, (a halfword) and contain this fields:

    ╭────────────────┬────────────────────────┬───┬───┬───────┬─────────╮
    │Page Table Entry│Physical page address:12│w:1│x:1│dirty:1│present:1│
    ╰────────────────┴────────────────────────┴───┴───┴───────┴─────────╯
    ╭──────────────────────────┬─────────────────────────────────┬──────────╮
    │Page Directory Table Entry│Physical address of Page Table:12│reserved:4│
    ╰──────────────────────────┴─────────────────────────────────┴──────────╯

The values for entries, and sizes, in bytes, are the following:

- Entry size: 2
- Page size: 4096
- Page table entries: 1024
- Page table size: Page table entries * entry_size
- Page directory entries: (memory size/page_size) / page_table_entries
