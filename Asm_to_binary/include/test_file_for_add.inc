%ifndef TEST_FILE_FOR_ADD_INC
%define TEST_FILE_FOR_ADD_INC

%macro input_str 2
    mov rax, 0
    mov rdi, 0
    mov rsi, %1
    mov rdx, %2
    syscall
%endmacro

%macro print_str 2
    mov rax, 1
    mov rdi, 1
    mov rsi, %1
    mov rdx, %2
    syscall
%endmacro

%endif