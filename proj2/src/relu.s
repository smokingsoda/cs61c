.globl relu

.text
# ==============================================================================
# FUNCTION: Performs an inplace element-wise ReLU on an array of ints
# Arguments:
# 	a0 (int*) is the pointer to the array
#	a1 (int)  is the # of elements in the array
# Returns:
#	None
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 78.
# ==============================================================================
relu:
    # Prologue
    addi sp, sp, -16
    sw s0, 0(sp)
    sw s1, 4(sp) #push
loop_start:
    mv t0, a0
    mv t1, a1
    addi t2, zero, 1
    blt t1, t2 error # if i >= 1
loop_continue:
    lw s0, 0(t0)
    mv a0, s0
    blt a0, zero, invert
    j no_invert
    invert:
        sub a0, zero, a0
    no_invert:
        add s0, s0, a0
        srli s0, s0, 1
        sw s0, 0(t0)
        beq t1, t2, loop_end
        addi t1, t1, -1
        addi t0, t0, 4
        j loop_continue
loop_end:
    # Epilogue
    lw s0, 0(sp)
    lw s1, 4(sp)
    addi sp, sp, 16
	ret
error:
    li a1, 78
    j exit2
    
