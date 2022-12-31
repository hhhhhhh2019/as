%org 17044482

jmp %&start

foo:
	mov r0 1
ret

start:

call %&foo

mov r1 2

hlt
