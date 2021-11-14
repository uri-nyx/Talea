from os import error
from typing import List, Tuple, Dict

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
    
    const_prefix = '*'
    addr_prefix = '$'
    litt_prefix = '#'
    dec_prefix = 'q'
    hex_prefix = 'x'
    str_prefix = '\"'
    comment_prefix = ';'
    registers = ["acc", "bcc", "r1", "r2", "r3", "r4", "hx", "lx"]
    mnemonics = ["and", "jmp", "ret", "swap", "psr", "ssr", "psp",
                 "ssp", "not", "sec", "clc", "nop", "push", "pop",
                 "shiftl", "shiftr", "lea", "bnz", "bez", "ben",
                 "call", "and", "xor", "add", "adc", "subb", "ldr",
                 "str", "sti", "ldi"]
    directives = [".org", "Format:Rom", ".const", ".byte", ".word", ".stringz", ".alloc", ".end"]
    macros = ["mov"]
    
    if w[0] == comment_prefix: return ("comment", "", (row, col))
    elif col == 1: return ("label_definition", w, (row, col))
    elif (col == 2) and (w in mnemonics): return ("mnemonic", w, (row, col))
    elif (col == 2) and (w in directives): return ("directive", w, (row, col))
    elif (col == 2) and (w in macros): return ("macro", w, (row, col))
    elif (col > 2) and (w in registers): return ("register", w, (row, col))
    elif (col > 2) and (w[0] == addr_prefix) and (len(w) == 3): return ("zp", int(w[1:], 16), (row, col))
    elif (col > 2) and (w[0] == addr_prefix) and (len(w) == 5): return ("address", int(w[1:], 16), (row, col))
    elif (col > 2) and (w[0] == const_prefix): return ("constant", w, (row, col))
    elif (col > 2) and (w[0] == str_prefix): return ("string_litteral", w[1:], (row, col))
    elif (col > 2) and (w[0:2] == litt_prefix + dec_prefix): return ("dec_litteral", int(w[2:], 10), (row, col))
    elif (col > 2) and (w[0:2] == litt_prefix + hex_prefix): return ("hex_litteral", int(w[2:], 16), (row, col))
    elif col > 2: return ("label_reference", w, (row, col))
    else: return ("unrecognized", w, (row, col))
    #raise Exception("Unrecognized token: '" + w + "' at " + str(row) + ":" + str(col))

def list_tokens_by_row(tokens: List[Tuple]) -> List:
    """Generate a list of tokens for each row

    Args:
        tokens (List[Tuple]): a list of all the tokens

    Returns:
        List: a matrix of all the rows
    """
    matrix = []
    r = []
    row = 1
    row_count, *_ =  reversed(tokens)
    
    while row_count[2][0] >= row:
        r = [token for token in tokens if token[2][0] == row]
        matrix.append(r)
        row += 1
        
    return matrix

def macro_expand(): pass #The last to implement

def index_constants(line: List[Tuple]) -> Dict:
    """Indexes the definition of constants and returns an enviroment

    Args:
        line (List[Tuple]): a line of source code

    Returns:
        Dict: an env of constants {name : token} (both strings)
    """
    
    env = {}
    directive = ()
    for token in line:
        if token[1] == ".const":
            directive = token
    
    args = []
    for token in line:
        if token [2][1] > 2:
            args.append(token)
    
    
    if (len(args) == 2) and (args[0][0] == "constant"):
        env.update({args[0][1] : args[1]})
    else:
        assert "Wrong number of arguments or formatting in constant directive"

    return env

def index_labels(line: List[Tuple]) -> Dict:
    """Indexes the definition of labels and returns an enviroment

    Args:
        line (List[Tuple]): a line of source code

    Returns:
        Dict: an env of constants {name : line}
    """
    
    env = {}

    for token in line:
        if token[0] == "label_definition":
            env.update({token[1] : token[2][0] }) #row of deinition
    
    return env

def expand_constants(env: Dict, line: List[Tuple]) -> List[Tuple]:
    """Expand the constants founded in a line to its value

    Args:
        env (Dict): An environment of constants
        line (List[Tuple]): A line of source code

    Returns:
        List[Tuple]: A line of source code with the constants found expanded
    """
    expanded_line = []
    for token in line:
        if token[0] == "constant":
            expanded_line.append(env[token[1]])
        else:
            expanded_line.append(token)
            
    return expanded_line
    
        
        

def check_instruction_syntax_and_count_bytes(line: List[Tuple]) -> Tuple:
    """Checks the syntax for the different instructions. Comments shod've already be ommited

    Args:
        line (List[Tuple]): a line of code

    Returns:
        Tuple: a tuple, containing the bytes of that line, and wheter the syntax is correct or incorrect and an error message
    """
    correct = True
    incorrect = False
    
    instrN = ["jmp", "ret", "swap", "psr", "ssr", "psp", "ssp", "not", "sec", "clc", "nop"]
    instrR = ["push", "pop", "shiftl", "shiftr"]
    intrA_ZP = ["lea"]
    instrA_ZP_HL = ["bnz", "bez", "ben", "call"]
    instrR_A_L_ZP = ["and", "xor", "add", "adc", "subb"]
    instrR_AZPHL = ["str", "sti", "ldi"]
    instrR_RALZP = ["ldr"]
    
    token_type = ()
    for token in line:
        if token[2][1] == 2:
            token_type = token
    
    args = []
    for token in line:
        if token[0] == "comment":
            break
        elif token [2][1] > 2:
            args.append(token)
              
    if token_type[0] == "mnemonic":
        m = token_type[1]
        if m in instrN:
            if args == []: return (1, correct, "No error")
            
        if len(args) == 1:
            if m in instrR:
                if (args[0][0] == "register") : return (1, correct, "No error")
            if m in intrA_ZP:
                if (args[0][0] == "address") or (args[0][0] == "label_reference")  : return (3, correct, "No error")
                if (args[0][0] == "zp") : return (2, correct, "No error")        
            if m in instrR_A_L_ZP:
                if (args[0][0] == "register") : return (1, correct, "No error")
                elif (args[0][0] == "address") or (args[0][0] == "label_reference")  : return (3, correct, "No error")
                elif (args[0][0] == "zp") : return (2, correct, "No error")
                elif (args[0][0] == "dec_litteral") or (args[0][0] == "hex_litteral"): return (2, correct, "No error")
        
        if m in instrA_ZP_HL:
                if args == []: return (1, correct, "No error")
                elif ((args[0][0] == "address") or (args[0][0] == "label_reference")) and (len(args) == 1): return (3, correct, "No error")
                elif (args[0][0] == "zp") and (len(args) == 1): return (2, correct, "No error")
                
        if m in instrR_AZPHL:
            if (len(args) >= 1) and (args[0][0] == "register"):
                if len(args) == 2:
                    if (args[1][0] == "address") or (args[1][0] == "label_reference") : return (3, correct, "No error")
                    if args[1][0] == "zp": return (2, correct, "No error")
                else: return (1, correct, "No error") #HL
                
        if m in instrR_RALZP:
            if (len(args) >= 1) and (args[0][0] == "register"):
                if len(args) == 2:
                    if args[1][0] == "register": return (3, correct, "No error")
                    elif (args[1][0] == "address") or (args[1][0] == "label_reference") : return (3, correct, "No error")
                    elif args[1][0] == "zp": return (2, correct, "No error")
                    elif (args[1][0] == "dec_litteral") or (args[1][0] == "hex_litteral"): return (3, correct, "No error")
                    errmsg = "Error: wrong number of arguments for LDR in line:" + str(token_type[2][0])
                else: return (-1, incorrect, errmsg)
        
        errmsg = "Error wrong number of arguments for " + m.upper() + " in line:" + str(token_type[2][0])
        return (-1, incorrect, errmsg)
                                    
                
                 
        
    elif token_type[0] == "directive": 
        d = token_type[1]
        if d == ".org":
            if (len(args) == 1) and ((args[0][0] == "dec_litteral") or (args[0][0] == "hex_litteral")):
                return (args[0][1], correct, "No error")
            else:
                errmsg = "Error in .ORG directive: wrong number of arguments, in line:" + str(token_type[2][0])
                return (-1, incorrect, errmsg)
        elif d == "Format:Rom": return (0, correct, "No error")
        elif d == ".const": return (0, correct, "No error") #has already been handled
        elif d == ".byte":
            if (len(args) == 1) and ((args[0][0] == "dec_litteral") or (args[0][0] == "hex_litteral")):
                return (1, correct, "No error")
            else:
                errmsg = "Error in .BYTE directive: wrong number of arguments, in line:" + str(token_type[2][0])
                return (-1, incorrect, errmsg)
            
        elif d == ".word": 
            if (len(args) == 1) and ((args[0][0] == "dec_litteral") or (args[0][0] == "hex_litteral")):
                return (2, correct, "No error")
            else:
                errmsg = "Error in .WORD directive: wrong number of arguments, in line:" + str(token_type[2][0])
                return (-1, incorrect, errmsg)
        
        elif d == ".stringz":
            if (len(args) > 1) and (args[0][0] == "string_litteral"):
                s = ""
                for w in args:
                    s += w[1] + " "
                    
                s = s.strip()
                
                return (len(s) + 1, correct, "No error")
            else:
                errmsg = "Error in .STRINGZ directive: wrong number of arguments, in line:" + str(token_type[2][0])
                return (-1, incorrect, errmsg)
        
        elif d == ".alloc":
            if (len(args) == 1) and ((args[0][0] == "dec_litteral") or (args[0][0] == "hex_litteral")):
                return (args[0][1], correct, "No error")
            else:
                errmsg = "Error in .ALLOC directive: wrong number of arguments, in line:" + str(token_type[2][0])
                return (-1, incorrect, errmsg)
        
        elif d == ".end": return (0, correct, "No error")
        
def expand_labels(env: Dict, bytecount: List[int], line: List[Tuple]) -> List[Tuple]:
    """Expands the labels in a line to absolute adresses

    Args:
        env (Dict[]): An environment of labels (str : line)
        bytecount (List[int]): A list with the byte size of every line in the file
        line (List[Tuple]): A line of source to expand  

    Returns:
        List[Tuple]: A new line with the expanded labels
    """
    expanded_line = []
    for token in line:
        if token[0] == "label_reference":
            try:
                l = env[token[1]]
                addr = sum(bytecount[:l])
                expanded_line.append(("address", addr, (token[2][0], token[2][1])))
            except KeyError:
                #label is not actually a label, but a comment
                pass
        else:
            expanded_line.append(token)
            
    return expanded_line
    

def parse(tokens: List[Tuple]) -> List[List[Tuple]]:
    token_matrix = list_tokens_by_row(tokens)
    
    constant_env = {}
    for d in list(filter(None, [index_constants(line) for line in token_matrix])):
        constant_env.update(d)
    
    label_env = {}
    for d in list(filter(None, [index_labels(line) for line in token_matrix])):
        label_env.update(d)
    
    source_lines = [expand_constants(constant_env, line) for line in token_matrix]
    
    #first syntax check 
    syn = [check_instruction_syntax_and_count_bytes(line) for line in source_lines]
    syntax_status = list(filter(lambda x: x == True, [s[1] for s in syn]))
    if len(syn) != len(syntax_status):
        assert str(list(filter(lambda x: x == False, [s[1] for s in syn]))) 
    
    bytecount = [line[0] for line in syn]
    print(bytecount)
    final_lines = [expand_labels(label_env, bytecount, line) for line in source_lines]
    
    final_lines_syn = [check_instruction_syntax_and_count_bytes(line) for line in final_lines]
    syntax_status = list(filter(lambda x: x == True, [s[1] for s in final_lines_syn]))
    if len(final_lines_syn) != len(syntax_status):
        assert str(list(filter(lambda x: x == False, [s[1] for s in final_lines_syn])))
        
    # remove label definitions, and line numbers and comments
    ast = []
    for line in final_lines:
        temp = [tk[:2] for tk in line] #remove line numbers
        temp = list(filter(lambda tk: False if tk[0] == "label_definition" else True, temp))
        temp = list(filter(lambda tk: False if tk[0] == "comment" else True, temp))
        ast.append(temp)
       
    return ast
