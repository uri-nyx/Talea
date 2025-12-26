_start:
    not     %x1, %x2 ; OP_NOT
    ctz     %x1, %x2 ; OP_CTZ
    clz     %x1, %x2 ; OP_CLZ
    pcount  %x1, %x2 ; OP_PCNT
    thro    %x1, %x2 ; OP_THRO
    from    %x1, %x2 ; OP_FROM
    popb    %x1, %x2 ; OP_POPB
    poph    %x1, %x2 ; OP_POPH
    pop     %x1, %x2 ; OP_POP
    pushb   %x1, %x2 ; OP_PUSHB
    pushh   %x1, %x2 ; OP_PUSHH
    push    %x1, %x2 ; OP_PUSH
    exch    %x1, %x2 ; OP_EXCH
    