beq $0, $0, wain
wain:
;  initialization
.import print
.import init
.import new
.import delete
lis $12
.word init
lis $13
.word new
lis $14
.word delete
lis $10
.word print
lis $4
.word 4
lis $11
.word 1
;  initialize reg 29
sub  $29, $30, $4
;  store reg 1 and 2 onto the stack
sw $1, -4($30)
sub $30, $30, $4
sw $2, -4($30)
sub $30, $30, $4
;  store other variables onto the stack
add $3, $0, $11
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word -8
add $3, $29, $3
sw $3, -4($30)
sub $30, $30, $4
;   call init
sw $31, -4($30)
sub $30, $30, $4
jalr $12
lw $31, 0($30)
add $30, $30, $4
;  allocation of an array
lis $3
.word 10
add $1, $3, $0
sw $31, -4($30)
sub $30, $30, $4
jalr $13
lw $31, 0($30)
add $30, $30, $4
bne $3, $0, 1
add $3, $0, $1
lw $5, 0($30)
add $30, $30, $4
sw $3, 0($5)
lw $3, -8($29)
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 4
lw $5, 0($30)
add $30, $30, $4
sw $3, 0($5)
lw $3, -8($29)
lw $3, 0($3)
add $1, $3, $0
sw $31, -4($30)
sub $30, $30, $4
jalr $10
lw $31, 0($30)
add $30, $30, $4
lw $3, -8($29)
beq $3, $11, skip0
add $1, $3, $0
sw $31, -4($30)
sub $30, $30, $4
jalr $14
lw $31, 0($30)
add $30, $30, $4
;    set the pointer to NULL
lis $3
.word -8
add $3, $3, $29
sw $11, 0($3)
skip0:
lw $3, 0($29)
jr $31
