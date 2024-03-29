"""TALLUM: Taleä Language Layer for Universal Machines.

Usage:
  tallum.py (sft) -o <file> [-Os] [--std=<(n2t|tal)>] <file>...
  tallum.py (-h | --help)
  tallum.py (-v | --version)

Options:
  -h --help         Show this screen.
  -v --version      Show version.
  -o <file>         Output file.
  -O                Allow optimization.
  -s                Compile for Supervisor Mode.
  --std=<(n2t|tal)> Standard of the language to use (n2t strict from Nand to Tetris, tal includes extensions) [default: tal].

"""
from typing import List
from docopt import docopt
from sys import stderr
import target, ir, gen.sirius

VERSION = "TALLUM v0.5, 23/09/2023"

def add_extensions(source: List[str]) -> List[str]:
    # Add and replace some extensions not present in the canonical implementation
    new = [f"{ir.MUL}\n" if l == "call Math.multiply 2\n" else l for l in source]
    new = [f"{ir.DIV}\n" if l == "call Math.divide 2\n" else l for l in new]
    new = [f"{ir.MOD}\n" if l == "call Math.mod 2\n" else l for l in new]
    new = [f"{ir.XOR}\n" if l == "call Math.xor 2\n" else l for l in new]
    new = [f"{ir.SHRA}\n" if l == "call Math.shra 2\n" else l for l in new]
    new = [f"{ir.SHRL}\n" if l == "call Math.shrl 2\n" else l for l in new]
    new = [f"{ir.SHLL}\n" if l == "call Math.shll 2\n" else l for l in new]
    new = [f"{ir.NEQ}\n" if l == "call Math.neq 2\n" else l for l in new]
    new = [f"{ir.GE}\n" if l == "call Math.ge 2\n" else l for l in new]
    new = [f"{ir.LE}\n" if l == "call Math.le 2\n" else l for l in new]
    return new

def generate(target: target.Target, filename: str, standard: str, source: List[str]) -> (str, str, str):
    if standard == "tal":
        source = add_extensions(source)
    
    ir.set_name(filename)
    asm = f"{filename}:\n"
    for number, line in enumerate(source):
        line = line.replace("//", ";")
        asm += f"; {line}\n"
        asm += target.translate(line, number) + "\n"
    
    #filter newlines
    asm = "\n".join(filter(lambda l: l.strip() != "", asm.split("\n")))
    data = target.emit_data()
    static = target.emit_static()
    return (asm, data, static)

if __name__ == "__main__":
    arguments = docopt(__doc__, version=VERSION)

    cpu = None
    text = []
    data = []
    bss  = []
    
    standard = "tal"
    match arguments["--std"]:
        case None:
            print("Warning: no standard provided, default is `tal`", file=stderr)
        case "tal":
            pass
        case "n2t":
            standard = "n2t"
        case _:
            print(__doc__)
            print("Error: no such standard", file=stderr)
            exit(1)
    
    if arguments["sft"]:
        # Sirius freestanding
        cpu = gen.sirius.Sirius(arguments["-s"])
        text.append("#bank text")
        data.append("#bank data")
        bss.append("#bank bss")
        
    for fname in arguments["<file>"]:
        ir.FNAME = fname
        name = fname.split("/")[-1].split(".")[0]
        with open(fname, "r") as f:
            if arguments["-O"]:
                a = f.read()
                a = "".join(add_extensions(a.splitlines(keepends=True)))
                a = ir.optimize_constant_arithmetic(a)
                #with open(f"{name}_ir.vm", "w+") as fi:
                #    fi.write(a)
                asm, dat, static = generate(cpu, name, standard, a.splitlines(keepends=True))
            else:
                asm, dat, static = generate(cpu, name, standard, f.readlines())
                
            text.append(asm)
            data.append(dat)
            bss.append(static)
    
    text = "\n".join(text)
    data = "\n".join(data)
    bss = "\n".join(bss)
    if arguments["-O"]:
        text = ir.remove_comments(text)
        text = cpu.optimize(text)
    
    with open(arguments["-o"], "w") as f:
        f.write(text + "\n")
        f.write(data + "\n")
        f.write(bss)

    
    
        
        
    
    
    