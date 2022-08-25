; Simple bootloader for TpsOs
; It starts at address 0 in the tps medium
; TODO: make a configuration for the assembler to emit PIC (it does for some reason?)

    .org 0

open_tps:
    ori a0 a0 $200              ; The command to mount the tps (opening the file). If it is forgotten, the emulator segfaults
    shc a0 zero PORT_TPS_OUT    ; We send the comand to the tps drive (the drive number should be stored on a0, per the BIOS)
    addi a0 a0 -512             ; we clear the command without alteringthe high byte

load:
    ; For testing purposes, I will load the next sector and start it's program as if it was a FILE
    ori a0 a0 $501              ; The command to load sector 1. The high byte of a0 is loaded with the Tps Reader id (0 or 0x8)
    addi x6 zero $7             ; We want to mount the sector at mounting point 7 (address $e00)
    shc a0 zero PORT_TPS_OUT    ; send command to the tps drive

start:
    ; Entry point of executable test file is at, let's say, byte 16 of the file
    addi t6 zero $700           ; mounting point 7
    add t6 t6 t6                ; double that (to give $e00)
    jalr ra t6 16               ; jump to the program (we should swich to user mode, but this is suposed to be the kernel)
    beqz zero 0                 ; infinite loop