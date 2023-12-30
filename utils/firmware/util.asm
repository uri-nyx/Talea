; utility functions to read from disk and print to screen
; a very barebones BIOS

    PUTS = 100
    TOBIGENDIAN16 = 101
    CHANGEVIDEOMODE = 102
    TPSLOADSECTOR = 103
    TPSLOADSECTORS = 104
    j INITIALIZE_BIOS

; prints rich text
; a0: *char (null terminated str)
; a1: char  (colors to use)
puts:
    li  t1, V_CHARBUFFER       ; load charbuffer address
    lw  t2, cursor(zero)       ; load cursor position
    add t1, t2, t1             ; get position in charbuffer
    lbu  t0, 0(a0)             ; load char from string
    sb   t0, 0(t1)             ; else send it to charbuffer
    sb   a1, 1(t1)             ; send the color and blink config
    .loop:
    lbu  t0, 0(a0)             ; load char from string
    beqz t0, .end              ; if == 0, end printing
    sb   t0, 0(t1)             ; else send it to charbuffer
    sb   a1, 1(t1)             ; send the color and blink config
    addi t1, t1, 2             ; inc cursor
    addi a0, a0, 1             ; inc str address
    j .loop
    .end:
    li  t2, V_CHARBUFFER       ; update cursor new position
    sub t1, t1, t2
    sw  t1, cursor(zero)
    sysret
cursor: #d32 0

; returns the big endian of a 16 bit number in a0
; a0: short (the destination)
; a1: short (the number to convert)
toBigEndian16:
    shlli a0, a1, 8
    shrli a1, a1, 8
    or a0, a0, a1
    li a1, 0xffff
    and a0, a0, a1
    sysret

; changes the video mode
; a0: char (new video mode)
changeVideoMode:
; change video mode to combined text and graphics
    lwd t0, video_device(zero)
    li a0, 0x5
    sbd a0, V_DATAH(t0)
    li a0, V_setmode
    sbd a0, V_COMMAND(t0)
    sysret

; loads a sector from tps
; a0: char  (sector)
; a1: short (point to load in)
TpsALoadSector:
    lwd t0, drive_device(zero)
    sbd a0, TPS_DATA(t0)
    shd a1, TPS_POINTH(t0)
    li a0, Tps_load
    sbd a0, TPS_COMMAND(t0)
    sysret

; loads a sectors from tps
; a0: char  (start sector)
; a1: char  (number of sectors)
; a2: *void (buffer to load in (aligned to 512 bytes))
TpsALoadSectors:
    beqz a1, .end
    lwd t0, drive_device(zero)
    shlli a2, a2, 9
    li t1, Tps_load

    .again:
    sbd a0, TPS_DATA(t0)
    shd a2, TPS_POINTH(t0)
    sbd t1, TPS_COMMAND(t0)
    beqz a1, .end
    addi a2, a2, 1
    addi a0, a0, 1
    addi a1, a1, -1
    j .again

    .end:
    sysret

    INITIALIZE_BIOS:
	la a1, _IVT
	la a0, puts
	swd a0, 100(a1)
	la a0, toBigEndian16
    swd a0, 101(a1)
	la a0, changeVideoMode
    swd a0, 102(a1)
	la a0, TpsALoadSector
    swd a0, 103(a1)
	la a0, TpsALoadSectors
