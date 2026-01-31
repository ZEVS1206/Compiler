;3 _start func2 func
%include "include/test_file_for_add.inc"
section .data
    msg db 20 dup(0)
    len equ $ - msg
    msg1 db 'Hello World!', 10
    len1 equ $ - msg1

section .text
    global _start

func:
    input_str msg, len
    print_str msg, len
    ret

func2:
    print_str msg1, len1
    ret

_start:
    call func
    call func2
    mov rax, 60
    xor rdi, rdi
    syscall
