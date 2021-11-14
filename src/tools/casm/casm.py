#!/bin/python3
# Cross assembler for Talea
from os import error
from typing import List, Tuple

def file_to_string_list(path: str) -> List[str]:
    """Generate a string list with contents of file at path

    Args:
        path (str): path to the input fileº

    Returns:
        List[str]: The lines of the input file
    """
    
    try:
        with open(path, "r") as file:
            return file.readlines()
    except error:
        assert error        

def tokenize_lines(xs: List[str]) -> List[Tuple]:
    """Generate a list of tuples which contain the words and its position

    Args:
        xs (List[str]): a List of strings to tokenize

    Returns:
        List[Tuple]: a list containing a tuple for each word (word, row, col)
    """
    words = []
    
    row = 1
    col = 1
    for line in xs:
        
        if line[0].isspace():
            col += 1
            
        for word in line.split():
            if word != '\n':
                words.append((word, row, col))
                col += 1
        col = 1
        row += 1
                
    return words

def lex(word: Tuple) -> Tuple:
    """Generate a Token from a word tuple

    Args:
        word (Tuple): a tuple containing a word and its position in file (word, row, col)

    Returns:
        Tuple: a tuple containing a token (token type, token, position (row, col))
    """
    w = word[0].lower()
    row = word[1]
    col = word[2]
    
    addr_prefix = '$'
    litt_prefix = '#'
    dec_prefix = 'q'
    hex_prefix = 'x'
    comment_prefix = ';'
    registers = ["acc", "bcc", "r1", "r2", "r3", "r4", "hx", "lx"]
    mnemonics = ["and"]
    directives = [".org"]
    macros = ["mov"]
    
    if w[0] == comment_prefix: return ("comment", "", (row, col))
    elif col == 1: return ("label_definition", w, (row, col))
    elif (col == 2) and (w in mnemonics): return ("mnemonic", w, (row, col))
    elif (col == 2) and (w in directives): return ("directive", w, (row, col))
    elif (col == 2) and (w in macros): return ("macro", w, (row, col))
    elif (col > 2) and (w in registers): return ("register", w, (row, col))
    elif (col > 2) and (w[0] == addr_prefix): return ("address", int(w[1:], 16), (row, col))
    elif (col > 2) and (w[0:2] == litt_prefix + dec_prefix): return ("dec_litteral", int(w[2:], 10), (row, col))
    elif (col > 2) and (w[0:2] == litt_prefix + hex_prefix): return ("hex_litteral", int(w[2:], 16), (row, col))
    elif col > 2: return ("label_refernce", w, (row, col))
    else: return ("unrecognized", w, row, col)
    #raise Exception("Unrecognized token: '" + w + "' at " + str(row) + ":" + str(col))
    
     
    

def main():
    xs = tokenize_lines(file_to_string_list("./test.txt"))
    for s in xs:
        print(lex(s))

if __name__ == "__main__":
    main()
