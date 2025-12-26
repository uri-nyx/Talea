_start:
    ;j #100  ; works here, but not in the other assembler
    ;j #-100 ; works here, but not in the other assembler
    ;j $100 ; works here, but not in the other assembler
    j _end
    
    ;jal %x1, #100 ; works here, but not in the other assembler
    ;jal %x1, #-100 ; works here, but not in the other assembler
    ;jal %x1, $100 ; works here, but not in the other assembler
    jal %x1, _end
    jal _end
_end:    
