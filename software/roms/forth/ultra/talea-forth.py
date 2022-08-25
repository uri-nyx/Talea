# This is the implementation of the assembly implementation of the Talea Forth, written in python
# Executing this script will generate a file with the assembly code for the Talea Forth
# It uses ultra-macros: simple python functions that emit complex assembly code tedious to handcraft

OUTFILE = "talea-forth.S"

asm = """
; ---
; title: Sirius Forth
; author: Uri Nyx
; ---

; # Sirius Forth
; Sirius Forth is the first working environment for the Taleä Tabula System
; Inspired on the previous forth systems for the Taleä family, and jonesforth 
; and moving forth.

; # 1. Computing model, design architecture and memory layout
; Sirius forth is a 16 bit forth for a 16 bit computer, ~though it can address the full
; 24 bit address space of the machine~. It is a Direct Threaded forth implementation, and
; maintains the Top of the stack in a register (t1). It uses 7 registers for its operation,
; all mapped to hardware registers:
;--- asm
c W s1
c IP s2
c PSP s3
c RSP s4
c X s5
c USP s6
c TOS s7
;--- asm

    ; ## 1.1 The dictionary
    ; ### 1.1.1 Header
;--- asm
"""
LINK = 0
F_HIDDEN = 0x01 # hidden word
F_IMMEDIATE = 0x02 # immediate word
def make_header(name, label, flags, action):
    """
    Make the header of a word:
        link points to the link of the previous word in the dictionary
        name is the name of the word
        label is the entry point of the word
        flags are either Immediate or not
        action is the code action of the word (DOCOL, DOCON, etc)
    """
    s = """
    head_{label}:
        .word {link}
        .byte {flags}
        .byte {length}
        .string "{name}"
        .align 4
    {label}:
    """.format(link=LINK, flags=flags, length=len(name), name=name, label=label)
    s += "    jl {action}" if action != "DOCODE" else ""
    
    LINK = "head_{label}" # update the link for the next word
    
    return s
asm += """
    ; ## 1.2 The Interpreter
(IP) -> W   fetch memory pointed by IP into "W" register
IP+2 -> IP  advance IP (assuming 2-byte addresses)
JP (W)      jump to the address in the W register
;--- asm
m NEXT {
    ; (IP) -> W  
    lh W IP 0 \      ; fetch the address
    ; IP+2 -> IP
    addi IP IP +2 \  ; advance IP to the next word in the thread
    ; JP (W)
    jr W             ; jump to the address in the W register (without linking?)
}
;--- asm

Macros to push and pop the return stack

;--- asm
m pushR reg {                                                                   RSP <------- RSP (old)
    ; Push the value of reg to the return stack (2 bytes)                      v              v
    addi RSP RSP -2 \   ; make room for an address                              -3   -2   -1    0
    sh %reg RSP 0   \   ; store first the offset (2 bytes) in little endian      | sh | sh | sb | ...                          
    
}

m popR reg {                                                                   RSP --------> RSP (new)
    ; Pop the top of the return stack into reg (2 bytes)                         v              v
    lh %reg RSP 0    \   ; load the offset                                        0   +1   +2   +3
    addi RSP RSP +2     ; reduce the stack pointer by 2 bytes   
}
%ENDMACRO
;--- asm

; # 2. VECTOR TABLES:
; The vector tables control traps and interrupts of the system. The service 
; routines for these will be provided by the high level Forth kernel as words.
; The data structure, however, is initialized to 0.

    ; ## 2.1 TRAP VECTOR TABLE
; The trap vector table holds pointers to the trap routines. This routines
; must have their entry points within the first 64K of memory.
;--- asm
trap_vector_table:
    .word 0
    .dblock $ff
;---

    ; ## 2.2 INTERRUPT VECTOR TABLE
    ; 0x0100-0x01FF
; The trap vector table holds pointers to the interupt service routines. This routines
; must also have their entry points within the first 64K of memory.
;--- asm
interrupt_vector_table:
    .word 0
    .dblock $ff
;---

;# 3. Primitives

    ;## 3.1 ENTER, DOCOL, DOCOLON
;PUSH IP     onto the "return address stack"
;W+4 -> IP   W still points to the Code Field, so W+4 is 
             the address of the Body!  (All Taleä instructions are 4 bytes wide.)
;JUMP to interpreter ("NEXT")

;--- asm
j forth_enter ; jump to the entry point of FORTH

ENTER:
DOCOL:
DOCOLON:
    ; PUSH IP
    pushR IP 
    ; W+4 -> IP
    addi IP W +4 
    ; JUMP to interpreter ("NEXT")
    NEXT
;--- asm

;## 3.2 DOVAR, code action of VARIABLE, entered by CALL
; DOCREATE, code action of newly created words

;--- asm
docreate:
dovar:  ; -- a-addr
                    ; parameter field address (W + 4)
    push TOS PSP    ; push old TOS
    addi TOS W +4   ; pfa = variable's adrs -> TOS
    next
;--- asm

;## 3.3 DOCON, code action of CONSTANT,
; entered by CALL DOCON

;--- asm
docon:  ; -- x
    push TOS PSP    ; push old TOS
    lh TOS W +4     ; fetch contents of parameter field -> TOS
    next
;--- asm

; INTERPRETER LOGIC =============================
; See also "defining words" at end of this file

;C EXIT     --      exit a colon definition
"""
asm += make_header("EXIT", "EXIT", 0, "DOCODE")
asm += """
;POP IP   from the "return address stack"
;JUMP to interpreter
    popR IP
    NEXT

;Z lit      -- x    fetch inline literal to stack
; This is the primtive compiled by LITERAL.
; litterals are encoded 16 bit with a byte of padding
"""
asm += make_header("lit", "lit", 0, "DOCODE")
asm += """
; Take the next cell from the thread and put it on the stack
    push TOS PSP        ; push old TOS
    lh TOS IP 0         ; fetch cell at IP to TOS,
    inc IP +3           ; advancing IP to the next address
    NEXT

;C EXECUTE   i*x xt -- j*x   execute Forth word
;C                           at 'xt'
"""
asm += make_header("EXECUTE", "EXECUTE", 0, "DOCODE")
asm += """
; Execute the word at the top of the stack
    mv TOS W         ;put address of word in W
    pop TOS PSP      ;pop new TOS
    jr W             ;jump to address given in W
"""
asm += make_header("dup", "DUP", 0, "DOCODE")
asm += """
;C DUP      x -- x x      duplicate top of stack
pushtos:
    push TOS PSP     ;push TOS (effectively a copy of the top of the stack)
    next
"""
asm += make_header("?dup", "QDUP", 0, "DOCODE")
asm += """
;C ?DUP     x -- 0 | x x    DUP if nonzero
    bnez TOS pushtos ;if TOS is zero, do nothing
    next
"""
asm += make_header("drop", "drop", 0, "DOCODE")
asm += """
;C DROP     x --          drop top of stack
poptos:
    pop TOS PSP     ;pop TOS (effectively drops the previous top of the stack)
    next
"""
asm += make_header("swap", "swap", 0, "DOCODE")
asm += """
;C SWAP     x1 x2 -- x2 x1    swap top two items
    pop X PSP       ;pop X
    push TOS PSP    ;push TOS
    mv TOS X        ;move X to TOS
    next
"""
asm += make_header("over", "over", 0, "DOCODE")
asm += """
;C OVER    x1 x2 -- x1 x2 x1   per stack diagram
    pop X PSP       ;pop X
    push TOS PSP    ;push TOS
    push X PSP      ;push X
    next
"""