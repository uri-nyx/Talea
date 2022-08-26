; A very simple forth that fits in one sector
; heavily based in zserge/lc3-forth (essentially a port)

; ---- MACROS -------

m priority n { spsr zero %n }
m disable_interrupts { spsr zero $7 }
m enable_all_interrupts { spsr zero $0 }

m mvptr rd rs {
    gsr %rd %rs     \
    ssr %rd %rd 0   \
    addi %rd %rs 0  \
}

; ---- CONSTANTS ----

c WORD 2
c NEWLINE 10
c SPACE 32

c TIB $1000    ; start of the Terminal Input Buffer
c TOIN $1100    ; Pointer to next character in TIB (>IN)

%ENDMACRO

    .org 0
start:
    enable_all_interrupts
    jl readln
    jl parse_and_print
    j 0

; readln: read a line until CR and fill in the TIB
readln:
    la s1 tib                   ; load TIB address
    sh s1 s1 WORD               ; make >IN point at the start of TIB
    lh s1 s1 0                  ; s1 is >IN

readloop:
    trap zero VECTOR_TRAP_GETC  ; get a character
    sb a0 s1 0
    mv t1 a0                    ; temporary storage for the character
    trap zero VECTOR_TRAP_PUTC  ; print the character
    inc s1 1                    ; point to the next byte in tib
    dec t1 NEWLINE              ; test if the character was a newline
    bnez t1 readloop            ; if it wasn't (if (a0 - '\n') == 0), read another character
readln_done:                    ; else we're done
    dec s1 1                    ; erase the newline
    sb zero s1 0                ; Write a null ('\0')
    ret

parse_and_print:
    li12 s1 SPACE
    la a1 tib                   ; load the address of the variable tib a1 = &tib
    mv a2 a1                    ; copy of the pointer

pap_next_char:
    lh a0 a1 0                  ; dereference (a0 = *a1)
pap_checks:
    beqz a0 pap_done            ; if a0 == '\0', end
    beq a0 s1 print_token       ; if a0 == ' ', print token
pap_token:                      ; else, it is a token, buffer it
    inc a1 1
    j pap_next_char

print_token:                    ; print token and advance the pointer
    mv a0 a2                    ; a2 holds a pointer to the start of the token
    trap zero VECTOR_TRAP_PUTC  ; output a0
    inc a2 1                    ; increment pointer
    beq a2 a1 pap_next_char     ; if a2 is equal to the las char of the token, we're done
    j print_token

pap_done:
    ret

    .section data
tib: 
    .word TIB       ; start of the Terminal Input Buffer

toin: 
    .word TOIN      ; Pointer to next character in TIB (>IN)
