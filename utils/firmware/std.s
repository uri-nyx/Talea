T:
F:
W:
E:
Main:
.main:
Array:
.new:
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s8, s9
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 2
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -0(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Memory.alloc
pop ra, s7
restore s2, s6, s7
ret
.dispose:
push s8, s2
lw s8, -0(s4)
mv s5, s8
mv s8, s5
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Memory.deAlloc
pop ra, s7
mv t0, s8
li s8, 0
restore s2, s6, s7
ret
Keyboard:
.init:
push s8, s2
li s8, 0
restore s2, s6, s7
ret
.keyPressed:
push s8, s2
li s8, 24576
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Memory.peek
pop ra, s7
restore s2, s6, s7
ret
.readChar:
push zero, s2
mv s8, zero
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Output.printChar
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -4(s3)
push s8, s2
li s8, 0
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call Keyboard.keyPressed
pop ra, s7
sw s8, -0(s3)
lw s8, -0(s3)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s8, s9
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s3)
sw s8, -4(s3)
pop s8, s2

j W

push ra, s7
save s2, s6, s7
subi s3, s2, 4
call String.backSpace
pop ra, s7
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Output.printChar
pop ra, s7
mv t0, s8
lw s8, -4(s3)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Output.printChar
pop ra, s7
mv t0, s8
lw s8, -4(s3)
restore s2, s6, s7
ret
.readLine:
push zero, s2
push zero, s2
push zero, s2
push zero, s2
mv s8, zero
push s8, s2
li s8, 80
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call String.new
pop ra, s7
sw s8, -12(s3)
lw s8, -0(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Output.printString
pop ra, s7
mv t0, s8
pop s8, s2
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call String.newLine
pop ra, s7
sw s8, -4(s3)
pop s8, s2
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call String.backSpace
pop ra, s7
sw s8, -8(s3)
pop s8, s2

push s8, s2
lw s8, -16(s3)
not s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call Keyboard.readChar
pop ra, s7
sw s8, -0(s3)
lw s8, -0(s3)
push s8, s2
lw s8, -4(s3)
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
sw s8, -16(s3)
lw s8, -16(s3)
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -8(s3)
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -12(s3)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call String.eraseLastChar
pop ra, s7
mv t0, s8
pop s8, s2
j E

push s8, s2
lw s8, -12(s3)
push s8, s2
lw s8, -0(s3)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call String.appendChar
pop ra, s7
sw s8, -12(s3)
pop s8, s2


j W

push s8, s2
lw s8, -12(s3)
restore s2, s6, s7
ret
.readInt:
push zero, s2
mv s8, zero
push s8, s2
lw s8, -0(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Keyboard.readLine
pop ra, s7
sw s8, -0(s3)
lw s8, -0(s3)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call String.intValue
pop ra, s7
sw s8, -4(s3)
lw s8, -0(s3)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call String.dispose
pop ra, s7
mv t0, s8
lw s8, -4(s3)
restore s2, s6, s7
ret
Math:
.init:
mv s8, zero
push s8, s2
li s8, 16
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Array.new
pop ra, s7
ssw s8, STATIC_Math.s1, s1
li s8, 16
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Array.new
pop ra, s7
ssw s8, STATIC_Math.s0, s1
li s8, 0
push s8, s2
llw s8, STATIC_Math.s0
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 1
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
pop s8, s2

push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 15
pop s9, s2
slt s8, s9, s8
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
lw s8, -0(s3)
push s8, s2
llw s8, STATIC_Math.s0
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
push s8, s2
llw s8, STATIC_Math.s0
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
push s8, s2
llw s8, STATIC_Math.s0
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
pop s9, s2
add s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
pop s8, s2
j W

push s8, s2
li s8, 0
restore s2, s6, s7
ret
.abs:
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s4)
neg s8, s8
sw s8, -0(s4)
pop s8, s2

push s8, s2
lw s8, -0(s4)
restore s2, s6, s7
ret
.multiply:
push zero, s2
push zero, s2
push zero, s2
push zero, s2
mv s8, zero
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
and s8, s9, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s8, s9
neg s8, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
pop s9, s2
and s8, s9, s8
pop s9, s2
or s8, s9, s8
sw s8, -16(s3)
lw s8, -0(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Math.abs
pop ra, s7
sw s8, -0(s4)
lw s8, -4(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Math.abs
pop ra, s7
sw s8, -4(s4)
lw s8, -0(s4)
push s8, s2
lw s8, -4(s4)
pop s9, s2
slt s8, s9, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s4)
sw s8, -4(s3)
lw s8, -4(s4)
sw s8, -0(s4)
lw s8, -4(s3)
sw s8, -4(s4)
pop s8, s2


push s8, s2
lw s8, -8(s3)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
pop s9, s2
slt s8, s9, s8
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -12(s3)
push s8, s2
llw s8, STATIC_Math.s0
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -4(s4)
pop s9, s2
and s8, s9, s8
push s8, s2
li s8, 0
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -0(s4)
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
lw s8, -8(s3)
push s8, s2
lw s8, -12(s3)
push s8, s2
llw s8, STATIC_Math.s0
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
pop s9, s2
add s8, s9, s8
sw s8, -8(s3)
pop s8, s2

push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -0(s4)
pop s9, s2
add s8, s9, s8
sw s8, -0(s4)
lw s8, -12(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -12(s3)
pop s8, s2
j W

push s8, s2
lw s8, -16(s3)
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s3)
neg s8, s8
sw s8, -0(s3)
pop s8, s2

push s8, s2
lw s8, -0(s3)
restore s2, s6, s7
ret
.divide:
push zero, s2
push zero, s2
push zero, s2
mv s8, zero
push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 0
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 3
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
and s8, s9, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s8, s9
neg s8, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
pop s9, s2
and s8, s9, s8
pop s9, s2
or s8, s9, s8
sw s8, -8(s3)
li s8, 0
push s8, s2
llw s8, STATIC_Math.s1
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -4(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Math.abs
pop ra, s7
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
lw s8, -0(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Math.abs
pop ra, s7
sw s8, -0(s4)
pop s8, s2

push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 15
pop s9, s2
slt s8, s9, s8
neg s8, s8
push s8, s2
lw s8, -12(s3)
not s8, s8
pop s9, s2
and s8, s9, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
li s8, 32767
push s8, s2
lw s8, -0(s3)
push s8, s2
llw s8, STATIC_Math.s1
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
pop s9, s2
sub s8, s9, s8
push s8, s2
lw s8, -0(s3)
push s8, s2
llw s8, STATIC_Math.s1
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
pop s9, s2
slt s8, s9, s8
neg s8, s8
sw s8, -12(s3)
lw s8, -12(s3)
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
push s8, s2
llw s8, STATIC_Math.s1
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s3)
push s8, s2
llw s8, STATIC_Math.s1
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -0(s3)
push s8, s2
llw s8, STATIC_Math.s1
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
pop s9, s2
add s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
push s8, s2
llw s8, STATIC_Math.s1
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
pop s9, s2
slt s8, s8, s9
neg s8, s8
sw s8, -12(s3)
lw s8, -12(s3)
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
pop s8, s2


j W


push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
neg s8, s8
pop s9, s2
slt s8, s8, s9
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -0(s3)
push s8, s2
llw s8, STATIC_Math.s1
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
pop s9, s2
slt s8, s8, s9
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -4(s3)
push s8, s2
lw s8, -0(s3)
push s8, s2
llw s8, STATIC_Math.s0
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
pop s9, s2
add s8, s9, s8
sw s8, -4(s3)
lw s8, -0(s4)
push s8, s2
lw s8, -0(s3)
push s8, s2
llw s8, STATIC_Math.s1
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
pop s9, s2
sub s8, s9, s8
sw s8, -0(s4)
pop s8, s2

push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
sw s8, -0(s3)
pop s8, s2
j W

push s8, s2
lw s8, -8(s3)
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -4(s3)
neg s8, s8
sw s8, -4(s3)
pop s8, s2

push s8, s2
lw s8, -4(s3)
restore s2, s6, s7
ret
.sqrt:
push zero, s2
push zero, s2
push zero, s2
mv s8, zero
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 4
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
li s8, 7
sw s8, -0(s3)
pop s8, s2

push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
neg s8, s8
pop s9, s2
slt s8, s8, s9
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -12(s3)
push s8, s2
lw s8, -0(s3)
push s8, s2
llw s8, STATIC_Math.s0
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
pop s9, s2
add s8, s9, s8
sw s8, -4(s3)
lw s8, -4(s3)
push s8, s2
lw s8, -4(s3)
pop s9, s2
mul zero, s8, s9, s8
sw s8, -8(s3)
lw s8, -8(s3)
push s8, s2
lw s8, -0(s4)
pop s9, s2
slt s8, s8, s9
neg s8, s8
not s8, s8
push s8, s2
lw s8, -8(s3)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
not s8, s8
pop s9, s2
and s8, s9, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -4(s3)
sw s8, -12(s3)
pop s8, s2

push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
sw s8, -0(s3)
pop s8, s2
j W

push s8, s2
lw s8, -12(s3)
restore s2, s6, s7
ret
.max:
push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -4(s4)
pop s9, s2
slt s8, s8, s9
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s4)
sw s8, -4(s4)
pop s8, s2

push s8, s2
lw s8, -4(s4)
restore s2, s6, s7
ret
.min:
push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -4(s4)
pop s9, s2
slt s8, s9, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s4)
sw s8, -4(s4)
pop s8, s2

push s8, s2
lw s8, -4(s4)
restore s2, s6, s7
ret
Memory:
.init:
push s8, s2
li s8, 0
ssw s8, STATIC_Memory.s0, s1
li s8, 2048
push s8, s2
llw s8, STATIC_Memory.s0
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 14334
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 2049
push s8, s2
llw s8, STATIC_Memory.s0
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 2050
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 0
restore s2, s6, s7
ret
.peek:
push s8, s2
lw s8, -0(s4)
push s8, s2
llw s8, STATIC_Memory.s0
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
restore s2, s6, s7
ret
.poke:
push s8, s2
lw s8, -0(s4)
push s8, s2
llw s8, STATIC_Memory.s0
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -4(s4)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 0
restore s2, s6, s7
ret
.alloc:
push zero, s2
mv s8, zero
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 5
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 1
sw s8, -0(s4)
pop s8, s2

push s8, s2
li s8, 2048
sw s8, -0(s3)
pop s8, s2

push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 16383
pop s9, s2
slt s8, s9, s8
neg s8, s8
push s8, s2
li s8, 0
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -0(s4)
pop s9, s2
slt s8, s9, s8
neg s8, s8
pop s9, s2
and s8, s9, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
li s8, 1
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
sw s8, -4(s3)
li s8, 0
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 0
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
push s8, s2
lw s8, -4(s3)
push s8, s2
li s8, 16382
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
li s8, 0
push s8, s2
lw s8, -4(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 0
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
pop s9, s2
or s8, s9, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -4(s3)
sw s8, -0(s3)
pop s8, s2
j E

push s8, s2
li s8, 0
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 1
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -0(s3)
pop s9, s2
sub s8, s9, s8
push s8, s2
li s8, 0
push s8, s2
lw s8, -4(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
pop s9, s2
add s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 1
push s8, s2
lw s8, -4(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -4(s3)
push s8, s2
li s8, 2
pop s9, s2
add s8, s9, s8
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 1
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 2
pop s9, s2
add s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
pop s8, s2
j E

push s8, s2
li s8, 1
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 1
push s8, s2
lw s8, -4(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
pop s8, s2


j W

push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -0(s4)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 16379
pop s9, s2
slt s8, s8, s9
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 6
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
li s8, 0
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 2
pop s9, s2
add s8, s9, s8
pop s9, s2
slt s8, s8, s9
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 2
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 0
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -0(s4)
pop s9, s2
sub s8, s9, s8
push s8, s2
li s8, 2
pop s9, s2
sub s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 1
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 2
pop s9, s2
add s8, s9, s8
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 3
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -0(s4)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 4
pop s9, s2
add s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
pop s8, s2
j E

push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 3
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 1
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
pop s8, s2

push s8, s2
li s8, 1
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -0(s4)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 2
pop s9, s2
add s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
pop s8, s2

push s8, s2
li s8, 0
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 0
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
lw s8, -0(s3)
push s8, s2
li s8, 2
pop s9, s2
add s8, s9, s8
restore s2, s6, s7
ret
.deAlloc:
push zero, s2
mv s8, zero
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 2
pop s9, s2
sub s8, s9, s8
sw s8, -0(s3)
li s8, 1
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
sw s8, -4(s3)
li s8, 0
push s8, s2
lw s8, -4(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 0
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 0
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 1
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -0(s3)
pop s9, s2
sub s8, s9, s8
push s8, s2
li s8, 2
pop s9, s2
sub s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
pop s8, s2
j E

push s8, s2
li s8, 0
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 1
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -0(s3)
pop s9, s2
sub s8, s9, s8
push s8, s2
li s8, 0
push s8, s2
lw s8, -4(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
pop s9, s2
add s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 1
push s8, s2
lw s8, -4(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -4(s3)
push s8, s2
li s8, 2
pop s9, s2
add s8, s9, s8
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 1
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 2
pop s9, s2
add s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
pop s8, s2
j E

push s8, s2
li s8, 1
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 1
push s8, s2
lw s8, -4(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
pop s8, s2


push s8, s2
li s8, 0
restore s2, s6, s7
ret
Output:
.init:
push s8, s2
li s8, 16384
ssw s8, STATIC_Output.s4, s1
li s8, 0
not s8, s8
ssw s8, STATIC_Output.s2, s1
li s8, 32
ssw s8, STATIC_Output.s1, s1
li s8, 0
ssw s8, STATIC_Output.s0, s1
li s8, 6
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call String.new
pop ra, s7
ssw s8, STATIC_Output.s3, s1
pop s8, s2
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call Output.initMap
pop ra, s7
mv t0, s8
pop s8, s2
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call Output.createShiftedMap
pop ra, s7
mv t0, s8
li s8, 0
restore s2, s6, s7
ret
.initMap:
push s8, s2
li s8, 127
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Array.new
pop ra, s7
ssw s8, STATIC_Output.s5, s1
li s8, 0
push s8, s2
li s8, 63
push s8, s2
li s8, 63
push s8, s2
li s8, 63
push s8, s2
li s8, 63
push s8, s2
li s8, 63
push s8, s2
li s8, 63
push s8, s2
li s8, 63
push s8, s2
li s8, 63
push s8, s2
li s8, 63
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 32
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 33
push s8, s2
li s8, 12
push s8, s2
li s8, 30
push s8, s2
li s8, 30
push s8, s2
li s8, 30
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 34
push s8, s2
li s8, 54
push s8, s2
li s8, 54
push s8, s2
li s8, 20
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 35
push s8, s2
li s8, 0
push s8, s2
li s8, 18
push s8, s2
li s8, 18
push s8, s2
li s8, 63
push s8, s2
li s8, 18
push s8, s2
li s8, 18
push s8, s2
li s8, 63
push s8, s2
li s8, 18
push s8, s2
li s8, 18
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 36
push s8, s2
li s8, 12
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 3
push s8, s2
li s8, 30
push s8, s2
li s8, 48
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 37
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 35
push s8, s2
li s8, 51
push s8, s2
li s8, 24
push s8, s2
li s8, 12
push s8, s2
li s8, 6
push s8, s2
li s8, 51
push s8, s2
li s8, 49
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 38
push s8, s2
li s8, 12
push s8, s2
li s8, 30
push s8, s2
li s8, 30
push s8, s2
li s8, 12
push s8, s2
li s8, 54
push s8, s2
li s8, 27
push s8, s2
li s8, 27
push s8, s2
li s8, 27
push s8, s2
li s8, 54
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 39
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 6
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 40
push s8, s2
li s8, 24
push s8, s2
li s8, 12
push s8, s2
li s8, 6
push s8, s2
li s8, 6
push s8, s2
li s8, 6
push s8, s2
li s8, 6
push s8, s2
li s8, 6
push s8, s2
li s8, 12
push s8, s2
li s8, 24
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 41
push s8, s2
li s8, 6
push s8, s2
li s8, 12
push s8, s2
li s8, 24
push s8, s2
li s8, 24
push s8, s2
li s8, 24
push s8, s2
li s8, 24
push s8, s2
li s8, 24
push s8, s2
li s8, 12
push s8, s2
li s8, 6
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 42
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 63
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 43
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 63
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 44
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 6
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 45
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 63
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 46
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 47
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 32
push s8, s2
li s8, 48
push s8, s2
li s8, 24
push s8, s2
li s8, 12
push s8, s2
li s8, 6
push s8, s2
li s8, 3
push s8, s2
li s8, 1
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 48
push s8, s2
li s8, 12
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 49
push s8, s2
li s8, 12
push s8, s2
li s8, 14
push s8, s2
li s8, 15
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 63
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 50
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 48
push s8, s2
li s8, 24
push s8, s2
li s8, 12
push s8, s2
li s8, 6
push s8, s2
li s8, 3
push s8, s2
li s8, 51
push s8, s2
li s8, 63
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 48
push s8, s2
li s8, 48
push s8, s2
li s8, 28
push s8, s2
li s8, 48
push s8, s2
li s8, 48
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 52
push s8, s2
li s8, 16
push s8, s2
li s8, 24
push s8, s2
li s8, 28
push s8, s2
li s8, 26
push s8, s2
li s8, 25
push s8, s2
li s8, 63
push s8, s2
li s8, 24
push s8, s2
li s8, 24
push s8, s2
li s8, 60
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 53
push s8, s2
li s8, 63
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 31
push s8, s2
li s8, 48
push s8, s2
li s8, 48
push s8, s2
li s8, 48
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 54
push s8, s2
li s8, 28
push s8, s2
li s8, 6
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 31
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 55
push s8, s2
li s8, 63
push s8, s2
li s8, 49
push s8, s2
li s8, 48
push s8, s2
li s8, 48
push s8, s2
li s8, 24
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 56
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 57
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 62
push s8, s2
li s8, 48
push s8, s2
li s8, 48
push s8, s2
li s8, 24
push s8, s2
li s8, 14
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 58
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 59
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 6
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 60
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 24
push s8, s2
li s8, 12
push s8, s2
li s8, 6
push s8, s2
li s8, 3
push s8, s2
li s8, 6
push s8, s2
li s8, 12
push s8, s2
li s8, 24
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 61
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 63
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 63
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 62
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 3
push s8, s2
li s8, 6
push s8, s2
li s8, 12
push s8, s2
li s8, 24
push s8, s2
li s8, 12
push s8, s2
li s8, 6
push s8, s2
li s8, 3
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 64
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 59
push s8, s2
li s8, 59
push s8, s2
li s8, 59
push s8, s2
li s8, 27
push s8, s2
li s8, 3
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 63
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 24
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 65
push s8, s2
li s8, 12
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 63
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 66
push s8, s2
li s8, 31
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 31
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 31
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 67
push s8, s2
li s8, 28
push s8, s2
li s8, 54
push s8, s2
li s8, 35
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 35
push s8, s2
li s8, 54
push s8, s2
li s8, 28
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 68
push s8, s2
li s8, 15
push s8, s2
li s8, 27
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 27
push s8, s2
li s8, 15
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 69
push s8, s2
li s8, 63
push s8, s2
li s8, 51
push s8, s2
li s8, 35
push s8, s2
li s8, 11
push s8, s2
li s8, 15
push s8, s2
li s8, 11
push s8, s2
li s8, 35
push s8, s2
li s8, 51
push s8, s2
li s8, 63
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 70
push s8, s2
li s8, 63
push s8, s2
li s8, 51
push s8, s2
li s8, 35
push s8, s2
li s8, 11
push s8, s2
li s8, 15
push s8, s2
li s8, 11
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 71
push s8, s2
li s8, 28
push s8, s2
li s8, 54
push s8, s2
li s8, 35
push s8, s2
li s8, 3
push s8, s2
li s8, 59
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 54
push s8, s2
li s8, 44
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 72
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 63
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 73
push s8, s2
li s8, 30
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 74
push s8, s2
li s8, 60
push s8, s2
li s8, 24
push s8, s2
li s8, 24
push s8, s2
li s8, 24
push s8, s2
li s8, 24
push s8, s2
li s8, 24
push s8, s2
li s8, 27
push s8, s2
li s8, 27
push s8, s2
li s8, 14
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 75
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 27
push s8, s2
li s8, 15
push s8, s2
li s8, 27
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 76
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 35
push s8, s2
li s8, 51
push s8, s2
li s8, 63
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 77
push s8, s2
li s8, 33
push s8, s2
li s8, 51
push s8, s2
li s8, 63
push s8, s2
li s8, 63
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 78
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 55
push s8, s2
li s8, 55
push s8, s2
li s8, 63
push s8, s2
li s8, 59
push s8, s2
li s8, 59
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 79
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 80
push s8, s2
li s8, 31
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 31
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 81
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 63
push s8, s2
li s8, 59
push s8, s2
li s8, 30
push s8, s2
li s8, 48
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 82
push s8, s2
li s8, 31
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 31
push s8, s2
li s8, 27
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 83
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 6
push s8, s2
li s8, 28
push s8, s2
li s8, 48
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 84
push s8, s2
li s8, 63
push s8, s2
li s8, 63
push s8, s2
li s8, 45
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 85
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 86
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 30
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 87
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 63
push s8, s2
li s8, 63
push s8, s2
li s8, 63
push s8, s2
li s8, 18
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 88
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 30
push s8, s2
li s8, 12
push s8, s2
li s8, 30
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 89
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 90
push s8, s2
li s8, 63
push s8, s2
li s8, 51
push s8, s2
li s8, 49
push s8, s2
li s8, 24
push s8, s2
li s8, 12
push s8, s2
li s8, 6
push s8, s2
li s8, 35
push s8, s2
li s8, 51
push s8, s2
li s8, 63
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 91
push s8, s2
li s8, 30
push s8, s2
li s8, 6
push s8, s2
li s8, 6
push s8, s2
li s8, 6
push s8, s2
li s8, 6
push s8, s2
li s8, 6
push s8, s2
li s8, 6
push s8, s2
li s8, 6
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 92
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 1
push s8, s2
li s8, 3
push s8, s2
li s8, 6
push s8, s2
li s8, 12
push s8, s2
li s8, 24
push s8, s2
li s8, 48
push s8, s2
li s8, 32
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 93
push s8, s2
li s8, 30
push s8, s2
li s8, 24
push s8, s2
li s8, 24
push s8, s2
li s8, 24
push s8, s2
li s8, 24
push s8, s2
li s8, 24
push s8, s2
li s8, 24
push s8, s2
li s8, 24
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 94
push s8, s2
li s8, 8
push s8, s2
li s8, 28
push s8, s2
li s8, 54
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 95
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 63
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 96
push s8, s2
li s8, 6
push s8, s2
li s8, 12
push s8, s2
li s8, 24
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 97
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 14
push s8, s2
li s8, 24
push s8, s2
li s8, 30
push s8, s2
li s8, 27
push s8, s2
li s8, 27
push s8, s2
li s8, 54
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 98
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 15
push s8, s2
li s8, 27
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 99
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 100
push s8, s2
li s8, 48
push s8, s2
li s8, 48
push s8, s2
li s8, 48
push s8, s2
li s8, 60
push s8, s2
li s8, 54
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 101
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 63
push s8, s2
li s8, 3
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 102
push s8, s2
li s8, 28
push s8, s2
li s8, 54
push s8, s2
li s8, 38
push s8, s2
li s8, 6
push s8, s2
li s8, 15
push s8, s2
li s8, 6
push s8, s2
li s8, 6
push s8, s2
li s8, 6
push s8, s2
li s8, 15
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 103
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 62
push s8, s2
li s8, 48
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 104
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 27
push s8, s2
li s8, 55
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 105
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push s8, s2
li s8, 14
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 106
push s8, s2
li s8, 48
push s8, s2
li s8, 48
push s8, s2
li s8, 0
push s8, s2
li s8, 56
push s8, s2
li s8, 48
push s8, s2
li s8, 48
push s8, s2
li s8, 48
push s8, s2
li s8, 48
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 107
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 51
push s8, s2
li s8, 27
push s8, s2
li s8, 15
push s8, s2
li s8, 15
push s8, s2
li s8, 27
push s8, s2
li s8, 51
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 108
push s8, s2
li s8, 14
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 109
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 29
push s8, s2
li s8, 63
push s8, s2
li s8, 43
push s8, s2
li s8, 43
push s8, s2
li s8, 43
push s8, s2
li s8, 43
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 110
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 29
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 111
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 112
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 31
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 113
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 62
push s8, s2
li s8, 48
push s8, s2
li s8, 48
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 114
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 29
push s8, s2
li s8, 55
push s8, s2
li s8, 51
push s8, s2
li s8, 3
push s8, s2
li s8, 3
push s8, s2
li s8, 7
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 115
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 6
push s8, s2
li s8, 24
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 116
push s8, s2
li s8, 4
push s8, s2
li s8, 6
push s8, s2
li s8, 6
push s8, s2
li s8, 15
push s8, s2
li s8, 6
push s8, s2
li s8, 6
push s8, s2
li s8, 6
push s8, s2
li s8, 54
push s8, s2
li s8, 28
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 117
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 27
push s8, s2
li s8, 27
push s8, s2
li s8, 27
push s8, s2
li s8, 27
push s8, s2
li s8, 27
push s8, s2
li s8, 54
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 118
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 119
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 63
push s8, s2
li s8, 63
push s8, s2
li s8, 18
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 120
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 51
push s8, s2
li s8, 30
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 30
push s8, s2
li s8, 51
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 121
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 51
push s8, s2
li s8, 62
push s8, s2
li s8, 48
push s8, s2
li s8, 24
push s8, s2
li s8, 15
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 122
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 63
push s8, s2
li s8, 27
push s8, s2
li s8, 12
push s8, s2
li s8, 6
push s8, s2
li s8, 51
push s8, s2
li s8, 63
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 123
push s8, s2
li s8, 56
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 7
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 56
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 124
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 125
push s8, s2
li s8, 7
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 56
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 12
push s8, s2
li s8, 7
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 126
push s8, s2
li s8, 38
push s8, s2
li s8, 45
push s8, s2
li s8, 25
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 44
subi s3, s2, 4
call Output.create
pop ra, s7
mv t0, s8
li s8, 0
restore s2, s6, s7
ret
.create:
mv s8, zero
push s8, s2
li s8, 11
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Array.new
pop ra, s7
sw s8, -0(s3)
lw s8, -0(s4)
push s8, s2
llw s8, STATIC_Output.s5
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s3)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 0
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -4(s4)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 1
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -8(s4)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 2
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -12(s4)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 3
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -16(s4)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 4
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -20(s4)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 5
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -24(s4)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 6
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -28(s4)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 7
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -32(s4)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 8
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -36(s4)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 9
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -40(s4)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 10
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -44(s4)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 0
restore s2, s6, s7
ret
.createShiftedMap:
push zero, s2
push zero, s2
push zero, s2
mv s8, zero
push s8, s2
li s8, 127
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Array.new
pop ra, s7
ssw s8, STATIC_Output.s6, s1
li s8, 0
sw s8, -8(s3)
pop s8, s2

push s8, s2
lw s8, -8(s3)
push s8, s2
li s8, 127
pop s9, s2
slt s8, s9, s8
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -8(s3)
push s8, s2
llw s8, STATIC_Output.s5
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
sw s8, -0(s3)
li s8, 11
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Array.new
pop ra, s7
sw s8, -4(s3)
lw s8, -8(s3)
push s8, s2
llw s8, STATIC_Output.s6
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -4(s3)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 0
sw s8, -12(s3)
pop s8, s2

push s8, s2
lw s8, -12(s3)
push s8, s2
li s8, 11
pop s9, s2
slt s8, s9, s8
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -12(s3)
push s8, s2
lw s8, -4(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -12(s3)
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 256
pop s9, s2
mul zero, s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
lw s8, -12(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -12(s3)
pop s8, s2
j W

push s8, s2
lw s8, -8(s3)
push s8, s2
li s8, 0
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 32
sw s8, -8(s3)
pop s8, s2
j E

push s8, s2
lw s8, -8(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -8(s3)
pop s8, s2

j W

push s8, s2
li s8, 0
restore s2, s6, s7
ret
.getMap:
mv s8, zero
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 32
pop s9, s2
slt s8, s9, s8
neg s8, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 126
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 0
sw s8, -0(s4)
pop s8, s2

push s8, s2
llw s8, STATIC_Output.s2
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s4)
push s8, s2
llw s8, STATIC_Output.s5
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
sw s8, -0(s3)
pop s8, s2
j E

push s8, s2
lw s8, -0(s4)
push s8, s2
llw s8, STATIC_Output.s6
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
sw s8, -0(s3)
pop s8, s2

push s8, s2
lw s8, -0(s3)
restore s2, s6, s7
ret
.drawChar:
push zero, s2
push zero, s2
push zero, s2
mv s8, zero
push s8, s2
lw s8, -0(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Output.getMap
pop ra, s7
sw s8, -8(s3)
llw s8, STATIC_Output.s1
sw s8, -0(s3)
pop s8, s2

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
bnez s9, E
push s8, s2
llw s8, STATIC_Output.s2
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s3)
push s8, s2
llw s8, STATIC_Output.s4
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 256
neg s8, s8
pop s9, s2
and s8, s9, s8
sw s8, -12(s3)
pop s8, s2
j E

push s8, s2
lw s8, -0(s3)
push s8, s2
llw s8, STATIC_Output.s4
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 255
pop s9, s2
and s8, s9, s8
sw s8, -12(s3)
pop s8, s2

push s8, s2
lw s8, -0(s3)
push s8, s2
llw s8, STATIC_Output.s4
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -4(s3)
push s8, s2
lw s8, -8(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -12(s3)
pop s9, s2
or s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
lw s8, -0(s3)
push s8, s2
li s8, 32
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
lw s8, -4(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -4(s3)
pop s8, s2
j W

push s8, s2
li s8, 0
restore s2, s6, s7
ret
.moveCursor:
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 22
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 63
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 20
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 2
pop s9, s2
idiv s8, zero, s9, s8
ssw s8, STATIC_Output.s0, s1
li s8, 32
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 352
pop s9, s2
mul zero, s8, s9, s8
pop s9, s2
add s8, s9, s8
push s8, s2
llw s8, STATIC_Output.s0
pop s9, s2
add s8, s9, s8
ssw s8, STATIC_Output.s1, s1
lw s8, -4(s4)
push s8, s2
llw s8, STATIC_Output.s0
push s8, s2
li s8, 2
pop s9, s2
mul zero, s8, s9, s8
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
ssw s8, STATIC_Output.s2, s1
li s8, 32
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Output.drawChar
pop ra, s7
mv t0, s8
li s8, 0
restore s2, s6, s7
ret
.printChar:
push s8, s2
lw s8, -0(s4)
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call String.newLine
pop ra, s7
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push ra, s7
save s2, s6, s7
subi s3, s2, 4
call Output.println
pop ra, s7
mv t0, s8
pop s8, s2
j E

push s8, s2
lw s8, -0(s4)
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call String.backSpace
pop ra, s7
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push ra, s7
save s2, s6, s7
subi s3, s2, 4
call Output.backSpace
pop ra, s7
mv t0, s8
pop s8, s2
j E

push s8, s2
lw s8, -0(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Output.drawChar
pop ra, s7
mv t0, s8
llw s8, STATIC_Output.s2
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
llw s8, STATIC_Output.s0
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
ssw s8, STATIC_Output.s0, s1
llw s8, STATIC_Output.s1
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
ssw s8, STATIC_Output.s1, s1
pop s8, s2

push s8, s2
llw s8, STATIC_Output.s0
push s8, s2
li s8, 32
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push ra, s7
save s2, s6, s7
subi s3, s2, 4
call Output.println
pop ra, s7
mv t0, s8
pop s8, s2
j E

push s8, s2
llw s8, STATIC_Output.s2
not s8, s8
ssw s8, STATIC_Output.s2, s1
pop s8, s2



push s8, s2
li s8, 0
restore s2, s6, s7
ret
.printString:
push zero, s2
mv s8, zero
push s8, s2
lw s8, -0(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call String.length
pop ra, s7
sw s8, -4(s3)
pop s8, s2

push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -4(s3)
pop s9, s2
slt s8, s9, s8
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -0(s3)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call String.charAt
pop ra, s7
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Output.printChar
pop ra, s7
mv t0, s8
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
pop s8, s2
j W

push s8, s2
li s8, 0
restore s2, s6, s7
ret
.printInt:
push s8, s2
llw s8, STATIC_Output.s3
push s8, s2
lw s8, -0(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call String.setInt
pop ra, s7
mv t0, s8
llw s8, STATIC_Output.s3
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Output.printString
pop ra, s7
mv t0, s8
li s8, 0
restore s2, s6, s7
ret
.println:
push s8, s2
llw s8, STATIC_Output.s1
push s8, s2
li s8, 352
pop s9, s2
add s8, s9, s8
push s8, s2
llw s8, STATIC_Output.s0
pop s9, s2
sub s8, s9, s8
ssw s8, STATIC_Output.s1, s1
li s8, 0
ssw s8, STATIC_Output.s0, s1
li s8, 0
not s8, s8
ssw s8, STATIC_Output.s2, s1
llw s8, STATIC_Output.s1
push s8, s2
li s8, 8128
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 32
ssw s8, STATIC_Output.s1, s1
pop s8, s2

push s8, s2
li s8, 0
restore s2, s6, s7
ret
.backSpace:
push s8, s2
llw s8, STATIC_Output.s2
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
llw s8, STATIC_Output.s0
push s8, s2
li s8, 0
pop s9, s2
slt s8, s8, s9
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
llw s8, STATIC_Output.s0
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
ssw s8, STATIC_Output.s0, s1
llw s8, STATIC_Output.s1
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
ssw s8, STATIC_Output.s1, s1
pop s8, s2
j E

push s8, s2
li s8, 31
ssw s8, STATIC_Output.s0, s1
llw s8, STATIC_Output.s1
push s8, s2
li s8, 32
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 8128
ssw s8, STATIC_Output.s1, s1
pop s8, s2

push s8, s2
llw s8, STATIC_Output.s1
push s8, s2
li s8, 321
pop s9, s2
sub s8, s9, s8
ssw s8, STATIC_Output.s1, s1
pop s8, s2

push s8, s2
li s8, 0
ssw s8, STATIC_Output.s2, s1
pop s8, s2
j E

push s8, s2
li s8, 0
not s8, s8
ssw s8, STATIC_Output.s2, s1
pop s8, s2

push s8, s2
li s8, 32
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Output.drawChar
pop ra, s7
mv t0, s8
li s8, 0
restore s2, s6, s7
ret
Screen:
.init:
mv s8, zero
push s8, s2
li s8, 16384
ssw s8, STATIC_Screen.s1, s1
li s8, 0
not s8, s8
ssw s8, STATIC_Screen.s2, s1
li s8, 17
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Array.new
pop ra, s7
ssw s8, STATIC_Screen.s0, s1
li s8, 0
push s8, s2
llw s8, STATIC_Screen.s0
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 1
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
pop s8, s2

push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 16
pop s9, s2
slt s8, s9, s8
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
lw s8, -0(s3)
push s8, s2
llw s8, STATIC_Screen.s0
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
push s8, s2
llw s8, STATIC_Screen.s0
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
push s8, s2
llw s8, STATIC_Screen.s0
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
pop s9, s2
add s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
pop s8, s2
j W

push s8, s2
li s8, 0
restore s2, s6, s7
ret
.clearScreen:
mv s8, zero

push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 8192
pop s9, s2
slt s8, s9, s8
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -0(s3)
push s8, s2
llw s8, STATIC_Screen.s1
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 0
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
pop s8, s2
j W

push s8, s2
li s8, 0
restore s2, s6, s7
ret
.updateLocation:
push s8, s2
llw s8, STATIC_Screen.s2
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s4)
push s8, s2
llw s8, STATIC_Screen.s1
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
llw s8, STATIC_Screen.s1
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -4(s4)
pop s9, s2
or s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
pop s8, s2
j E

push s8, s2
lw s8, -0(s4)
push s8, s2
llw s8, STATIC_Screen.s1
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
llw s8, STATIC_Screen.s1
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
lw s8, -4(s4)
not s8, s8
pop s9, s2
and s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
pop s8, s2

push s8, s2
li s8, 0
restore s2, s6, s7
ret
.setColor:
push s8, s2
lw s8, -0(s4)
ssw s8, STATIC_Screen.s2, s1
li s8, 0
restore s2, s6, s7
ret
.drawPixel:
push zero, s2
push zero, s2
mv s8, zero
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 511
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 255
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 7
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 16
pop s9, s2
idiv s8, zero, s9, s8
sw s8, -0(s3)
lw s8, -0(s4)
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 16
pop s9, s2
mul zero, s8, s9, s8
pop s9, s2
sub s8, s9, s8
sw s8, -4(s3)
lw s8, -4(s4)
push s8, s2
li s8, 32
pop s9, s2
mul zero, s8, s9, s8
push s8, s2
lw s8, -0(s3)
pop s9, s2
add s8, s9, s8
sw s8, -8(s3)
lw s8, -8(s3)
push s8, s2
lw s8, -4(s3)
push s8, s2
llw s8, STATIC_Screen.s0
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call Screen.updateLocation
pop ra, s7
mv t0, s8
li s8, 0
restore s2, s6, s7
ret
.drawConditional:
push s8, s2
lw s8, -8(s4)
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, -0(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call Screen.drawPixel
pop ra, s7
mv t0, s8
pop s8, s2
j E

push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -4(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call Screen.drawPixel
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
li s8, 0
restore s2, s6, s7
ret
.drawLine:
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
mv s8, zero
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
push s8, s2
lw s8, -8(s4)
push s8, s2
li s8, 511
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -12(s4)
push s8, s2
li s8, 255
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 8
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -8(s4)
push s8, s2
lw s8, -0(s4)
pop s9, s2
sub s8, s9, s8
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Math.abs
pop ra, s7
sw s8, -12(s3)
lw s8, -12(s4)
push s8, s2
lw s8, -4(s4)
pop s9, s2
sub s8, s9, s8
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Math.abs
pop ra, s7
sw s8, -8(s3)
lw s8, -12(s3)
push s8, s2
lw s8, -8(s3)
pop s9, s2
slt s8, s9, s8
neg s8, s8
sw s8, -24(s3)
lw s8, -24(s3)
push s8, s2
lw s8, -12(s4)
push s8, s2
lw s8, -4(s4)
pop s9, s2
slt s8, s9, s8
neg s8, s8
pop s9, s2
and s8, s9, s8
push s8, s2
lw s8, -24(s3)
not s8, s8
push s8, s2
lw s8, -8(s4)
push s8, s2
lw s8, -0(s4)
pop s9, s2
slt s8, s9, s8
neg s8, s8
pop s9, s2
and s8, s9, s8
pop s9, s2
or s8, s9, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s4)
sw s8, -16(s3)
lw s8, -8(s4)
sw s8, -0(s4)
lw s8, -16(s3)
sw s8, -8(s4)
lw s8, -4(s4)
sw s8, -16(s3)
lw s8, -12(s4)
sw s8, -4(s4)
lw s8, -16(s3)
sw s8, -12(s4)
pop s8, s2

push s8, s2
lw s8, -24(s3)
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -12(s3)
sw s8, -16(s3)
lw s8, -8(s3)
sw s8, -12(s3)
lw s8, -16(s3)
sw s8, -8(s3)
lw s8, -4(s4)
sw s8, -4(s3)
lw s8, -0(s4)
sw s8, -0(s3)
lw s8, -12(s4)
sw s8, -32(s3)
lw s8, -0(s4)
push s8, s2
lw s8, -8(s4)
pop s9, s2
slt s8, s8, s9
neg s8, s8
sw s8, -28(s3)
pop s8, s2
j E

push s8, s2
lw s8, -0(s4)
sw s8, -4(s3)
lw s8, -4(s4)
sw s8, -0(s3)
lw s8, -8(s4)
sw s8, -32(s3)
lw s8, -4(s4)
push s8, s2
lw s8, -12(s4)
pop s9, s2
slt s8, s8, s9
neg s8, s8
sw s8, -28(s3)
pop s8, s2

push s8, s2
li s8, 2
push s8, s2
lw s8, -8(s3)
pop s9, s2
mul zero, s8, s9, s8
push s8, s2
lw s8, -12(s3)
pop s9, s2
sub s8, s9, s8
sw s8, -20(s3)
li s8, 2
push s8, s2
lw s8, -8(s3)
pop s9, s2
mul zero, s8, s9, s8
sw s8, -36(s3)
li s8, 2
push s8, s2
lw s8, -8(s3)
push s8, s2
lw s8, -12(s3)
pop s9, s2
sub s8, s9, s8
pop s9, s2
mul zero, s8, s9, s8
sw s8, -40(s3)
lw s8, -4(s3)
push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -24(s3)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 8
subi s3, s2, 4
call Screen.drawConditional
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -4(s3)
push s8, s2
lw s8, -32(s3)
pop s9, s2
slt s8, s9, s8
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -20(s3)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -20(s3)
push s8, s2
lw s8, -36(s3)
pop s9, s2
add s8, s9, s8
sw s8, -20(s3)
pop s8, s2
j E

push s8, s2
lw s8, -20(s3)
push s8, s2
lw s8, -40(s3)
pop s9, s2
add s8, s9, s8
sw s8, -20(s3)
lw s8, -28(s3)
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
sw s8, -0(s3)
pop s8, s2
j E

push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
pop s8, s2


push s8, s2
lw s8, -4(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -4(s3)
lw s8, -4(s3)
push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -24(s3)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 8
subi s3, s2, 4
call Screen.drawConditional
pop ra, s7
mv t0, s8
pop s8, s2
j W

push s8, s2
li s8, 0
restore s2, s6, s7
ret
.drawRectangle:
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
mv s8, zero
push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -8(s4)
pop s9, s2
slt s8, s8, s9
neg s8, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, -12(s4)
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -8(s4)
push s8, s2
li s8, 511
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -12(s4)
push s8, s2
li s8, 255
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 9
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 16
pop s9, s2
idiv s8, zero, s9, s8
sw s8, -12(s3)
lw s8, -0(s4)
push s8, s2
lw s8, -12(s3)
push s8, s2
li s8, 16
pop s9, s2
mul zero, s8, s9, s8
pop s9, s2
sub s8, s9, s8
sw s8, -28(s3)
lw s8, -8(s4)
push s8, s2
li s8, 16
pop s9, s2
idiv s8, zero, s9, s8
sw s8, -16(s3)
lw s8, -8(s4)
push s8, s2
lw s8, -16(s3)
push s8, s2
li s8, 16
pop s9, s2
mul zero, s8, s9, s8
pop s9, s2
sub s8, s9, s8
sw s8, -32(s3)
lw s8, -28(s3)
push s8, s2
llw s8, STATIC_Screen.s0
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
not s8, s8
sw s8, -24(s3)
lw s8, -32(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
push s8, s2
llw s8, STATIC_Screen.s0
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
sw s8, -20(s3)
lw s8, -4(s4)
push s8, s2
li s8, 32
pop s9, s2
mul zero, s8, s9, s8
push s8, s2
lw s8, -12(s3)
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
lw s8, -16(s3)
push s8, s2
lw s8, -12(s3)
pop s9, s2
sub s8, s9, s8
sw s8, -8(s3)
pop s8, s2

push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, -12(s4)
pop s9, s2
slt s8, s8, s9
neg s8, s8
not s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -8(s3)
pop s9, s2
add s8, s9, s8
sw s8, -4(s3)
lw s8, -8(s3)
push s8, s2
li s8, 0
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -20(s3)
push s8, s2
lw s8, -24(s3)
pop s9, s2
and s8, s9, s8
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call Screen.updateLocation
pop ra, s7
mv t0, s8
pop s8, s2
j E

push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -24(s3)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call Screen.updateLocation
pop ra, s7
mv t0, s8
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
pop s8, s2

push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -4(s3)
pop s9, s2
slt s8, s9, s8
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
neg s8, s8
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call Screen.updateLocation
pop ra, s7
mv t0, s8
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
pop s8, s2
j W

push s8, s2
lw s8, -4(s3)
push s8, s2
lw s8, -20(s3)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call Screen.updateLocation
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -4(s4)
lw s8, -4(s3)
push s8, s2
li s8, 32
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -8(s3)
pop s9, s2
sub s8, s9, s8
sw s8, -0(s3)
pop s8, s2
j W

push s8, s2
li s8, 0
restore s2, s6, s7
ret
.drawHorizontal:
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
push zero, s2
mv s8, zero
push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, -8(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call Math.min
pop ra, s7
sw s8, -28(s3)
lw s8, -4(s4)
push s8, s2
lw s8, -8(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call Math.max
pop ra, s7
sw s8, -32(s3)
lw s8, -0(s4)
push s8, s2
li s8, 1
neg s8, s8
pop s9, s2
slt s8, s8, s9
neg s8, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 256
pop s9, s2
slt s8, s9, s8
neg s8, s8
pop s9, s2
and s8, s9, s8
push s8, s2
lw s8, -28(s3)
push s8, s2
li s8, 512
pop s9, s2
slt s8, s9, s8
neg s8, s8
pop s9, s2
and s8, s9, s8
push s8, s2
lw s8, -32(s3)
push s8, s2
li s8, 1
neg s8, s8
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
and s8, s9, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -28(s3)
push s8, s2
li s8, 0
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call Math.max
pop ra, s7
sw s8, -28(s3)
lw s8, -32(s3)
push s8, s2
li s8, 511
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call Math.min
pop ra, s7
sw s8, -32(s3)
lw s8, -28(s3)
push s8, s2
li s8, 16
pop s9, s2
idiv s8, zero, s9, s8
sw s8, -4(s3)
lw s8, -28(s3)
push s8, s2
lw s8, -4(s3)
push s8, s2
li s8, 16
pop s9, s2
mul zero, s8, s9, s8
pop s9, s2
sub s8, s9, s8
sw s8, -36(s3)
lw s8, -32(s3)
push s8, s2
li s8, 16
pop s9, s2
idiv s8, zero, s9, s8
sw s8, -8(s3)
lw s8, -32(s3)
push s8, s2
lw s8, -8(s3)
push s8, s2
li s8, 16
pop s9, s2
mul zero, s8, s9, s8
pop s9, s2
sub s8, s9, s8
sw s8, -40(s3)
lw s8, -36(s3)
push s8, s2
llw s8, STATIC_Screen.s0
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
not s8, s8
sw s8, -20(s3)
lw s8, -40(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
push s8, s2
llw s8, STATIC_Screen.s0
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
sw s8, -16(s3)
lw s8, -0(s4)
push s8, s2
li s8, 32
pop s9, s2
mul zero, s8, s9, s8
push s8, s2
lw s8, -4(s3)
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
lw s8, -8(s3)
push s8, s2
lw s8, -4(s3)
pop s9, s2
sub s8, s9, s8
sw s8, -24(s3)
lw s8, -0(s3)
push s8, s2
lw s8, -24(s3)
pop s9, s2
add s8, s9, s8
sw s8, -12(s3)
lw s8, -24(s3)
push s8, s2
li s8, 0
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -16(s3)
push s8, s2
lw s8, -20(s3)
pop s9, s2
and s8, s9, s8
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call Screen.updateLocation
pop ra, s7
mv t0, s8
pop s8, s2
j E

push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -20(s3)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call Screen.updateLocation
pop ra, s7
mv t0, s8
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
pop s8, s2

push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -12(s3)
pop s9, s2
slt s8, s9, s8
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
neg s8, s8
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call Screen.updateLocation
pop ra, s7
mv t0, s8
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
pop s8, s2
j W

push s8, s2
lw s8, -12(s3)
push s8, s2
lw s8, -16(s3)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 4
subi s3, s2, 4
call Screen.updateLocation
pop ra, s7
mv t0, s8
pop s8, s2


push s8, s2
li s8, 0
restore s2, s6, s7
ret
.drawSymetric:
push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, -12(s4)
pop s9, s2
sub s8, s9, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -8(s4)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -8(s4)
pop s9, s2
sub s8, s9, s8
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 8
subi s3, s2, 4
call Screen.drawHorizontal
pop ra, s7
mv t0, s8
lw s8, -4(s4)
push s8, s2
lw s8, -12(s4)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -8(s4)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -8(s4)
pop s9, s2
sub s8, s9, s8
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 8
subi s3, s2, 4
call Screen.drawHorizontal
pop ra, s7
mv t0, s8
lw s8, -4(s4)
push s8, s2
lw s8, -8(s4)
pop s9, s2
sub s8, s9, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -12(s4)
pop s9, s2
sub s8, s9, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -12(s4)
pop s9, s2
add s8, s9, s8
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 8
subi s3, s2, 4
call Screen.drawHorizontal
pop ra, s7
mv t0, s8
lw s8, -4(s4)
push s8, s2
lw s8, -8(s4)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -12(s4)
pop s9, s2
sub s8, s9, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -12(s4)
pop s9, s2
add s8, s9, s8
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 8
subi s3, s2, 4
call Screen.drawHorizontal
pop ra, s7
mv t0, s8
li s8, 0
restore s2, s6, s7
ret
.drawCircle:
push zero, s2
push zero, s2
mv s8, zero
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 511
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 255
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 12
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -8(s4)
pop s9, s2
sub s8, s9, s8
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
push s8, s2
lw s8, -0(s4)
push s8, s2
lw s8, -8(s4)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 511
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, -8(s4)
pop s9, s2
sub s8, s9, s8
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, -8(s4)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 255
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 13
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -8(s4)
sw s8, -4(s3)
li s8, 1
push s8, s2
lw s8, -8(s4)
pop s9, s2
sub s8, s9, s8
sw s8, -8(s3)
lw s8, -0(s4)
push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -4(s3)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 12
subi s3, s2, 4
call Screen.drawSymetric
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -4(s3)
push s8, s2
lw s8, -0(s3)
pop s9, s2
slt s8, s8, s9
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -8(s3)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -8(s3)
push s8, s2
li s8, 2
push s8, s2
lw s8, -0(s3)
pop s9, s2
mul zero, s8, s9, s8
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 3
pop s9, s2
add s8, s9, s8
sw s8, -8(s3)
pop s8, s2
j E

push s8, s2
lw s8, -8(s3)
push s8, s2
li s8, 2
push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -4(s3)
pop s9, s2
sub s8, s9, s8
pop s9, s2
mul zero, s8, s9, s8
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 5
pop s9, s2
add s8, s9, s8
sw s8, -8(s3)
lw s8, -4(s3)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
sw s8, -4(s3)
pop s8, s2

push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
lw s8, -0(s4)
push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -4(s3)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 12
subi s3, s2, 4
call Screen.drawSymetric
pop ra, s7
mv t0, s8
pop s8, s2
j W

push s8, s2
li s8, 0
restore s2, s6, s7
ret
String:
.new:
push s8, s2
li s8, 3
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Memory.alloc
pop ra, s7
mv s5, s8
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 14
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s8, s9
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Array.new
pop ra, s7
sw s8, 1*4(s5)
pop s8, s2

push s8, s2
lw s8, -0(s4)
sw s8, 0*4(s5)
li s8, 0
sw s8, 2*4(s5)
mv s8, s5
restore s2, s6, s7
ret
.dispose:
push s8, s2
lw s8, -0(s4)
mv s5, s8
lw s8, 0*4(s5)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s8, s9
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, 1*4(s5)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Array.dispose
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
mv s8, s5
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Memory.deAlloc
pop ra, s7
mv t0, s8
li s8, 0
restore s2, s6, s7
ret
.length:
push s8, s2
lw s8, -0(s4)
mv s5, s8
lw s8, 2*4(s5)
restore s2, s6, s7
ret
.charAt:
push s8, s2
lw s8, -0(s4)
mv s5, s8
lw s8, -4(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, 2*4(s5)
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, 2*4(s5)
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
pop s9, s2
or s8, s9, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 15
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, 1*4(s5)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
restore s2, s6, s7
ret
.setCharAt:
push s8, s2
lw s8, -0(s4)
mv s5, s8
lw s8, -4(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, 2*4(s5)
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, 2*4(s5)
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
pop s9, s2
or s8, s9, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 16
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, 1*4(s5)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -8(s4)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 0
restore s2, s6, s7
ret
.appendChar:
push s8, s2
lw s8, -0(s4)
mv s5, s8
lw s8, 2*4(s5)
push s8, s2
lw s8, 0*4(s5)
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 17
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, 2*4(s5)
push s8, s2
lw s8, 1*4(s5)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -4(s4)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
lw s8, 2*4(s5)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, 2*4(s5)
mv s8, s5
restore s2, s6, s7
ret
.eraseLastChar:
push s8, s2
lw s8, -0(s4)
mv s5, s8
lw s8, 2*4(s5)
push s8, s2
li s8, 0
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 18
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, 2*4(s5)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
sw s8, 2*4(s5)
li s8, 0
restore s2, s6, s7
ret
.intValue:
push zero, s2
push zero, s2
push zero, s2
push zero, s2
mv s8, zero
push s8, s2
lw s8, -0(s4)
mv s5, s8
lw s8, 2*4(s5)
push s8, s2
li s8, 0
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 0
restore s2, s6, s7
ret

push s8, s2
li s8, 0
not s8, s8
sw s8, -12(s3)
li s8, 0
push s8, s2
lw s8, 1*4(s5)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 45
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 0
not s8, s8
sw s8, -16(s3)
li s8, 1
sw s8, -0(s3)
pop s8, s2


push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, 2*4(s5)
pop s9, s2
slt s8, s9, s8
neg s8, s8
push s8, s2
lw s8, -12(s3)
pop s9, s2
and s8, s9, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, 1*4(s5)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
push s8, s2
li s8, 48
pop s9, s2
sub s8, s9, s8
sw s8, -8(s3)
lw s8, -8(s3)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
push s8, s2
lw s8, -8(s3)
push s8, s2
li s8, 9
pop s9, s2
slt s8, s8, s9
neg s8, s8
pop s9, s2
or s8, s9, s8
not s8, s8
sw s8, -12(s3)
lw s8, -12(s3)
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -4(s3)
push s8, s2
li s8, 10
pop s9, s2
mul zero, s8, s9, s8
push s8, s2
lw s8, -8(s3)
pop s9, s2
add s8, s9, s8
sw s8, -4(s3)
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
pop s8, s2

j W

push s8, s2
lw s8, -16(s3)
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -4(s3)
neg s8, s8
sw s8, -4(s3)
pop s8, s2

push s8, s2
lw s8, -4(s3)
restore s2, s6, s7
ret
.setInt:
push zero, s2
push zero, s2
push zero, s2
mv s8, zero
push s8, s2
lw s8, -0(s4)
mv s5, s8
lw s8, 0*4(s5)
push s8, s2
li s8, 0
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 19
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
li s8, 6
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Array.new
pop ra, s7
sw s8, -8(s3)
lw s8, -4(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 0
not s8, s8
sw s8, -12(s3)
lw s8, -4(s4)
neg s8, s8
sw s8, -4(s4)
pop s8, s2

push s8, s2
lw s8, -4(s4)
sw s8, -4(s3)
pop s8, s2

push s8, s2
lw s8, -4(s3)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s8, s9
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -4(s4)
push s8, s2
li s8, 10
pop s9, s2
idiv s8, zero, s9, s8
sw s8, -4(s3)
lw s8, -0(s3)
push s8, s2
lw s8, -8(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 48
push s8, s2
lw s8, -4(s4)
push s8, s2
lw s8, -4(s3)
push s8, s2
li s8, 10
pop s9, s2
mul zero, s8, s9, s8
pop s9, s2
sub s8, s9, s8
pop s9, s2
add s8, s9, s8
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
lw s8, -4(s3)
sw s8, -4(s4)
pop s8, s2
j W

push s8, s2
lw s8, -12(s3)
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, -8(s3)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 45
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, -0(s3)
pop s8, s2

push s8, s2
lw s8, 0*4(s5)
push s8, s2
lw s8, -0(s3)
pop s9, s2
slt s8, s9, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 19
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2

push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 0
pop s9, s2
sub s8, s9, s8
seqz s8, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 0
push s8, s2
lw s8, 1*4(s5)
pop s9, s2
add s8, s9, s8
push s8, s2
li s8, 48
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
li s8, 1
sw s8, 2*4(s5)
pop s8, s2
j E

push s8, s2
li s8, 0
sw s8, 2*4(s5)
pop s8, s2

push s8, s2
lw s8, 2*4(s5)
push s8, s2
lw s8, -0(s3)
pop s9, s2
slt s8, s9, s8
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, 2*4(s5)
push s8, s2
lw s8, 1*4(s5)
pop s9, s2
add s8, s9, s8
push s8, s2
lw s8, -0(s3)
push s8, s2
lw s8, 2*4(s5)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
pop s9, s2
sub s8, s9, s8
push s8, s2
lw s8, -8(s3)
pop s9, s2
add s8, s9, s8
mv s6, s8
lb s8, 0(s6)
mv t0, s8
pop s8, s2
mv s6, s8
mv s8, t0
sb s8, 0(s6)
lw s8, 2*4(s5)
push s8, s2
li s8, 1
pop s9, s2
add s8, s9, s8
sw s8, 2*4(s5)
pop s8, s2
j W


push s8, s2
lw s8, -8(s3)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Array.dispose
pop ra, s7
mv t0, s8
li s8, 0
restore s2, s6, s7
ret
.newLine:
push s8, s2
li s8, 128
restore s2, s6, s7
ret
.backSpace:
push s8, s2
li s8, 129
restore s2, s6, s7
ret
.doubleQuote:
push s8, s2
li s8, 34
restore s2, s6, s7
ret
Sys:
.init:
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call Memory.init
pop ra, s7
mv t0, s8
pop s8, s2
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call Math.init
pop ra, s7
mv t0, s8
pop s8, s2
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call Screen.init
pop ra, s7
mv t0, s8
pop s8, s2
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call Output.init
pop ra, s7
mv t0, s8
pop s8, s2
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call Keyboard.init
pop ra, s7
mv t0, s8
pop s8, s2
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call Main.main
pop ra, s7
mv t0, s8
pop s8, s2
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call Sys.halt
pop ra, s7
mv t0, s8
li s8, 0
restore s2, s6, s7
ret
.halt:

push s8, s2
li s8, 0
not s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
j W

push s8, s2
li s8, 0
restore s2, s6, s7
ret
.wait:
mv s8, zero
push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s9, s8
neg s8, s8
mv s9, s8
pop s8, s2
bnez s9, T
j F

push s8, s2
li s8, 1
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Sys.error
pop ra, s7
mv t0, s8
pop s8, s2


push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s8, s9
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
li s8, 50
sw s8, -0(s3)
pop s8, s2

push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 0
pop s9, s2
slt s8, s8, s9
neg s8, s8
not s8, s8
mv s9, s8
pop s8, s2
bnez s9, E
push s8, s2
lw s8, -0(s3)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
sw s8, -0(s3)
pop s8, s2
j W

push s8, s2
lw s8, -0(s4)
push s8, s2
li s8, 1
pop s9, s2
sub s8, s9, s8
sw s8, -0(s4)
pop s8, s2
j W

push s8, s2
li s8, 0
restore s2, s6, s7
ret
.error:
push s8, s2
li s8, 69
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Output.printChar
pop ra, s7
mv t0, s8
li s8, 82
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Output.printChar
pop ra, s7
mv t0, s8
li s8, 82
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Output.printChar
pop ra, s7
mv t0, s8
lw s8, -0(s4)
push ra, s7
save s2, s6, s7
push s8, s2
addi s4, s2, 0
subi s3, s2, 4
call Output.printInt
pop ra, s7
mv t0, s8
pop s8, s2
push ra, s7
save s2, s6, s7
subi s3, s2, 4
call Sys.halt
pop ra, s7
mv t0, s8
li s8, 0
restore s2, s6, s7
ret
DATA_Array:

DATA_Keyboard:

DATA_Math:

DATA_Memory:

DATA_Output:

DATA_Screen:

DATA_String:

DATA_Sys:

STATIC_Array:

STATIC_Keyboard:

STATIC_Math:
.s1: #d32 0
.s0: #d32 0
STATIC_Memory:
.s1: #d32 0
.s0: #d32 0
STATIC_Output:
.s1: #d32 0
.s0: #d32 0
.s4: #d32 0
.s2: #d32 0
.s3: #d32 0
.s5: #d32 0
.s6: #d32 0
STATIC_Screen:
.s1: #d32 0
.s0: #d32 0
.s4: #d32 0
.s2: #d32 0
.s3: #d32 0
.s5: #d32 0
.s6: #d32 0
STATIC_String:
.s1: #d32 0
.s0: #d32 0
.s4: #d32 0
.s2: #d32 0
.s3: #d32 0
.s5: #d32 0
.s6: #d32 0
STATIC_Sys:
.s1: #d32 0
.s0: #d32 0
.s4: #d32 0
.s2: #d32 0
.s3: #d32 0
.s5: #d32 0
.s6: #d32 0