; This is the most minimal and barebones firmware for the Taleä system.
; It initializes the text mode and tries to boot from TPS A. On failure,
; it powers off.

drive_device = 0x110
video_device = 0x114
stk_device   = 0x118

#addr 0

; disable interrups ;TODO: make a rule for this
; supervisor, intterupt disabled, mmu disabled priority 7, ivt at 0xf800
li  a0, 0b1_0_0_111_111110_11111111_000000000000
ssreg a0

li a1, "0" ; Error code

; get the video device address
lbud a0, SYS_DEVICES(zero)   ; number of devices installed
addi a0, a0, DEV_MAP
li t0, DEV_MAP               ; the devices are registered at DEV_MAP
li t1, "V"                   ; Video device ID is 'V'
search_v:
    blt a0, t0, error.e0       ; if ndevices < index, video was not found
    lbud t2, 0(t0)
    addi t0, t0, 1
bne t1, t2, search_v

addi t0, t0, (-DEV_MAP - 1) ; found video index in table
shlli t0, t0, 4             ; mapping index to installed device (index << 4)
swd t0, video_device(zero)   ; storing the device address for bookeeping

; initialize text mode      ; the text mode is 0, so no need to issue it
V_set_mode = 0x2            ; command to set mode
V_COMMAND  = 0              ; command register    
CHARBUFFER = 0xe5_10_00
li  t1,  V_set_mode
sbd t1, V_COMMAND(t0)

; get the STK device address
li t0, DEV_MAP               ; the devices are registered at DEV_MAP
li t1, "K"                   ; Disk drive device ID is 'D'
search_s:
    blt a0, t0, error.e1       ; if ndevices < index, drive was not found
    lbud t2, 0(t0)
    addi t0, t0, 1
bne t1, t2, search_s

addi t0, t0, (-DEV_MAP - 1) ; found drive index in table
shlli t0, t0, 4             ; mapping index to installed device (index << 4)
swd t0, stk_device(zero)    ; storing the device address for bookeeping

; get the drive device address
li t0, DEV_MAP               ; the devices are registered at DEV_MAP
li t1, "D"                   ; Disk drive device ID is 'D'
search_d:
    blt a0, t0, error.e2       ; if ndevices < index, drive was not found
    lbud t2, 0(t0)
    addi t0, t0, 1
bne t1, t2, search_d

addi t0, t0, (-DEV_MAP - 1) ; found drive index in table
shlli t0, t0, 4             ; mapping index to installed device (index << 4)
swd t0, drive_device(zero)   ; storing the device address for bookeeping


; query for bootable tps A   
D_set_current = 0x7            ; command to set tps
D_is_bootable = 0x3            ; command to query bootability
D_is_present  = 0x4            ; command to check if tps is present
D_load_sector = 0x2            ; command to load sector
D_COMMAND     = 0              ; command register               
D_DATA        = 1              ; data register               
D_POINTH      = 2              ; point register               
D_POINTL      = 3              ; point register               
D_STATUSH     = 4              ; status register               
D_STATUSL     = 5              ; status register 

li  t1,  D_set_current
sbd t1, D_COMMAND(t0)          ; Set current 0 (A)

li t1, D_is_present            ; 
sbd t1, D_COMMAND(t0)          ; test if is present
lbud t1, D_STATUSL(t0)         ; result of present (1 true)
beq  t1, zero, error.e3        ; if not present, error

li t1, D_is_bootable           ; 
sbd t1, D_COMMAND(t0)          ; test for bootability
lbud t1, D_STATUSL(t0)         ; result of bootabilty (1 true)
beq  t1, zero, error.e4       ; if not bootable, error

li t1, (0x1000 >> 9)           ; the point to load the sector (0x1000)
sbd t1, D_POINTL(t0)
sbd zero, D_DATA(t0)
li t1, D_load_sector
sbd t1, D_COMMAND(t0)          ; load sector 0 (boot sector)
lw t0, 0x1000(zero)
j 0x1000                       ; jump to boot sector


error:
.e4:
    addi a1, a1, 1
.e3:
    addi a1, a1, 1
.e2:
    addi a1, a1, 1
.e1:
    addi a1, a1, 1
    li   t0, CHARBUFFER
    sb   a1, 0(t0)
.e0: 
    trace a1, zero, zero, zero

poweroff:
j $
sbd zero, SYS_POWER(zero)      ; power off the system