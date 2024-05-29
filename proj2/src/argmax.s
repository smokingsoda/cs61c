.globl argmax

.text
# =================================================================
# FUNCTION: Given a int vector, return the index of the largest
#	element. If there are multiple, return the one
#	with the smallest index.
# Arguments:
# 	a0 (int*) is the pointer to the start of the vector
#	a1 (int)  is the # of elements in the vector
# Returns:
#	a0 (int)  is the first index of the largest element
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 77.
# =================================================================
argmax:
    # Prologue
    addi sp, sp, -16
    sw s0, 0(sp)
    sw s1, 4(sp)
loop_start:
    mv t0, a0
    mv t1, a1
    li t2, 1
    mv t3, zero
    blt t1, t2, error
    addi t1, t1, -1
    lw a1, 0(t0) # t0 is a pointer, a1 is the current max element, a0 is its index
loop_continue:
    lw s0, 0(t0)
    bge a1, s0, jump
    mv a1, s0
    mv a0, t3
    jump:
        beq t3, t1, loop_end
        addi t3, t3, 1
        addi t0, t0, 4
        j loop_continue
loop_end:
    # Epilogue
    lw s0, 0(sp)
    lw s1, 4(sp)
    addi sp, sp, 16
    ret
error:
    li a1, 77
    j exit2
