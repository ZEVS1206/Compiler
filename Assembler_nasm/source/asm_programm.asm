section .text
	global _start
_start:
	push rbp
	mov rbp, rsp
	pop rbp
	mov eax, 1
	xor ebx, ebx
	int 0x80
