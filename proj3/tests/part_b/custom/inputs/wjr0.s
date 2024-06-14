addi t0, x0, 10
addi t1, x0, 20
mul t2, t0, t1
addi sp, sp, -12
sw t2, 0(sp)
sw t1, 4(sp)
sw t0, 8(sp)