;1 _start
%include "include/test_file_for_add.inc"
section .data
    msg db 'HELLO, World', 10
    len equ $ - msg

section .text
    global _start
_start:
    print_str msg, len
    mov rax, 60
    xor rdi, rdi
    syscall
