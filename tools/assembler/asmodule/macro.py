# Simple macro expander for Talea Tabula Systems Cross Assemblers
import os
import re
from sys import argv
from typing import Dict, List, Tuple
from .config.definitions import ROOT_DIR, SOURCE_PATH_DIR

class Macro():
    def __init__(self, defstring: str) -> None:
        self.defstring = defstring
        self.definition = defstring.split("{")
        self.args = self.definition[0].split()
        self.argc = len(self.args)
        self.body = " ".join(self.definition[1].split())
        
    def expand(self, tokens: List[str]) -> str:
        assert len(tokens) == self.argc, "Macro expansion error: wrong number of arguments: it takes " + str(self.argc) + " but " + str(len(tokens)) + " were given: " + str(tokens) + " (" + self.defstring + ")"
        expanded = self.body
        
        for i in range(self.argc):
            expanded = expanded.replace("%" + self.args[i], tokens[i])
        
        expanded = expanded.replace("\\", "\n")
        
        return expanded

class Reference():
    def __init__(self, line, index, token):
        self.line = line
        self.index = index
        self.token = token
        

def tokenize(source: str) -> List[str]:
    tokens = []
    for line in source.splitlines():
        if len(line) > 0:
            if line[0].isspace():
                tokens += " "
        tokens += line.split() + ["\n"]
        
    return tokens

def tokenize_line(line: str) -> List[str]:
    tokens = []
    if line[0].isspace():
        tokens += " "
    tokens += line.split() + ["\n"]

    return tokens

def get_constants(tokens: List[str]) -> dict:
    constants = {}
    recording = 0
    name = ""
    
    for token in tokens:
        
        if recording == 1:
            name = token
            recording += 1
            continue
        
        if token == "c":
            recording += 1
            continue
        
        elif recording == 2:
            constants.update({name: token})
            recording = 0

   
    return constants

def replace_constants(tokens: List[str], constants) -> List[str]:
    for i in range(len(tokens)):
        if tokens[i] in constants:
            tokens[i] = constants[tokens[i]]
    
    return tokens
    

def get_definitions(tokens: List[str]) -> dict:
    env = {} # An environment as such name:definition
    definition = [] # This is the temporary storage of a definition
    recording = False # This is the starting mode for the state machine
    
    for token in tokens:
        if recording:
            definition.append(token)
        
        if token == "m":
            recording = True
            definition = []
            continue
        
        elif token == "}":
            recording = False
            env.update({definition[0] : Macro(" ".join(definition[1:-1]))})
    
    return env
    

def get_references(source: List[str], env: dict) -> List[Reference]:
    references = []
    
    for l in range(len(source)):
        line = tokenize_line(source[l]) if  len(source[l]) > 0 else ["\n"]
        
        for i in range(len(line)):
            token = line[i]

            if token == "m": # do not index definitions
                break
            
            if token in env:
                references.append(Reference(l, i, token))
    
    return references
               
def expand_all(source: str) -> str:
    env_constants = get_constants(tokenize(source))
    source_tokens_with_constants_replaced = replace_constants(tokenize(source), env_constants)
    env_macros = get_definitions(source_tokens_with_constants_replaced)
    
    
    replaced_str = ""
    for token in source_tokens_with_constants_replaced:
        if token != "\n":
            replaced_str += token + " "
        else:
            replaced_str += "\n"
    
    replaced_lines = replaced_str.splitlines()

    references = get_references(replaced_lines, env_macros)
    
    lines = []
    for l in range(len(replaced_lines)):
        lines.append(tokenize_line(replaced_lines[l]) if  len(replaced_lines[l]) > 0 else ["\n"])
    
    for ref in references:
        macro = env_macros[ref.token]
        expanded = ""
        
        if macro.argc != 0:
            expanded = macro.expand(lines[ref.line][ref.index + 1 : ref.index + 1 + macro.argc])
            #substitution
            lines[ref.line][ref.index] = expanded
            for i in range(macro.argc):
                lines[ref.line][ref.index + 1 + i] = ""
            
        else: 
            expanded = macro.expand("")
            lines[ref.line][ref.index] = expanded
    
    expansion = []
    for line in lines:
        expansion.append(" ".join(line))
    
    return "\n".join(expansion)

def prepend_includes(source: str) -> str:
    essential = os.path.join(ROOT_DIR, 'lib', 'essential.S')
    with open (essential, 'r') as builtin:
        included_macros = builtin.read() + "\n"

    included_code = ""
    
    for line in source.splitlines():
        if line.startswith(";include"):
            filepath = os.path.join(SOURCE_PATH_DIR, line.split()[1])
            with open(filepath, 'r') as inf:
                inf = inf.read()
                if "%ENDMACRO" in inf:
                    included_macros += inf.split("%ENDMACRO")[0] + "\n"
                    included_code += inf.split("%ENDMACRO")[1] + "\n"
                else:
                    included_code += inf + "\n"
    
    if "%ENDMACRO" in source:
        source_macros = source.split("%ENDMACRO")[0] + "\n"
        source_code = source.split("%ENDMACRO")[1] + "\n"
    else:
        source_macros = ""
        source_code = source
                
    prepended = included_macros + source_macros + "\n%ENDMACRO\n" + included_code + source_code
        
    return prepended

def remove_comments(source: str) -> str:
        new_source: str = ""
        for line in source.split("\n"):
            if line.lstrip().startswith(";"):
                continue
            elif ";" in line:
                new_source += line.split(";")[0] + "\n"
                continue
            else:
                new_source += line + "\n"
        return new_source

def expand_macros_and_remove_definitions(asm: str) -> str:
    
    source = prepend_includes(asm)
    
    #remove comments
    source = remove_comments(source)
    
    expanded = expand_all(source).split("%ENDMACRO")[1]
    return expanded

def main(argv):

    if len(argv) < 2 or len(argv) > 3:
        print("Usage: macro <input-file> [output]")
        return -1

    infile = argv[1]
    outfile = "expanded.s"

    if len(argv) == 3:
        outfile = argv[2]

    with open(infile, 'r') as inf:
        asm = inf.read()
    
    source = prepend_includes(asm)
    
    #remove comments
    source = remove_comments(source)
    
    with open(outfile, 'w') as outf:
        outf.write(expand_all(source).split("%ENDMACRO")[1])
    
    return 0

if __name__ == "__main__":
    main(argv)
