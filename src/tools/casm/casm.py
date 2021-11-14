#!/bin/python3
# Cross assembler for Talea
from sys import argv
from casm_frontend import file_to_string_list, tokenize_lines, lex, parse
from casm_backend import codegen


def main(argv):
    
    if (len(argv) < 2) or (len(argv) > 4):
        print("Usage: " + __name__ + " <source-file> [output = a.out] [ram = default/rom]")
        
    file = argv[1]
    
    if len(argv) == 3:
        out = argv[2]
        mode = "ram"
    elif len(argv) == 4:
        out = argv[2]
        mode = argv[3]
    else:
        out = "a.out"
        mode = "ram"
        
    source = tokenize_lines(file_to_string_list(file))
    syntax_tree = parse([lex(s) for s in source])
    temp_2d_array = list(filter(lambda s: False if s == None else True, [codegen(line) for line in syntax_tree]))
    raw_bytes = []
    for s in temp_2d_array:
        if not isinstance(s, list):
            raw_bytes.extend([s])
        else:
            raw_bytes.extend(s)
    
    # TODO: expand macros;
    with open(out, "w+b") as fout:
        if mode == "rom":
            bin_bytes = bytearray(0x8ff) # 3.25kb of ROM
        else:
            bin_bytes = bytearray()
            
        bin_bytes[:len(raw_bytes)] = raw_bytes
        fout.write(bin_bytes)
        fout.close()


if __name__ == "__main__":
    main(argv)
