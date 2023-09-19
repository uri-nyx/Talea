# Abstract interfaces for target platforms
from abc import ABC, abstractmethod
from dataclasses import dataclass
import ir

@dataclass
class VMmap:
    sp: str
    gp: str
    lcl: str
    arg: str
    this: str
    that: str
    thatb: str
    temp: any
    acc: str
    bcc: str
    var1: str
    static_reg: str
    callstack: str

class Target(ABC):
    def __init__(self, name: str, bits: int, vmmap: VMmap):
        self.name = name
        self.bits = bits
        self.word = bits // 8
        self.map: VMmap = vmmap
        self.static = {}
        self.data = []
    
    @abstractmethod
    def push(self, segment: str, index: int, lineno: int) -> str:
        pass
    
    @abstractmethod
    def pop(self, segment: str, index: int, lineno: int) -> str:
        pass
    
    @abstractmethod
    def unary(self, op: str) -> str:
        pass
    
    @abstractmethod
    def binary(self, op: str) -> str:
        pass
    
    @abstractmethod
    def immediate(self, op: str, immediate: int) -> str:
        pass
    
    @abstractmethod
    def move(self, segd: str, dindex: int, segs: str, sindex: int) -> str:
        pass
    
    @abstractmethod
    def moveimmediate(self, segd: str, dindex: int, segs: str, sindex: int, op: str, immediate: int) -> str:
        pass
    
    @abstractmethod
    def dup(self, seg: str, index: int) -> str:
        pass
    
    @abstractmethod
    def label(self, label: str) -> str:
        pass
        
    @abstractmethod
    def goto(self, label: str) -> str:
        pass

    @abstractmethod
    def if_goto(self, label: str) -> str:
        pass
    @abstractmethod
    def call(self, func: str, args: int) -> str:
        pass
    @abstractmethod
    def function(self, func: str, nlocals: int) -> str:
        pass
    @abstractmethod
    def ret(self) -> str:
        pass
    
    @abstractmethod
    def pushref(self, type: str, segment: str, index: int, lineno) -> str:
        pass
    
    @abstractmethod
    def cstring(self, label: str, s: str) -> str:
        pass
    
    @abstractmethod
    def array(self, index: int, size: int, array: str) -> str:
        pass

    @abstractmethod
    def res(self, index: int, size: int) -> str:
        pass       

    @abstractmethod
    def pushlabel(self, label: str) -> str:
        pass      

    @abstractmethod
    def calltos(self, args: int) -> str:
        pass   

    @abstractmethod
    def pushcommon(self) -> str:
        pass  

    @abstractmethod
    def popcommon(self, temp: int) -> str:
        pass      

    @abstractmethod
    def optimize(self, asm: str) -> str:
        pass
    
    def emit_static(self) -> str:
        return "\n".join([self.static[i] for i in self.static])
    
    def emit_data(self) -> str:
        return "\n".join(self.data)
        
    def translate(self, s: str, lineno: int) -> str:
        ss = s.split()
        if len(s) == 0 or len(ss) == 0:
            return ""
        
        command = ss[0]
        match command:
            case op if op in [ir.PUSH, ir.POP]:
                err, segment, index = ir.check_push_pop(ss, lineno)
                if err:
                    return segment
                else:
                    return self.push(segment, index, lineno) + "\n" if op == ir.PUSH else self.pop(segment, index, lineno)
            case op if op in ir.PUSHREF:
                err, typ, segment, index = ir.check_pushref(ss, lineno)
                if err:
                    return segment
                else:
                    return self.pushref(typ, segment, index, lineno) + "\n"
            case op if op in ir.BINARY:
                return self.binary(op) + "\n"
            case op if op in ir.UNARY:
                return self.unary(op) + "\n"
            case ir.LABEL:
                err, lab = ir.check_branch(ss, lineno)
                if err:
                    return lab
                else: 
                    return self.label(lab)
            case ir.GOTO:
                err, lab = ir.check_branch(ss, lineno)
                if err:
                    return lab
                else: 
                    return self.goto(lab)
            case ir.IFGOTO:
                err, lab = ir.check_branch(ss, lineno)
                if err:
                    return lab
                else: 
                    return self.if_goto(lab)
            
            case ir.FUNCTION:
                err, func, var = ir.check_func(ss, lineno)
                if err:
                    return func
                else:
                    return self.function(func, var) 
            case ir.CALL:
                err, func, arg = ir.check_func(ss, lineno)
                if err:
                    return func
                else: 
                    return self.call(func, arg)
            case ir.RETURN:
                return self.ret()
            
            case ir.CSTRING:
                err, label, string = ir.check_string(s, lineno)
                if err:
                    return label
                else:
                    return self.cstring(label, string)
            case ir.ARRAY:
                err, index, size, array  = ir.check_array(s, lineno)
                if err:
                    return index
                else:
                    return self.array(index, size, array)
            case ir.RES:
                err, index, size = ir.check_res(s, lineno)
                if err:
                    return index
                else:
                    return self.res(index, size)
            case ir.PUSHLABEL:
                err, lab = ir.check_pushlabel(ss, lineno)
                if err: return lab
                else: return self.pushlabel(lab)
            case ir.CALLTOS:
                err, arg = ir.check_calltos(ss, lineno)
                if err: return arg
                else: return self.calltos(arg)
            case ir.PUSHCOMMON:
                return self.pushcommon()
            case ir.POPCOMMON:
                err, temp = ir.check_popcommon(ss, lineno)
                if err: return temp
                else: return self.popcommon(temp)
            case ir.IMMEDIATE:
                err, op, immediate = ir.check_immediate(ss, lineno)
                if err:
                    return op
                else:
                    return self.immediate(op, immediate)
                
            case _:
                ir.error(f"`{command}` is not a valid instruction", lineno)
                return f";! syntax error: invalid instrutction in line {lineno}"