%ifndef ASM_BUILT_IN_FUNCTIONS_IO
%define ASM_BUILT_IN_FUNCTIONS_IO


%macro int_to_double 2
    cvtsi2sd xmm%1, %2
%endmacro

%macro double_to_int 2
    cvtsd2si %1, xmm%2
%endmacro

%macro floor_double 2
    cvttsd2si %1, xmm%2
%endmacro

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

input:
    push rbp
    mov  rbp, rsp
    input_str buffer_for_input, buffer_size
    lea  rdi, [buffer_for_input]
    mov  r11, 0
    mov  al, [rdi]
    cmp  al, '-'
    jne  .check_double
    mov  r11, 1
    inc  rdi

.check_double:
    call check_if_double
    cmp  rax, 0
    je   .int_number
    jmp  .double_number

.int_number:
    lea  rdi, [buffer_for_input]
    cmp  r11, 1
    jne  .parse_int
    inc  rdi
.parse_int:
    call convert_to_number
    mov r12, 0
    cmp  r11, 1
    jne  .end_input
    neg  rax
    jmp  .end_input

.double_number:
    lea  rdi, [buffer_for_input]
    cmp  r11, 1
    jne  .parse_double
    inc  rdi
.parse_double:
    call get_double
    mov r12, 1
    cmp  r11, 1
    jne  .end_input
    pxor xmm11, xmm11
    subsd xmm11, xmm0
    movsd xmm0, xmm11

.end_input:
    pop  rbp
    ret


check_if_double:
check_cycle:
    mov dl, [rdi]
    cmp dl, '.'
    je founded
    inc rdi
    cmp dl, 10
    jne check_cycle
    mov rax, 0
    jmp end__
founded:
    mov rax, 1
end__:
    ret

get_double:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    lea rbx, [buffer_for_int_part]
int_part:
    mov dl, [rdi]
    cmp dl, '.'
    je end_int_part
    mov [rbx], dl
    inc rbx
    inc rdi
    jmp int_part
end_int_part:
    mov dl, 10
    mov [rbx], dl
    mov rsi, rdi
    mov [rbp - 8], rsi
    lea rdi, [buffer_for_int_part]
    call convert_to_number
    int_to_double 0, rax
    mov rsi, [rbp - 8]
    mov [rbp - 16], rax
    inc rsi
    lea rbx, [buffer_for_frac_part]
frac_part:
    mov dl, [rsi]
    cmp dl, 10
    je end_frac_part
    mov [rbx], dl
    inc rbx
    inc rsi
    jmp frac_part
end_frac_part:
    mov dl, 10
    mov [rbx], dl
    lea rdi, [buffer_for_frac_part]
    call convert_to_number
    int_to_double 1, rax
    cmp rax, 5
    jae imp
    jmp next_
imp:
    mov rax, [rbp - 16]
    inc rax
    mov [rbp - 16], rax
next_:
    mov rax, [rbp - 16]
    pxor xmm4, xmm4
    comisd xmm1, xmm4
    je input_int
    jmp not_int
input_int:
    int_to_double 4, rax
    movsd xmm0, xmm4
not_int:
    movsd xmm2, [ten]
    movsd xmm3, [one]
get_frac_part:
    comisd xmm1, xmm3
    jb end_get_frac_part
    divsd xmm1, xmm2
    jmp get_frac_part
end_get_frac_part:
    addsd xmm0, xmm1
    mov rsp, rbp
    pop rbp
    ret

    



convert_to_number:
    xor     rax, rax
    mov     ebx, 10
.processing_cycle:
    mov     dl, [rdi]
    cmp     dl, 10
    je      .end_cycle
    sub     dl, '0'
    imul    eax, ebx
    movzx   edx, dl
    add     eax, edx
    inc     rdi
    jmp     .processing_cycle
.end_cycle:
    ret

print_enter:
	mov dl, 10
	call buffer_write_char
	call flush_buffer
	ret

buffer_write_char:
    mov rcx, [buffer_index]      
    cmp rcx, len_of_buffer       
    jge flush_buffer             

    mov [buffer_for_text + rcx], dl
    inc rcx                      
    mov [buffer_index], rcx      
    ret


flush_buffer:
    print_str buffer_for_text, [buffer_index]
    mov byte [buffer_for_text], 0
    mov qword [buffer_index], 0
    ret

print_double:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    movsd [rbp - 8], xmm0
    xor rcx, rcx
    floor_double rax, 0
    mov rdi, rax
    mov r15, rax
    call print_number
    mov rax, r15
    movsd xmm0, [rbp - 8]
    int_to_double 1, rax
    subsd xmm0, xmm1
    mov dl, '.'
    call buffer_write_char
    mov rcx, 6
    movsd xmm1, [ten]
.convert_frac:
    mulsd xmm0, xmm1
    floor_double rax, 0
    mov r15, rax
    push rcx
    mov rdi, rax
    ;mov r14, 0
    call print_number
    ;mov r14, 1
    pop rcx
    mov rax, r15
    int_to_double 2, rax
    subsd xmm0, xmm2
    loop .convert_frac

    mov rsp, rbp
    pop rbp
    ret


print_number:
	push rbp
	mov rbp, rsp
    mov r9, rdi
    mov r11, 10
    xor rcx, rcx
    ;cmp r14, 1
    ;jne .convert_number
;Process negative number
;--------------------------
    cmp r9d, 0
    jns .convert_number
    neg r9d
    mov dl, '-'
    call buffer_write_char
    xor rcx, rcx
;--------------------------

.convert_number:
    mov rax, r9
    xor rdx, rdx
    div r11

    mov r9, rax
    add dl, '0'
    push rdx
    inc rcx
    cmp r9, 0
    jne .convert_number

.add_to_buffer:
    pop rdx
    mov r10, rcx
    call buffer_write_char
    mov rcx, r10
    loop .add_to_buffer

	mov rsp, rbp
	pop rbp
    ret

print:
	push rbp
	mov rbp, rsp
	cmp rdi, 0
	je null_param
    ;mov r14, 1
	mov r13, rdi
	lea rbx, [rbp + 8 * (rdi + 1)]
cycle_processing:
	cmp r13, 0
	je end_of_cycle
    sub rbx, 8
	mov rdi, [rbx]
    add rbx, 8
    cmp rdi, 0
    je number
    movsd xmm0, [rbx]
    call print_double
    jmp next
number:
    mov rdi, [rbx]
	call print_number
next:
    call print_enter
	sub rbx, 16
	sub r13, 2
	jmp cycle_processing
end_of_cycle:
    jmp end_
null_param:
    call print_enter
end_:
	mov rsp, rbp
	pop rbp
	ret


%endif