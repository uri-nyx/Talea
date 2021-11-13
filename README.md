# Talea: the earliest computer for the Guild of Talandel

## History

Talea is the first computer machine that was developed during the late 4th and mid 7th Dreams of Nar Naellan (in both, the exact same deisgn was developed, due to a scheme woven by Nimue against the Purity of the Dream wished by the Fae Queens -- a clash that was actually intended to be following the Plan of The Wanderer)

It's main purpose was, opposed to that of the Machine of the Archive of Arkade, accounting and basic data processing, to be used in commerce and banking by merchants and companies of Talandel. It was cheaper and smaller than the one in the Academy, and a net of these standard machines, once tested, began spreading over the city and some of its colonies, to be eventually become mandatory for buissnes by decree of the prince 20 years past its creation.

## Architecture

+ Little Endian
+ No interruptions (added in standard official model)
+ 64 Kb of addressable memory (byte addressable)
+ 8 general purpose 8-bit registers
+ 16-bit program counter
+ 8-bit stack pointer
+ 8-bit flag register
+ Serial and parallel I/O

### Memory map

+ 0x0000 ... 0x00ff: Zero Page
+ 0x0100 ... 0x01ff: Input
+ 0x0200 ... 0x02ff: Output
+ 0x0300 ... 0x03ff: Basic I/O Routines
+ 0x0400 ... 0x0bff: BIOS ROM (or general RAM)
+ 0x0c00 ... 0xffff: General RAM

### Registers

+ acc, bcc: accumulators
+ r1, r2, r3, r4: general purpose, guaranteed to ever be avilable
+ hx, lx: indirect adress registers

+ sp: hardware stack pointer
+ pc: program counter

+ status: ---NV-ZC 

### ISA

+ x0 and    r,$,#,zp: acc &= arg (a,b,g)
+ x1 xor    r,$,#,zp: acc ^= arg (a,b,g)
+ x2 add    r,$,#,zp: acc += arg (a,b,g)
+ x3 adc    r,$,#,zp: acc += arg + carry (a,b,g)
+ x4 subc   r,$,#,zp: acc -= arg - carry (a,b,g)

+ x5 push   r: stack[sp++] = r
+ x5 pop    r: r = stack[sp]; sp--
+ x6 shiftl r: r = r << 1
   - shiftr r: r = r >> 1 (fill with N)

+ x7 ldr    r;r,$,#,zp: r = arg

+ x8 str    r;$,zp,(hl): mem[arg] = r
+ x9 sti    r;$,zp,(hl): mem[mem[arg]] = r

+ xA ldi    r;$,zp,(hl): r = mem[mem[arg]]
+ xB lea    $.zp: hl = $ 

+ xC bnz    $,zp,(hl): Z != 0 ? pc = arg : pc++
   - bez    $,zp,(hl): Z == 0 ? pc = arg : pc++
   - ben    $,zp,(hl): N == 1 ? pc = arg : pc++
   - call   $,zp,(hl): push(pc); pc = arg

+ xD jmp    (hl): push(pc); pc = mem[arg]
   - ret    : pc = pop 16 bytes from stack
   - swap   : acc <-> bcc
   - psr    : push status
   - ssr    : set status
   - psp    : push sp    
   - ssp    : set sp
   - not    : acc = ~(acc)∫
   - sec    : set carry
   - clc    : clear carry
   - nop