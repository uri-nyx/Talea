.const TX       #0x0 ; when writing to
.const RX       #0x0 ; when reading to
.const STATUS   #0x1 ; STATUS register r/w
.const CONTROL  #0x2 ; CONTROL retister r/w
.const RXCOUNT  #0x3 ; bytes left in RX fifo


; STATUS register flags
.const SER_S_DAT_AV #0x01 ; data available in rx fifo
.const SER_S_CARR_D #0x02 ; CARRIER DETECT flag

; CONTROL register flags
.const SER_C_INT_EN #0x01 ; Enable interrupt upon receiving data
.const SER_C_MRESET #0x80 ; Master reset (clear all buffers)

.const SCREEN $0xe51000

; Hello world to the serial port!
; Start in ROM
.org $0xFFE000

.const SREG #3421499392 ; cpu status register: supervisor, intterupt enabled, 
                        ; mmu disabled priority 2, ivt at 0xf800

_start:
    li      %x10, SREG
    ssreg   %x10
    cli

; Set the stack pointer to 1MB
    li      %x2, #0xfffff

; print as soon as we connect, and halt
    call wait_for_connection
    la      %x11, msg
    call serial_print
    
; poll the serial port
    cli
    li      %x5, SCREEN
    call poll

wait_for_connection:
    lbud    %x10, STATUS(%x0)               ; Load serial port's status register
    andi    %x10, %x10, SER_S_CARR_D        ; Mask for CARRIER DETECT 
    beq     %x10,  %x0, wait_for_connection ; If flag is 0, loop
    ret

poll:
    call    wait_for_connection
    lbud    %x10, STATUS(%x0)
    andi    %x10, %x10, SER_S_DAT_AV
    beq     %x10, %x0,  poll
    lbud    %x6, RX(%x0)
    call    print_one
    j       poll

print_one:
    trace   %x6, %x5, %x0, %x0
    sbd     %x6, TX(%x0)
    sb      %x6, #0(%x5)
    addi    %x5, %x5, #1
    ret


; When we get a connection, we print a string to the serial port
serial_print:
	li      %x10, SCREEN
print_loop:
        lbu %x12, #0(%x11)
        trace   %x11, %x12, %x10, %x0
        beq %x12, %x0, print_end
        sbd %x12, TX(%x0)
		sb  %x12, #0(%x10)
        addi %x11, %x11, #1
		addi %x10, %x10, #1
        j   print_loop
print_end: ret


.align #4
msg:
    .byte "Talea System v0.1"
    .byte #13
    .byte #10
    .byte "Hello, World!"
    .byte #0
    