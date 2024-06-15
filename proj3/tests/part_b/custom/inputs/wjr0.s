add t0, x0, x0
addi t1, x0, 20
add t2, x0, x0
Label1:
add t2, t1, t2
addi t0, t0, 1
blt t0, t1, Label1
addi s0, s0, 1
add t0, x0, x0
Label2:
add t2, t0, t2
addi t0, t0, 1
blt t0, t1, Label2
mul t2, t2, t1
sub t2, t2, t1

