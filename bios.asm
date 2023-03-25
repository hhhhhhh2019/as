movl %15 start

func:
	movb %0 0x12
	ret

start:
	call func
	hlt
