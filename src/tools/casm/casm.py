#!/bin/python3
# Cross assembler for Talea
from frontend import file_to_string_list, tokenize_lines, lex, parse

def main():
    source = tokenize_lines(file_to_string_list("./test.txt"))
    syntax_tree = parse([lex(s) for s in source])
    
    for line in syntax_tree:
        print(line)
    # Check syntax; expand macros; count bytes; index labels; substitute labels & check syntax; generate code


if __name__ == "__main__":
    main()
