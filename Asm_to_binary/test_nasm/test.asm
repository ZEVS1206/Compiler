;1 _start
%include "include/test_file_for_add.inc"
section .data
    msg db 20 dup(0)
    len equ $ - msg
    msg1 db 'Hello World!', 10
    len1 equ $ - msg1
    buffer_index dq 10

section .text
    global _start

;func:
;    jmp .label
;    mov rdx, 10
;.label:
;    print_str msg1, len1
;    ret

_start:
    mov rcx, [rbp + rdx * 8 + 8]
    mov byte [rel msg], 0
    mov rax, [rel buffer_index]
    mov qword [rel buffer_index], 0
    mov rax, 60
    xor rdi, rdi
    syscall
