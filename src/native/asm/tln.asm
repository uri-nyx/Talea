   .org #x400
 call enter
 call end
id_+    pop r4
    pop r3
    str r3 +_return_addrL
    str r4 +_return_addrH
   pop r4
   str r4 param_+_a
   pop r4
   str r4 param_+_b
sum ldr acc param_+_a
 adc param_+_b 
    ldr r4  +_return_addrH
   push r4
    ldr r4  +_return_addrL
   push r4
  ret
enter   nop
 ldr r4 #q2
 push r4
 ldr r4 #q1
 push r4
    call id_+
end    .end
+_return_addrL .byte #q0
+_return_addrH .byte #q0
param_+_a   .byte #q0
param_+_b   .byte #q0