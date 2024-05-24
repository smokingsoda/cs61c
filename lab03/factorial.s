.globl factorial

.data
n: .word 7

.text
main:
    la t0, n
    lw a0, 0(t0)
    jal ra, factorial

    addi a1, a0, 0
    addi a0, x0, 1
    ecall # Print Result

    addi a1, x0, '\n'
    addi a0, x0, 11
    ecall # Print newline

    addi a0, x0, 10
    ecall # Exit

factorial:
    # YOUR CODE HERE
    bne a0, x0, else # if n == 0
    addi a0, x0, 1 # 
    jr ra
else:
    addi sp, sp, -8 
    sw a0, 0(sp) # push
    sw ra, 4(sp)
    addi a0, a0, -1 # pass n-1 to the recursive function call
    jal factorial # call the recursive function
    mv a1, a0
    lw a0, 0(sp)
    lw ra, 4(sp)
    addi sp, sp, 8 # pop
    mul a0, a1, a0 # n * factorial(n - 1)
    jr ra # return
    
    