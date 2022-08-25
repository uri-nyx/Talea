; Bare metal graphic hello world program for the Taleä system
; This program shows the system cannot keep up setting pixels
; until filling the screeen before the screen refresh.
; The Drivers for the video module should be used instead of writing pixels directly.
; e.g. a framebuffer and the blit method.

; Starts in .section text
    .org 0
start:
    addi t1 zero 0          ; increment the sgment register to point other regions of the screen
    addi a0 zero $300       ; command to set the graphics mode
    shc a0 zero 2           ; send the command to the GPU
    addi a0 zero 0          ; a0 is our color, starting black
    addi x5 zero 0          ; x5 is our index in the framebuffer up to 0x3ffff
colors:
    andi a0 a0 $ff          ; Ensure x5 holds an 8-bit value
    ori a0 a0 $500          ; command to emit pixel to the  (anded with pixel data)
    shc a0 zero 2           ; Send the command to the GPU
    addi x5 x5 $1           ; Increment x5 to point to the next pixel
    beq x5 zero overflow    ; If x5 is 0 after incrementing, there was an overflow
    addi a0 a0 $1           ; Increment a0 to point to the next color
    jal zero colors         ; Jump back to the start of the loop
overflow:
    addi t1 t1 $1           ; Increment the overflow counter to point to the next segment of the screen
    ssr x5 t1 0             ; Set the segment register to point to the next segment of the screen
    jal zero colors         ; Jump back to the next color

