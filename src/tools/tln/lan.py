#!/bin/python3
# Tln, short for Taleä'ntar, Language of Taleä, is the first compiled language developed for Taleä Chapter
# This is its cross-compiler

# Tln uses prefix notation, alla Lisp, but it does not accept nested statements except in function definition. IT IS NOT FUNCTIONAL, but procedural
# It natively supports 3 types: characters/bytes (unsigned), and words of 16 bits (unsigned).
# It also supports vectors, simple structs, and pointers (as words). The only loop is while, the conditional execution statements
# are If and if/else. Functions pass arguments by reference and there is no operator precedence

from typing import List, Dict
from sys import argv

# Lexer: breaks the source file into tokens

class Token():
    """A Token object contains a valid token type and a value
        (The List[Tuple] notation should be converted to this)"""
    
    def __init__(self, typ, value):
        self.typ = typ
        self.value = value

class Source():
    def __init__(self, tokens):
        self.index = 0
        self.tokens = tokens
        

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
    keywords = ["create", "malloc", "ref", "deref", "defun", "inline"]
    
    if token == ":*": return "comment_start"
    elif token == "*:": return "comment_end"
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
        return "identifier"
    
def index_ids(src: List[Token]) -> Dict:
    env = {}
    
    for token in src:
        if token.typ == "identifier":
            env.update({"id_" + token.value : token.value})
       
    return env

def create_g(args: List[Token], env: Dict) -> str:
    if len(args) == 2:
        ident = args[0]
        amount = args[1]
        
        if amount.typ == "dec_litteral":
            return (env[ident.value] + "  .alloc  #q" + amount.value + "\n")
        elif amount.typ == "hex_litteral":
            return (env[ident.value] + "  .alloc  #x" + amount.value+ "\n")
        else: assert "Expected litteral or symbol in malloc"

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
                asm += codegen([call], env) 
            elif len(call) > 1:
                for token in call:
                    if token in fparams:
                        param_id = "param_" + fname.value + "_" + token.value
                        token = Token("function_param", param_id)
                
                asm += fcall_g(call, env) +"\n"
                
            else:
                asm += fcall_g(call, env) +"\n"
        
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