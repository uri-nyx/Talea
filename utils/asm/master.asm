; This is the basic assembler implementation for the Taleä Computer System
#once

SYS    = 0b000
MEM    = 0b001
BRANCH_  = 0b011
LOAD   = 0b100
ALUI   = 0b101
ALUR   = 0b110
STOR_E_  = 0b111

BLANK5 = 0b00000
BLANK10 = 0b00000_00000
BLANK15 = 0b00000_00000_00000

LUI     = 0b010_0001
AUIPC   = 0b010_0010
JAL     = 0b010_0000
JALR    = 0b100_0001

BEQ     = BRANCH_  @ 0x0
BNE     = BRANCH_  @ 0x1
BLT     = BRANCH_  @ 0x2
BGE     = BRANCH_  @ 0x3
BLTU    = BRANCH_  @ 0x4
BGEU    = BRANCH_  @ 0x5

LB      = LOAD @ 0x2
LBU     = LOAD @ 0x3
LBD     = LOAD @ 0x4
LBUD    = LOAD @ 0x5
LH      = LOAD @ 0x6
LHU     = LOAD @ 0x7
LHD     = LOAD @ 0x8
LHUD    = LOAD @ 0x9
LW      = LOAD @ 0xa
LWD     = LOAD @ 0xb

MULI    = ALUI @ 0x0
MULIH   = ALUI @ 0x1
IDIVI   = ALUI @ 0x2
ADDI    = ALUI @ 0x3
SUBI    = ALUI @ 0x4

OR_I     = ALUI @ 0x5
AND_I    = ALUI @ 0x6
XOR_I    = ALUI @ 0x7
SHIRA   = ALUI @ 0x8
SHIRL   = ALUI @ 0x9
SHILL   = ALUI @ 0xa

SLTI    = ALUI @ 0xb
SLTIU   = ALUI @ 0xc

ADD_    = ALUR @ 0x0 
SUB_    = ALUR @ 0x1 
IDIV    = ALUR @ 0x2
MUL     = ALUR @ 0x3

OR_      = ALUR @ 0x4 
AND_     = ALUR @ 0x5
XOR_     = ALUR @ 0x6 

NOT     = ALUR @ 0x7
CTZ     = ALUR @ 0x8
CLZ     = ALUR @ 0x9
PCOUNT  = ALUR @ 0xa

SHRA    = ALUR @ 0xb 
SHRL    = ALUR @ 0xc 
SHLL    = ALUR @ 0xd 
ROR_     = ALUR @ 0xe
ROL     = ALUR @ 0xf

SB      = STOR_E_ @ 0x0
SBD     = STOR_E_ @ 0x1
SH      = STOR_E_ @ 0x2
SHD     = STOR_E_ @ 0x3
SW      = STOR_E_ @ 0x4
SWD     = STOR_E_ @ 0x5

COPY    = MEM @ 0x0
SWAP_    = MEM @ 0x1
FILL_    = MEM @ 0x2
THRO    = MEM @ 0x3
FROM    = MEM @ 0x4

POPB    = MEM @ 0x5
POPH    = MEM @ 0x6
POP     = MEM @ 0x7
PUSHB   = MEM @ 0x8
PUSHH   = MEM @ 0x9
PUSH    = MEM @ 0xa

SAVE    = MEM @ 0xb
RESTOR_E_ = MEM @ 0xc
EXCH    = MEM @ 0xd
SLT     = MEM @ 0xe 
SLTU   = MEM @ 0xf 

SYSCALL  = SYS @ 0x2
GSREG    = SYS @ 0x3
SSREG    = SYS @ 0x4
TRACE   = SYS @ 0x5
SYSRET   = SYS @ 0x6
MMUTOGGLE = SYS @ 0x7
MMUMAP = SYS @ 0x8
MMUUNMAP = SYS @ 0x9
MMUSTAT = SYS @ 0xa
MMUSETPT = SYS @ 0xb
MMUUPDATE = SYS @ 0xc


#ruledef reg {
    zero => 0b00000
    ra  =>  0b00001
    sp  =>  0b00010
    gp  =>  0b00011
    tp  =>  0b00100
    t0  =>  0b00101
    t1  =>  0b00110
    t2  =>  0b00111
    s1  =>  0b01001
    fp  =>  0b01000
    s0  =>  0b01000
    a0  =>  0b01010
    a1  =>  0b01011
    a2  =>  0b01100
    a3  =>  0b01101
    a4  =>  0b01110
    a5  =>  0b01111
    a6  =>  0b10000
    a7  =>  0b10001
    s2  =>  0b10010
    s3  =>  0b10011
    s4  =>  0b10100
    s5  =>  0b10101
    s6  =>  0b10110
    s7  =>  0b10111
    s8  =>  0b11000
    s9  =>  0b11001
    s10 =>  0b11010
    s11 =>  0b11011
    t3  =>  0b11100
    t4  =>  0b11101
    t5  =>  0b11110
    t6  =>  0b11111
    r {n: u5} => n
}

#ruledef {
    lui   {rd: reg}, {imm: u20}  => LUI   @ rd @ imm
    auipc {rd: reg}, {imm: u20}  => AUIPC @ rd @ imm
    jal   {rd: reg}, {label}  => {
        imm = label - $
        assert((imm % 4) == 0)
        ; NOTE OFFSETS FOR_ JUMPS MUST BE 4 BYTE ALIGNED,
        ; OTHERWISE IT WILL RESULT IN AN EXCEPTION
        imm20 = imm >> 2
        JAL @ rd @ imm20`20
    }

    jal   {rd: reg}, [{offset: i22}]  => {
        assert((offset % 4) == 0)
        ; NOTE OFFSETS FOR_ JUMPS MUST BE 4 BYTE ALIGNED,
        ; OTHERWISE IT WILL RESULT IN AN EXCEPTION
        imm20 = offset >> 2
        JAL @ rd @ imm20`20
    }
    jalr {rd: reg}, {imm: s17}({rs1: reg}) => { 
        assert((imm % 4) == 0)
        ; NOTE OFFSETS FOR_ JUMPS MUST BE 4 BYTE ALIGNED,
        ; OTHERWISE IT WILL RESULT IN AN EXCEPTION
        JALR @ rd @ rs1 @ (imm >> 2)`15
    }

    beq  {rs1: reg}, {rs2: reg}, {label} => {
        imm = label - $
        assert((imm % 4) == 0)
        ; NOTE OFFSETS FOR_ JUMPS MUST BE 4 BYTE ALIGNED,
        ; OTHERWISE IT WILL RESULT IN AN EXCEPTION
        imm15 = imm >> 2
        BEQ @ rs1 @ rs2 @ imm15`15
    }
    bne  {rs1: reg}, {rs2: reg}, {label} => {
        imm = label - $
        assert((imm % 4) == 0)
        ; NOTE OFFSETS FOR_ JUMPS MUST BE 4 BYTE ALIGNED,
        ; OTHERWISE IT WILL RESULT IN AN EXCEPTION
        imm15 = imm >> 2
        BNE @ rs1 @ rs2 @ imm15`15
    }
    blt  {rs1: reg}, {rs2: reg}, {label} => {
        imm = label - $
        assert((imm % 4) == 0)
        ; NOTE OFFSETS FOR_ JUMPS MUST BE 4 BYTE ALIGNED,
        ; OTHERWISE IT WILL RESULT IN AN EXCEPTION
        imm15 = imm >> 2
        BLT @ rs1 @ rs2 @ imm15`15
    }
    bge  {rs1: reg}, {rs2: reg}, {label} => {
        imm = label - $
        assert((imm % 4) == 0)
        ; NOTE OFFSETS FOR_ JUMPS MUST BE 4 BYTE ALIGNED,
        ; OTHERWISE IT WILL RESULT IN AN EXCEPTION
        imm15 = imm >> 2
        BGE @ rs1 @ rs2 @ imm15`15
    }
    bltu {rs1: reg}, {rs2: reg}, {label} => {
        imm = label - $
        assert((imm % 4) == 0)
        ; NOTE OFFSETS FOR_ JUMPS MUST BE 4 BYTE ALIGNED,
        ; OTHERWISE IT WILL RESULT IN AN EXCEPTION
        imm15 = imm >> 2
        BLTU @ rs1 @ rs2 @ imm15`15
    }
    bgeu {rs1: reg}, {rs2: reg}, {label} => {
        imm = label - $
        assert((imm % 4) == 0)
        ; NOTE OFFSETS FOR_ JUMPS MUST BE 4 BYTE ALIGNED,
        ; OTHERWISE IT WILL RESULT IN AN EXCEPTION
        imm15 = imm >> 2
        BGEU @ rs1 @ rs2 @ imm15`15
    }

    beq  {rs1: reg}, {rs2: reg}, [{offset: i17}] => {
        assert((offset % 4) == 0)
        ; NOTE OFFSETS FOR_ JUMPS MUST BE 4 BYTE ALIGNED,
        ; OTHERWISE IT WILL RESULT IN AN EXCEPTION
        imm15 = offset >> 2
        BEQ @ rs1 @ rs2 @ imm15`15
    }
    bne  {rs1: reg}, {rs2: reg}, [{offset: i17}] => {
        assert((offset % 4) == 0)
        ; NOTE OFFSETS FOR_ JUMPS MUST BE 4 BYTE ALIGNED,
        ; OTHERWISE IT WILL RESULT IN AN EXCEPTION
        imm15 = offset >> 2
        BNE @ rs1 @ rs2 @ imm15`15
    }
    blt  {rs1: reg}, {rs2: reg}, [{offset: i17}] => {
        assert((offset % 4) == 0)
        ; NOTE OFFSETS FOR_ JUMPS MUST BE 4 BYTE ALIGNED,
        ; OTHERWISE IT WILL RESULT IN AN EXCEPTION
        imm15 = offset >> 2
        BLT @ rs1 @ rs2 @ imm15`15
    }
    bge  {rs1: reg}, {rs2: reg}, [{offset: i17}] => {
        assert((offset % 4) == 0)
        ; NOTE OFFSETS FOR_ JUMPS MUST BE 4 BYTE ALIGNED,
        ; OTHERWISE IT WILL RESULT IN AN EXCEPTION
        imm15 = offset >> 2
        BGE @ rs1 @ rs2 @ imm15`15
    }
    bltu {rs1: reg}, {rs2: reg}, [{offset: i17}] => {
        assert((offset % 4) == 0)
        ; NOTE OFFSETS FOR_ JUMPS MUST BE 4 BYTE ALIGNED,
        ; OTHERWISE IT WILL RESULT IN AN EXCEPTION
        imm15 = offset >> 2
        BLTU @ rs1 @ rs2 @ imm15`15
    }
    bgeu {rs1: reg}, {rs2: reg}, [{offset: i17}] => {
        assert((offset % 4) == 0)
        ; NOTE OFFSETS FOR_ JUMPS MUST BE 4 BYTE ALIGNED,
        ; OTHERWISE IT WILL RESULT IN AN EXCEPTION
        imm15 = offset >> 2
        BGEU @ rs1 @ rs2 @ imm15`15
    }

    lb   {rd: reg}, {imm: s15}({rs1: reg})  => LB @ rd @ rs1 @ imm
    lbu  {rd: reg}, {imm: s15}({rs1: reg})  => LBU @ rd @ rs1 @ imm
    lbd  {rd: reg}, {imm: s15}({rs1: reg})  => LBD @ rd @ rs1 @ imm
    lbud {rd: reg}, {imm: s15}({rs1: reg})  => LBUD @ rd @ rs1 @ imm
    lh   {rd: reg}, {imm: s15}({rs1: reg})  => LH @ rd @ rs1 @ imm
    lhu  {rd: reg}, {imm: s15}({rs1: reg})  => LHU @ rd @ rs1 @ imm
    lhd  {rd: reg}, {imm: s15}({rs1: reg})  => LHD @ rd @ rs1 @ imm
    lhud {rd: reg}, {imm: s15}({rs1: reg})  => LHUD @ rd @ rs1 @ imm
    lw   {rd: reg}, {imm: s15}({rs1: reg})  => LW @ rd @ rs1 @ imm
    lwd  {rd: reg}, {imm: s15}({rs1: reg})  => LWD @ rd @ rs1 @ imm

    muli  {rd: reg}, {rs1: reg}, {imm: i15} => MULI @ rd @ rs1 @ imm
    mulih {rd: reg}, {rs1: reg}, {imm: i15} => MULIH @ rd @ rs1 @ imm
    idivi {rd: reg}, {rs1: reg}, {imm: i15} => {
        assert(imm != 0)
        IDIVI @ rd @ rs1 @ imm
    }
    addi  {rd: reg}, {rs1: reg}, {imm: i15} => ADDI @ rd @ rs1 @ imm
    subi  {rd: reg}, {rs1: reg}, {imm: i15} => SUBI @ rd @ rs1 @ imm

    ori   {rd: reg}, {rs1: reg}, {imm: i15} => OR_I @ rd @ rs1 @ imm
    andi  {rd: reg}, {rs1: reg}, {imm: i15} => AND_I @ rd @ rs1 @ imm
    xori  {rd: reg}, {rs1: reg}, {imm: i15} => XOR_I @ rd @ rs1 @ imm
    shira {rd: reg}, {rs1: reg}, {imm: i15} => SHIRA @ rd @ rs1 @ imm
    shirl {rd: reg}, {rs1: reg}, {imm: i15} => SHIRL @ rd @ rs1 @ imm
    shill {rd: reg}, {rs1: reg}, {imm: i15} => SHILL @ rd @ rs1 @ imm

    slti  {rd: reg}, {rs1: reg}, {imm: i15} => SLTI @ rd @ rs1 @ imm
    sltiu {rd: reg}, {rs1: reg}, {imm: u15} => SLTIU @ rd @ rs1 @ imm

    add    {rd: reg}, {rs1: reg}, {rs2: reg} => ADD_ @ rd @ rs1 @ rs2 @ BLANK10
    sub    {rd: reg}, {rs1: reg}, {rs2: reg} => SUB_ @ rd @ rs1 @ rs2 @ BLANK10
    idiv   {rd: reg}, {rd2: reg}, {rs1: reg}, {rs2: reg} => IDIV @ rd @ rd2 @ rs1 @ rs2 @ BLANK5
    mul    {rd: reg}, {rd2: reg}, {rs1: reg}, {rs2: reg} => MUL @ rd @ rd2 @ rs1 @ rs2  @ BLANK5

    or     {rd: reg}, {rs1: reg}, {rs2: reg} => OR_ @ rd @ rs1 @ rs2  @ BLANK10
    and    {rd: reg}, {rs1: reg}, {rs2: reg} => AND_ @ rd @ rs1 @ rs2 @ BLANK10
    xor    {rd: reg}, {rs1: reg}, {rs2: reg} => XOR_ @ rd @ rs1 @ rs2 @ BLANK10

    slt    {rd: reg}, {rs1: reg}, {rs2: reg} => SLT @ rd @ rs1 @ rs2 @ BLANK10
    sltu   {rd: reg}, {rs1: reg}, {rs2: reg} => SLTU @ rd @ rs1 @ rs2 @ BLANK10

    not    {rd: reg}, {rs1: reg} => NOT @ rd @ rs1 @ BLANK15
    ctz    {rd: reg}, {rs1: reg} => CTZ @ rd @ rs1 @ BLANK15
    clz    {rd: reg}, {rs1: reg} => CLZ @ rd @ rs1 @ BLANK15
    pcount {rd: reg}, {rs1: reg} => PCOUNT @ rd @ rs1 @ BLANK15

    shra   {rd: reg}, {rs1: reg}, {rs2: reg} => SHRA @ rd @ rs1 @ rs2 @ BLANK10
    shrl   {rd: reg}, {rs1: reg}, {rs2: reg} => SHRL @ rd @ rs1 @ rs2 @ BLANK10
    shll   {rd: reg}, {rs1: reg}, {rs2: reg} => SHLL @ rd @ rs1 @ rs2 @ BLANK10
    ror    {rd: reg}, {rs1: reg}, {rs2: reg} => ROR_ @ rd @ rs1 @ rs2  @ BLANK10
    rol    {rd: reg}, {rs1: reg}, {rs2: reg} => ROL @ rd @ rs1 @ rs2  @ BLANK10

    sb  {rs2: reg}, {imm: i15}({rs1: reg})   => SB @ rs2 @ rs1 @ imm
    sbd {rs2: reg}, {imm: i15}({rs1: reg})   => SBD @ rs2 @ rs1 @ imm
    sh  {rs2: reg}, {imm: i15}({rs1: reg})   => SH @ rs2 @ rs1 @ imm
    shd {rs2: reg}, {imm: i15}({rs1: reg})   => SHD @ rs2 @ rs1 @ imm
    sw  {rs2: reg}, {imm: i15}({rs1: reg})   => SW @ rs2 @ rs1 @ imm
    swd {rs2: reg}, {imm: i15}({rs1: reg})   => SWD @ rs2 @ rs1 @ imm

    copy    {rd: reg}, {rs1: reg}, {rs2: reg} => COPY @ rd @ rs1 @ rs2 @ BLANK10
    swap    {rd: reg}, {rs1: reg}, {rs2: reg} => SWAP_ @ rd @ rs1 @ rs2 @ BLANK10
    fill    {rd: reg}, {rs1: reg}, {rs2: reg} => FILL_ @ rd @ rs1 @ rs2 @ BLANK10
    thro    {rd: reg}, {rs1: reg} => THRO @ rd @ rs1 @ BLANK15
    from    {rd: reg}, {rs1: reg} => FROM @ rd @ rs1 @ BLANK15

    popb    {rd: reg}, {rs1: reg} => POPB @ rd @ rs1 @ BLANK15
    poph    {rd: reg}, {rs1: reg} => POPH @ rd @ rs1 @ BLANK15
    pop     {rd: reg}, {rs1: reg} => POP @ rd @ rs1  @ BLANK15
    pushb   {rd: reg}, {rs1: reg} => PUSHB @ rd @ rs1 @ BLANK15
    pushh   {rd: reg}, {rs1: reg} => PUSH @ rd @ rs1  @ BLANK15
    push    {rd: reg}, {rs1: reg} => PUSH @ rd @ rs1  @ BLANK15

    save    {rd: reg}, {rs1: reg}, {rs2: reg} => SAVE @ rd @ rs1 @ rs2 @ BLANK10
    restore {rd: reg}, {rs1: reg}, {rs2: reg} => RESTOR_E_ @ rd @ rs1 @ rs2 @ BLANK10
    exch    {rd: reg}, {rs1: reg}  => EXCH @ rd @ rs1 @ BLANK15
    
    syscall {rd: reg}, {vector: u8} => SYSCALL @ rd @ 0x000 @ vector 
    gsreg   {rd: reg} => GSREG @ rd @ BLANK10 @ BLANK10
    ssreg   {rs1: reg} => SSREG @ rs1 @ BLANK10 @ BLANK10
    trace   {r1: reg}, {r2: reg}, {r3: reg}, {r4: reg} => TRACE @ r1 @ r2 @ r3 @ r4 @ BLANK5
    sysret  => SYSRET @ BLANK15 @ BLANK10

    mmu.toggle  {r1: reg} => MMUTOGGLE @ r1@ BLANK10 @ BLANK10
    mmu.map     {r1: reg}, {r2: reg}, {r3: reg}, ({w: u1}, {x: u1}) => MMUMAP @ r1 @ r2 @ r3 @ BLANK5 @ 0b000 @ w @ x
    mmu.unmap   {r1: reg} => MMUUNMAP @ r1@ BLANK10 @ BLANK10
    mmu.stat    {rd: reg}, {rs1: reg} => MMUSTAT @ rd @ rs1 @ BLANK15
    mmu.setpt   {r1: reg}, {r2: reg}, {imm: u12} => MMUSETPT @ r1 @ r2 @ 0b000 @ imm
    mmu.update  {r1: reg}, {r2: reg}, ({dirty: u1}, {present: u1}) => MMUUPDATE @ r1 @ r2 @ BLANK10 @ 0b000 @ dirty @ present

}


#include "pseudo.asm"