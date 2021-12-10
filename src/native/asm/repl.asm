        .const *printer-out $0200
        .const *printer-options $0201
        .const *teletype-in $0100
        .const *teletype-status $0101
        .const *key-del #q127
        .const *key-enter #q10        
        .org #x400
main    bez repl  
ibuff   .alloc #xff     ;input buffer for terminal (aligned)
icount  .byte  #x00     ;index in input buffer
to-compare .byte #x00
comp-eq        ldr bcc to-compare
        subb bcc
        ret
repl    call fill-buffer
        call end
fill-buffer     ldr acc *teletype-status
        bnz fill-byte
        bez fill-buffer
fill-byte       ldr r4 *teletype-in
        ldr acc r4
        str acc to-compare
        ldr acc *key-enter
        call comp-eq
        bez OnEnter
        ldr acc r4
        str acc to-compare
        ldr acc *key-del
        call comp-eq
        bez OnDel
        lea ibuff
        ldr acc lx
        add icount
        ldr lx acc
        str r4      ;at ibuff
        ldr r1 #q0
        str r1 *teletype-status
        ret
OnEnter lea ibuff
putc    ldi bcc
        ldr acc lx
        str bcc *printer-out
        str acc *printer-options
        str r1  *printer-options
        str r1  *teletype-status
        add #q1
        ldr lx acc
        subb icount
        bnz putc
        ret
OnDel   ldr acc icount
        subb #q1
        str acc icount
        bnz fill-buffer
        bez fill-buffer
end     .end