_start:
    jalr x1, 100(x2)
    jalr x1, -100(x2)
    jalr x1, 900(x2)
    jr   x1
    ret
_end:
