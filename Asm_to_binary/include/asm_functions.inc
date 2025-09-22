%ifndef ASM_FUNCTIONS_INC
%define ASM_FUNCTIONS_INC
%macro print_str 2
    mov rax, 1
    mov rdi, 1
    mov rsi, %1
    mov rdx, %2
    syscall
%endmacro

%endif