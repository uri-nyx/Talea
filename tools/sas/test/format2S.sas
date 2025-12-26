_start:
    sb      %x1, #100(%x2) ; OP_SB
    sh      %x1, #100(%x2) ; OP_SH
    sw      %x1, #100(%x2) ; OP_SW
    sbd     %x1, #100(%x2) ; OP_SBD
    shd     %x1, #100(%x2) ; OP_SHD
    swd     %x1, #100(%x2) ; OP_SWD

    sb      %x1, _start, %x2 ; OP_SB
    sh      %x1, _start, %x2 ; OP_SH
    sw      %x1, _start, %x2 ; OP_SW
    sbd     %x1, _start, %x2 ; OP_SBD
    shd     %x1, _start, %x2 ; OP_SHD
    swd     %x1, _start, %x2 ; OP_SWD
    