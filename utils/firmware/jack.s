#bank text
Test:
.main:
mv s9, zero
push s9, s2
mv s9, s5
push s9, s2
lw s9, -0(s3)
push ra, s8
save s2, s7, s8
push s9, s2
addi s4, s2, 4
subi s3, s2, 4
call Test.isr
pop ra, s8
mv t0, s9
li s9, 0
restore s2, s7, s8
ret
.isr:
push s9, s2
li s9, 0
restore s2, s7, s8
ret
#bank data
DATA_Test:

#bank bss
STATIC_Test:
