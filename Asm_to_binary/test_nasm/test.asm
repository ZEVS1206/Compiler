;1 _start
%include "include/test_file_for_add.inc"
section .data
    msg db 20 dup(0)
    len equ $ - msg
    msg1 db 'Hello World!', 10
    len1 equ $ - msg1

section .text
    global _start

label:
    mov rax, 60
    xor rdi, rdi
    syscall
_start:
    mov rax, 5
    cmp rax, 20
    cmp rbx, 30
;    jne some_label
    mov rax, 10
    jmp label
;some_label:
;    mov rax, 60
;    xor rdi, rdi
;    syscall
