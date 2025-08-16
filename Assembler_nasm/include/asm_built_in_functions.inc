%ifndef ASM_BUILT_IN_FUNCTIONS_IO
%define ASM_BUILT_IN_FUNCTIONS_IO


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
    mov rbp, rsp
    input_str buffer_for_input, buffer_size
	lea rdi, buffer_for_input
	call convert_to_number
    pop rbp
    ret


convert_to_number:
	xor rax, rax
	mov ebx, 10
processing_cycle:
	mov dl, [rdi]
	cmp dl, 10
	je end_cycle
	sub dl, '0'
	imul eax, ebx
	add al, dl
	inc rdi
	jmp processing_cycle
end_cycle:
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
;Process negative number
;--------------------------
    cmp r9, 0
    jns .convert_number
    neg r9
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
	mov r13, rdi
	lea rbx, [rbp + 8 * (rdi + 1)]
cycle_processing:
	cmp r13, 0
	je end_of_cycle
	mov rdi, [rbx]
	call print_number
	call print_enter
	sub rbx, 8
	dec r13
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