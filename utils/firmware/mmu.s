#bankdef text {
    #addr 0
    #addr_end 4096
    #outp 8 * 4096
}
#bankdef boot {
    #addr 8192
    #addr_end 12288
    #outp 8 * 0
}
#bankdef pagetable {
    #addr 4096
    #addr_end 8192
    #outp 8 * 8192
}
#bank pagetable
#d16 0x1c       ; map real address 0x1000 to logical 0 w,x

#bank boot
; map real address 4096 (0x1000) to 0
;li t0, 0x1000
;mmu.map t0, zero, zero, (1, 1)
; Use mmu.setpt
li t0, 0x2000         ; the pointer to the page table
mmu.setpt t0, zero, 1 ; len as an immediate
mmu.toggle zero ; jump to new address 0

#bank text
li t0, 0x2 ; set mode
li t1, 0x5 ; combined text + graphics mode
V_COMMAND = 0x10 + 0x0
V_DATAH = 0x10 + 0x1
sbd t1, V_DATAH(zero)
sbd t0, V_COMMAND(zero)

j $

