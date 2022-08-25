; kernel dummy test it is hello pixel world buffered

    .org 16     ; this fictional executable file starts at offset 16 in its file
start:
    addi x5 x5 $100
    la a0 msg                   ; simply print a message
    trap zero VECTOR_TRAP_PUTS  ; put the message
    spsr zero $3                ; allow interrupts
    trap zero VECTOR_TRAP_GETC  ; wait for user input
    spsr zero $7                ; disable interrupts
    addi t3 zero 640
    addi a0 zero $300       ; command to set the graphics mode
    shc a0 zero 2           ; send the command to the GPU
    ssr x5 zero $f9         ; Load in x5 pointer to address of buffer

iterate:
    addi a0 zero 0
    ssr t1 zero $f9         ; Load in x6 a pointer to the buffer that will increment
    addi t2 zero $fd        ; Load upper bound of the buffer

draw:
    sb a0 t1 0              ; set color in a0 to [t1]
    addi a0 a0 1
    addi t1 t1 1            ; increment color and pointer
    beq t1 zero overflow    ; check for ovweflow
    beq t1 t3 resetA
    jal zero draw

overflow:
    gsr t1 t1
    addi t1 t1 1            ; increment segment
    beq t1 t2 blit          ; if t1 == t2 it's time to blit
    ssr t1 t1 0             ; else we increment the segment and continue
resetA:
    addi a0 zero 0
    jal zero draw

blit:
    addi a0 zero $600       ; command to blit
    shc a0 zero 2           ; send command to video

done:
    ret

    .section data
msg:
    .string "Loaded Executable File"
    .byte 10
    .string "Press a key to execute Hello Pixel World"
    .byte 0
    .align 4