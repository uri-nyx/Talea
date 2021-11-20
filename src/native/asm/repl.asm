        .const *printer-out $0200
        .const *printer-options $0201
        .const *teletype-in $0100
        .const *teletype-status $0101  
        
main    call repl
        .end
        
        .org #x400

ibuff   .alloc #xff     ;input buffer for terminal (aligned)
icount  .byte  #x00     ;index in input buffer


to-compare .byte

comp-eq
        ldr bcc to-compare
        subc bcc
        ret

repl    call fill-buffer
        ret

fill-buffer
        ldr acc *teletype-status
        bnz fill-byte
        bez fill-buffer

fill-byte
        ldr r4 *teletype-in

        ;   case Enter
        ldr acc r4
        str acc to-compare
        ldr acc *key-enter
        call comp-eq
        bez OnEnter

        ;   case Del
        ldr acc r4
        str acc to-compare
        ldr acc *key-del
        call comp-eq
        bez OnDel

        ;   Default
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
        subc icount
        bnz putc
        ret

OnDel   ldr acc icount
        subc #q1
        str acc icount
        bnz fill-buffer
        bez fill-buffer

        

        
