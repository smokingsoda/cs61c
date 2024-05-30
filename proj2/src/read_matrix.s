.globl read_matrix

.text
# ==============================================================================
# FUNCTION: Allocates memory and reads in a binary file as a matrix of integers
#
# FILE FORMAT:
#   The first 8 bytes are two 4 byte ints representing the # of rows and columns
#   in the matrix. Every 4 bytes afterwards is an element of the matrix in
#   row-major order.
# Arguments:
#   a0 (char*) is the pointer to string representing the filename
#   a1 (int*)  is a pointer to an integer, we will set it to the number of rows
#   a2 (int*)  is a pointer to an integer, we will set it to the number of columns
# Returns:
#   a0 (int*)  is the pointer to the matrix in memory
# Exceptions:
# - If malloc returns an error,
#   this function terminates the program with error code 88.
# - If you receive an fopen error or eof, 
#   this function terminates the program with error code 90.
# - If you receive an fread error or eof,
#   this function terminates the program with error code 91.
# - If you receive an fclose error or eof,
#   this function terminates the program with error code 92.
# ==============================================================================
read_matrix:

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
    
    mv s4, a0
    mv s5, a1
    mv s6, a2
	
    # open the file
    mv a1, s4
    mv a2, zero
    jal fopen
    mv s0, a0
    li t0, -1
    beq t0, s0, error_90
        
    # read the num of row and col
    li a0, 8
    jal malloc
    mv s1, a0 # s1 is the pointer to the heap memory
    beq zero, s1, error_88
    
    mv a1, s0 # a1 is file descriptor
    mv a2, s1 # a2 is pointer to the buffer
    li a3, 8 # the number of bytes to be read
    jal fread
    li t0, 8
    blt a0, t0, error_91
    lw s2, 0(s1)
    lw s3, 4(s1)
    mv t1, s1 # t1 is the pointer of the num of row and col in memory 
    
    # store row and col
    sw s2, 0(s5)
    sw s3, 0(s6)
    
    # read the matrix
    mul s7, s2, s3
    slli a0, s7, 2
    jal malloc
    mv s1, a0
    beq zero, s1, error_88
    
    mv a1, s0
    mv a2, s1
    slli a3, s7, 2
    jal fread
    
    slli t0, s7, 2
    bne a0, t0, error_91
    
    mv a1, s0
    jal fclose 
    bne a0, zero, error_92 
    
    mv a0, s1
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

error_90:
    li a1, 90
    j exit2
    
error_88:
    li a1, 88
    j exit2
    
error_91:
    li a1, 91
    j exit2

error_92:
    li a1, 92
    j exit2
