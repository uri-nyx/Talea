_start:
    add     x1, x2, x3 ; OP_ADD 
    sub     x1, x2, x3 ; OP_SUB 
    shll    x1, x2, x3 ; OP_SLL 
    shrl    x1, x2, x3 ; OP_SRL 
    shra    x1, x2, x3 ; OP_SRA 
    and     x1, x2, x3 ; OP_AND 
    or      x1, x2, x3 ; OP_OR 
    xor     x1, x2, x3 ; OP_XOR 
    slt     x1, x2, x3 ; OP_SLT 
    sltu    x1, x2, x3 ; OP_SLTU 
    ror     x1, x2, x3 ; OP_ROR 
    rol     x1, x2, x3 ; OP_ROL
    save    x1, x2, x3 ; OP_SAVE
    restore x1, x2, x3 ; OP_RESTORE
    