;2 func _start
section .text
    global _start

func:
    mov rax, 60
    xor rdi, rdi
    syscall

_start:
    mov rax, 60
    xor rdi, rdi
    syscall
