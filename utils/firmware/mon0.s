; Mon0 is a very simple (text-mode) machine code monitor for 
; the Taleä System. It is akin to Wozmon and inspired by it, 
; but obviously less ingenious and clever.
; Provides basic machine code and inspecting fucntionality, and
; a boot routine (command B), that searches the drives for booable
; sectors, loads them at address 0x1000 and jumps to them.

;------------------------------------------------------------------------------
; Utility BIOS routines in Assembly
;------------------------------------------------------------------------------

