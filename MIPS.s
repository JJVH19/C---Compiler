
# Entering factorial function
.text
_factorial:
la $sp, -8($sp)
sw $fp, 4($sp)
sw $ra, 0($sp)
la $fp, 0($sp)
la $sp, -28($sp)

# ASSGN
li $t0, 0
sw $t0, -24($fp)

# Conditional IF_LE
lw $t0, 8($fp)
lw $t1, -24($fp)
ble $t0, $t1, _L0
# GOTO 
j _L1
# Label
_L0:
# ASSGN
li $t0, 1
sw $t0, -4($fp)

# Exiting function
lw $v0, -4($fp)
la $sp, 0($fp)
lw $ra, 0($sp)
lw $fp, 4($sp)
la $sp, 8($sp)
jr $ra

# GOTO 
j _L2
# Label
_L1:
# ASSGN
li $t0, 1
sw $t0, -8($fp)

# MINUS Operation
lw $t0, 8($fp)
lw $t1, -8($fp)
sub $t2, $t0, $t1
sw $t2, -12($fp)

# PARAM
lw $t0 -12($fp)
la $sp -4($sp)
sw $t0 0($sp)
# CALL
jal _factorial
la $sp, 4($sp)

# RETRIEVE
sw $v0, -16($fp)

# MULTIPLY Operation
lw $t0, 8($fp)
lw $t1, -16($fp)
mul $t2, $t0, $t1
sw $t2, -20($fp)

# Exiting function
lw $v0, -20($fp)
la $sp, 0($fp)
lw $ra, 0($sp)
lw $fp, 4($sp)
la $sp, 8($sp)
jr $ra

# Label
_L2:

# Entering fib function
.text
_fib:
la $sp, -8($sp)
sw $fp, 4($sp)
sw $ra, 0($sp)
la $fp, 0($sp)
la $sp, -40($sp)

# ASSGN
li $t0, 0
sw $t0, -36($fp)

# Conditional IF_LE
lw $t0, 8($fp)
lw $t1, -36($fp)
ble $t0, $t1, _L3
# GOTO 
j _L4
# Label
_L3:
# ASSGN
li $t0, 1
sw $t0, -4($fp)

# Exiting function
lw $v0, -4($fp)
la $sp, 0($fp)
lw $ra, 0($sp)
lw $fp, 4($sp)
la $sp, 8($sp)
jr $ra

# GOTO 
j _L5
# Label
_L4:
# ASSGN
li $t0, 1
sw $t0, -8($fp)

# MINUS Operation
lw $t0, 8($fp)
lw $t1, -8($fp)
sub $t2, $t0, $t1
sw $t2, -12($fp)

# PARAM
lw $t0 -12($fp)
la $sp -4($sp)
sw $t0 0($sp)
# CALL
jal _fib
la $sp, 4($sp)

# RETRIEVE
sw $v0, -16($fp)

# ASSGN
li $t0, 2
sw $t0, -20($fp)

# MINUS Operation
lw $t0, 8($fp)
lw $t1, -20($fp)
sub $t2, $t0, $t1
sw $t2, -24($fp)

# PARAM
lw $t0 -24($fp)
la $sp -4($sp)
sw $t0 0($sp)
# CALL
jal _fib
la $sp, 4($sp)

# RETRIEVE
sw $v0, -28($fp)

# PLUS Operation
lw $t0, -16($fp)
lw $t1, -28($fp)
add $t2, $t0, $t1
sw $t2, -32($fp)

# Exiting function
lw $v0, -32($fp)
la $sp, 0($fp)
lw $ra, 0($sp)
lw $fp, 4($sp)
la $sp, 8($sp)
jr $ra

# Label
_L5:
.data
_n: .space 4

# Entering main function
.text
_main:
la $sp, -8($sp)
sw $fp, 4($sp)
sw $ra, 0($sp)
la $fp, 0($sp)
la $sp, -56($sp)

# ASSGN
li $t0, 7
sw $t0, -12($fp)

# ASSGN
lw $t0, -12($fp)
sw $t0, _n

# ASSGN
li $t0, 0
sw $t0, -16($fp)

# ASSGN
lw $t0, -16($fp)
sw $t0, -4($fp)

# Label
_L6:
# Conditional IF_LE
lw $t0, -4($fp)
lw $t1, _n
ble $t0, $t1, _L7
# GOTO 
j _L8
# Label
_L7:
# PARAM
lw $t0 -4($fp)
la $sp -4($sp)
sw $t0 0($sp)
# CALL
jal _factorial
la $sp, 4($sp)

# RETRIEVE
sw $v0, -20($fp)

# ASSGN
lw $t0, -20($fp)
sw $t0, -8($fp)

# PARAM
lw $t0 -8($fp)
la $sp -4($sp)
sw $t0 0($sp)
# CALL
jal _println
la $sp, 4($sp)

# RETRIEVE
sw $v0, -24($fp)

# ASSGN
li $t0, 1
sw $t0, -28($fp)

# PLUS Operation
lw $t0, -4($fp)
lw $t1, -28($fp)
add $t2, $t0, $t1
sw $t2, -32($fp)

# ASSGN
lw $t0, -32($fp)
sw $t0, -4($fp)

# GOTO 
j _L6
# Label
_L8:
# ASSGN
li $t0, 0
sw $t0, -36($fp)

# ASSGN
lw $t0, -36($fp)
sw $t0, -4($fp)

# Label
_L9:
# Conditional IF_LE
lw $t0, -4($fp)
lw $t1, _n
ble $t0, $t1, _L10
# GOTO 
j _L11
# Label
_L10:
# PARAM
lw $t0 -4($fp)
la $sp -4($sp)
sw $t0 0($sp)
# CALL
jal _fib
la $sp, 4($sp)

# RETRIEVE
sw $v0, -40($fp)

# ASSGN
lw $t0, -40($fp)
sw $t0, -8($fp)

# PARAM
lw $t0 -8($fp)
la $sp -4($sp)
sw $t0 0($sp)
# CALL
jal _println
la $sp, 4($sp)

# RETRIEVE
sw $v0, -44($fp)

# ASSGN
li $t0, 1
sw $t0, -48($fp)

# PLUS Operation
lw $t0, -4($fp)
lw $t1, -48($fp)
add $t2, $t0, $t1
sw $t2, -52($fp)

# ASSGN
lw $t0, -52($fp)
sw $t0, -4($fp)

# GOTO 
j _L9
# Label
_L11:
li $v0, 0
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
