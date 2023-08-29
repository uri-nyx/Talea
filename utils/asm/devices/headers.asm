; Headers for device drivers of the Taleä System
#once

; KEYBOARD
; Device location in Data Memory
KBD  = __DEV_BASE + 0x000c

; Registers
K_MOD   = KBD + 0x00
K_CHAR  = KBD + 0x01
k_CODE  = KBD + 0x02

; Inline Functions

#ruledef {
	; TIMEOUT
	__asm_inline_kbd_get_modifiers => asm {
		lbud a0, K_MOD(r __DEV_BASE_REG)
	}
	__asm_inline_kbd_get_char => asm {
		lbud a0, K_CHAR(r __DEV_BASE_REG)
	}
	__asm_inline_kbd_get_key => asm {
		lhud a0, K_CODE(r __DEV_BASE_REG)
	}
}

; STORAGE
; Device location in Data Memory
TPS  = __DEV_BASE + 0x0020
DSK  = __DEV_BASE + 0x0026

; TPS Registers
TPS_COMMAND = TPS + 0x00
TPS_DATA    = TPS + 0x01
TPS_POINTH  = TPS + 0x02
TPS_POINTL  = TPS + 0x03
TPS_STATUSH = TPS + 0x04
TPS_STATUSL = TPS + 0x05
TPS_A = 0
TPS_B = 1

; DISK Registers
DISK_COMMAND = DSK + 0x00
DISK_DATA    = DSK + 0x01
DISK_SECTORH = DSK + 0x02
DISK_SECTORL = DSK + 0x03
DISK_POINTH  = DSK + 0x04
DISK_POINTL  = DSK + 0x05
DISK_STATUSH = DSK + 0x06
DISK_STATUSL = DSK + 0x07

; Commands
DT_nop         = 0x00
DT_store       = 0x01
DT_load        = 0x02
DT_isBootable  = 0x03
TPS_open       = 0x04
TPS_close      = 0x05
TPS_setCurrent = 0x06

; Inline Functions

#ruledef {
	; DISK
	__asm_inline_disk_nop => asm {
		sbd zero, DISK_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_disk_store => asm {
		; byte a0 is drive, 
		; half a1 is sector, 
		; half a2 is point
		sbd a0, DISK_DATA(r __DEV_BASE_REG)
		shd a1, DISK_SECTORH(r __DEV_BASE_REG)
		shd a2, DISK_POINTH(r __DEV_BASE_REG)
		li a0, DT_store
		sbd a0, DISK_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_disk_load => asm {
		; byte a0 is drive, 
		; half a1 is sector, 
		; half a2 is point
		sbd a0, DISK_DATA(r __DEV_BASE_REG)
		shd a1, DISK_SECTORH(r __DEV_BASE_REG)
		shd a2, DISK_POINTH(r __DEV_BASE_REG)
		li a0, DT_load
		sbd a0, DISK_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_disk_bootable => asm {
		li t0, DT_isBootable
		sbd t0, DISK_COMMAND(r __DEV_BASE_REG)
		lbd a1, DISK_STATUSL(r __DEV_BASE_REG)
		andi a0, a1, 1
	}

	; TPS
	__asm_inline_tps_nop => asm {
		sbd zero, TPS_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_tps_store => asm {
		; byte a0 is sector, 
		; half a1 is point, 
		sbd a0, TPS_DATA(r __DEV_BASE_REG)
		shd a1, TPS_POINTH(r __DEV_BASE_REG)
		li a0, DT_store
		sbd a0, TPS_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_tps_load => asm {
		; byte a0 is sector, 
		; half a1 is point, 
		sbd a0, TPS_DATA(r __DEV_BASE_REG)
		shd a1, TPS_POINTH(r __DEV_BASE_REG)
		li a0, DT_load
		sbd a0, TPS_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_tps_bootable => asm {
		li t0, DT_isBootable
		sbd t0, TPS_COMMAND(r __DEV_BASE_REG)
		lbd a1, TPS_STATUSL(r __DEV_BASE_REG)
		andi a0, a1, 1
	}	
	__asm_inline_tps_open => asm {
		li t0, TPS_open
		sbd t0, TPS_COMMAND(r __DEV_BASE_REG)
	}	
	__asm_inline_tps_close => asm {
		li t0, TPS_close
		sbd t0, TPS_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_tps_set_a => asm {
		li t0, TPS_A
		sbd t0, TPS_DATA(r __DEV_BASE_REG)
		li t0, TPS_setCurrent
		sbd t0, TPS_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_tps_set_b => asm {
		li t0, TPS_B
		sbd t0, TPS_DATA(r __DEV_BASE_REG)
		li t0, TPS_setCurrent
		sbd t0, TPS_COMMAND(r __DEV_BASE_REG)
	}

}

; TIMER


; Device location in Data Memory
TIM  = __DEV_BASE + 0x0006

; Registers
TIM_TIMEOUT  = TIM + 0
TIM_INTERVAL = TIM + 2
TIM_FREQ_T   = TIM + 4
TIM_FREQ_I   = TIM + 5

; Inline Functions

#ruledef {
	; TIMEOUT
	__asm_inline_timer_set_timeout => asm {
		shd a0, TIM_TIMEOUT(r __DEV_BASE_REG)
	}
	__asm_inline_timer_get_timeout => asm {
		lhud a0, TIM_TIMEOUT(r __DEV_BASE_REG)
	}
	__asm_inline_timer_set_timeout_freq => asm {
		shd a1, TIM_FREQ_T(r __DEV_BASE_REG)
	}
	__asm_inline_timer_get_timeout_freq => asm {
		lhud a1, TIM_FREQ_T(r __DEV_BASE_REG)
	}

	; INTERVAL
	__asm_inline_timer_set_interval => asm {
		shd a0, TIM_INTERVAL(r __DEV_BASE_REG)
	}
	__asm_inline_timer_get_interval => asm {
		lhud a0, TIM_INTERVAL(r __DEV_BASE_REG)
	}
	__asm_inline_timer_set_interval_freq => asm {
		shd a1, TIM_FREQ_I(r __DEV_BASE_REG)
	}
	__asm_inline_timer_get_interval_freq => asm {
		lhud a1, TIM_FREQ_I(r __DEV_BASE_REG)
	}
}

; TTY
; Device location in Data Memory
TTY  = __DEV_BASE + 0x0000

; Registers
T_RX    = TTY + 0x00
T_RXLEN = TTY + 0x01
T_TX    = TTY + 0x02
T_STAT  = TTY + 0x04
T_CTRL  = TTY + 0x05

; Modes
T_mode_LIFO = 0
T_mode_FIFO = 1

; Inline Functions

#ruledef {
	; TIMEOUT
	__asm_inline_tty_send => asm {
		sbd a0, T_TX(r __DEV_BASE_REG)
	}
	__asm_inline_tty_input_len => asm {
		lbud a0, T_RXLEN(r __DEV_BASE_REG)
	}
	__asm_inline_tty_set_mode => asm {
		sbd a0, T_CTRL(r __DEV_BASE_REG)
	}
	__asm_inline_tty_receive_one => asm {
		lbd a0, T_TX(r __DEV_BASE_REG)
	}
	__asm_inline_tty_receive_one_unsigned => asm {
		lbud a0, T_TX(r __DEV_BASE_REG)
	}
}

; VIDEO

; Device location in Data Memory
VDO  = __DEV_BASE + 0x0010

; Registers
V_COMMAND = VDO + 0x0
V_DATAH   = VDO + 0x1
V_DATAM   = VDO + 0x2
V_DATAL   = VDO + 0x3
V_GPU0    = VDO + 0x4
V_GPU1    = VDO + 0x5
V_GPU2    = VDO + 0x6
V_GPU3    = VDO + 0x7
V_GPU4    = VDO + 0x8
V_GPU5    = VDO + 0x9
V_GPU6    = VDO + 0xa
V_GPU7    = VDO + 0xb
V_STATUS0 = VDO + 0xc
V_STATUS1 = VDO + 0xd
V_STATUS2 = VDO + 0xe
V_STATUS3 = VDO + 0xf


; Commands
V_nop           = 0x0
V_clear         = 0x1
V_setmode       = 0x2
V_setfont       = 0x4
V_blit          = 0x6
V_setfb         = 0x7
V_setvblank     = 0x8
V_loadfont      = 0x9
V_loadpalette   = 0xa
V_setbg         = 0xb
V_setfg         = 0xc
V_clearregs     = 0xd

; Inline Functions

#ruledef {
	; TIMEOUT
	__asm_inline_video_nop => asm {
        sbd zero, V_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_video_clear => asm {
        li t0, V_clear
        sbd t0, V_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_video_clearregs => asm {
        li t0, V_clearregs
        sbd t0, V_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_video_setmode => asm {
        sbd a0, V_DATAH(r __DEV_BASE_REG)
        li t0, V_setmode
        sbd t0, V_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_video_setfont => asm {
        sbd a0, V_DATAH(r __DEV_BASE_REG)
        li t0, V_setfont
        sbd t0, V_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_video_setfb => asm {
        swd a0, V_DATAH-1(r __DEV_BASE_REG)
        li t0, V_setfb
        sbd t0, V_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_video_setvblank=> asm {
        sbd a0, V_DATAH(r __DEV_BASE_REG)
        li t0, V_setvblank
        sbd t0, V_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_video_setbg => asm {
        swd a0, V_GPU0(r __DEV_BASE_REG)
        li t0, V_setbg
        sbd t0, V_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_video_setfg => asm {
        sbd t0, V_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_video_nop => asm {
        swd a0, V_GPU0(r __DEV_BASE_REG)
        li t0, V_setfg
        sbd t0, V_COMMAND(r __DEV_BASE_REG)
        sbd t0, V_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_video_loadfont => asm {
        ; a0 is address
        ; a1 is length (halfword)
        swd a0, V_DATAH-1(r __DEV_BASE_REG)
        shd a1, V_GPU0(r __DEV_BASE_REG)
        li t0, V_setfont
        sbd t0, V_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_video_loadpalette => asm {
        swd a0, V_DATAH-1(r __DEV_BASE_REG)
        li t0, V_loadpalette
        sbd t0, V_COMMAND(r __DEV_BASE_REG)
	}
	__asm_inline_video_blit => asm {
        li t0, V_blit
        sbd t0, V_COMMAND(r __DEV_BASE_REG)
	}


}