   .org #x400
 call enter
 call end
id_:=    pop r4
    pop r3
    str r3 :=_return_addrL
    str r4 :=_return_addrH
   pop r4
   str r4 param_:=_id
   pop r4
   str r4 param_:=_value
assign ldr r4 param_:=_value
str r4 param_:=_id
    ldr r4  :=_return_addrH
   push r4
    ldr r4  :=_return_addrL
   push r4
  ret
id_set    pop r4
    pop r3
    str r3 set_return_addrL
    str r4 set_return_addrH
   pop r4
   str r4 param_set_addrH
   pop r4
   str r4 param_set_addrL
   pop r4
   str r4 param_set_value
set ldr lx param_set_addrL
ldr hx param_set_addrH
ldr r4 param_set_value
str r4
    ldr r4  set_return_addrH
   push r4
    ldr r4  set_return_addrL
   push r4
  ret
id_set    pop r4
    pop r3
    str r3 set_return_addrL
    str r4 set_return_addrH
   pop r4
   str r4 param_set_addrL
pulse ldr lx param_->_addrL
ldr hx #x02
ldr r4 #x1
ldr r3 #x0
str r4
str r3
    ldr r4  set_return_addrH
   push r4
    ldr r4  set_return_addrL
   push r4
  ret
id_putc    pop r4
    pop r3
    str r3 putc_return_addrL
    str r4 putc_return_addrH
   pop r4
   str r4 param_putc_char
   pop r4
   str r4 param_putc_out
   pop r4
   str r4 param_putc_status
    ; calling set with args  ldr r4 id_char
 push r4
 ldr r4 id_out
 push r4
 ldr r4 #x0x02
 push r4
    call id_set    ; calling -> with args  ldr r4 id_status
 push r4
    call id_->    ldr r4  putc_return_addrH
   push r4
    ldr r4  putc_return_addrL
   push r4
  ret
enter   nop
 ldr r4 #x0x01
 push r4
 ldr r4 #x0x00
 push r4
 ldr r4 #q65
 push r4
    call id_putc
end    .end
:=_return_addrL .byte #q0
:=_return_addrH .byte #q0
param_:=_id   .byte #q0
param_:=_value   .byte #q0
set_return_addrL .byte #q0
set_return_addrH .byte #q0
param_set_addrH   .byte #q0
param_set_addrL   .byte #q0
param_set_value   .byte #q0
set_return_addrL .byte #q0
set_return_addrH .byte #q0
param_set_addrL   .byte #q0
putc_return_addrL .byte #q0
putc_return_addrH .byte #q0
param_putc_char   .byte #q0
param_putc_out   .byte #q0
param_putc_status   .byte #q0