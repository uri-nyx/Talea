;include vector_tables.s


enter:
    spsr zero $3      ; set priority to 3
    la sp ss_start    ; set supervisor stack
    j infinite_prompt ; jump to infinite prompt that echoes the characters typed


resume_bios_thread:
    j enter

;;; BAD_TRAP - code to execute for undefined trap. It will halt
BAD_TRAP:
    j TRAP_HALT

;;; BAD_INT - code to execute for undefined interrupt. It will halt
BAD_INT: 
    j TRAP_HALT               ; This is an undefined instruction. It will halt and reset to the entry point


;;; EXCEPTIONS
;;; EXCEPTION_ILLEGAL_OPCODE - code to execute for illegal opcode. It will halt (because of the emulator)
EXCEPTION_ILLEGAL_OPCODE:
    j enter                 ; jumps to the system entry point

;;; EXCEPTION_BAD_PRIVILEGE - code to execute for bad privilege. It will NOT halt 
EXCEPTION_BAD_PRIVILEGE:
    j resume_bios_thread    ; resumes the system thread to the BIOS

;;; INTERRUPTS
;;; INTERRUPT_KEYBOARD - code to execute for keyboard interrupt (keypress, not ^C). It will NOT halt
INTERRUPT_KEYBOARD:
    push t6 sp                  ; we will use t6 to store the keypress
    push t5 sp                  ; we will use t5 some variables for now
    lhc t6 zero PORT_KB_INPUT   ; load the character from the keyboard port

    ;TODO: STORE THE CHARACTER IN A RING BUFFER IN CACHE
    lbg t5 getc_requested        ; load getc_requested variable into t5
    beqz t5 +8                   ; if getc_requested is zero, skip the next instruction
    mv a0 t6
    
    pop t5 sp
    pop t6 sp                   ; restore t6
    rti
    

;;; TRAPS

TRAP_GETC: ;TODO: Implement traps
    addi a0 zero 1
    sbg a0 getc_requested t6    ; set getc_requested to 1
    mv a0 zero                  ; set a0 to 0
    beqz a0 0                   ; if a0 does not contain a character from the interrupt, jump to self
    rti                         ; if a0 contains a character from the interrupt, return to the system


TRAP_PUTC:                      ; PUTC puts to video, not to tty see PRINT
    ori a0 a0 $400              ; a0 is the first argument to the trap, the character to print. We set the command to print 0x4 into the high byte
    shc a0 zero PORT_VIDEO_OUT  ; send the command to the video port
    addi x5 x5 1                ; increment x5 to point to the next position in the video buffer
    rti

TRAP_PUTS:                      ; PUTS outputs a null terminated string to video.
    mv a1 a0
t_puts_loop:
    lbu a0 a1 0                 ; a1 is the first argument to the trap, points to the string to print. We load the character into a0
    beqz a0 t_puts_end          ; if a0 is zero (NULL), we are done
    trap zero VECTOR_TRAP_PUTC  ; we invoke the trap to output a character to the video port
    addi a1 a1 1                ; increment the pointer to the string
    j t_puts_loop                ; Repeat for the next character
t_puts_end: 
    rti

                         
TRAP_IN:                        ; Gets a Character (blocking) from stdin in a0, echoes it at desired location in s:a1
    trap zero VECTOR_TRAP_GETC  ; invoke the trap to get a character from the keyboard
    sb a0 a1 0                  ; store the character in the desired location
    trap zero VECTOR_TRAP_PUTC  ; invoke the trap to output the character to the video port
    rti

infinite_prompt:
    ; greeting
    la a0 prompt                ; Load the prompt string into a0
    trap zero VECTOR_TRAP_PUTS  ; Output the prompt to the video port
    addi x5 zero $100           ; print in the next line
    addi a0 zero 62             ; Ascii code for '>'
    trap zero VECTOR_TRAP_PUTC  ; Output the '>' to the video port

    la a1 prompt_buffer         ; a1 points to the prompt buffer
infinite_prompt_loop:           ; infinite loop
    trap zero VECTOR_TRAP_IN    ; Get a character from stdin
    addi a1 a1 1                ; Increment the pointer to the prompt buffer
    j infinite_prompt_loop

;;; DATA SECTION --------------------------------------------------
    .section data
TRAP_HALT: 
    .dword $0 ; This is an undefined instruction. It will halt and reset to the entry point
getc_requested:   
    .byte 0
    .align 4

prompt: 
        .string "Talea System minimal BIOS a0.1"
        .byte 0
        .align 4
prompt_buffer:    
    .dblock 1024 ; 1024 bytes of space for the prompt buffer

;Stacks (grow from high to low)
            .dblock 4096
ss_start:   
            .dword 0    ; start of the supervisor stack
            .dblock 4096
us_start:   
            .dword 0    ; start of the user stack


