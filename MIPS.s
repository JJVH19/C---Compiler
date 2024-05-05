.data
_a: .space 4
_b: .space 4

# Entering f function
.text
_f:
la $sp, -8($sp)
sw $fp, 4($sp)
sw $ra, 0($sp)
la $fp, 0($sp)
la $sp, -68($sp)

# ASSGN
li $t0, 1
sw $t0, -12($fp)

# ASSGN
lw $t0, -12($fp)
sw $t0, 8($fp)

# ASSGN
li $t0, 2
sw $t0, -16($fp)

# ASSGN
lw $t0, -16($fp)
sw $t0, -4($fp)

# Label
_L3:
# PARAM
lw $t0 -4($fp)
la $sp -4($sp)
sw $t0 0($sp)
# CALL
jal _f
la $sp, 4($sp)

# RETRIEVE
sw $v0, -20($fp)

# PLUS Operation
lw $t0, 8($fp)
lw $t1, -20($fp)
add $t2, $t0, $t1
sw $t2, -60($fp)

# ASSGN
li $t0, 0
sw $t0, -64($fp)

# Conditional IF_GT
lw $t0, -60($fp)
lw $t1, -64($fp)
bgt $t0, $t1, _L4
# GOTO 
j _L5
# Label
_L4:
# Conditional IF_LE
lw $t0, -4($fp)
lw $t1, 8($fp)
ble $t0, $t1, _L0
# GOTO 
j _L1
# Label
_L0:
# ASSGN
li $t0, 2
sw $t0, -24($fp)

# PLUS Operation
lw $t0, -4($fp)
lw $t1, -24($fp)
add $t2, $t0, $t1
sw $t2, -28($fp)

# ASSGN
lw $t0, -28($fp)
sw $t0, -4($fp)

# ASSGN
li $t0, 2
sw $t0, -32($fp)

# MULTIPLY Operation
lw $t0, 8($fp)
lw $t1, -32($fp)
mul $t2, $t0, $t1
sw $t2, -36($fp)

# MINUS Operation
lw $t0, -36($fp)
lw $t1, -4($fp)
sub $t2, $t0, $t1
sw $t2, -40($fp)

# ASSGN
li $t0, 9
sw $t0, -44($fp)

# ASSGN
li $t0, 2
sw $t0, -48($fp)

# DIVIDE Operation
lw $t0, -44($fp)
lw $t1, -48($fp)
div $t2, $t0, $t1
sw $t2, -52($fp)

# PLUS Operation
lw $t0, -40($fp)
lw $t1, -52($fp)
add $t2, $t0, $t1
sw $t2, -56($fp)

# ASSGN
lw $t0, -56($fp)
sw $t0, 8($fp)

# GOTO 
j _L2
# Label
_L1:
# Label
_L2:
# GOTO 
j _L3
# Label
_L5:
# Exiting function
lw $v0, 8($fp)
la $sp, 0($fp)
lw $ra, 0($sp)
lw $fp, 4($sp)
la $sp, 8($sp)
jr $ra


.align 2
.data
_nl: .asciiz "\n"

.align 2
.text
# println: print out an integer followed by a newline
_println:
li $v0, 1
lw $a0, 0($sp)
syscall
li $v0, 4
la $a0, _nl
syscall
jr $ra

# dumping global symbol table into mips
.data

# hard coded main call
.align 2
.text
main: j _main
