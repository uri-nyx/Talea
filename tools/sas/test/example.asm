.const SREG #3421499392

.const VIDEO_MODE #0

.text

_start:
; Set status register as: supervisor, intterupt enabled, mmu disabled
; priority 2, ivt at 0xf800
    li      %x10, SREG
    ssreg   %x10
    cli

; Set the video mode to combined text and graphics
    li      %x10, VIDEO_MODE
    sbd     %x10, #0x11(%x0)
    li      %x5, #2
    sbd     %x5, #0x10(%x0)

; Set the stack pointer to the top of memory - 4K (TODO: here should be a macro)
    li      %x2, #0xffefff
; print something
    sti
    call    print

_halt:
    j       _halt

print:
	.const SCREEN $0xe51000
.data
	msg: 
		.byte "Hello, World!"
		.byte #0
.text
	li %x10, SCREEN
	la %x11, msg
print_loop: 
			lbu %x12, #0(%x11)
			beq %x12, %x0, print_end
			sb  %x12, #0(%x10)
			addi %x11, %x11, #1
			addi %x10, %x10, #1
			j print_loop
print_end: ret



