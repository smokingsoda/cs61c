main:
    add t0, x0, x0
    addi t1, x0, 1
    li t3, 6
fib:
    beq t3, x0, finish
    add t2, t1, t0
    mv t0, t1
    mv t1, t2
    addi t3, t3, -1
    j fib
finish:
    addi a0, x0, 1
    addi a1, t0, 0
    addi a0, x0, 10