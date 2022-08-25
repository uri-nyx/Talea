    .org 0
; the TRAP vector table
    jal zero $400    ; vectors 0  and  1 are reserved for the jump to the system entry point at 1k (1024 bytes)
    ;.word BAD_TRAP  ; x00
    ;.word BAD_TRAP  ; x01
    .word BAD_TRAP  ; x02
    .word BAD_TRAP  ; x03
    .word BAD_TRAP  ; x04
    .word BAD_TRAP  ; x05
    .word BAD_TRAP  ; x06
    .word BAD_TRAP  ; x07
    .word BAD_TRAP  ; x08
    .word BAD_TRAP  ; x09
    .word BAD_TRAP  ; x0A
    .word BAD_TRAP  ; x0B
    .word BAD_TRAP  ; x0C
    .word BAD_TRAP  ; x0D
    .word BAD_TRAP  ; x0E
    .word BAD_TRAP  ; x0F
    .word BAD_TRAP  ; x10
    .word BAD_TRAP  ; x11
    .word BAD_TRAP  ; x12
    .word BAD_TRAP  ; x13
    .word BAD_TRAP  ; x14
    .word BAD_TRAP  ; x15
    .word BAD_TRAP  ; x16
    .word BAD_TRAP  ; x17
    .word BAD_TRAP  ; x18
    .word BAD_TRAP  ; x19
    .word BAD_TRAP  ; x1A
    .word BAD_TRAP  ; x1B
    .word BAD_TRAP  ; x1C
    .word BAD_TRAP  ; x1D
    .word BAD_TRAP  ; x1E
    .word BAD_TRAP  ; x1F
    .word TRAP_GETC ; x20
    .word TRAP_PUTC ; x21
    .word TRAP_PUTS ; x22
    .word TRAP_IN   ; x23
    .word TRAP_HALT ; x24        
    .word BAD_TRAP  ;TRAP_SYSCALL  ; x25
    .word BAD_TRAP  ; x26
    .word BAD_TRAP  ; x27
    .word BAD_TRAP  ; x28
    .word BAD_TRAP  ; x29
    .word BAD_TRAP  ; x2A
    .word BAD_TRAP  ; x2B
    .word BAD_TRAP  ; x2C
    .word BAD_TRAP  ; x2D
    .word BAD_TRAP  ; x2E
    .word BAD_TRAP  ; x2F
    .word BAD_TRAP  ; x30
    .word BAD_TRAP  ; x31
    .word BAD_TRAP  ; x32
    .word BAD_TRAP  ; x33
    .word BAD_TRAP  ; x34
    .word BAD_TRAP  ; x35
    .word BAD_TRAP  ; x36
    .word BAD_TRAP  ; x37
    .word BAD_TRAP  ; x38
    .word BAD_TRAP  ; x39
    .word BAD_TRAP  ; x3A
    .word BAD_TRAP  ; x3B
    .word BAD_TRAP  ; x3C
    .word BAD_TRAP  ; x3D
    .word BAD_TRAP  ; x3E
    .word BAD_TRAP  ; x3F
    .word BAD_TRAP  ; x40
    .word BAD_TRAP  ; x41
    .word BAD_TRAP  ; x42
    .word BAD_TRAP  ; x43
    .word BAD_TRAP  ; x44
    .word BAD_TRAP  ; x45
    .word BAD_TRAP  ; x46
    .word BAD_TRAP  ; x47
    .word BAD_TRAP  ; x48
    .word BAD_TRAP  ; x49
    .word BAD_TRAP  ; x4A
    .word BAD_TRAP  ; x4B
    .word BAD_TRAP  ; x4C
    .word BAD_TRAP  ; x4D
    .word BAD_TRAP  ; x4E
    .word BAD_TRAP  ; x4F
    .word BAD_TRAP  ; x50
    .word BAD_TRAP  ; x51
    .word BAD_TRAP  ; x52
    .word BAD_TRAP  ; x53
    .word BAD_TRAP  ; x54
    .word BAD_TRAP  ; x55
    .word BAD_TRAP  ; x56
    .word BAD_TRAP  ; x57
    .word BAD_TRAP  ; x58
    .word BAD_TRAP  ; x59
    .word BAD_TRAP  ; x5A
    .word BAD_TRAP  ; x5B
    .word BAD_TRAP  ; x5C
    .word BAD_TRAP  ; x5D
    .word BAD_TRAP  ; x5E
    .word BAD_TRAP  ; x5F
    .word BAD_TRAP  ; x60
    .word BAD_TRAP  ; x61
    .word BAD_TRAP  ; x62
    .word BAD_TRAP  ; x63
    .word BAD_TRAP  ; x64
    .word BAD_TRAP  ; x65
    .word BAD_TRAP  ; x66
    .word BAD_TRAP  ; x67
    .word BAD_TRAP  ; x68
    .word BAD_TRAP  ; x69
    .word BAD_TRAP  ; x6A
    .word BAD_TRAP  ; x6B
    .word BAD_TRAP  ; x6C
    .word BAD_TRAP  ; x6D
    .word BAD_TRAP  ; x6E
    .word BAD_TRAP  ; x6F
    .word BAD_TRAP  ; x70
    .word BAD_TRAP  ; x71
    .word BAD_TRAP  ; x72
    .word BAD_TRAP  ; x73
    .word BAD_TRAP  ; x74
    .word BAD_TRAP  ; x75
    .word BAD_TRAP  ; x76
    .word BAD_TRAP  ; x77
    .word BAD_TRAP  ; x78
    .word BAD_TRAP  ; x79
    .word BAD_TRAP  ; x7A
    .word BAD_TRAP  ; x7B
    .word BAD_TRAP  ; x7C
    .word BAD_TRAP  ; x7D
    .word BAD_TRAP  ; x7E
    .word BAD_TRAP  ; x7F
    .word BAD_TRAP  ; x80
    .word BAD_TRAP  ; x81
    .word BAD_TRAP  ; x82
    .word BAD_TRAP  ; x83
    .word BAD_TRAP  ; x84
    .word BAD_TRAP  ; x85
    .word BAD_TRAP  ; x86
    .word BAD_TRAP  ; x87
    .word BAD_TRAP  ; x88
    .word BAD_TRAP  ; x89
    .word BAD_TRAP  ; x8A
    .word BAD_TRAP  ; x8B
    .word BAD_TRAP  ; x8C
    .word BAD_TRAP  ; x8D
    .word BAD_TRAP  ; x8E
    .word BAD_TRAP  ; x8F
    .word BAD_TRAP  ; x90
    .word BAD_TRAP  ; x91
    .word BAD_TRAP  ; x92
    .word BAD_TRAP  ; x93
    .word BAD_TRAP  ; x94
    .word BAD_TRAP  ; x95
    .word BAD_TRAP  ; x96
    .word BAD_TRAP  ; x97
    .word BAD_TRAP  ; x98
    .word BAD_TRAP  ; x99
    .word BAD_TRAP  ; x9A
    .word BAD_TRAP  ; x9B
    .word BAD_TRAP  ; x9C
    .word BAD_TRAP  ; x9D
    .word BAD_TRAP  ; x9E
    .word BAD_TRAP  ; x9F
    .word BAD_TRAP  ; xA0
    .word BAD_TRAP  ; xA1
    .word BAD_TRAP  ; xA2
    .word BAD_TRAP  ; xA3
    .word BAD_TRAP  ; xA4
    .word BAD_TRAP  ; xA5
    .word BAD_TRAP  ; xA6
    .word BAD_TRAP  ; xA7
    .word BAD_TRAP  ; xA8
    .word BAD_TRAP  ; xA9
    .word BAD_TRAP  ; xAA
    .word BAD_TRAP  ; xAB
    .word BAD_TRAP  ; xAC
    .word BAD_TRAP  ; xAD
    .word BAD_TRAP  ; xAE
    .word BAD_TRAP  ; xAF
    .word BAD_TRAP  ; xB0
    .word BAD_TRAP  ; xB1
    .word BAD_TRAP  ; xB2
    .word BAD_TRAP  ; xB3
    .word BAD_TRAP  ; xB4
    .word BAD_TRAP  ; xB5
    .word BAD_TRAP  ; xB6
    .word BAD_TRAP  ; xB7
    .word BAD_TRAP  ; xB8
    .word BAD_TRAP  ; xB9
    .word BAD_TRAP  ; xBA
    .word BAD_TRAP  ; xBB
    .word BAD_TRAP  ; xBC
    .word BAD_TRAP  ; xBD
    .word BAD_TRAP  ; xBE
    .word BAD_TRAP  ; xBF
    .word BAD_TRAP  ; xC0
    .word BAD_TRAP  ; xC1
    .word BAD_TRAP  ; xC2
    .word BAD_TRAP  ; xC3
    .word BAD_TRAP  ; xC4
    .word BAD_TRAP  ; xC5
    .word BAD_TRAP  ; xC6
    .word BAD_TRAP  ; xC7
    .word BAD_TRAP  ; xC8
    .word BAD_TRAP  ; xC9
    .word BAD_TRAP  ; xCA
    .word BAD_TRAP  ; xCB
    .word BAD_TRAP  ; xCC
    .word BAD_TRAP  ; xCD
    .word BAD_TRAP  ; xCE
    .word BAD_TRAP  ; xCF
    .word BAD_TRAP  ; xD0
    .word BAD_TRAP  ; xD1
    .word BAD_TRAP  ; xD2
    .word BAD_TRAP  ; xD3
    .word BAD_TRAP  ; xD4
    .word BAD_TRAP  ; xD5
    .word BAD_TRAP  ; xD6
    .word BAD_TRAP  ; xD7
    .word BAD_TRAP  ; xD8
    .word BAD_TRAP  ; xD9
    .word BAD_TRAP  ; xDA
    .word BAD_TRAP  ; xDB
    .word BAD_TRAP  ; xDC
    .word BAD_TRAP  ; xDD
    .word BAD_TRAP  ; xDE
    .word BAD_TRAP  ; xDF
    .word BAD_TRAP  ; xE0
    .word BAD_TRAP  ; xE1
    .word BAD_TRAP  ; xE2
    .word BAD_TRAP  ; xE3
    .word BAD_TRAP  ; xE4
    .word BAD_TRAP  ; xE5
    .word BAD_TRAP  ; xE6
    .word BAD_TRAP  ; xE7
    .word BAD_TRAP  ; xE8
    .word BAD_TRAP  ; xE9
    .word BAD_TRAP  ; xEA
    .word BAD_TRAP  ; xEB
    .word BAD_TRAP  ; xEC
    .word BAD_TRAP  ; xED
    .word BAD_TRAP  ; xEE
    .word BAD_TRAP  ; xEF
    .word BAD_TRAP  ; xF0
    .word BAD_TRAP  ; xF1
    .word BAD_TRAP  ; xF2
    .word BAD_TRAP  ; xF3
    .word BAD_TRAP  ; xF4
    .word BAD_TRAP  ; xF5
    .word BAD_TRAP  ; xF6
    .word BAD_TRAP  ; xF7
    .word BAD_TRAP  ; xF8
    .word BAD_TRAP  ; xF9
    .word BAD_TRAP  ; xFA
    .word BAD_TRAP  ; xFB
    .word BAD_TRAP  ; xFC
    .word BAD_TRAP  ; xFD
    .word BAD_TRAP  ; xFE
    .word BAD_TRAP  ; xFF

; the interrupt vector table
; interrupts are not currently implemented
    .word EXCEPTION_ILLEGAL_OPCODE   ; x00
    .word EXCEPTION_BAD_PRIVILEGE   ; x01
    .word BAD_INT   ; x02
    .word BAD_INT   ; x03
    .word BAD_INT   ; x04
    .word BAD_INT   ; x05
    .word BAD_INT   ; x06
    .word BAD_INT   ; x07
    .word BAD_INT   ; x08
    .word BAD_INT   ; x09
    .word BAD_INT   ; x0A
    .word BAD_INT   ; x0B
    .word BAD_INT   ; x0C
    .word BAD_INT   ; x0D
    .word BAD_INT   ; x0E
    .word BAD_INT   ; x0F
    .word BAD_INT   ; x10
    .word BAD_INT   ; x11
    .word BAD_INT   ; x12
    .word BAD_INT   ; x13
    .word BAD_INT   ; x14
    .word BAD_INT   ; x15
    .word BAD_INT   ; x16
    .word BAD_INT   ; x17
    .word BAD_INT   ; x18
    .word BAD_INT   ; x19
    .word BAD_INT   ; x1A
    .word BAD_INT   ; x1B
    .word BAD_INT   ; x1C
    .word BAD_INT   ; x1D
    .word BAD_INT   ; x1E
    .word BAD_INT   ; x1F
    .word BAD_INT   ; x20
    .word BAD_INT   ; x21
    .word BAD_INT   ; x22
    .word BAD_INT   ; x23
    .word BAD_INT   ; x24
    .word BAD_INT   ; x25
    .word BAD_INT   ; x26
    .word BAD_INT   ; x27
    .word BAD_INT   ; x28
    .word BAD_INT   ; x29
    .word BAD_INT   ; x2A
    .word BAD_INT   ; x2B
    .word BAD_INT   ; x2C
    .word BAD_INT   ; x2D
    .word BAD_INT   ; x2E
    .word BAD_INT   ; x2F
    .word BAD_INT   ; x30
    .word BAD_INT   ; x31
    .word BAD_INT   ; x32
    .word BAD_INT   ; x33
    .word BAD_INT   ; x34
    .word BAD_INT   ; x35
    .word BAD_INT   ; x36
    .word BAD_INT   ; x37
    .word BAD_INT   ; x38
    .word BAD_INT   ; x39
    .word BAD_INT   ; x3A
    .word BAD_INT   ; x3B
    .word BAD_INT   ; x3C
    .word BAD_INT   ; x3D
    .word BAD_INT   ; x3E
    .word BAD_INT   ; x3F
    .word BAD_INT   ; x40
    .word BAD_INT   ; x41
    .word BAD_INT   ; x42
    .word BAD_INT   ; x43
    .word BAD_INT   ; x44
    .word BAD_INT   ; x45
    .word BAD_INT   ; x46
    .word BAD_INT   ; x47
    .word BAD_INT   ; x48
    .word BAD_INT   ; x49
    .word BAD_INT   ; x4A
    .word BAD_INT   ; x4B
    .word BAD_INT   ; x4C
    .word BAD_INT   ; x4D
    .word BAD_INT   ; x4E
    .word BAD_INT   ; x4F
    .word BAD_INT   ; x50
    .word BAD_INT   ; x51
    .word BAD_INT   ; x52
    .word BAD_INT   ; x53
    .word BAD_INT   ; x54
    .word BAD_INT   ; x55
    .word BAD_INT   ; x56
    .word BAD_INT   ; x57
    .word BAD_INT   ; x58
    .word BAD_INT   ; x59
    .word BAD_INT   ; x5A
    .word BAD_INT   ; x5B
    .word BAD_INT   ; x5C
    .word BAD_INT   ; x5D
    .word BAD_INT   ; x5E
    .word BAD_INT   ; x5F
    .word BAD_INT   ; x60
    .word BAD_INT   ; x61
    .word BAD_INT   ; x62
    .word BAD_INT   ; x63
    .word BAD_INT   ; x64
    .word BAD_INT   ; x65
    .word BAD_INT   ; x66
    .word BAD_INT   ; x67
    .word BAD_INT   ; x68
    .word BAD_INT   ; x69
    .word BAD_INT   ; x6A
    .word BAD_INT   ; x6B
    .word BAD_INT   ; x6C
    .word BAD_INT   ; x6D
    .word BAD_INT   ; x6E
    .word BAD_INT   ; x6F
    .word BAD_INT   ; x70
    .word BAD_INT   ; x71
    .word BAD_INT   ; x72
    .word BAD_INT   ; x73
    .word BAD_INT   ; x74
    .word BAD_INT   ; x75
    .word BAD_INT   ; x76
    .word BAD_INT   ; x77
    .word BAD_INT   ; x78
    .word BAD_INT   ; x79
    .word BAD_INT   ; x7A
    .word BAD_INT   ; x7B
    .word BAD_INT   ; x7C
    .word BAD_INT   ; x7D
    .word BAD_INT   ; x7E
    .word BAD_INT   ; x7F
    .word INTERRUPT_KEYBOARD   ; x80
    .word BAD_INT   ; x81
    .word BAD_INT   ; x82
    .word BAD_INT   ; x83
    .word BAD_INT   ; x84
    .word BAD_INT   ; x85
    .word BAD_INT   ; x86
    .word BAD_INT   ; x87
    .word BAD_INT   ; x88
    .word BAD_INT   ; x89
    .word BAD_INT   ; x8A
    .word BAD_INT   ; x8B
    .word BAD_INT   ; x8C
    .word BAD_INT   ; x8D
    .word BAD_INT   ; x8E
    .word BAD_INT   ; x8F
    .word BAD_INT   ; x90
    .word BAD_INT   ; x91
    .word BAD_INT   ; x92
    .word BAD_INT   ; x93
    .word BAD_INT   ; x94
    .word BAD_INT   ; x95
    .word BAD_INT   ; x96
    .word BAD_INT   ; x97
    .word BAD_INT   ; x98
    .word BAD_INT   ; x99
    .word BAD_INT   ; x9A
    .word BAD_INT   ; x9B
    .word BAD_INT   ; x9C
    .word BAD_INT   ; x9D
    .word BAD_INT   ; x9E
    .word BAD_INT   ; x9F
    .word BAD_INT   ; xA0
    .word BAD_INT   ; xA1
    .word BAD_INT   ; xA2
    .word BAD_INT   ; xA3
    .word BAD_INT   ; xA4
    .word BAD_INT   ; xA5
    .word BAD_INT   ; xA6
    .word BAD_INT   ; xA7
    .word BAD_INT   ; xA8
    .word BAD_INT   ; xA9
    .word BAD_INT   ; xAA
    .word BAD_INT   ; xAB
    .word BAD_INT   ; xAC
    .word BAD_INT   ; xAD
    .word BAD_INT   ; xAE
    .word BAD_INT   ; xAF
    .word BAD_INT   ; xB0
    .word BAD_INT   ; xB1
    .word BAD_INT   ; xB2
    .word BAD_INT   ; xB3
    .word BAD_INT   ; xB4
    .word BAD_INT   ; xB5
    .word BAD_INT   ; xB6
    .word BAD_INT   ; xB7
    .word BAD_INT   ; xB8
    .word BAD_INT   ; xB9
    .word BAD_INT   ; xBA
    .word BAD_INT   ; xBB
    .word BAD_INT   ; xBC
    .word BAD_INT   ; xBD
    .word BAD_INT   ; xBE
    .word BAD_INT   ; xBF
    .word BAD_INT   ; xC0
    .word BAD_INT   ; xC1
    .word BAD_INT   ; xC2
    .word BAD_INT   ; xC3
    .word BAD_INT   ; xC4
    .word BAD_INT   ; xC5
    .word BAD_INT   ; xC6
    .word BAD_INT   ; xC7
    .word BAD_INT   ; xC8
    .word BAD_INT   ; xC9
    .word BAD_INT   ; xCA
    .word BAD_INT   ; xCB
    .word BAD_INT   ; xCC
    .word BAD_INT   ; xCD
    .word BAD_INT   ; xCE
    .word BAD_INT   ; xCF
    .word BAD_INT   ; xD0
    .word BAD_INT   ; xD1
    .word BAD_INT   ; xD2
    .word BAD_INT   ; xD3
    .word BAD_INT   ; xD4
    .word BAD_INT   ; xD5
    .word BAD_INT   ; xD6
    .word BAD_INT   ; xD7
    .word BAD_INT   ; xD8
    .word BAD_INT   ; xD9
    .word BAD_INT   ; xDA
    .word BAD_INT   ; xDB
    .word BAD_INT   ; xDC
    .word BAD_INT   ; xDD
    .word BAD_INT   ; xDE
    .word BAD_INT   ; xDF
    .word BAD_INT   ; xE0
    .word BAD_INT   ; xE1
    .word BAD_INT   ; xE2
    .word BAD_INT   ; xE3
    .word BAD_INT   ; xE4
    .word BAD_INT   ; xE5
    .word BAD_INT   ; xE6
    .word BAD_INT   ; xE7
    .word BAD_INT   ; xE8
    .word BAD_INT   ; xE9
    .word BAD_INT   ; xEA
    .word BAD_INT   ; xEB
    .word BAD_INT   ; xEC
    .word BAD_INT   ; xED
    .word BAD_INT   ; xEE
    .word BAD_INT   ; xEF
    .word BAD_INT   ; xF0
    .word BAD_INT   ; xF1
    .word BAD_INT   ; xF2
    .word BAD_INT   ; xF3
    .word BAD_INT   ; xF4
    .word BAD_INT   ; xF5
    .word BAD_INT   ; xF6
    .word BAD_INT   ; xF7
    .word BAD_INT   ; xF8
    .word BAD_INT   ; xF9
    .word BAD_INT   ; xFA
    .word BAD_INT   ; xFB
    .word BAD_INT   ; xFC
    .word BAD_INT   ; xFD
    .word BAD_INT   ; xFE
    .word BAD_INT   ; xFF