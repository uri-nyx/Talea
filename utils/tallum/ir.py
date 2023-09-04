# IR description, instructions and parser
from sys import stderr
from typing import List
import re, operator

# Segments
PUSH = "push"
POP = "pop"
ARGUMENT = "argument"
LOCAL    = "local"
STATIC   = "static"
CONSTANT = "constant"
THIS     = "this"
THAT     = "that"
POINTER  = "pointer"
TEMP     = "temp"
SEGMENTS = [ARGUMENT, LOCAL, STATIC, CONSTANT, THIS, THAT, POINTER, TEMP]

# BINARY OPERATIONS
ADD = "add"
SUB = "sub"
MUL = "mul"
DIV = "div"
MOD = "mod"
AND = "and"
OR = "or"
XOR = "xor"
SHRA = "shra"
SHRL = "shrl"
SHLL = "shll"
EQ = "eq"
NEQ = "neq"
GT = "gt"
LT = "lt"
GE = "ge"
LE = "le"
BINARY = [ADD, SUB, MUL, DIV, MOD, AND, OR, XOR, SHRA, SHRL, SHLL, EQ, NEQ, GT, LT, GE, LE]

# UNARY OPERATIONS
NOT = "not"
NEG = "neg"
UNARY = [NOT, NEG]

# CONTROL FLOW
LABEL = "label"
GOTO = "goto"
IFGOTO = "if-goto"

# FUNCTIONS
FUNCTION = "function"
CALL = "call"
RETURN = "return"

# EXTENSIONS
CSTRING = "#cstring"
IMMEDIATE = "#immediate"
MOVE = "#move"
MOVEIMMEDIATE = "#moveimmediate"
DUP = "#dup"


# Bookeeping
def error(msg: str, lineno: int) -> None:
    print("Syntax Error in line", lineno, file=stderr)
    print("\t", msg, file=stderr)

def get_pointer(index: int, lineno: int) -> bool:
    # Only valid pointers are 0 (this) and 1 (that), pop sets, push stores
    if index < 0 or index > 1:
        error("Pointer segment addresses only 0 and 1, not " + str(index), lineno)
        return(";! segment error: invalid pointer offset " + str(index))
    
    return index == 0 #true for this, false for that

LASTSTATIC = 0
def get_static(new: int) -> int:
    global LASTSTATIC
    LASTSTATIC += new
    return LASTSTATIC

CURRENTFILE = ""
def get_name() -> str:
    global CURRENTFILE
    return CURRENTFILE

def set_name(name: str) -> None:
    global CURRENTFILE
    CURRENTFILE = name

# Parser
def check_push_pop(ss: List[str], number: int) -> (bool, str, int):
    if len(ss) < 3:
        error("`push/pop` instruction takes `segment` and `index`", number)
        return (True, ";! syntax error: argument mismatch at line " + str(number), 0)
    
    segment = ss[1]
    if segment not in SEGMENTS:
        error("Segment " + segment + " not available only " 
                + str(SEGMENTS) + " are valid", number)
        return (True, ";! syntax error: invalid segment " + segment + " at line " + str(number), 0)
    
    if not re.search("^-?[0-9]+$", ss[2]):
        if ss[2][1] == "-" and ss[1] != CONSTANT:
            error("`index` argument to pop/push must be positive except in the chonstant segment", number)
            return (True, ";! syntax error: invalid index " + ss[2] + " at line " + str(number), 0)
        
        error("`index` argument to pop/push must be an integer", number)
        return (True, ";! syntax error: invalid index " + ss[2] + " at line " + str(number), 0)
        
    index = int(ss[2])
    
    return(False, segment, index)

def check_branch(ss: List[str], number: int) -> (bool, str):
    if len(ss) < 2:
        error("`branch` instructions take one `label` argument", number)
        return (True, ";! syntax error: argument mismatch at line " + str(number))
    
    return(False, ss[1])

def check_func(ss: List[str], number: int) -> (bool, str, int):
    if len(ss) < 3:
        error("`function` instructions take two argument", number)
        return (True, ";! syntax error: argument mismatch at line " + str(number))
    
    if not ss[2].isdigit():
        error("`function` instructions take one integer argument", number)
        return (True, ";! syntax error: argument mismatch at line " + str(number))
    
    return(False, ss[1], int(ss[2]))

def check_string(s: str, number: int) -> (bool, str):
    string = re.search("\"([^\"]*)\"", s)
    if string != None:
        string = string.group(1)
        return (False, string)
    else:
        error("`#cstring` takes a constant string enclosed in `\"`", number)
        return (True, ";! syntax error: no string provided to #cstring at line " + str(number))

def remove_comments(asm: str) -> str:
    asm = re.sub(";.*?(\r\n?|\n)", "", asm.strip())
    return "\n".join(filter(lambda l: l.strip() != "", asm.split("\n")))

def optimize_constant_arithmetic(asm: str) -> str:
    # Get patterns like push constant n; push constant m; add; 
    # and replace them for push constant n+m (recursively until this pattern is not found)
    
    # Constant folding for unary operations
    ops = {
        NOT: operator.inv,
        NEG: operator.neg,
    }

    unary = f"push constant (-?\d+)\n({'|'.join(UNARY)})";
    while re.search(unary, asm, re.MULTILINE) != None:
        for m in re.finditer(unary, asm, re.MULTILINE):
            n = int(m.group(1))
            op = m.group(2)
            folded = ops[op](n)
            asm = asm.replace(m.group(0), f"push constant {folded}")
        
    # Constant folding for binary operations
    ops = {
        ADD : operator.add,
        SUB : operator.sub,
        MUL : operator.mul,
        DIV : operator.floordiv,
        MOD : operator.mod,
        AND : operator.and_,
        OR : operator.or_,
        XOR : operator.xor,
        SHRA : operator.rshift,
        SHRL : lambda n, m: (n & 0xff_ff_ff_ff) >> m, # python has no shift right logical
        SHLL : operator.lshift,
        EQ : operator.eq,
        NEQ : operator.ne,
        GT : operator.gt,
        LT : operator.lt,
        GE : operator.ge,
        LE : operator.le,
    }
    
    binary = f"push constant (-?\d+)\npush constant (-?\d+)\n({'|'.join(BINARY)})";
    while re.search(binary, asm, re.MULTILINE) != None:
        for m in re.finditer(binary, asm, re.MULTILINE):
            n = int(m.group(1))
            m = int(m.group(2))
            op = m.group(3)
            folded = ops[op](n, m)
            asm = asm.replace(m.group(0), f"push constant {folded}")
        
        
    # Constant integration as inline values
    # the idea is to search for patterns like push constant n; add; and replace them for addi n
    
    const_arithmetic = f"push constant (-?\d+)\n({'|'.join(BINARY)})";
    immediate_arithmetic = r"#immediate \2 \1"
    asm = re.sub(const_arithmetic, immediate_arithmetic, asm)
    
    return asm

def optimize_transfer_immediate(asm: str) -> str:
    # Get metapatterns like push segment n; #immediate add x; pop semgment m 
    # and replace them with #moveimmediate segment m segment n add x;
    transfer = f"push (\w+) (-?\d+)\n#immediate ({'|'.join(BINARY)}) (\d+)\n pop (\w+) (-?\d+)"
    imm_transfer = r"#moveimmediate \5 \6 \1 \2 \3 \4"
    asm = re.sub(transfer, imm_transfer, asm)

def optimize_segment_transfer(asm: str) -> str:
    # Get patterns like push segment n; pop segment m; and replace them with move segment m segment n
    push_pop_transfer = f"push (\w+) (-?\d+)\npop (\w+) (-?\d+)"
    move_transfer = r"#move \3 \4 \1 \2"
    asm = re.sub(push_pop_transfer, move_transfer, asm)
    
    push_pop_copy = f"pop (\w+) (-?\d+)\push \1"
    copy_transfer = r"#dup \1 \2"
    asm = re.sub(push_pop_copy, copy_transfer, asm)
    return asm