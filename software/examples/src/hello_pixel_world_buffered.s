; Bare metal graphic hello world program for the Taleä system
; This program is a more efficient aproach to the previous example
; It uses the BLIT video module command and an in memory buffer

; Starts in .section text
    .org 0
start:
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
    jal zero done
