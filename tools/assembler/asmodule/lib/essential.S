; ESSENTIAL MACROS FOR THE ASSEMBLY LANGUAGE OF THE TALEÄ SYSTEM
; ----------------------------------------------------------------------------

; 1. Pseudo-operations
; -------------------

m nop { addi zero zero 0 }
    ; No operation
    ;
    ; Syntax: nop
    ;
    ; Description:
    ; 	No operation.
    ;
    ; Example:
    ; 	nop
    ;
    ; ----------------------------------------------------------------------------
m li12 rd imm { addi %rd zero %imm }
    ; Load immediate
    ;
    ; Syntax: li12 rd imm
    ;
    ; Description:
    ; 	Loads the immediate value (up to 12 bits signed) imm into register rd.
    ;
    ; Example:
    ; 	li12 rd 12
    ;
    ; ----------------------------------------------------------------------------
m mv rd rs { addi %rd %rs 0 }
    ; Move
    ;
    ; Syntax: mv rd rs
    ;
    ; Description:
    ; 	Move the value of register rs into register rd.
    ;
    ; Example:
    ; 	mv rd rs
    ;
    ; ----------------------------------------------------------------------------
m not rd rs { xori %rd %rs -1 }
    ; Not
    ;
    ; Syntax: not rd rs
    ;
    ; Description:
    ; 	Invert the value of register rs and store the result in register rd. (Ones complement)
    ;
    ; Example:
    ; 	not rd rs
    ;
    ; ----------------------------------------------------------------------------
m neg rd rs { sub %rd zero %rs }
    ; Negate
    ;
    ; Syntax: neg rd rs
    ;
    ; Description:
    ; 	Negate the value of register rs and store the result in register rd. (Twos complement)
    ;
    ; Example:
    ; 	neg rd rs
    ;
    ; ----------------------------------------------------------------------------
m seqz rd rs { sltiu %rd %rs 1 }
    ; Set if equal to zero
    ;
    ; Syntax: seqz rd rs
    ;
    ; Description:
    ; 	Set the register rd to 1 if the value of register rs is equal to zero.
    ;
    ; Example:
    ; 	seqz rd rs
    ;
    ; ----------------------------------------------------------------------------
m snez rd rs { sltu %rd zero %rs }
    ; Set if not equal to zero
    ;
    ; Syntax: snez rd rs
    ;
    ; Description:
    ; 	Set the register rd to 1 if the value of register rs is not equal to zero.
    ;
    ; Example:
    ; 	snez rd rs
    ;
    ; ----------------------------------------------------------------------------
m sltz rd rs { slt %rd %rs zero }
    ; Set if less than zero
    ;
    ; Syntax: sltz rd rs
    ;
    ; Description:
    ; 	Set the register rd to 1 if the value of register rs is less than zero.
    ;
    ; Example:
    ; 	sltz rd rs
    ;
    ; ----------------------------------------------------------------------------
m sgtz rd rs { slt %rd zero %rs }
    ; Set if greater than zero
    ;
    ; Syntax: sgtz rd rs
    ;
    ; Description:
    ; 	Set the register rd to 1 if the value of register rs is greater than zero.
    ;
    ; Example:
    ; 	sgtz rd rs
    ;
    ; ----------------------------------------------------------------------------
m beqz rs offset { beq %rs zero %offset }
    ; Branch if equal to zero
    ;
    ; Syntax: beqz rs offset
    ;
    ; Description:
    ; 	Branch to the instruction at offset if the value of register rs is equal to zero.
    ;
    ; Example:
    ; 	beqz rs 12
    ;
    ; ----------------------------------------------------------------------------
m bnez rs offset { bne %rs zero %offset }
    ; Branch if not equal to zero
    ;
    ; Syntax: bnez rs offset
    ;
    ; Description:
    ; 	Branch to the instruction at offset if the value of register rs is not equal to zero.
    ;
    ; Example:
    ; 	bnez rs 12
    ;
    ; ----------------------------------------------------------------------------
m blez rs offset { bge zero %rs %offset }
    ; Branch if less than or equal to zero
    ;
    ; Syntax: blez rs offset
    ;
    ; Description:
    ; 	Branch to the instruction at offset if the value of register rs is less than or equal to zero.
    ;
    ; Example:
    ; 	blez rs 12
    ;
    ; ----------------------------------------------------------------------------
m bgez rs offset { bge %rs zero %offset }
    ; Branch if greater than or equal to zero
    ;
    ; Syntax: bgez rs offset
    ;
    ; Description:
    ; 	Branch to the instruction at offset if the value of register rs is greater than or equal to zero.
    ;
    ; Example:
    ; 	bgez rs 12
    ;
    ; ----------------------------------------------------------------------------
m bltz rs offset { blt %rs zero %offset }
    ; Branch if less than zero
    ;
    ; Syntax: bltz rs offset
    ;
    ; Description:
    ; 	Branch to the instruction at offset if the value of register rs is less than zero.
    ;
    ; Example:
    ; 	bltz rs 12
    ;
    ; ----------------------------------------------------------------------------
m bgtz rs offset { blt zero %rs %offset }
    ; Branch if greater than zero
    ;
    ; Syntax: bgtz rs offset
    ;
    ; Description:
    ; 	Branch to the instruction at offset if the value of register rs is greater than zero.
    ;
    ; Example:
    ; 	bgtz rs 12
    ;
    ; ----------------------------------------------------------------------------
m bgt rs rt offset { blt %rt %rs %offset }
    ; Branch if greater than
    ;
    ; Syntax: bgt rs rt offset
    ;
    ; Description:
    ; 	Branch to the instruction at offset if the value of register rs is greater than the value of register rt.
    ;
    ; Example:
    ; 	bgt rs rt 12
    ;
    ; ----------------------------------------------------------------------------
m ble rs rt offset { bge %rt %rs %offset }
    ; Branch if less than or equal to
    ;
    ; Syntax: ble rs rt offset
    ;
    ; Description:
    ; 	Branch to the instruction at offset if the value of register rs is less than or equal to the value of register rt.
    ;
    ; Example:
    ; 	ble rs rt 12
    ;
    ; ----------------------------------------------------------------------------
m bgtu rs rt offset { bltu %rt %rs %offset }
    ; Branch if greater than or equal to unsigned
    ;
    ; Syntax: bgtu rs rt offset
    ;
    ; Description:
    ; 	Branch to the instruction at offset if the value of register rs is greater than or equal to the value of register rt.
    ;
    ; Example:
    ; 	bgtu rs rt 12
    ;
    ; ----------------------------------------------------------------------------
m bleu rs rt offset { bgeu %rt %rs %offset }
    ; Branch if less than or equal to unsigned
    ;
    ; Syntax: bleu rs rt offset
    ;
    ; Description:
    ; 	Branch to the instruction at offset if the value of register rs is less than or equal to the value of register rt.
    ;
    ; Example:
    ; 	bleu rs rt 12
    ;
    ; ----------------------------------------------------------------------------
m lj addr { longjump zero %addr }
    ; Jump absolute
    ;
    ; Syntax: j address
    ;
    ; Description:
    ; 	Jump to the instruction at address.
    ;
    ; Example:
    ; 	j 12
    ;
    ; ----------------------------------------------------------------------------
m ljl addr { longjump ra %addr }
    ; Jump and link absolute
    ;
    ; Syntax: jl address
    ;
    ; Description:
    ; 	Jump to the instruction at address and store the address of the next instruction in the return address register (x1, ra).
    ;
    ; Example:
    ; 	jl 12
    ;
    ; ----------------------------------------------------------------------------
m j offset { jal zero %offset }
    ; Jump relative
    ;
    ; Syntax: j offset
    ;
    ; Description:
    ; 	Jump to the instruction at offset.
    ;
    ; Example:
    ; 	j 12
    ;
    ; ----------------------------------------------------------------------------
m jl offset { jal ra %offset }
    ; Jump and link relative
    ;
    ; Syntax: jal offset
    ;
    ; Description:
    ; 	Jump to the instruction at offset and store the address of the next instruction in the return address register.
    ;
    ; Example:
    ; 	jal 12
    ;
    ; ----------------------------------------------------------------------------
m jr rs { jalr zero %rs 0 }
    ; Jump register
    ;
    ; Syntax: jr rs
    ;
    ; Description:
    ; 	Jump to the instruction at the address in register rs.
    ;
    ; Example:
    ; 	jr rs
    ;
    ; ----------------------------------------------------------------------------
m jalra rs { jalr ra %rs 0 }
    ; Jump and link register
    ;
    ; Syntax: jalr rs
    ;
    ; Description:
    ; 	Jump to the instruction at the address in register rs and store the address of the next instruction in the return address register.
    ;
    ; Example:
    ; 	jalr rs
    ;
    ; ----------------------------------------------------------------------------
m ret { jalr zero ra 0 }
    ; Return
    ;
    ; Syntax: ret
    ;
    ; Description:
    ; 	Return to the instruction at the address in the return address register.
    ;
    ; Example:
    ; 	ret
    ;
    ; ----------------------------------------------------------------------------
m inc rd n { addi %rd %rd %n }
    ; Increment
    ;
    ; Syntax: inc rd n
    ;
    ; Description:
    ; 	Add n to the value of register rd.
    ;
    ; Example:
    ; 	inc rd 12
    ;
    ; ----------------------------------------------------------------------------
m dec rd n { addi %rd %rd -%n }

; ----------------------------------------------------------------------------+

; 1.1 More complex quality of life macros
m push rs1 sp {
    addi %sp %sp -2 \
    sh %rs1 %sp 0
}
    ; Push register onto stack
    ;
    ; Syntax: push rs1 sp
    ;
    ; Description:
    ; 	Push the value of register rs1 onto the stack.
    ;
    ; Example:
    ; 	push rs1 sp
    ;
    ; ----------------------------------------------------------------------------
m pop rd sp {
    lh %rd %sp 0 \
    addi %sp %sp 2
}
    ; Pop register from stack
    ;
    ; Syntax: pop rd sp
    ;
    ; Description:
    ; 	Pop the value of register rd from the stack.
    ;
    ; Example:
    ; 	pop rd sp
    ;
    ; ----------------------------------------------------------------------------


; 2. Taleä Base System Constants
; ----------------------------------------------------------------------------

c PORT_KB_INPUT $8
    ; Keyboard input port
    ;
    ; Syntax: PORT_KB_INPUT
    ;
    ; Description:
    ; 	The keyboard input port address.
    ;
    ; Example:
    ; 	PORT_KB_INPUT
    ;
    ; ----------------------------------------------------------------------------
c PORT_TTY_OUT $1 
    ; TTY output port
    ;
    ; Syntax: PORT_TTY_OUT
    ;
    ; Description:
    ; 	The TTY output port address.
    ;
    ; Example:
    ; 	PORT_TTY_OUT
    ;
    ; ----------------------------------------------------------------------------
c port_video_data $2 
c port_video_command $3 
c PORT_VIDEO_OUT $2
    ; Video output port
    ;
    ; Syntax: PORT_VIDEO_OUT
    ;
    ; Description:
    ; 	The video output port address.
    ;
    ; Example:
    ; 	PORT_VIDEO_OUT
    ;
    ; ----------------------------------------------------------------------------
c port_disk_data $4
c port_disk_command $5
c PORT_DISK_OUT $4
    ; Disk output port
    ;
    ; Syntax: PORT_DISK_OUT
    ;
    ; Description:
    ; 	The disk output port address.
    ;
    ; Example:
    ; 	PORT_DISK_OUT
    ;
    ; ----------------------------------------------------------------------------
c port_tiny_portable_storage_data $6
c port_tiny_portable_storage_command $7
c PORT_TPS_OUT $6
    ; Tiny portable storage output port
    ;
    ; Syntax: PORT_TPS_OUT
    ;
    ; Description:
    ; 	The tiny portable storage output port address.
    ;
    ; Example:
    ; 	PORT_TPS_OUT
    ;
    ; ----------------------------------------------------------------------------
; 2.1 Exception Vectors
c VECTOR_EXCEPTION_DIVIDE_ERROR ;TODO: assign exception vector addresses 
    ; Divide error
    ;
    ; Syntax: EXCEPTION_VECTOR_DIVIDE_ERROR
    ;
    ; Description:
    ; 	The divide error exception vector address.
    ;
    ; Example:
    ; 	EXCEPTION_VECTOR_DIVIDE_ERROR
    ;
    ; ----------------------------------------------------------------------------
c VECTOR_EXCEPTION_ILLEGAL_OPCODE $100 ;
    ; Illegal instruction
    ;
    ; Syntax: EXCEPTION_VECTOR_ILLEGAL_INSTRUCTION
    ;
    ; Description:
    ; 	The illegal instruction exception vector address.
    ;
    ; Example:
    ; 	EXCEPTION_VECTOR_ILLEGAL_INSTRUCTION
    ;
c VECTOR_EXCEPTION_BAD_PRIVILEGE $101
    ; ----------------------------------------------------------------------------
    ; Bad privilege level
c VECTOR_EXCEPTION_SEGMENT_VIOLATION ;TODO: assign exception vector addresses 
    ; Segment violation
    ;
    ; Syntax: EXCEPTION_VECTOR_SEGMENT_VIOLATION
    ;
    ; Description:
    ; 	The segment violation exception vector address.
    ;
    ; Example:
    ; 	EXCEPTION_VECTOR_SEGMENT_VIOLATION
    ;
    ; ----------------------------------------------------------------------------
c VECTOR_EXCEPTION_PAGE_FAULT ;TODO: assign exception vector addresses
    ; Page fault
    ;
    ; Syntax: EXCEPTION_VECTOR_PAGE_FAULT
    ;
    ; Description:
    ; 	The page fault exception vector address.
    ;
    ; Example:
    ; 	EXCEPTION_VECTOR_PAGE_FAULT
    ;
    ; ----------------------------------------------------------------------------
c VECTOR_EXCEPTION_ILLEGAL_INSTRUCTION ;TODO: assign exception vector addresses
    ; Illegal instruction
    ;
    ; Syntax: EXCEPTION_VECTOR_ILLEGAL_INSTRUCTION
    ;
    ; Description:
    ; 	The illegal instruction exception vector address.
    ;
    ; Example:
    ; 	EXCEPTION_VECTOR_ILLEGAL_INSTRUCTION
    ;
    ; ----------------------------------------------------------------------------
c VECTOR_EXCEPTION_BAD_MEMORY_ACCESS ;TODO: assign exception vector addresses
    ; Bad memory access
    ;
    ; Syntax: EXCEPTION_VECTOR_BAD_MEMORY_ACCESS
    ;
    ; Description:
    ; 	The bad memory access exception vector address.
    ;
    ; Example:
    ; 	EXCEPTION_VECTOR_BAD_MEMORY_ACCESS
    ;
    ; ----------------------------------------------------------------------------
c VECTOR_EXCEPTION_GENERAL_PROTECTION_FAULT ;TODO: assign exception vector addresses
    ; General protection fault
    ;
    ; Syntax: EXCEPTION_VECTOR_GENERAL_PROTECTION_FAULT
    ;
    ; Description:
    ; 	The general protection fault exception vector address.
    ;
    ; Example:
    ; 	EXCEPTION_VECTOR_GENERAL_PROTECTION_FAULT
    ;
    ; ----------------------------------------------------------------------------

; 2.2 Interrupt Vectors
c VECTOR_INTERRUPT_TIMER ;TODO: assign interrupt vector addresses 
    ; Timer interrupt
    ;
    ; Syntax: INTERRUPT_VECTOR_TIMER
    ;
    ; Description:
    ; 	The timer interrupt interrupt vector address.
    ;
    ; Example:
    ; 	INTERRUPT_VECTOR_TIMER
    ;
    ; ----------------------------------------------------------------------------
c VECTOR_INTERRUPT_KEYBOARD $180
    ; Keyboard interrupt
    ;
    ; Syntax: INTERRUPT_VECTOR_KEYBOARD
    ;
    ; Description:
    ; 	The keyboard interrupt interrupt vector address.
    ;
    ; Example:
    ; 	INTERRUPT_VECTOR_KEYBOARD
    ;
    ; ----------------------------------------------------------------------------
c VECTOR_INTERRUPT_SERIAL_PORT ;TODO: assign interrupt vector addresses
    ; Serial port interrupt
    ;
    ; Syntax: INTERRUPT_VECTOR_SERIAL_PORT
    ;
    ; Description:
    ; 	The serial port interrupt interrupt vector address.
    ;
    ; Example:
    ; 	INTERRUPT_VECTOR_SERIAL_PORT
    ;
    ; ----------------------------------------------------------------------------
c VECTOR_INTERRUPT_DISK ;TODO: assign interrupt vector addresses
    ; Disk interrupt
    ;
    ; Syntax: INTERRUPT_VECTOR_DISK
    ;
    ; Description:
    ; 	The disk interrupt interrupt vector address.
    ;
    ; Example:
    ; 	INTERRUPT_VECTOR_DISK
    ;
    ; ----------------------------------------------------------------------------
c VECTOR_INTERRUPT_VIDEO ;TODO: assign interrupt vector addresses
    ; Video interrupt
    ;
    ; Syntax: INTERRUPT_VECTOR_VIDEO
    ;
    ; Description:
    ; 	The video interrupt interrupt vector address.
    ;
    ; Example:
    ; 	INTERRUPT_VECTOR_VIDEO
    ;
    ; ----------------------------------------------------------------------------
c VECTOR_INTERRUPT_CLOCK ;TODO: assign interrupt vector addresses
    ; Clock interrupt
    ;
    ; Syntax: INTERRUPT_VECTOR_CLOCK
    ;
    ; Description:
    ; 	The clock interrupt interrupt vector address.
    ;
    ; Example:
    ; 	INTERRUPT_VECTOR_CLOCK
    ;
    ; ----------------------------------------------------------------------------
c VECTOR_INTERRUPT_PRINTER ;TODO: assign interrupt vector addresses
    ; Printer interrupt
    ;
    ; Syntax: INTERRUPT_VECTOR_PRINTER
    ;
    ; Description:
    ; 	The printer interrupt interrupt vector address.
    ;
    ; Example:
    ; 	INTERRUPT_VECTOR_PRINTER
    ;
    ; ----------------------------------------------------------------------------
c VECTOR_INTERRUPT_FLOPPY ;TODO: assign interrupt vector addresses
    ; Floppy interrupt
    ;
    ; Syntax: INTERRUPT_VECTOR_FLOPPY
    ;
    ; Description:
    ; 	The floppy interrupt interrupt vector address.
    ;
    ; Example:
    ; 	INTERRUPT_VECTOR_FLOPPY
    ;
    ; ----------------------------------------------------------------------------

; 2.3 Trap Vectors
c VECTOR_TRAP_GETC $020
    ; ----------------------------------------------------------------------------
    ; Get character
c VECTOR_TRAP_PUTC $021
    ; ----------------------------------------------------------------------------
    ; Put character
c VECTOR_TRAP_PUTS $022
    ; ----------------------------------------------------------------------------
    ; Put string
c VECTOR_TRAP_IN $023
    ; ----------------------------------------------------------------------------
    ; Input
c VECTOR_TRAP_HALT $024
    ; ----------------------------------------------------------------------------
    ; Halt
c VECTOR_TRAP_SYSCALL $025
    ; System call
    ;
    ; Syntax: TRAP_VECTOR_SYSCALL
    ;
    ; Description:
    ; 	The system call trap vector address.
    ;
    ; Example:
    ; 	TRAP_VECTOR_SYSCALL
    ;
    ; ----------------------------------------------------------------------------
c VECTOR_TRAP_DEBUG ;TODO: assign trap vector addresses
    ; Debug trap
    ;
    ; Syntax: TRAP_VECTOR_DEBUG
    ;
    ; Description:
    ; 	The debug trap trap vector address.
    ;
    ; Example:
    ; 	TRAP_VECTOR_DEBUG
    ;
    ; ----------------------------------------------------------------------------
c VECTOR_TRAP_OVERFLOW ;TODO: assign trap vector addresses
    ; Overflow trap
    ;
    ; Syntax: TRAP_VECTOR_OVERFLOW
    ;
    ; Description:
    ; 	The overflow trap trap vector address.
    ;
    ; Example:
    ; 	TRAP_VECTOR_OVERFLOW
    ;
    ; ----------------------------------------------------------------------------
