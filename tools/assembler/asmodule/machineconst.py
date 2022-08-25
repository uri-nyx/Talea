# Constant opcodes for the assembler

Opcodes = {
    "lui" : 0b0110111,
    "auipc" : 0b0010111,
    "jal" : 0b1101111,
    "jalr" : 0b1100111,
    "jump" : 0b1101011,
    
    "branch": 0b1100011,
    "load": 0b0000011,
    "store": 0b0100011,
    "mathi": 0b0010011,
    "mathr": 0b0110011,
    
    "system": 0b1110011,
    "supervisor": 0b1110001
}

InstructionTypes = {
    "lui": ["lui"],
    "auipc": ["auipc"],
    "jal": ["jal"],
    "jalr": ["jalr"],
    "longjump": ["longjump"],
    
    "branch": ["beq", "bne", "blt", "bge", "bltu", "bgeu"],
    "load": ["lb", "lh", "lbu", "lhc", "lbc", "lhbu"],
    "store": ["sb", "sh", "sbc", "shc"],
    "mathi": ["addi", "slti", "sltiu", "xori", "ori", "andi", "slli", "srli", "srai"],
    "mathr": ["add", "sub", "sll", "slt", "sltu", "xor", "srl", "sra", "or", "and"],
    
    "system": ["trap", "ssr", "gsr", "gpsr"],
    "supervisor": ["rti", "spsr"],
    
    "pseudo-label": ["la", "call", "tail", "lhg", "lbg", "shg", "sbg"]
}

Funct7Set = ["srai", "sub", "sra"]

FunctionCodes = {
    "beq": 0b000,
    "bne": 0b001,
    "blt": 0b100,
    "bge": 0b101,
    "bltu": 0b110,
    "bgeu": 0b111,

    "lb": 0b000,
    "lh": 0b001,
    "lbu": 0b100,

    "lbc": 0b101,
    "lhc": 0b010,
    "lbuc": 0b110,

    "sb": 0b000,
    "sh": 0b001,

    "sbc": 0b011,
    "shc": 0b010,

    "addi": 0b000,
    "slti": 0b010,
    "sltiu": 0b011,
    "xori": 0b100,
    "ori": 0b110,
    "andi": 0b111,
    "slli": 0b001,
    "srli": 0b101,
    "srai": 0b101,

    "add": 0b000,
    "sub": 0b000,
    "sll": 0b001,
    "slt": 0b010,
    "sltu": 0b011,
    "xor": 0b100,
    "srl": 0b101,
    "sra": 0b101,
    "or": 0b110,
    "and": 0b111,

    "trap": 0b000,
    "ssr": 0b001,
    "gsr": 0b010,
    "gpsr": 0b011,

    "rti": 0b000, 
    "spsr": 0b001
}

Registers = {
    "x0":0,

	"x1":1,
    "x2":2,
    "x3":3,
    "x4":4,
    "x5":5,
    "x6":6,
    "x7":7,
    "x8":8,
    "x9":9,

	"x10":10,
    "x11":11,
    "x12":12,
    "x13":13,
    "x14":14,
    "x15":15,
    "x16":16,

	"x17":17,
    "x18":18,
    "x19":19,
    "x20":20,
    "x21":21,
    "x22":22,
    "x23":23,

	"x24":24,
    "x25":25,
    "x26":26,
    "x27":27,
    "x28":28,
    "x29":29,
    "x30":30,

	"x31":31,
 
    "zero":0,
    "ra":1,
    "sp":2,
    "gp":3,
    "tp":4,
    "t0":5,
    "t1":6,
    "t2":7,
    "fp":8,
    "s1":9,
    "a0":10,
    "a1":11,
    "a2":12,
    "a3":13,
    "a4":14,
    "a5":15,
    "a6":16,
    "a7":17,
    "s2":18,
    "s3":19,
    "s4":20,
    "s5":21,
    "s6":22,
    "s7":23,
    "s8":24,
    "s9":25,
    "s10":26,
    "s11":27,
    "t3":28,
    "t4":29,
    "t5":30,
    "t6":31
}