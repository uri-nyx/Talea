; These are the system constants for the Taleä Computer System
#once

NULL = 0

; DEVICES
__DEV_BASE = 0
__DEV_BASE_REG = 0
_IVT = 0xf800
_PDT = 0xff00

; Exceptions
IVT_RESET               = 0x00 * 4
IVT_BUS_ERROR           = 0x02 * 4
IVT_ADDRESS_ERROR       = 0x03 * 4
IVT_ILLEGAL_INSTRUCTION = 0x04 * 4
IVT_DIVISION_ZERO       = 0x05 * 4
IVT_PRIVILEGE_VIOLATION = 0x06 * 4
IVT_PAGE_FAULT          = 0x07 * 4
IVT_ACCESS_VIOLATION    = 0x08 * 4

; Interrupts
IVT_TTY_TRANSMIT        = 0x0a * 4
IVT_KBD_CHARACTER       = 0x0b * 4
IVT_KBD_SCANCODE        = 0x0c * 4
IVT_TPS_LOAD_FINISHED   = 0x0d * 4
IVT_DISK_LOAD_FINISHED  = 0x0e * 4
IVT_TIMER_TIMEOUT		= 0x0f * 4
IVT_TIMER_INTERVAL 		= 0x10 * 4 
IVT_VIDEO_REFRESH		= 0x11 * 4 