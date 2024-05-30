.globl matmul

.text
# =======================================================
# FUNCTION: Matrix Multiplication of 2 integer matrices
# 	d = matmul(m0, m1)
# Arguments:
# 	a0 (int*)  is the pointer to the start of m0 
#	a1 (int)   is the # of rows (height) of m0
#	a2 (int)   is the # of columns (width) of m0
#	a3 (int*)  is the pointer to the start of m1
# 	a4 (int)   is the # of rows (height) of m1
#	a5 (int)   is the # of columns (width) of m1
#	a6 (int*)  is the pointer to the the start of d
# Returns:
#	None (void), sets d = matmul(m0, m1)
# Exceptions:
#   Make sure to check in top to bottom order!
#   - If the dimensions of m0 do not make sense,
#     this function terminates the program with exit code 72.
#   - If the dimensions of m1 do not make sense,
#     this function terminates the program with exit code 73.
#   - If the dimensions of m0 and m1 don't match,
#     this function terminates the program with exit code 74.
# =======================================================
matmul:
    # Error checks
    li t0, 1
    blt a1, t0, error_72
    blt a2, t0, error_72
    blt a4, t0, error_73
    blt a5, t0, error_73
    bne a2, a4, error_74
    

    # Prologue
    addi sp, sp, -28
    sw s0, 0(sp) # i
    sw s1, 4(sp) # j
    sw s2, 8(sp) # cur_row_pointer
    sw s3, 12(sp) # cur_col_pointer
    sw s4, 16(sp) # v0_stride
    sw s5, 20(sp) # v1_stride
    sw s6, 24(sp)
    li s0, 0 # i = 0
    li s1, 0 # j = 0
    mv s2, a0
    mv s3, a3
    li s4, 1
    mv s5, a5
    li s6, 0 # new matrix pointer
outer_loop_start:
    bgeu s0, a1, outer_loop_end
    li s1, 0
inner_loop_start:
    bgeu s1, a5, inner_loop_end
    addi sp, sp, -32
    sw a0, 0(sp)
    sw a1, 4(sp)
    sw a2, 8(sp)
    sw a3, 12(sp)
    sw a4, 16(sp)
    sw a5, 20(sp)
    sw a6, 24(sp)
    sw ra, 28(sp)
    li t1, 4
    mul t0, s0, a2 # t0 = col0 * i
    mul t0, t0, t1 # t0 = t0 * 4
    add a0, a0, t0
    mul t0, s1, t1 # t0 = j * 4
    add a1, a3, t0
    mv a2, a2 # length = col0
    mv a3, s4
    mv a4, s5
    jal dot
    mv t0, a0
    lw a0, 0(sp)
    lw a1, 4(sp)
    lw a2, 8(sp)
    lw a3, 12(sp)
    lw a4, 16(sp)
    lw a5, 20(sp)
    lw a6, 24(sp)
    lw ra, 28(sp)
    addi sp, sp, 32
    li t1, 4
    mul t1, t1, s6 # t1 = 4 * nmp
    add t1, t1, a6 # a6 = t1 + a6
    sw t0, 0(t1)
    addi s1, s1, 1
    addi s6, s6, 1
    j inner_loop_start
inner_loop_end:
    addi s0, s0, 1
    j outer_loop_start
outer_loop_end:
    # Epilogue
    lw s0, 0(sp) # i
    lw s1, 4(sp) # j
    lw s2, 8(sp) # cur_row_pointer
    lw s3, 12(sp) # cur_col_pointer
    lw s4, 16(sp) # v0_stride
    lw s5, 20(sp) # v1_stride
    lw s6, 24(sp)
    addi sp, sp, 28
    ret
error_72:
    li a1, 72
    j exit2
    
error_73:
    li a1, 73
    j exit2
    
error_74:
    li a1, 74
    j exit2


