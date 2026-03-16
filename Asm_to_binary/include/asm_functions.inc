%ifndef ASM_FUNCTIONS_INC
%define ASM_FUNCTIONS_INC

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
    jne  .int_number
    mov  r11, 1
    inc  rdi

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

.end_input:
    pop  rbp
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
	lea rbx, [rbp + rdi * 8 + 8]
cycle_processing:
	cmp r13, 0
	je end_of_cycle
    sub rbx, 8
	mov rdi, [rbx]
    add rbx, 8
    cmp rdi, 0
    je number
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