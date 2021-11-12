# Talea: the earliest computer for the Guild of Talandel

+ Little Endian
+ No interruptions
+ 64 Kb of addressable memory (byte addressable)
+ 8 general purpose 8-bit registers
+ 16-bit program counter
+ 8-bit stack pointer
+ 8-bit flag register

## Memory map

+ 0x0000 ... 0x00ff: Zero Page
+ 0x0100 ... 0x01ff: Input
+ 0x0200 ... 0x02ff: Output
+ 0x0300 ... 0x03ff: Basic I/O Routines
+ 0x0400 ... 0x0bff: BIOS ROM (or general RAM)
+ 0x0c00 ... 0xffff: General RAM

## Registers

+ acc, bcc: accumulators
+ r1, r2, r3, r4: general purpose, guaranteed to ever be avilable
+ hx, lx: indirect adress registers

+ sp: hardware stack pointer
+ pc: program counter

+ status: ---NVDZC

# ISA

+ x0 and    r,$,#: acc &= arg (a,b,g)
+ x1 xor    r,$,#: acc ^= arg (a,b,g)
+ x2 add    r,$,#: acc += arg (a,b,g)
+ x3 adc    r,$,#: acc += arg + carry (a,b,g)
+ x4 subc   r,$,#: acc -= arg - carry (a,b,g)

+ x5 push   r: stack[sp++] = r
+ x5 pop    r: r = stack[sp]; sp--
+ x6 shiftl r: r = r << 1
   - shiftr r: r = r >> 1 (fill with N)

+ x7 ldr    r;r,$,#: r = arg

+ x8 str    r;$,(hl): mem[arg] = r
+ x9 sti    r;$,(hl): mem[mem[arg]] = r

+ xA ldi    r;$,(hl): r = mem[mem[arg]]
+ xB lea    $: hl = $ 

+ xC bnz    $,(hl): Z != 0 ? pc = arg : pc++
   - bez    $,(hl): Z == 0 ? pc = arg : pc++
   - ben    $,(hl): N == 1 ? pc = arg : pc++
   - call   $,(hl): push(pc); pc = arg

+ xD jmp    (hl): push(pc); pc = mem[arg]
   - ret    : pc = pop 16 bytes from stack
   - swap   : acc <-> bcc
   - psr    : push status
   - ssr    : set status
   - psp    : push sp    
   - ssp    : set sp
   - not    : acc = ~(acc)∫
   - nop
   - sec    : set carry