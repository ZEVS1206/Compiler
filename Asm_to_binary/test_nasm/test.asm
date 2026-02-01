;2 _start func
%include "include/test_file_for_add.inc"
section .data
    msg db 20 dup(0)
    len equ $ - msg
    msg1 db 'Hello World!', 10
    len1 equ $ - msg1

section .text
    global _start

func:
    jmp .label
    mov rdx, 10
.label:
    print_str msg1, len1
    ret

_start:
    call func
    mov rax, 15
    cmp rax, 14
    jne .label
    print_str msg1, len1
.label:
    mov rax, 60
    xor rdi, rdi
    syscall
