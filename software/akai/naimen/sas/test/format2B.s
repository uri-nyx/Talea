_start:
    beq     x1, x2, 100 ; OP_BEQ
    bne     x1, x2, 100 ; OP_BNE
    ble     x1, x2, 100 ; OP_BLE
    bleu    x1, x2, 100 ; OP_BLEU
    blt     x1, x2, 100 ; OP_BLT
    bltu    x1, x2, 100 ; OP_BLTU
    bge     x1, x2, 100 ; OP_BGE
    bgeu    x1, x2, 100 ; OP_BGEU
    bgt     x1, x2, 100 ; OP_BGT
    bgtu    x1, x2, 100 ; OP_BGTU

    beq     x1, x2, -100 ; OP_BEQ
    bne     x1, x2, -100 ; OP_BNE
    ble     x1, x2, -100 ; OP_BLE
    bleu    x1, x2, -100 ; OP_BLEU
    blt     x1, x2, -100 ; OP_BLT
    bltu    x1, x2, -100 ; OP_BLTU
    bge     x1, x2, -100 ; OP_BGE
    bgeu    x1, x2, -100 ; OP_BGEU
    bgt     x1, x2, -100 ; OP_BGT
    bgtu    x1, x2, -100 ; OP_BGTU

    beq     x1, x2, _start ; OP_BEQ
    bne     x1, x2, _start ; OP_BNE
    ble     x1, x2, _start ; OP_BLE
    bleu    x1, x2, _start ; OP_BLEU
    blt     x1, x2, _start ; OP_BLT
    bltu    x1, x2, _start ; OP_BLTU
    bge     x1, x2, _start ; OP_BGE
    bgeu    x1, x2, _start ; OP_BGEU
    bgt     x1, x2, _start ; OP_BGT
    bgtu    x1, x2, _start ; OP_BGTU
    