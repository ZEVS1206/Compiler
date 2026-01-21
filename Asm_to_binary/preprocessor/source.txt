;1 _start
section .data
    msg: db 'HELLO, World', 10
    len equ $ - msg

section .text
    global _start
_start:
    print_str msg, len

    mov rax, 60
    xor rdi, rdi
    syscall
