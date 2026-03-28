.text
_init:
    # a counter for error checking
    li x5, 1
    trace x5, x5, x5, x5

    # claim devices: framebuffer, textbuffer, serial and keyboard

    #1
    li x12, 15 # SYSCALL_DEV_CLAIM

    # li x13, 0  # framebuffer
    # syscall x0, 0x40
    # bne x0, x10, _exit

    #2
    addi x5, x5, 1
    li x13, 1  # textbuffer
    syscall x0, 0x40
    bne x0, x10, _exit
    
    # #3 not anymore
    # addi x5, x5, 1
    # li x13, 4  # serial
    # syscall x0, 0x40
    # bne x0, x10, _exit

    #3
    addi x5, x5, 1
    li x13, 5  # kb
    syscall x0, 0x40
    bne x0, x10, _exit

    #4
    addi x5, x5, 1
    # attach textbuffer and keyboard to stdin, stderr, and stdout
    li x12, 14   # SYSCALL_DEV_CTL
    li x13, 255  # STDIN
    li x14, 1000 # ATTACH
    la x15, kb
    li x16, 8    # len
    syscall x0, 0x40
    bne x0, x10, _exit

    #5
    addi x5, x5, 1
    li x13, 256  # STDOUT
    li x14, 1000 # ATTACH
    la x15, tb
    li x16, 8    # len
    syscall x0, 0x40
    bne x0, x10, _exit

    #6
    addi x5, x5, 1
    li x13, 257  # STDERR
    li x14, 1000 # ATTACH
    la x15, tb
    li x16, 8    # len
    syscall x0, 0x40
    bne x0, x10, _exit


    #7
    addi x5, x5, 1
    # exec the terminal
    li x12, 3 	    # SYSCALL_EXEC
	la x13, shell
	li x14, 2      # argc
	la x15, argv   # argv
	li x16, 1      # EXEC_AOUT_FLAT
	syscall x0, 0x40
    trace x0, x31, x7, x9

    # in case of failure, exec will fall trhough

_exit:
    trace x5, x10, x5, x31
    mv x13, x5
    mv x12, x0 # SYSCALL_EXIT
    .word 0
    syscall x0, 0x40

.data
.align 4
kb:
    .word 1 # hardware
    .word 5 # kb
.align 4
tb:
    .word 1 # hardware
    .word 1 # tb
.align 4
shell:
    .ascii "/A/BIN/SH"
    .byte 0

.align 4
opt:
    .ascii "-x"
    .byte 0

.align 4
autoexec:
    .ascii "/A/AUTOEXEC.ASH"
    .byte 0

.align 4
argv:
    .word opt
    .word autoexec
    .word 0
