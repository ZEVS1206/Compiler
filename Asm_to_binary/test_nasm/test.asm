;3 _start func func2
%include "include/test_file_for_add.inc"
section .data
    msg db 20 dup(0)
    len equ $ - msg
    msg1 db 'Hello World!', 10
    len1 equ $ - msg1

section .text
    global _start

func:
    mov rax, 60
    xor rdi, rdi
    syscall

func2:
    mov rax, 60
    xor rdi, rdi
    syscall

some_label:
    print_str msg1, len1
    jmp label

_start:
    mov rax, 5
    cmp rbx, 30
    jne some_label
    mov rax, 10
    jmp some_label
label:
    input_str msg, len
    print_str msg, len
    mov rax, 60
    xor rdi, rdi
    syscall
