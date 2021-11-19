        .org #x0300
        lea hello
        ldr bcc #q2
putc    ldi acc
        bez endprog
        str acc $0200
        str bcc $0201
        str r1  $0201
        ldr acc #q1
        add lx
        ldr lx acc
        bnz putc
hello   .stringz "Hello "world!
endprog .end
pointer .word putc