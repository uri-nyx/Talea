#bank text
Main:
.main:
push zero, s2
mv s8, zero
push s8, s2
li s8, 15538160
ssw s8, STATIC_Main.s0, s1
la s8, DATA_Main.d0
sw s8, -0(s3)
li s8, 0
sw s8, -4(s3)
pop s8, s2
WHILE_EXP0:
push s8, s2
lw s8, -4(s3)
push s8, s2
li s8, 11
pop s9, s2
slt s8, s9, s8
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, WHILE_END0
push s8, s2
llw s8, STATIC_Main.s0
push s8, s2
lw s8, -4(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 4
push s8, s2
lw s8, -4(s3)
pop s9, s2
add s8, s9, s8
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
lw s8, -4(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -4(s3)
pop s8, s2
j WHILE_EXP0
WHILE_END0:
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 3
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
restore s2, s6, s7
ret
#bank data
DATA_Main:
.d0:
	#d32 11
	#d "Hello World"
	#align 32

#bank bss
STATIC_Main:
.s0: #d32 0