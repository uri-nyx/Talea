#!/bin/python3
# Tln, short for Taleä'ntar, Language of Taleä, is the first compiled language developed for Taleä Chapter
# This is its cross-compiler

# Tln uses prefix notation, alla Lisp, but it does not accept nested statements except in function definition. IT IS NOT FUNCTIONAL, but procedural
# It natively supports 3 types: characters/bytes (unsigned), and words of 16 bits (unsigned).
# It also supports vectors, simple structs, and pointers (as words). The only loop is while, the conditional execution statements
# are If and if/else. Functions pass arguments by reference and there is no operator precedence

from os import write
from typing import List, Dict
from sys import argv

# Lexer: breaks the source file into tokens

class Token():
    """A Token object contains a valid token type and a value
        (The List[Tuple] notation should be converted to this)"""
    
    def __init__(self, typ, value):
        self.typ = typ
        self.value = value
        

def lex(source_file_path: str) -> List[Token]:
    """Takes a source file and tokenizes it

    Args:
        source_file_path (str): The path to the source file 

    Returns:
        List[Token]: A list of the tokens the file contains
    """
    
    tokens = []
    
    with open(source_file_path, "r") as source:
        s = source.readlines()
    
    # s contains the lines of the source file
    # split lines by spaces (TODO: aisle comment, parens, and quotes)
    s = " ".join(s) 
    s = s.replace("\n", "")
    s = s.replace("(", " ( ").replace(")", " ) ")
    s = s.replace(":*", " :* ").replace("*:", " *: ")
    s = s.replace(".\"", " .\" ").replace("\".", " \". ")
    s = s.split(" ")
    s = list(filter(lambda x: False if x == "" else True, s))
    for t in s:
        tokens.append(Token(identify_tk(t), t))
    
    return tokens

def identify_tk(token: str) -> str:
    """Identifies a token's type by deduction

    Args:
        token (str): The token's value

    Returns:
        str: a string indicating the token type
    """
    keywords = ["let", "malloc", "ref", "deref", "defun", "inline"]
    
    if token == ":*": return "comment_start"
    elif token == "*:": return "comment_end"
    elif token == "(": return "Lparen"
    elif token == ")": return "Rparen"
    elif token in keywords: return "keyword"
    elif token == ".\"": return "string_litteral_start"
    elif token == "\".": return "string_litteral_end"
    elif token.isdecimal(): return "dec_litteral"
    elif token[0:2] == "0x":
        try:
            int(token, 16)
            return "hex_litteral"
        except ValueError as e:
            raise(e)
    else:
        return "symbol_id"

# To parse this language, the first stage is simply make nested lists of tokens
def parse(expression: List[Token]) -> List[Token]:
    ast = ""
    comment = False
    string = False
    s = ""
    
    for e in expression:
        if e.typ == "comment_end":
            comment = False
            continue
            
        if not comment:
            if not string:
                if e.typ == "Lparen":
                    ast += " [ "
                elif e.typ == "Rparen":
                    ast += " ], "
                elif e.typ == "comment_start":
                    comment = True
                elif e.typ == "string_litteral_start":
                    string = True
                else:
                    ast += "Token('" + e.typ + "', '" + e.value + "'), "
            else:
                if e.typ == "string_litteral_end":
                    s = s.rstrip()
                    ast += "Token('string_litteral', '" + s + "'), "
                    s = ""
                    string = False
                else:
                    s += e.value + " "
            
    return eval(ast)

def index_ids(ast: List[Token]) -> Dict:
    env = {}
    
    for statement in ast:
        if len(statement) == 2:
            key = statement[0]
            ident = statement[1]
            if key.value == "let":
                env.update({ident.value : ("id_" + ident.value)})
    
    return env

def malloc_g(args: List[Token], env: Dict) -> str:
    if len(args) == 2:
        ident = args[0]
        amount = args[1]
        
        if amount.typ == "dec_litteral":
            return (env[ident.value] + "  .alloc  #q" + amount.value + "\n")
        elif amount.typ == "hex_litteral":
            return (env[ident.value] + "  .alloc  #x" + amount.value+ "\n")
        else: assert "Expected litteral or symbol in malloc"

def ref_g(args: List, env: Dict) -> str:
    if len(args) == 2:
        ident = args[0]
        pointer = args[1]
        
        if pointer.typ == "symbol_id":
            return (env[pointer.value] + "  .word  " + env[ident.value] + "\n")
        else: assert "Expected symbol in ref"

def deref_g(args: List, env: Dict) -> str:
    if len(args) == 2:
        pointer = args[0]
        ident = args[1]
        
        if ident.typ == "symbol_id":
            return ("   ldr r4 " + env[pointer.value] + "\n"
                    + " str r4 " + env[ident.value] + "\n")
        else: assert "Expected symbol in deref"
    
def inline_g(args: List, env: Dict) -> str:
    if len(args) == 1:
        if args[0].typ == "string_litteral":
            return args[0].value
    
def defun_g(args: List, env: Dict) -> str:
    if len(args) == 3:
        fname = args[0]

        fparams = args[1]
        fbody = args[2]

        
        #save return address, which is on top of the stack
        ret_addrL = fname.value + "_return_addrL"
        ret_addrH = fname.value + "_return_addrH"
        asm = (env[fname.value] + "    pop r4\n    pop r3\n" +
               ret_addrL + " .byte #q0\n" +
               ret_addrH + " .byte #q0\n" +
               "    str r3 " + ret_addrL + "\n"
               "    str r4 " + ret_addrH + "\n")
                              
        for param in fparams:
            param_id = "param_" + fname.value + "_" + param.value
            asm += (param_id + 
                    "   .byte #q0\n" +
                    "   pop r4\n" + 
                    "   str r4 " + param_id + "\n")
        
        for call in fbody:
            callee = call[0]
            if callee.typ == "keyword":
                asm += codegen([call]) 
            elif len(call) > 1:
                for token in call:
                    if token in fparams:
                        param_id = "param_" + fname.value + "_" + token.value
                        token = Token("function_param", param_id)
                
                asm += "    ; calling " + callee.value + " with args "
                asm += fcall_g(call, env)
                
            else:
                asm += "    ; calling " + callee.value +"\n"
                asm += fcall_g(call, env)
        
        asm += ("    ldr r4  " + ret_addrH + "\n" +
                "   push r4\n" +
                "    ldr r4  " + ret_addrL + "\n" +
                "   push r4\n" + "  ret\n")
        
        
        return asm     
        
    else:
        assert "defun takes 3 arguments (defun fname (fparams) (fbody))"
    
def fcall_g(call: List, env: Dict) -> str:
    asm = ""
    fname = call[0]
            
    if len(call) > 1:
        args = call[1:]
        
        for arg in reversed(args):
            val = argument_type_handler(arg, env)
            asm += (" ldr r4 " + val + "\n"+
                    " push r4\n")
    
    asm += "    call " + env[fname.value]
    return asm

def enter_g(args: List, env: Dict) -> str:
    return "enter   nop\n"
    
def argument_type_handler(arg: Token, env: Dict) -> str:
    if arg.typ == "hex_litteral":
        return "#x" + arg.value
    elif arg.typ == "dec_litteral":
        return "#q" + arg.value
    elif arg.typ == "symbol_id":
        return env[arg]
    else:
        assert "Tln only supports symbols or byte litterals as function arguments"

def codegen(ast: List[Token]) -> str:
    keywords = {"malloc" : malloc_g, "ref" : ref_g, 
                "deref" : deref_g, "defun" : defun_g, 
                "inline" : inline_g, "enter" : enter_g}
    
    Env = index_ids(ast)
    
    asm = ""
    
    for statement in ast:
        if len(statement) > 0:
            head = statement[0]
            
            if head.value in keywords:
                asm += keywords[head.value](statement[1:], Env)
            elif head.value == "let":
                pass
            else:
                asm += fcall_g(statement, Env)

    return asm

def split_data_code(src: str) -> str:
    src = src.splitlines()
    data = []
    code = []
    for line in src:
        if ".byte" in line:
            data.append(line)
        elif ".word" in line:
            data.append(line)
        elif ".alloc" in line:
            data.append(line)
        else:
            code.append(line)

    return data, code
    

def compile(path: str) -> str:
    asm = codegen(parse(lex(path)))
    
    data, code = split_data_code(asm)
    
    code = ["   .org #x400", " call enter" , " call end"] + code
    
    code.append("end    .end")
    
    return "\n".join(code + data)

def main(argv):
    if len(argv) != 3:
        print("Usage " + argv[0] + " <source-file> [-t|<destination-file]")
        exit(1)
    
    src = argv[1]
    mode = argv[2]
    
    if mode == "-t":
        print(compile(src))
    else:
        with open(mode, "w+") as out:
            out.write(compile(src))
    
if __name__ == "__main__":
    main(argv)

    

                  
                
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
    
