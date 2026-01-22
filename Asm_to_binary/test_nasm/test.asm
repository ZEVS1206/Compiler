;1 _start
section .data
    msg: db 'HELLO, World', 10
    len equ $ - msg

section .text
    global _start
_start:
    mov rbx, 1
    push rbp
    pop rbp
    mov rax, 60
    xor rdi, rdi
    syscall
