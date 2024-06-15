add t0, x0, x0
addi t1, x0, 100
add t2, x0, x0
Label:
add t2, t1, t2
addi t0, t0, 1
bne t0, t1, Label
