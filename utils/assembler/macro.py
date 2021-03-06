# Simple macro expander for Talea Tabula Systems Cross Assemblers
from sys import argv
from typing import Dict, List, Tuple

class Macro():
    def __init__(self, defstring: str) -> None:
        definition = defstring.split("<")
        self.args = tokenize(definition[0])
        self.argc = len(self.args)
        self.body = " ".join(definition[1].split())
        
    def expand(self, tokens: List[str]) -> str:
        assert len(tokens) == self.argc
        expanded = self.body
        
        for i in range(self.argc):
            expanded = expanded.replace("%" + self.args[i], tokens[i])
        
        expanded = expanded.replace("; ", "\n")
        
        return expanded

class Reference():
    def __init__(self, line, index, token):
        self.line = line
        self.index = index
        self.token = token
        

def tokenize(source: str) -> List[str]:
    return source.split()

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
        
        elif token == ">":
            recording = False
            env.update({definition[0] : Macro(" ".join(definition[1:-1]))})
    
    return env
    

def get_references(source: List[str], env: dict) -> List[Reference]:
    references = []
    
    for l in range(len(source)):
        line = tokenize(source[l])
        
        for i in range(len(line)):
            token = line[i]

            if token == "m": # do not index definitions
                break
            
            if token in env:
                references.append(Reference(l, i, token))
    
    return references
               
def expand_all(source: str) -> str:
    lines = [line.split() for line in source.splitlines()]
    env_macros = get_definitions(tokenize(source))
    references = get_references(source.splitlines(), env_macros)
    
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

def main(argv):

    if len(argv) < 2 or len(argv) > 3:
        print("Usage: macro <input-file> [output]")
        return -1

    builtin = "essential.S"
    infile = argv[1]
    outfile = "expanded.s"

    if len(argv) == 3:
        outfile = argv[2]

    with open(builtin, 'r') as builtin:
        source = builtin.read()

    with open(infile, 'r') as inf:
        source += inf.read()
    
    with open(outfile, 'w') as outf:
        outf.write(expand_all(source))
    
    return 0

if __name__ == "__main__":
    main(argv)
