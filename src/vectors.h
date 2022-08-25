// 2.1 Exception Vectors
#define VECTOR_EXCEPTION_DIVIDE_ERROR //TODO: assign exception vector addresses 
    // Divide error
    //
    // Syntax: EXCEPTION_VECTOR_DIVIDE_ERROR
    //
    // Description:
    // 	The divide error exception vector address.
    //
    // Example:
    // 	EXCEPTION_VECTOR_DIVIDE_ERROR
    //
    // ----------------------------------------------------------------------------
#define VECTOR_EXCEPTION_ILLEGAL_OPCODE 0x00
    // Illegal instruction
    //
    // Syntax: EXCEPTION_VECTOR_ILLEGAL_INSTRUCTION
    //
    // Description:
    // 	The illegal instruction exception vector address.
    //
    // Example:
    // 	EXCEPTION_VECTOR_ILLEGAL_INSTRUCTION
    //
#define VECTOR_EXCEPTION_BAD_PRIVILEGE 0x01
    // ----------------------------------------------------------------------------
    // Bad privilege level
#define VECTOR_EXCEPTION_SEGMENT_VIOLATION //TODO: assign exception vector addresses 
    // Segment violation
    //
    // Syntax: EXCEPTION_VECTOR_SEGMENT_VIOLATION
    //
    // Description:
    // 	The segment violation exception vector address.
    //
    // Example:
    // 	EXCEPTION_VECTOR_SEGMENT_VIOLATION
    //
    // ----------------------------------------------------------------------------
#define VECTOR_EXCEPTION_PAGE_FAULT //TODO: assign exception vector addresses
    // Page fault
    //
    // Syntax: EXCEPTION_VECTOR_PAGE_FAULT
    //
    // Description:
    // 	The page fault exception vector address.
    //
    // Example:
    // 	EXCEPTION_VECTOR_PAGE_FAULT
    //
    // ----------------------------------------------------------------------------
#define VECTOR_EXCEPTION_ILLEGAL_INSTRUCTION //TODO: assign exception vector addresses
    // Illegal instruction
    //
    // Syntax: EXCEPTION_VECTOR_ILLEGAL_INSTRUCTION
    //
    // Description:
    // 	The illegal instruction exception vector address.
    //
    // Example:
    // 	EXCEPTION_VECTOR_ILLEGAL_INSTRUCTION
    //
    // ----------------------------------------------------------------------------
#define VECTOR_EXCEPTION_BAD_MEMORY_ACCESS //TODO: assign exception vector addresses
    // Bad memory access
    //
    // Syntax: EXCEPTION_VECTOR_BAD_MEMORY_ACCESS
    //
    // Description:
    // 	The bad memory access exception vector address.
    //
    // Example:
    // 	EXCEPTION_VECTOR_BAD_MEMORY_ACCESS
    //
    // ----------------------------------------------------------------------------
#define VECTOR_EXCEPTION_GENERAL_PROTECTION_FAULT //TODO: assign exception vector addresses
    // General protection fault
    //
    // Syntax: EXCEPTION_VECTOR_GENERAL_PROTECTION_FAULT
    //
    // Description:
    // 	The general protection fault exception vector address.
    //
    // Example:
    // 	EXCEPTION_VECTOR_GENERAL_PROTECTION_FAULT
    //
    // ----------------------------------------------------------------------------

// 2.2 Interrupt Vectors
#define VECTOR_INTERRUPT_TIMER //TODO: assign interrupt vector addresses 
    // Timer interrupt
    //
    // Syntax: INTERRUPT_VECTOR_TIMER
    //
    // Description:
    // 	The timer interrupt interrupt vector address.
    //
    // Example:
    // 	INTERRUPT_VECTOR_TIMER
    //
    // ----------------------------------------------------------------------------
#define VECTOR_INTERRUPT_KEYBOARD 0x80
    // Keyboard interrupt
    //
    // Syntax: INTERRUPT_VECTOR_KEYBOARD
    //
    // Description:
    // 	The keyboard interrupt interrupt vector address.
    //
    // Example:
    // 	INTERRUPT_VECTOR_KEYBOARD
    //
    // ----------------------------------------------------------------------------
#define VECTOR_INTERRUPT_SERIAL_PORT //TODO: assign interrupt vector addresses
    // Serial port interrupt
    //
    // Syntax: INTERRUPT_VECTOR_SERIAL_PORT
    //
    // Description:
    // 	The serial port interrupt interrupt vector address.
    //
    // Example:
    // 	INTERRUPT_VECTOR_SERIAL_PORT
    //
    // ----------------------------------------------------------------------------
#define VECTOR_INTERRUPT_DISK //TODO: assign interrupt vector addresses
    // Disk interrupt
    //
    // Syntax: INTERRUPT_VECTOR_DISK
    //
    // Description:
    // 	The disk interrupt interrupt vector address.
    //
    // Example:
    // 	INTERRUPT_VECTOR_DISK
    //
    // ----------------------------------------------------------------------------
#define VECTOR_INTERRUPT_VIDEO //TODO: assign interrupt vector addresses
    // Video interrupt
    //
    // Syntax: INTERRUPT_VECTOR_VIDEO
    //
    // Description:
    // 	The video interrupt interrupt vector address.
    //
    // Example:
    // 	INTERRUPT_VECTOR_VIDEO
    //
    // ----------------------------------------------------------------------------
#define VECTOR_INTERRUPT_CLOCK //TODO: assign interrupt vector addresses
    // Clock interrupt
    //
    // Syntax: INTERRUPT_VECTOR_CLOCK
    //
    // Description:
    // 	The clock interrupt interrupt vector address.
    //
    // Example:
    // 	INTERRUPT_VECTOR_CLOCK
    //
    // ----------------------------------------------------------------------------
#define VECTOR_INTERRUPT_PRINTER //TODO: assign interrupt vector addresses
    // Printer interrupt
    //
    // Syntax: INTERRUPT_VECTOR_PRINTER
    //
    // Description:
    // 	The printer interrupt interrupt vector address.
    //
    // Example:
    // 	INTERRUPT_VECTOR_PRINTER
    //
    // ----------------------------------------------------------------------------
#define VECTOR_INTERRUPT_FLOPPY //TODO: assign interrupt vector addresses
    // Floppy interrupt
    //
    // Syntax: INTERRUPT_VECTOR_FLOPPY
    //
    // Description:
    // 	The floppy interrupt interrupt vector address.
    //
    // Example:
    // 	INTERRUPT_VECTOR_FLOPPY
    //
    // ----------------------------------------------------------------------------

// 2.3 Trap Vectors
#define VECTOR_TRAP_GETC 0x020
    // ----------------------------------------------------------------------------
    // Get character
#define VECTOR_TRAP_PUTC 0x021
    // ----------------------------------------------------------------------------
    // Put character
#define VECTOR_TRAP_PUTS 0x022
    // ----------------------------------------------------------------------------
    // Put string
#define VECTOR_TRAP_IN 0x023
    // ----------------------------------------------------------------------------
    // Input
#define VECTOR_TRAP_HALT 0x024
    // ----------------------------------------------------------------------------
    // Halt
#define VECTOR_TRAP_SYSCALL 0x025
    // System call
    //
    // Syntax: TRAP_VECTOR_SYSCALL
    //
    // Description:
    // 	The system call trap vector address.
    //
    // Example:
    // 	TRAP_VECTOR_SYSCALL
    //
    // ----------------------------------------------------------------------------
#define VECTOR_TRAP_DEBUG //TODO: assign trap vector addresses
    // Debug trap
    //
    // Syntax: TRAP_VECTOR_DEBUG
    //
    // Description:
    // 	The debug trap trap vector address.
    //
    // Example:
    // 	TRAP_VECTOR_DEBUG
    //
    // ----------------------------------------------------------------------------
#define VECTOR_TRAP_OVERFLOW //TODO: assign trap vector addresses
