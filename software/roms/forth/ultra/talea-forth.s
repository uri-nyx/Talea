; ---
; title: Sirius Forth
; author: Uri Nyx
; ---

; # Sirius Forth
; Sirius Forth is the first working environment for the Taleä Tabula System
; Inspired on the previous forth systems for the Taleä family, and jonesforth 
; and moving forth.

; # 1. Computing model, design architecture and memory layout
; Sirius forth is a 16 bit forth for a 16 bit computer, though it can address the full
; 24 bit address space of the machine. It is a Direct Threaded forth implementation, and
; maintains the Top of the stack in a register (t1). It uses 7 registers for its operation,
; all mapped to hardware registers:
;--- asm
c W s1
c IP s2
c PSP s3
c Rsp s4
c X s5
c USP s6
c TOS s7
;--- asm

    ; 1.1 The dictionary
    ; 1.1.1 Header
;--- asm
m header name label flags {
    ?link:4 .dword 
    flags:3 .
    length:5
    name .string %name
    .align 4
}


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
