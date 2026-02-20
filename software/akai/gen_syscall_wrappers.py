import sys
import shlex
from typing import List, Tuple

def parse_return(r: str) -> str:
    if "old bkr" in r: return "void*" 
    elif "ERROR" in r: return "int"
    elif "none" in r : return "void"

def parse_args(a: str) -> List[Tuple[str, int]]:
    if a == "none": return [("void", None)]
    return [(f"{T[0:-3].strip()} {name.strip().replace(' ', '_')}", int(T[-2:])) for T, name in [arg.split(":") for arg in a.split(",")]]

    

def gen(l:str, n: int) -> Tuple[str, str]:
        
    # Strip 'X(' and trailing ')' and split by comma
    content = l[2:l.rfind(')')]
    parts = [p.strip()[0:-1] if p[-1] == ',' else p.strip() for p in shlex.split(content)]
    
    if ("NOT IMPLEMENTED" in parts or "MODIFY API" in parts):
        return ("", "")
    
    enum_name = parts[0]
    function_name = parts[1]
    arguments = parse_args(parts[2])
    return_type = parse_return(parts[3])
    desc = parts[4]
    
    c_declaration = f"{return_type} {function_name}({','.join([a for a, _ in arguments])});"
    asm_stub = f"""
.text
.globl {function_name}
{function_name}:
{chr(10).join([f'    mv x{reg}, x{reg - 1} # {a}' if reg != None else "" for  a, reg in reversed(arguments)])}
    li x12, {n}
    syscall x0, 0x40 
    ret
    """
    
    return (c_declaration, asm_stub)
    
    
def main(argv):
    
    if (len(argv) != 3):
        print(f"Usage: {argv[0]} <c-header> <asm-source>")
        exit(1)
        
    header = argv[1]
    asm_file = argv[2]
    
    syscall_no = 0
    multiline = ""
    wrappers = []
    
    for line in sys.stdin:
        line = line.strip()
        
        if not line.startswith("X("):
            if multiline == "":
                continue
            if '\\' in line and not '")' in line:
                multiline += line.replace('\\', ' ').strip() + ' '
                continue
            elif '")' in line:
                wrappers.append(gen(multiline + line, syscall_no))
                multiline = ""
                syscall_no += 1
                continue
        elif '\\' in line and not '")' in line:
            multiline = line.replace('\\', ' ').strip() + ' '
            continue
        else:
            wrappers.append(gen(line, syscall_no))
            syscall_no += 1
            continue
        
    
    with open(header, "a") as h:
        h.write("\n".join([decl for decl, _ in wrappers]))
        
    with open(asm_file, "a") as asm:
        asm.writelines([wrap for _, wrap in wrappers])
            
        
        
if __name__ == "__main__":
    main(sys.argv)