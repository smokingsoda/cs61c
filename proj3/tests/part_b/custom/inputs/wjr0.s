addi t0, x0, 10
addi t1, x0, 20
addi t2, x0, 30
addi sp, sp, -12
sw t0, 0(sp)
sw t1, 4(sp)
sw t2, 8(sp)
add t0, x0, x0
add t1, x0, x0
add t2, x0, x0
lw t0, 0(sp)
lw t1, 4(sp)
lw t2, 8(sp)
addi sp, sp, 12