;8 _start input convert_to_number print_enter buffer_write_char flush_buffer print_number print
%include "include/asm_functions.inc"

section .data
	buffer_for_text db 256 dup(0)
	len_of_buffer equ $ - buffer_for_text
	buffer_index dq 0
	buffer_for_input db 256 dup(0)
	buffer_size equ $ - buffer_for_input
section .text
	global _start

_start:
	push rbp
	mov rbp, rsp
	sub rsp, 128
	call input
	push rax
	pop rax
	mov DWORD [rbp - 4], eax
	movsxd rax, DWORD [rbp - 4]
	push rax
	push 0
	mov rdi, 2
	call print
	pop rbp
	mov rax, 60
	xor rdi, rdi
	syscall

