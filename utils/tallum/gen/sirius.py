# Code generator and target description for Sirius (freestanding)
import sys, re
sys.path.append("..") 
import target, ir

SIRIUS_MAP = target.VMmap(
    sp   = "s2",
    lcl  = "s3",
    arg  = "s4",
    this = "s5",
    that = "s6",
    temp = ['t0', 't1', 't2', 't3', 't4', 't5', 't6', "s11"],
    acc  = "s8",
    bcc  = "s9",
    var1 = "s1",
    static_reg = "s10",
    callstack = "s7",
)

class Sirius(target.Target):
    def __init__(self, supervisor=False):
        super().__init__("Sirius (freestanding)", 32, SIRIUS_MAP)
        self.supervisor = supervisor
    
    def map_segment(self, segment: str, index: int) -> str:
        match segment:
            case segment if segment in [ir.ARGUMENT, ir.LOCAL]:
                    s = self.map.arg if segment == ir.ARGUMENT else self.map.lcl
                    return f"-{index * 4}({s})"
            case ir.THIS:
                    return f"{index}*4({self.map.this})"
            case ir.THAT:
                    return f"{index}({self.map.that})"
            case ir.POINTER:
                pointer = self.map.this if ir.get_pointer(index, None) else self.map.that
                return f"{pointer}"
            case ir.TEMP:
                if index < 0 or index > 7:
                    ir.error("Temp segment addresses only 0 to 7, not " + str(index), None)
                    return(";! segment error: invalid temp offset " + str(index))
                
                return f"{self.map.temp[index]}"
            case ir.CONSTANT:         
                return f"{index}"
            case ir.STATIC:
                if self.supervisor:
                    if index not in self.static:
                        self.static[index] = ir.get_static(len(self.static) * 4)
                    
                    return f" {str(index) * 4}({self.map.static_reg})"
                else:
                    if index not in self.static:
                        self.static[index] = f".s{str(index)}: #d32 0"
                    
                    return f"STATIC_{ir.get_name() + '.s' + str(index)}"
                
    def push(self, segment: str, index: int, lineno: int) -> str:
        match segment:
            case segment if segment in [ir.ARGUMENT, ir.LOCAL]:
                    s = self.map.arg if segment == ir.ARGUMENT else self.map.lcl
                    return f"push {self.map.acc}, {self.map.sp}\nlw {self.map.acc}, -{index * 4}({s})"
            case ir.THIS:
                    return f"push {self.map.acc}, {self.map.sp}\nlw {self.map.acc}, {index}*4({self.map.this})"
            case ir.THAT:
                    return f"push {self.map.acc}, {self.map.sp}\nlb {self.map.acc}, {index}({self.map.that})"
            case ir.POINTER:
                pointer = self.map.this if ir.get_pointer(index, lineno) else self.map.that
                return f"push {self.map.acc}, {self.map.sp}\nmv {self.map.acc}, {pointer}"
            case ir.TEMP:
                #TODO: factorizar los checks
                if index < 0 or index > 7:
                    ir.error("Temp segment addresses only 0 to 7, not " + str(index), lineno)
                    return(";! segment error: invalid temp offset " + str(index))
                
                return f"push {self.map.acc}, {self.map.sp}\nmv {self.map.acc}, {self.map.temp[index]}"
            case ir.CONSTANT:         
                return f"push {self.map.acc}, {self.map.sp}\nli {self.map.acc}, {index}"
            case ir.STATIC:
                if self.supervisor:
                    if index not in self.static:
                        self.static[index] = ir.get_static(len(self.static) * 4)
                    
                    return f"push {self.map.acc}, {self.map.sp}\nlwd {self.map.acc}, {str(index) * 4}({self.map.static_reg})"
                else:
                    if index not in self.static:
                        self.static[index] = f".s{str(index)}: #d32 0"
                    
                    return f"push {self.map.acc}, {self.map.sp}\nllw {self.map.acc}, STATIC_{ir.get_name() + '.s' + str(index)}"

    
    def pop(self, segment: str, index: int, lineno: int) -> str:
        match segment:
            case segment if segment in [ir.ARGUMENT, ir.LOCAL]:
                    s = self.map.arg if segment == ir.ARGUMENT else self.map.lcl
                    return f"sw {self.map.acc}, -{index * 4}({s})\npop {self.map.acc}, {self.map.sp}"
            case ir.THIS:
                    return f"sw {self.map.acc}, {index}*4({self.map.this})\npop {self.map.acc}, {self.map.sp}"
            case ir.THAT:
                    return f"sb {self.map.acc}, {index}({self.map.that})\npop {self.map.acc}, {self.map.sp}"
            case ir.POINTER:
                pointer = self.map.this if ir.get_pointer(index, lineno) else self.map.that
                return f"mv {pointer}, {self.map.acc}\npop {self.map.acc}, {self.map.sp}"
            case ir.TEMP:
                if index < 0 or index > 7:
                    ir.error("Temp segment addresses only 0 to 7, not " + str(index), lineno)
                    return(";! segment error: invalid temp offset " + str(index))

                return f"mv {self.map.temp[index]}, {self.map.acc}\npop {self.map.acc}, {self.map.sp}"
            case ir.CONSTANT:
                ir.error("Constant segment cannot perform pop instruction", lineno)
                return(";! constant error: cannot pop to constant")
            case ir.STATIC:
                if self.supervisor:
                    if index not in self.static:
                        self.static[index] = ir.get_static(len(self.static) * 4)
                    
                    return f"swd {self.map.acc}, {str(index) * 4}({self.map.static_reg})\npop {self.map.acc}, {self.map.sp}"
                else:
                    if index not in self.static:
                        self.static[index] = f".s{str(index)}: #d32 0"
                    
                    return f"ssw {self.map.acc}, STATIC_{ ir.get_name() + '.s' + str(index)}, {self.map.var1}\npop {self.map.acc}, {self.map.sp}"
                
    def unary(self, op: str) -> str:
        match op:
            case ir.NOT:
                return f"not {self.map.acc}, {self.map.acc}"
            case ir.NEG:
                return f"neg {self.map.acc}, {self.map.acc}"
    
    def binary(self, op: str) -> str:
        asm = f"pop {self.map.bcc}, {self.map.sp}\n" #first we get the Second on top
        match op:
            case o if o in [ir.ADD, ir.SUB, ir.AND, ir.OR, ir.XOR, ir.SHRA, ir.SHRL, ir.SHLL]:
                asm += f"{o} {self.map.acc}, {self.map.bcc}, {self.map.acc}"
            case ir.MUL:
                asm += f"mul zero, {self.map.acc}, {self.map.bcc}, {self.map.acc}" # we get only the low byte
            case ir.DIV:
                asm += f"idiv {self.map.acc}, zero, {self.map.bcc}, {self.map.acc}" # we get only the quotient
            case ir.MOD:
                asm += f"idiv zero, {self.map.acc}, {self.map.bcc}, {self.map.acc}" # we get only the remainder
            
            #TODO: Consider using 1 as true instead of -1 (The ISA lends towards that)
            case ir.EQ:
                asm += f"sub {self.map.acc}, {self.map.bcc}, {self.map.acc}\n" # if a - b = 0, a == b, 
                asm += f"seqz {self.map.acc}, {self.map.acc}\n"       # true is represented by -1 (all ones)
                asm += f"neg {self.map.acc}, {self.map.acc}"       # true is represented by -1 (all ones)
            case ir.NEQ:
                asm += f"sub {self.map.acc}, {self.map.bcc}, {self.map.acc}\n"
                asm += f"snez {self.map.acc}, {self.map.acc}\n"       
                asm += f"neg {self.map.acc}, {self.map.acc}"       
            case ir.GT:
                asm += f"slt {self.map.acc}, {self.map.acc}, {self.map.bcc}\n" 
                asm += f"neg {self.map.acc}, {self.map.acc}"        
            case ir.LT:
                asm += f"slt {self.map.acc}, {self.map.bcc}, {self.map.acc}\n" 
                asm += f"neg {self.map.acc}, {self.map.acc}"
            case ir.GE:
                asm += f"slt {self.map.acc}, {self.map.bcc}, {self.map.acc}\n" 
                asm += f"addi {self.map.acc}, {self.map.acc}, -1"
            case ir.LE:
                asm += f"slt {self.map.acc}, {self.map.acc}, {self.map.bcc}\n" 
                asm += f"addi {self.map.acc}, {self.map.acc}, -1"          
        return asm
    
    def immediate(self, op: str, immediate: int) -> str:
        if immediate < -16_384 or immediate > 16_383:
            # Sirius immediates are 15 bits, if it's not in range, we have to revert to loading values in registers
            asm  = f"li {self.map.var1}, {immediate}"
            asm += self.binary(op)
            return asm
        
        asm = ""
        match op:
            case o if o in [ir.ADD, ir.SUB, ir.AND, ir.OR, ir.XOR, ir.SHRA, ir.SHRL, ir.SHLL]:
                asm += f"{o}i {self.map.acc}, {self.map.acc}, {immediate}"
            case ir.MUL:
                asm += f"mul {self.map.acc}, {self.map.acc}, {immediate}" # we get only the low byte
            case ir.DIV:
                asm += f"idivi {self.map.acc}, {self.map.acc}, {immediate}" # we get only the quotient
            case ir.MOD:
                asm += f"li {self.map.var1}, {immediate}" #there's no modulus immediate in sirius
                asm += f"idiv zero, {self.map.acc}, {self.map.acc}, {self.map.var1}" # we get only the remainder
            
            #TODO: Consider using 1 as true instead of -1 (The ISA lends towards that)
            case ir.EQ:
                asm += f"subi {self.map.acc}, {self.map.acc}, {immediate}\n" 
                asm += f"snez {self.map.acc}, {self.map.acc}\n"       
                asm += f"addi {self.map.acc}, {self.map.acc}, -1"
            case ir.NEQ:
                asm += f"subi {self.map.acc}, {self.map.acc}, {immediate}\n"
                asm += f"seqz {self.map.acc}, {self.map.acc}\n"       
                asm += f"addi {self.map.acc}, {self.map.acc}, -1"
            case ir.GT:
                if immediate > 16_383:
                    asm  = f"li {self.map.var1}, {immediate}"
                    asm += self.binary(op)
                    return asm
                asm += f"slti {self.map.acc}, {self.map.acc}, {immediate + 1}\n" 
                asm += f"addi {self.map.acc}, {self.map.acc}, -1"   
            case ir.LT:
                asm += f"slti {self.map.acc}, {self.map.acc}, {immediate}\n" 
                asm += f"neg {self.map.acc}, {self.map.acc}"
            case ir.GE:
                asm += f"slti {self.map.acc}, {self.map.acc}, {immediate}\n" 
                asm += f"addi {self.map.acc}, {self.map.acc}, -1"
            case ir.LE:
                if immediate > 16_383:
                    asm  = f"li {self.map.var1}, {immediate}"
                    asm += self.binary(op)
                    return asm
                asm += f"slti {self.map.acc}, {self.map.acc}, {immediate + 1}\n" 
                asm += f"neg {self.map.acc}, {self.map.acc}"
    
        return asm
    
    def dup(self, seg: str, index: int) -> str:
        pass
    
    def label(self, label: str) -> str:
        return f"{label}:"
        
    def goto(self, label: str) -> str:
        return f"j {label}"

    def if_goto(self, label: str) -> str:
        return f"mv {self.map.bcc}, {self.map.acc}\npop {self.map.acc}, {self.map.sp}\nbnez {self.map.bcc}, {label}"

    def call(self, func: str, args: int) -> str:
        # prepare a stack frame: push compiler registers to callstack and parameters to data stack    
        # Compiler registers
        asm  = f"push ra, {self.map.callstack}\n" #save return addr
        asm += f"save {self.map.sp}, {self.map.that}, {self.map.callstack}\n"
        # Parameters
        if args > 0:
            asm += f"push {self.map.acc}, {self.map.sp}\n" #push ACC to args
            asm += f"addi {self.map.arg}, {self.map.sp}, {(args - 1) * 4}\n" #relocate argument segment
        
        asm += f"subi {self.map.lcl}, {self.map.sp}, 4\n" #relocate local segment
        asm += f"call {func}\n"
        asm += f"pop ra, {self.map.callstack}"
        return asm
    
    def function(self, func: str, nlocals: int) -> str:
        # declare a function
        asm  = f"{func.replace(ir.get_name(), '', 1)}:\n"
        if nlocals == 0:
            return asm
        else:
            while nlocals > 1:
                asm += f"push zero, {self.map.sp}\n"
                nlocals -= 1
            asm += f"mv {self.map.acc}, zero"
            
        return asm
    
    def ret(self) -> str:
        asm  = f"restore {self.map.sp}, {self.map.that}, {self.map.callstack}\n"
        asm += f"ret"
        return asm

    def cstring(self, s: str) -> str:
        label = f".d{len(self.data)}"
        
        dat  = f"{label}:\n"
        dat += f"\t#d32 {len(s)}\n"
        dat += f'\t#d "{s}"\n'
        dat +=  "\t#align 32\n"
        self.data.append(dat)
        
        asm  = f"push {self.map.acc}, {self.map.sp}\n" 
        asm += f"la {self.map.acc}, DATA_{ir.get_name()}{label}\n"
        return asm 
        
    def optimize(self, asm: str) -> str:
        optimized = asm.replace(f"pop {self.map.acc}, {self.map.sp}\npush {self.map.acc}, {self.map.sp}\n", "") #remove redundancies
        return optimized
    
    def emit_static(self) -> str:
        if not self.supervisor:
            return f"STATIC_{ir.get_name()}:\n" + super().emit_static()
        else:
            return ""
    def emit_data(self) -> str:
        return f"DATA_{ir.get_name()}:\n" + super().emit_data()