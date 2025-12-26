_start:
    ;j 100
    ;j -100
    ;j 100
    j _end
    
    ;jal x1, 100
    ;jal x1, -100
    ;jal x1, 100
    jal x1, _end
    jal _end

_end:    
