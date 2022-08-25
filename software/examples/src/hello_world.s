; Hello world bare metal program for the Taleä system
; Starts in .section text
    .org 0
start:
    la a0 hello_str
    jal ra puts
    ;addi x5 zero 0         ; zero the video counter to make it static
    jal zero start          ; infinite printing loop make a trap to halt?

puts:
    lbu t1 a0 0             ; get the next character from the string
    beq t1 zero puts_end    ; if it's zero, we're done
    ori t1 t1 $400          ; prepend command code for printing a character
    jal t2 print_char       ; print the character return address is in t2
    addi a0 a0 1            ; increment the string pointer
    jal zero puts           ; loop

print_char:
    shc t1 zero 2          ; store the word (command code + character) in the video ports
    addi x5 x5 $1          ; increment the video pointer to the next character
    jalr zero t2 0         ; return to the puts function

puts_end:
    jalr zero ra 0         ; return to the start function

    .section data
hello_str:  .string "Hello, world!"
            .byte 0