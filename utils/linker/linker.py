# Simple linker to be used with the customasm assembler
# It provides the following functionality:
# - declaring and resolving global symbols with `#global` and `#extern`
# - relocating sections (`#text`, `#data`, `#bss`, and, `#rodata`)
# - implicit declaration section until any other is reached
# - allocating specific addresses for the sections and defining global symbols in TOML
# - able to extract symbols from a symbol file (corresponding to an object file) 
#   and link them.
# - the final address computations are left to the assembler 
#! In hindshight including a file defining the global symbols, a symbol file of other object 
#! files and defining banks achieves the exact job of this linker

from typing import List
from sys import argv
import toml

def collect(source: List[str], start: List[str], stop: List[str], implicit=False) -> List[str]:
    """collect returns a list of the tokens in source between start and stop"""
    collected: List[str] = []

    inside = implicit
    for token in source:
        if token in start:
            inside = True
        elif token in stop:
            inside = False
        else:
            if inside:
                text.append(token)

    return collected

def decl(source: List[str]) -> List[str]:
    return collect(source, [""], ["#text", "#data", "#rodata", "#bss"], implicit=True)

def text(source: List[str]) -> List[str]:
    return collect(source, ["#text"], ["#data", "#rodata", "#bss"])

def data(source: List[str]) -> List[str]:
    return collect(source, ["#data"], ["#text", "#rodata", "#bss"])

def rodata(source: List[str]) -> List[str]:
    return collect(source, ["#rodata"], ["#data", "#text", "#bss"])

def bss(source: List[str]) -> List[str]:
    return collect(source, ["#bss"], ["#data", "#text", "#rodata"])

def parse_linker_script(file) -> List[str]:
    """ Linker scripts are written in TOML, 
    and contains a [global] section to define symbols, 
    and a section for each section to define the order, starting and end addresses"""

    preamble = []
    script = toml.load(file)
    
    size = 0
    
    if "text" in script:
        start = script["text"]["start"]
        end = script["text"]["end"]
        bank = f"#bankdef text {{\n\t#addr {start}\n\t#addr_end {end}\n\t#outp 8 * {size}\n}}"
        size += int(end) - int(start)
        preamble.append(bank)
    
    if "data" in script:
        start = script["data"]["start"]
        end = script["data"]["end"]
        bank = f"#bankdef data {{\n\t#addr {start}\n\t#addr_end {end}\n\t#outp 8 * {size}\n}}"
        size += int(end) - int(start)
        preamble.append(bank)
    
    if "rodata" in script:
        start = script["rodata"]["start"]
        end = script["rodata"]["end"]
        bank = f"#bankdef rodata {{\n\t#addr {start}\n\t#addr_end {end}\n\t#outp 8 * {size}\n}}"
        size += int(end) - int(start)
        preamble.append(bank)
    
    if "bss" in script:
        start = script["bss"]["start"]
        end = script["bss"]["end"]
        bank = f"#bankdef bss {{\n\t#addr {start}\n\t#addr_end {end}\n\t#outp 8 * {size}\n}}"
        size += int(end) - int(start)
        preamble.append(bank)
        
        
    if "global" in script:
        preamble.append("\n; Global Symbols")
        for key in script["global"]:
            preamble.append(key + " = " + str(script["global"][key]))
    
    return preamble
    
def link(source: List[str], script) -> List[str]:
    """ First we link the declarations at the start in the file
        Then we link the global symbols and define the sections
        And last, we link each section statically
    """
    linked = []
    
    linked.append(decl(source))
    linked.append(parse_linker_script(script))
    linked.append("#text")
    linked.append(text(source))
    linked.append("#data")
    linked.append(data(source))
    linked.append("#rodata")
    linked.append(rodata(source))
    
    return linked

def main():
    l = len(argv)
    if (l < 2) or (l < 3 and argv[1] == "generate") or (l < 4 and argv[1] == "link"):
        print("Usage:", argv[0], "[command] [linker script] [asm files]")
        print("Available commands are:")
        print("+ `link` to completely parse the files into a new one")
        print("+ `generate` to generate descriptions for the sections based")
        print("             on the script and defer linking to the assembler")
        exit(1)
    
    if argv[1] == "link":
        
        script = argv[2]
        asm_files = argv[3:]
        
        asm = []
        for file in asm_files:
            with open(file, "r") as file:
                asm.append(file.read().split(" "))
        
        with open(script, "r") as s:
            linked = link(asm, s)
        
        with open("linked.s", "w") as l:
            l.write(" ".join(linked))
            
    elif argv[1] == "generate":
        script = argv[2]
        asm = []
        
        with open(script, "r") as s:
            tables = parse_linker_script(s)
        
        with open("tables.s", "w") as l:
            l.write("\n".join(tables))

if __name__ == "__main__":
    main()