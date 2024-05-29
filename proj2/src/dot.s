.globl dot

.text
# =======================================================
# FUNCTION: Dot product of 2 int vectors
# Arguments:
#   a0 (int*) is the pointer to the start of v0
#   a1 (int*) is the pointer to the start of v1
#   a2 (int)  is the length of the vectors
#   a3 (int)  is the stride of v0
#   a4 (int)  is the stride of v1
# Returns:
#   a0 (int)  is the dot product of v0 and v1
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 75.
# - If the stride of either vector is less than 1,
#   this function terminates the program with error code 76.
# =======================================================
dot:
    # Prologue
    addi sp, sp, -20
    sw, s0, 0(sp)
    sw, s1, 4(sp)
    sw, s2, 8(sp)
    sw, s3, 12(sp)
    sw, s4, 16(sp)
loop_start:
    li s0, 1
    blt a2, s0, error_75
    blt a3, s0, error_76
    blt a4, s0, error_76 #error
    li s0, 4 # calculate offset
    mul s3, a3, s0
    mul s4, a4, s0
    li s2, 1
    li s1, 0
loop_continue:
    lw t0, 0(a0)
    lw t1, 0(a1)
    mul s0, t0, t1
    add s1, s1, s0
    beq s2, a2, loop_end
    addi s2, s2, 1
    add a0, a0, s3
    add a1, a1, s4
    j loop_continue
loop_end:
    mv a0, s1
    # Epilogue
    lw s0, 0(sp)
    lw s1, 4(sp)
    lw s2, 8(sp)
    lw s3, 12(sp)
    lw s4, 16(sp)
    addi sp, sp, 20
    ret
    
error_75:
    li a1, 75
    j exit2
    
error_76:
    li a1, 76
    j exit2
