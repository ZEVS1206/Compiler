;1 _start
section .text
    global _start

_start:
    mov rax, 1
    mov rdi, 1
    mov rax, 60
    xor rdi, rdi
    syscall
