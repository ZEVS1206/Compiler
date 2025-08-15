%ifndef ASM_BUILT_IN_FUNCTIONS_IO
%define ASM_BUILT_IN_FUNCTIONS_IO




%macro print_str 2
    mov rax, 1
    mov rdi, 1
    mov rsi, %1
    mov rdx, %2
    syscall
%endmacro

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
	je end_of_cycle
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
	call print_enter
	mov rsp, rbp
	pop rbp
	ret


%endif