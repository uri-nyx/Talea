_start:
    addi    %x1, %x2, #-1000 ; OP_ADDI
    mv      %x1, %x2 ; OP_MV
    ori     %x1, %x2, #-1000 ; OP_ORI
    xori    %x1, %x2, #-1000 ; OP_XORI
    slti    %x1, %x2, #-1000 ; OP_SLTI
    sltiu   %x1, %x2, #-1000 ; OP_SLTIU
    shlli    %x1, %x2, #-1000 ; OP_SLLI
    shrli    %x1, %x2, #-1000 ; OP_SRLI
    shrai    %x1, %x2, #-1000 ; OP_SRAI
    mulih   %x1, %x2, #-1000 ; OP_MULIH
    muli    %x1, %x2, #-1000 ; OP_MULI
    idivi   %x1, %x2, #-1000 ; OP_DIVI 

    addi    %x1, %x2, $1000 ; OP_ADDI
    ori     %x1, %x2, $1000 ; OP_ORI
    xori    %x1, %x2, $1000 ; OP_XORI
    slti    %x1, %x2, $1000 ; OP_SLTI
    sltiu   %x1, %x2, $1000 ; OP_SLTIU
    shlli    %x1, %x2, $1000 ; OP_SLLI
    shrli    %x1, %x2, $1000 ; OP_SRLI
    shrai    %x1, %x2, $1000 ; OP_SRAI
    mulih   %x1, %x2, $1000 ; OP_MULIH
    muli    %x1, %x2, $1000 ; OP_MULI
    idivi   %x1, %x2, $1000 ; OP_DIVI

; We are explicitlly not supporting this for now
    ; addi    %x1, %x2, _start ; OP_ADDI
    ; ori     %x1, %x2, _start ; OP_ORI
    ; xori    %x1, %x2, _start ; OP_XORI
    ; slti    %x1, %x2, _start ; OP_SLTI
    ; sltiu   %x1, %x2, _start ; OP_SLTIU
    ; shlli    %x1, %x2, _start ; OP_SLLI
    ; shrli    %x1, %x2, _start ; OP_SRLI
    ; shrai    %x1, %x2, _start ; OP_SRAI
    ; mulih   %x1, %x2, _start ; OP_MULIH
    ; muli    %x1, %x2, _start ; OP_MULI
    ; idivi   %x1, %x2, _start ; OP_DIVI
