%org 17044482

mov r0 2
mov r1 2

cmp r0 r1

je %&equals
jl %&less
jb %&more

equals:
	mov r2 1
	hlt

less:
	mov r2 2
	hlt

more:
	mov r2 3
	hlt
