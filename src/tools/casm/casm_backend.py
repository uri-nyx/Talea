from typing import List, Tuple

registers = {"acc" : 0x0, "bcc" : 0x1, "r1" : 0x2, "r2" : 0x3, "r3" : 0x4, "r4" : 0x5, "hx" : 0x6, "lx" : 0x7}

def jmp_g(args):
    return 0xd0
def ret_g(args):
    return 0xd1
def swap_g(args):
    return 0xd2    
def psr_g(args):
    return 0xd3
def ssr_g(args):
    return 0xd4
def psp_g(args):
    return 0xd5
def ssp_g(args):
    return 0xd6
def not_g(args):
    return 0xd7
def sec_g(args):
    return 0xd8
def clc_g(args):
    return 0xd9
def nop_g(args):
    return 0xda

def push_g(args):
    opcode = 0x5
    r = registers[args[0][1]]
    b = (opcode << 4) | r
    return b
def pop_g(args):
    opcode = 0x5
    r = 0x8 | registers[args[0][1]]
    b = (opcode << 4) | r
    return b
def shiftl_g(args):
    opcode = 0x6
    r = registers[args[0][1]]
    b = (opcode << 4) | r
    return b
def shiftr_g(args):
    opcode = 0x6
    r = 0x8 | registers[args[0][1]]
    b = (opcode << 4) | r
    return b

def lea_g(args):
    if args[0][0] == "address":
        opcode = 0xb0
        addrL = args[0][1] & 0xff
        addrH = args[0][1] >> 8
        b = [opcode, addrL, addrH]
        return b
    elif args[0][0] == "zp":
        opcode = 0xe9
        addrL = args[0][1] & 0xff
        b = [opcode, addrL]
        return b

def bnz_g(args):
    if len(args) == 1:
        if args[0][0] == "address":
            opcode = 0xc8 | 0x0
            addrL = args[0][1] & 0xff
            addrH = args[0][1] >> 8
            b = [opcode, addrL, addrH]
            return b
        elif args[0][0] == "zp":
            opcode = 0xe5
            addrL = args[0][1] & 0xff
            b = [opcode, addrL]
            return b
    else:
        return 0xc0
    
def bez_g(args):
    if len(args) == 1:
        if args[0][0] == "address":
            opcode = 0xc8 | 0x1
            addrL = args[0][1] & 0xff
            addrH = args[0][1] >> 8
            b = [opcode, addrL, addrH]
            return b
        elif args[0][0] == "zp":
            opcode = 0xe6
            addrL = args[0][1] & 0xff
            b = [opcode, addrL]
            return b
    else:
        return 0xc1
    
def ben_g(args):
    if len(args) == 1:
        if args[0][0] == "address":
            opcode = 0xc8 | 0x2
            addrL = args[0][1] & 0xff
            addrH = args[0][1] >> 8
            b = [opcode, addrL, addrH]
            return b
        elif args[0][0] == "zp":
            opcode = 0xe7
            addrL = args[0][1] & 0xff
            b = [opcode, addrL]
            return b
    else:
        return 0xc2
    
def call_g(args):
    if len(args) == 1:
        if args[0][0] == "address":
            opcode = 0xc8 | 0x3
            addrL = args[0][1] & 0xff
            addrH = args[0][1] >> 8
            b = [opcode, addrL, addrH]
            return b
        elif args[0][0] == "zp":
            opcode = 0xe8
            addrL = args[0][1] & 0xff
            b = [opcode, addrL]
            return b
    else:
        return 0xc3

def and_g(args):
    if args[0][0] == "address":
        opcode = 0x0c
        addrL = args[0][1] & 0xff
        addrH = args[0][1] >> 8
        b = [opcode, addrL, addrH]
        return b
    if args[0][0] == "zp":
        opcode = 0xe0
        addrL = args[0][1] & 0xff
        b = [opcode, addrL]
        return b
    elif  "litteral" in args[0][0]:
        opcode = 0x08
        litt = args[0][1] & 0xff
        b = [opcode, litt]
        return b
    elif args[0][0] == "register":
        opcode = 0x0
        r = registers[args[0][1]]
        b = (opcode << 4) | r
        return b
def xor_g(args):
    if args[0][0] == "address":
        opcode = 0x1c
        addrL = args[0][1] & 0xff
        addrH = args[0][1] >> 8
        b = [opcode, addrL, addrH]
        return b
    if args[0][0] == "zp":
        opcode = 0xe1
        addrL = args[0][1] & 0xff
        b = [opcode, addrL]
        return b
    elif  "litteral" in args[0][0]:
        opcode = 0x18
        litt = args[0][1] & 0xff
        b = [opcode, litt]
        return b
    elif args[0][0] == "register":
        opcode = 0x1
        r = registers[args[0][1]]
        b = (opcode << 4) | r
        return b
def add_g(args):
    if args[0][0] == "address":
        opcode = 0x2c
        addrL = args[0][1] & 0xff
        addrH = args[0][1] >> 8
        b = [opcode, addrL, addrH]
        return b
    if args[0][0] == "zp":
        opcode = 0xe2
        addrL = args[0][1] & 0xff
        b = [opcode, addrL]
        return b
    elif  "litteral" in args[0][0]:
        opcode = 0x28
        litt = args[0][1] & 0xff
        b = [opcode, litt]
        return b
    elif args[0][0] == "register":
        opcode = 0x2
        r = registers[args[0][1]]
        b = (opcode << 4) | r
        return b
def adc_g(args):
    if args[0][0] == "address":
        opcode = 0x3c
        addrL = args[0][1] & 0xff
        addrH = args[0][1] >> 8
        b = [opcode, addrL, addrH]
        return b
    if args[0][0] == "zp":
        opcode = 0xe3
        addrL = args[0][1] & 0xff
        b = [opcode, addrL]
        return b
    elif  "litteral" in args[0][0]:
        opcode = 0x38
        litt = args[0][1] & 0xff
        b = [opcode, litt]
        return b
    elif args[0][0] == "register":
        opcode = 0x3
        r = registers[args[0][1]]
        b = (opcode << 4) | r
        return b
def subb_g(args):
    if args[0][0] == "address":
        opcode = 0x4c
        addrL = args[0][1] & 0xff
        addrH = args[0][1] >> 8
        b = [opcode, addrL, addrH]
        return b
    if args[0][0] == "zp":
        opcode = 0xe4
        addrL = args[0][1] & 0xff
        b = [opcode, addrL]
        return b
    elif  "litteral" in args[0][0]:
        opcode = 0x48
        litt = args[0][1] & 0xff
        b = [opcode, litt]
        return b
    elif args[0][0] == "register":
        opcode = 0x4
        r = registers[args[0][1]]
        b = (opcode << 4) | r
        return b

def ldr_g(args):
    reg1 = registers[args[0][1]]
    if args[1][0] == "address":
        opcode = 0x70 | reg1
        addrL = args[1][1] & 0xff
        addrH = args[1][1] >> 8
        b = [opcode, addrL, addrH]
        return b
    if args[1][0] == "zp":
        opcode = 0xf0 | reg1
        addrL = args[1][1] & 0xff
        b = [opcode, addrL]
        return b
    elif  "litteral" in args[1][0]:
        opcode = 0x78
        litt = args[1][1] & 0xff
        b = [opcode, litt, reg1]
        return b
    elif args[1][0] == "register":
        opcode = 0x7c
        reg2 = registers[args[1][1]]
        b = [opcode, reg2, reg1]
        return b
    
def str_g(args):
    reg1 = registers[args[0][1]]
    if len(args) == 2:
        if args[1][0] == "address":
            opcode = 0x88 | reg1
            addrL = args[1][1] & 0xff
            addrH = args[1][1] >> 8
            b = [opcode, addrL, addrH]
            return b
        elif args[1][0] == "zp":
            if reg1 < 6:
                opcode = 0xd0 | (0xa + reg1)
            else:
                opcode = 0xe0 | (0x5 + reg1)
                
            addrL = args[1][1] & 0xff
            b = [opcode, addrL]
            return b
    else:
        return (0x80 | reg1)
def sti_g(args):
    reg1 = registers[args[0][1]]
    if len(args) == 2:
        if args[1][0] == "address":
            opcode = 0x98 | reg1
            addrL = args[1][1] & 0xff
            addrH = args[1][1] >> 8
            b = [opcode, addrL, addrH]
            return b
        elif args[1][0] == "zp":
            opcode = 0xf8 | reg1               
            addrL = args[1][1] & 0xff
            b = [opcode, addrL]
            return b
    else:
        return (0x90 | reg1)

def ldi_g(args):
    reg1 = registers[args[0][1]]
    if len(args) == 2:
        if args[1][0] == "address":
            opcode = 0xa8 | reg1
            addrL = args[1][1] & 0xff
            addrH = args[1][1] >> 8
            b = [opcode, addrL, addrH]
            return b
        elif args[1][0] == "zp":
            opcode = 0xb8 | reg1               
            addrL = args[1][1] & 0xff
            b = [opcode, addrL]
            return b
    else:
        return (0xa0 | reg1)
    


def org_g(args):
    pass
def byte_g(args):
    b = args[0][1] & 0xff
    return b
def word_g(args):
    l = args[0][1] & 0xff
    h = (args[0][1] >> 8) & 0xff
    b = [l, h]
    return b
def stringz_g(args):
    xs = " ".join([s[0:][1] for s in args])
    b = [(ord(s) & 0xff) for s in xs]
    b.append(ord('\0'))
    return b
def alloc_g(args):
    bytes_to_alloc = args[0][1]
    b = [0 for s in range(bytes_to_alloc)]
    return b

def end_g(args):
    b = [0x78, 0xff, 0x00, 0x88, 0xfe, 0xff]

def codegen(line: List[Tuple]) -> bytearray:
    """Reads a line of tokens and outputs the desired binary

    Args:
        line (List[Tuple]): [A line of tokens]

    Returns:
        bytearray: [the representation of that line in binary]
    """
    
    addr_prefix = '$'
    litt_prefix = '#'
    dec_prefix = 'q'
    hex_prefix = 'x'
    str_prefix = '\"'
    mnemonics = {"jmp" : jmp_g, "ret" : ret_g, "swap" :swap_g, "psr" : psr_g, "ssr" : ssr_g, "psp": psp_g,
                 "ssp" :ssp_g, "not" : not_g, "sec" : sec_g, "clc" : clc_g, "nop" : nop_g, "push" : push_g, "pop" : pop_g,
                 "shiftl" : shiftl_g, "shiftr" : shiftr_g, "lea" : lea_g, "bnz" : bnz_g, "bez" : bez_g, "ben" : ben_g,
                 "call" : call_g, "and" : and_g, "xor" : xor_g, "add" : add_g, "adc" : adc_g, "subb" : subb_g, "ldr" : ldr_g,
                 "str" : str_g, "sti" : sti_g, "ldi" : ldi_g}
    directives = {".org" : org_g, "Format:Rom" : org_g, ".const" : org_g, ".byte" : byte_g,
                  ".word" : word_g, ".stringz" : stringz_g, ".alloc" : alloc_g, ".end" : end_g}
    
    statement_type = line[0][0]
    statement = line[0][1]
    args = line[1:]
    
    if statement_type == "mnemonic":
        if statement in mnemonics:
            return mnemonics[statement](args)

    elif statement_type == "directive":
        if statement in directives:
            return directives[statement](args)
