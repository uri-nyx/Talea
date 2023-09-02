# The Taleä Computer System Cheatsheet

## Instructions

- `lui rd, imm`:
- `auipc rd, imm`
- `jal rd, ±imm`
- `beq rs1, rs2, ±imm`
- `bne rs1, rs2, ±imm`
- `beq rs1, rs2, ±imm`
- `bge rs1, rs2, ±imm`
- `bltu rs1, rs2, ±imm`
- `bgeu rs1, rs2, ±imm`
- `jalr rd, ±imm(rs2)`
- `lb rd, ±imm(rs1)`
- `lbu rd, ±imm(rs1)`
- (**supervisor**) `lbd rd, ±imm(rs1)`
- (**supervisor**) `lbud rd, ±imm(rs1)`
- `lh rd, ±imm(rs1)`
- `lhu rd, ±imm(rs1)`
- (**supervisor**) `lhd rd, ±imm(rs1)`
- (**supervisor**) `lhud rd, ±imm(rs1)`
- `lw rd, ±imm(rs1)`
- (**supervisor**) `lwd rd, ±imm(rs1)`
- `muli rd, rs1, imm`
- `mulih rd, rs1, imm`
- `idivi rd, rs1, imm`
- `addi rd, rs1, imm`
- `Subi rd, rs1, imm`
- `ori rd, rs1, imm`
- `andi rd, rs1, imm`
- `xori rd, rs1, imm`
- `shira rd, rs1, imm`
- `shirl rd, rs1, imm`
- `shill rd, rs1, imm`
- `sb rs1, ±imm(rs2)`
- (**supervisor**) `sbd rs1, ±imm(rs2)`
- (**supervisor**) `sh rs1, ±imm(rs2)`
- (**supervisor**) `shd rs1, ±imm(rs2)`
- (**supervisor**) `sw rs1, ±imm(rs2)`
- (**supervisor**) `swd rs1, ±imm(rs2)`
- `add rd, rs1, rs2`
- `sub rd, rs1, rs2`
- `idiv rd, rd2, rs1, rs2`
- `mul rd, rd2, rs1, rs2`
- `or rd, rs1, rs2`
- `and rd, rs1, rs2`
- `xor rd, rs1, rs2`
- `not rd, rs1`
- `ctz rd, rs1`
- `clz rd, rs1`
- `popcount rd, rs1`
- `shra rd, rs1, rs2`
- `shrl rd, rs1, rs2`
- `shll rd, rs1, rs2`
- `ror rd, rs1, rs2`
- `rol rd, rs1, rs2`
- `copy src, dest, len`
- `swap buff1, buff2, len`
- `fill buff, n, fill`
- `through rd, rs1`
- `from rd, rs1`
- `popb rd, sp`
- `poph rd, sp`
- `pop rd, sp`
- `pushb rd, sp`
- `pushh rd, sp`
- `push rd, sp`
- `save start, end, dest`
- `restore start, end, src`
- `exch rs1, rs2`

    ╭────┬───────┬───────┬─────┬───────┬───────┬────────╮
    │s: 1│ irq: 1│ mmu: 1│ p: 3│ ivt: 6│ pdt: 8│ res: 12│
    ╰────┴───────┴───────┴─────┴───────┴───────┴────────╯
    
    s: supervisor, irq: interrupt enable, mmu: mmu enable,
    p: priority, ivt: interrupt vector table (value*1024),
    pdt: page directory table (value*256), res: reserved

## Memory Map

    DATA MEMORY MAP
    ╭───────────────────╮
    │╭────────╮╭───────╮│
    ││Teletype││ 0x0000││ 6  bytes DEVICE 0
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││Timer   ││ 0x0006││ 6  bytes DEVICE 0
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││Keyboard││ 0x000c││ 4  bytes DEVICE 0
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││Video   ││ 0x0010││ 16 bytes DEVICE 1
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││TPS     ││ 0x0020││ 6  bytes DEVICE 2
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││Disk    ││ 0x0026││ 8  bytes DEVICE 2 
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││Custom  ││ 0x0030││
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││........││.......││
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││System  ││ 0x00f0││ DEVICE F
    │╰────────╯╰───────╯│
    │╭────────╮╭───────╮│
    ││Dev Map ││ 0x0100││ DEVICE 10 (16)
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

## IVT

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

## MMU

    ╭────────────────┬────────────────────────┬───┬───┬───────┬─────────╮
    │Page Table Entry│Physical page address:12│w:1│x:1│dirty:1│present:1│
    ╰────────────────┴────────────────────────┴───┴───┴───────┴─────────╯
    ╭──────────────────────────┬─────────────────────────────────┬──────────╮
    │Page Directory Table Entry│Physical address of Page Table:12│reserved:4│
    ╰──────────────────────────┴─────────────────────────────────┴──────────╯

The default values for entries, and sizes, in bytes, are the following:

- Entry size: 2
- Page size: 4096
- Page table entries: 1024
- Page table size: Page table entries * entry_size
- Page directory entries: (memory size/page_size) / page_table_entries

## TTY

    ╭──────┬──────────┬─────╮
    │RX    │ halfword │ 0x00│
    ├──────┼──────────┼─────┤
    │TX    │ halfword │ 0x02│
    ├──────┼──────────┼─────┤
    │STATUS│ byte     │ 0x04│
    ├──────┼──────────┼─────┤
    │CTRL  │ byte     │ 0x05│
    ╰──────┴──────────┴─────╯

## VIDEO

    ╭────────┬────┬────╮
    │COMMAND │byte│ 0x0│
    ├────────┼────┼────┤
    │DATAH   │byte│ 0x1│
    ├────────┼────┼────┤
    │DATAM   │byte│ 0x2│
    ├────────┼────┼────┤
    │DATAL   │byte│ 0x3│
    ├────────┼────┼────┤
    │GPU0    │byte│ 0x4│
    ├────────┼────┼────┤
    │GPU1    │byte│ 0x5│
    ├────────┼────┼────┤
    │GPU2    │byte│ 0x6│
    ├────────┼────┼────┤
    │GPU3    │byte│ 0x7│
    ├────────┼────┼────┤
    │GPU4    │byte│ 0x8│
    ├────────┼────┼────┤
    │GPU5    │byte│ 0x9│
    ├────────┼────┼────┤
    │GPU6    │byte│ 0xa│
    ├────────┼────┼────┤
    │GPU7    │byte│ 0xb│
    ├────────┼────┼────┤
    │STATUS0 │byte│ 0xc│
    ├────────┼────┼────┤
    │STATUS1 │byte│ 0xd│
    ├────────┼────┼────┤
    │STATUS2 │byte│ 0xe│
    ├────────┼────┼────┤
    │STATUS3 │byte│ 0xf│
    ╰────────┴────┴────╯
    ╭────────┬──────────────┬──────────────╮
    │blink: 1│ background: 3│ foreground: 4│
    ╰────────┴──────────────┴──────────────╯

- (`0x0`) Nop: do nothing.
- (`0x1`) Clear: clear the framebuffer.
- (`0x2`) Set Mode (dh: mode): sets the requested mode.
- (`0x4`) Set Font (dh: font): sets the requested hardware font (if it does not exist, default to 0).
- (`0x6`) Blit (dh-dm-dl): blits the framebuffer with the contents of the buffer at address dh-dm-dl.

## KEYBOARD

    ╭─────────┬────╮
    │CHARACTER│0x00│
    ├─────────┼────┤
    │CODE     │0x01│
    ├─────────┼────┤
    │MODIFIERS│0x02│
    ├─────────┼────┤
    │KBDMODE  │0x03│
    ╰─────────┴────╯
    ╭──────────┬───────┬──────┬─────┬──────╮
    │reserved:4│shift:1│ctrl:1│alt:1│logo:1│
    ╰──────────┴───────┴──────┴─────┴──────╯

## STORAGE

    ╭───────┬─────╮
    │COMMAND│ 0x00│
    ├───────┼─────┤
    │DATA   │ 0x01│
    ├───────┼─────┤
    │POINTH │ 0x02│
    ├───────┼─────┤
    │POINTL │ 0x03│
    ├───────┼─────┤
    │STATUSH│ 0x04│
    ├───────┼─────┤
    │STATUSL│ 0x05│
    ╰───────┴─────╯

The commands available to address the TPS drive are the following:

- (`0x00`) Nop: does nothing.
- (`0x01`) Is Bootable: returns `1` in `STATUSL` if the selected drive is bootable.
- (`0x02`) Is Present: returns `1` in `STATUSL` if the selected drive is present.
- (`0x03`) Open: opens the selected drive.
- (`0x04`) Close: closes the selected drive.
- (`0x05`) Store Sector: stores the sector specified in `pointh-pointl` into the sector `data`.
- (`0x06`) Load Sector: stores the sector specified in `data` into `pointh-pointl`.

The command must designate in the high order byte if the drive `0` or `1` is addressed.

    ╭───────┬─────╮
    │COMMAND│ 0x00│
    ├───────┼─────┤
    │DATA   │ 0x01│
    ├───────┼─────┤
    │SECTORH│ 0x02│
    ├───────┼─────┤
    │SECTORL│ 0x03│
    ├───────┼─────┤
    │POINTH │ 0x04│
    ├───────┼─────┤
    │POINTL │ 0x05│
    ├───────┼─────┤
    │STATUS0│ 0x06│
    ├───────┼─────┤
    │STATUS1│ 0x07│
    ╰───────┴─────╯

The commands available to address the Disk Drive are the following:

- (`0x00`) Nop: does nothing.
- (`0x01`) Store Sector: stores the sector at `point` into `sector` at `data` drive.
- (`0x02`) Load Sector: loads the sector at `sector` at `data` into `point`.

## TIMER

    ╭─────────┬────╮
    │TIMEOUT  │0x00│
    ├─────────┼────┤
    │INTERVAL │0x02│
    ╰─────────┴────╯
