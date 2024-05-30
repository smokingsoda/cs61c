.globl write_matrix

.text
# ==============================================================================
# FUNCTION: Writes a matrix of integers into a binary file
# FILE FORMAT:
#   The first 8 bytes of the file will be two 4 byte ints representing the
#   numbers of rows and columns respectively. Every 4 bytes thereafter is an
#   element of the matrix in row-major order.
# Arguments:
#   a0 (char*) is the pointer to string representing the filename
#   a1 (int*)  is the pointer to the start of the matrix in memory
#   a2 (int)   is the number of rows in the matrix
#   a3 (int)   is the number of columns in the matrix
# Returns:
#   None
# Exceptions:
# - If you receive an fopen error or eof,
#   this function terminates the program with error code 93.
# - If you receive an fwrite error or eof,
#   this function terminates the program with error code 94.
# - If you receive an fclose error or eof,
#   this function terminates the program with error code 95.
# ==============================================================================
write_matrix:

    # Prologue
    addi sp, sp, -36
    sw ra 0(sp)
    sw s0, 4(sp) # file descriptor
    sw s1, 8(sp) # buffer pointer
    sw s2, 12(sp) # num of row
    sw s3, 16(sp) # num of col
    sw s4, 20(sp) # pointer to filename
    sw s5, 24(sp) # pointer to represent row
    sw s6, 28(sp) # pointer to represent col
    sw s7, 32(sp) # num of elem
    
    mv s1, a1
    mv s4, a0
    mv s2, a2
    mv s3, a3
    mul s7, s2, s3
	
    # open the file
    mv a1, s4
    li a2, 1
    jal fopen
    mv s0, a0
    li t0, -1
    beq t0, s0, error_93
    
    # write the row and col
    addi sp, sp, -8
    sw s2, 0(sp)
    sw s3, 4(sp)
    mv a1, s0
    mv a2, sp
    li a3, 2
    li a4, 4
    jal fwrite
    li t0, 2
    blt a0, t0, error_94
    addi sp, sp 8
    
    # write the matrix
    mv a1, s0
    mv a2, s1
    mv a3, s7
    li a4, 4
    jal fwrite
    blt a0, s7, error_94
    
    #close the file
    mv a1, s0
    jal fclose
    bne a0, zero, error_95


    # Epilogue

    lw ra 0(sp)
    lw s0, 4(sp) # file descriptor
    lw s1, 8(sp) # buffer pointer
    lw s2, 12(sp) # num of row
    lw s3, 16(sp) # num of col
    lw s4, 20(sp) # pointer to filename
    lw s5, 24(sp) # pointer to represent row
    lw s6, 28(sp) # pointer to represent col
    lw s7, 32(sp) # num of elem
    addi sp, sp, 36

    ret
error_93:
    li a1, 93
    j exit2
    
error_94:
    li a1, 94
    j exit2
    
error_95:
    li a1, 95
    j exit2
