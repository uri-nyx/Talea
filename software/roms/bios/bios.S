;include vector_tables.s


bios_start:
    la sp ss_start    ; set supervisor stack

greet:  ; greeting
    la a0 greet_msg             ; Load the prompt string into a0
    trap zero VECTOR_TRAP_PUTS  ; Output the prompt to the video port
    addi x5 zero $100           ; print in the next line

check_for_bootable_media:
    ; The boot order of the system is fixed as follows:
    ; 1. tps A
    ; 2. tps B
    ; 3. Disk boot
    ; If none of this media is present or bootable, the bios will print a message and wait for
    ; the user to insert a bootable medium

    ; Check for tps A (first if present, then if bootable)
is_TpsA_present:
    addi a0 zero $101           ; command to check if tpsA is present (0 $101)
    shc a0 zero PORT_TPS_OUT    ; send command to the tps drive
    beq t1 zero is_TpsB_present ; if t1 (x6) is 0, tpsA is not present
    addi a0 zero $100           ; else, it is, we check if bootable
    shc a0 zero PORT_TPS_OUT    ; send command to the tps drive
    beq t1 zero is_TpsB_present ; if t1 (x6) is 0, tpsA is not bootable
                                ; else, it is, we load its first sector (00) and boot!
    j Tps_loadSector            ; we jump to the routine to load the tpsA sector
    

is_TpsB_present:
    addi a0 zero $80
    slli a0 a0 8                ; shift code of tps to upper byte of a0
    addi a0 a0 $101             ; command to check if tpsB is present (1 $101)
    shc a0 zero PORT_TPS_OUT    ; send command to the tps drive
    beq t1 zero is_disk0_bootable    ; if t1 (x6) is 0, tpsA is not present
    addi a0 zero $80            ; else, it is, we check if bootable
    slli a0 a0 8                ; shift code of tps to upper byte of a0
    addi a0 zero $100           ; 
    shc a0 zero PORT_TPS_OUT    ; send command to the tps drive
    beq t1 zero is_disk0_bootable    ; if t1 (x6) is 0, tpsB is not bootable
    addi a0 zero $80            
    slli a0 a0 8                ; shift code of tps to upper byte of a0
    addi a0 a0 $101             ; command to check if tpsB is present (1 $101)
                                ; else, it is, we load its first sector (00) and boot!
    addi a0 zero $80            ; we indicate we want to load TpsB
    slli a0 a0 8                ; shift code of tps to upper byte of a0

    j Tps_loadSector            ; we jump to the routine to load the tpsb sector

is_disk0_bootable:
    addi a0 zero $300                       ; Command to query if Disk0 is bootable
    shc a0 zero PORT_DISK_OUT               ; send command to the disk drive
    beq t1 zero please_insert_bootable_medium    ; if t1 (x6) is 0, disk0 is not bootable
                                            ; else, it is, we load its first sector (00) and boot!
    addi x31 zero $6                        ; load point for the first sector of the disk (address 3072, 0xc00)
    addi x30 zero 0                         ; Sector to load is Sector 0
    addi a0 a0 $e0                          ; Argument to the disk drive command: Point in x31, Sector in x30, and drive is 0
    ori a0 a0 $200                          ; Command to the disk drive: Load sector
    shc a0 zero PORT_DISK_OUT               ; Send the command to the disk drive

boot:                                       ;disk shoud be loaded in A00. Jump There
    addi x31 zero $600                      ; Load point for the first sector of the disk (address 3072, 0xc00)
    jalr zero x31 $600                      ; Jump to the first sector of the disk (we add the addresses)

please_insert_bootable_medium:
    la a0 no_bootable_msg             ; Load the message string into a0
    trap zero VECTOR_TRAP_PUTS        ; Output tit to the video port
    addi x5 x5 $100                   ; print in the next line
    andi x5 x5 -256                   ; clear the low byte (sing extension makes this x5 & 0xff00) ;TODO: allow conversion of hex litterals to twoscomp

    spsr zero $3                      ; set priority to 3 (to allow for keyboard interrupts)
    trap zero VECTOR_TRAP_GETC         ; trick to wait for a keypress
    addi a0 zero 0                    ; discard the keypress, it was only for waiting
    spsr zero $7                      ; set priority to 7 (to mask all interrupts)

    j check_for_bootable_media        ; all the way around another time, maybe now we boot!

Tps_loadSector:
    ori a0 a0 $200              ; The command to mount the tps (opening the file). If it is forgotten, the emulator segfaults
    shc a0 zero PORT_TPS_OUT    ; We send the comand to the tps drive
    addi a0 a0 -512             ; we clear the command without alteringthe high byte
    ori a0 a0 $500              ; The command to load sector 0. The high byte of a0 is loaded with the Tps Reader id (0 or 0x8)
    addi x6 zero $6             ; We want to mount the sector at mounting point 6 (address $c00)
    shc a0 zero PORT_TPS_OUT    ; send command to the tps drive
    addi a0 a0 -1280            ; now that's loaded, we clean up a bit. 
    ori  a0 a0 $300             ; I'll unmount (close) the tps. It could be done by it's own code too
    shc a0 zero PORT_TPS_OUT
    srli a0 a0 $f               ; I'll also leave a flag in a0 for the tps bootloader to know in which reader it is (0 for A, 1 for B)
    j boot                      ; Now we boot!

;;; HALT AND UNDEFINED INTERRUPTS
TRAP_HALT: 
    .dword $0 ; This is an undefined instruction. It will halt and reset to the entry point

;;; BAD_TRAP - code to execute for undefined trap. It will halt
BAD_TRAP:
    j TRAP_HALT

;;; BAD_INT - code to execute for undefined interrupt. It will halt
BAD_INT: 
    j TRAP_HALT               ; This is an undefined instruction. It will halt and reset to the entry point

;;; EXCEPTIONS
;;; EXCEPTION_ILLEGAL_OPCODE - code to execute for illegal opcode. It will halt (because of the emulator)
EXCEPTION_ILLEGAL_OPCODE:
    addi a0 zero 0
    la a0 illegal_opcode_msg    ; Load the prompt string into a0
    addi x5 x5 $100             ; print in the next line
    trap zero VECTOR_TRAP_PUTS  ; Output the prompt to the video port
    addi x5 x5 $100             ; print in the next line
    beqz zero 0                 ; jump to self infinite loop

;;; EXCEPTION_BAD_PRIVILEGE - code to execute for bad privilege. It will NOT halt 
EXCEPTION_BAD_PRIVILEGE:
    j greet                 ; resumes the system thread to the BIOS

;;; INTERRUPTS
;;; INTERRUPT_KEYBOARD - code to execute for keyboard interrupt (keypress, not ^C). It will NOT halt MOST IMPORTANT BIOS TRAP
INTERRUPT_KEYBOARD: ; NOTE THE KERNEL SHOUD REDEFINE THIS INTERRUPT TO STORE THE CHARACTER IN A RING BUFFER IN CACHE
    push t6 sp                  ; we will use t6 to store the keypress
    push t5 sp                  ; we will use t5 some variables for now
    lhc t6 zero PORT_KB_INPUT   ; load the character from the keyboard port
    lbg t5 getc_requested       ; load getc_requested variable into t5
    beqz t5 +8                  ; if getc_requested is zero, skip the next instruction
    mv a0 t6
    pop t5 sp
    pop t6 sp                   ; restore t6
    rti

;;; TRAPS

TRAP_GETC: 
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
    sbc a0 a1 0                 ; store the character in the desired location
    trap zero VECTOR_TRAP_PUTC  ; invoke the trap to output the character to the video port
    rti

;DATA SECTION: THIS CAN BE (AND IT IS EXPECTED) TO BE OVERWRITTEN BY THE KERNEL
    .section data
getc_requested:   
    .byte 0
    .align 4

greet_msg: 
    .string "Talea System minimal BIOS 0.1"
    .byte 0
    .align 4

no_bootable_msg:
    .string "No bootable media found."
    .byte 10 ;newline                           
    .string "Please insert a bootable medium and press any key to continue..."
    .byte 0
    .align 4

illegal_opcode_msg:
    .string "Exception: Illegal Opcode"
    .byte 0
    .align 4

;Supervisor stack (grow from high to low) KERNEL SHOULD RELOCATE THIS
    .dblock 1020
ss_start:   
    .dword 0    ; start of the supervisor stack