start:
    addi a0 zero $101           ; command to check if tps0 is present (0 $101)
    shc a0 zero PORT_TPS_OUT    ; send command to the tps drive
    j start