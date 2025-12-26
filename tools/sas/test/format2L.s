_start:
    lb      x1, 100(x2); OP_LB
    lbd     x1, 100(x2); OP_LBD
    lh      x1, 100(x2); OP_LH
    lhd     x1, 100(x2); OP_LHD
    lw      x1, 100(x2); OP_LW
    lwd     x1, 100(x2); OP_LWD
    lbu     x1, 100(x2); OP_LBU
    lbud    x1, 100(x2); OP_LBUD
    lhu     x1, 100(x2); OP_LHU
    lhud    x1, 100(x2); OP_LHUD

    lb      x1, _start; OP_LB
    lbd     x1, _start; OP_LBD
    lh      x1, _start; OP_LH
    lhd     x1, _start; OP_LHD
    lw      x1, _start; OP_LW
    lwd     x1, _start; OP_LWD
    lbu     x1, _start; OP_LBU
    lbud    x1, _start; OP_LBUD
    lhu     x1, _start; OP_LHU
    lhud    x1, _start; OP_LHUD
    