        .org #0x300
        ldr r4 #q2
        push r4
        ldr r4 #q1
        push r4
        call id_+
id_+    pop r4
        pop r3
        pop r4
        str r4 param_+_a
        pop r4
        str r3 param_+_b
sum     ldr acc param_+_a
        adc param_+_b 
        ret
end   .end
param_+_b   .byte #q0
param_+_a   .byte #q0