extern printf
extern fgets
extern stdin
extern malloc
extern scanf
extern time

section .rodata
    prompt1: db "Enter first number: ", 0
    prompt2: db "Enter second number: ", 0
    output: db "%04hx", 0     
    newline: db 10, 0
    sum_msg: db "Sum is: ", 0
    plus_msg: db " + ", 0
    equals_msg: db " = ", 0
    carry_msg: db " (carry: ", 0
    close_msg: db ")", 0
    format: db "%x", 0
    
section .data
    input_buffer: times 1000 db 0
    x_struct: dw 5
    x_num: dw 0xaa, 1, 2, 0x44, 0x4f
    y_struct: dw 6
    y_num: dw 0xaa, 1, 2, 3, 0x44, 0x4f            
    current_num: dw 0               
    result_struct: dw 0             
    result_num: times 1000 dw 0 
    STATE: dd 0
    MASK: dw 0xB400     

section .bss
    num1: resd 2
    num2: resd 2

section .text
global main

main:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]     ; argc
    cmp eax, 1
    je default_mode

    mov ebx, [ebp+12]    ; argv
    mov ebx, [ebx+4]     ; argv[1]
    cmp byte [ebx], '-'
    jne default_mode

    cmp byte [ebx+1], 'I'
    je input_mode

    cmp byte [ebx+1], 'R'
    je random_mode

default_mode:
    ; Print first number (x_struct)
    movzx ecx, word [x_struct]        ; size
    lea edx, [x_num + ecx * 2 - 2]    ; pointer to array
    call print_multi
    call print_newline
    
    ; Print second number (y_struct)
    movzx ecx, word [y_struct]        ; size
    lea edx, [y_num + ecx * 2 - 2]    ; pointer to array
    call print_multi
    call print_newline
    
    ; Do addition
    call add_multi
    
    ; Print sum message and result
    push dword sum_msg
    call printf
    add esp, 4
    
    movzx ecx, word [result_struct]
    mov edx, ecx
    dec edx
.print_result:
    cmp edx, 0
    jl .print_result_done
    push ecx
    push edx
    movzx eax, word [result_num + edx*2]
    push eax
    push dword output
    call printf
    add esp, 8
    pop edx
    pop ecx
    dec edx
    jmp .print_result
.print_result_done:
    call print_newline
    jmp exit_prog

input_mode:
    ; Get first number
    push dword prompt1
    call printf
    add esp, 4
    call getmulti
    
    ; Copy first number to y_struct/y_num
    movzx ecx, word [x_struct]
    mov word [y_struct], cx
    xor ebx, ebx
.copy_first:
    cmp ebx, ecx
    jae .first_done
    mov ax, word [x_num + ebx*2]
    mov word [y_num + ebx*2], ax
    inc ebx
    jmp .copy_first
.first_done:
    ; Get second number
    push dword prompt2
    call printf
    add esp, 4
    call getmulti
    
    ; Print first number
    movzx ecx, word [y_struct]
    mov edx, ecx
    dec edx
.print_first:
    cmp edx, 0
    jl .print_first_done
    push ecx
    push edx
    movzx eax, word [y_num + edx*2]
    push eax
    push dword output
    call printf
    add esp, 8
    pop edx
    pop ecx
    dec edx
    jmp .print_first
.print_first_done:
    call print_newline
    
    ; Print second number
    movzx ecx, word [x_struct]
    mov edx, ecx
    dec edx
.print_second:
    cmp edx, 0
    jl .print_second_done
    push ecx
    push edx
    movzx eax, word [x_num + edx*2]
    push eax
    push dword output
    call printf
    add esp, 8
    pop edx
    pop ecx
    dec edx
    jmp .print_second
.print_second_done:
    call print_newline
    
    ; Do addition
    call add_multi
    
    ; Print sum message and result
    push dword sum_msg
    call printf
    add esp, 4
    
    movzx ecx, word [result_struct]
    mov edx, ecx
    dec edx
.print_result:
    cmp edx, 0
    jl .print_result_done
    push ecx
    push edx
    movzx eax, word [result_num + edx*2]
    push eax
    push dword output
    call printf
    add esp, 8
    pop edx
    pop ecx
    dec edx
    jmp .print_result
.print_result_done:
    call print_newline
    jmp exit_prog

random_mode:
    call init_random
    call PRmulti_both
    
    ; Print first number
    movzx ecx, word [x_struct]
    mov edx, ecx
    dec edx
.print_first:
    cmp edx, 0
    jl .first_done
    push ecx
    push edx
    movzx eax, word [x_num + edx*2]
    push eax
    push dword output
    call printf
    add esp, 8
    pop edx
    pop ecx
    dec edx
    jmp .print_first
.first_done:
    call print_newline
    
    ; Print second number
    movzx ecx, word [y_struct]
    mov edx, ecx
    dec edx
.print_second:
    cmp edx, 0
    jl .second_done
    push ecx
    push edx
    movzx eax, word [y_num + edx*2]
    push eax
    push dword output
    call printf
    add esp, 8
    pop edx
    pop ecx
    dec edx
    jmp .print_second
.second_done:
    call print_newline
    
    ; Do addition
    call add_multi
    
    ; Print sum message and result
    push dword sum_msg
    call printf
    add esp, 4
    
    movzx ecx, word [result_struct]
    mov edx, ecx
    dec edx
    
.print_result:
    cmp edx, 0
    jl .print_result_done
    push ecx
    push edx
    movzx eax, word [result_num + edx*2]
    push eax
    push dword output
    call printf
    add esp, 8
    pop edx
    pop ecx
    dec edx
    jmp .print_result
.print_result_done:
    call print_newline
    jmp exit_prog

add_multi:
    push ebp
    mov ebp, esp
    push esi
    push edi
    push ebx
    
    movzx ecx, word [x_struct]
    movzx edx, word [y_struct]
    cmp ecx, edx
    jae .x_bigger
    mov ecx, edx
.x_bigger:
    mov word [result_struct], cx
    
    xor ch, ch          ; Carry flag
    xor ebx, ebx        ; Start from LSB (index 0)
    
.add_loop:
    cmp ebx, ecx
    jae .done
    
    movzx eax, word [x_struct]
    cmp ebx, eax
    jae .use_zero_x
    movzx eax, word [x_num + ebx*2]
    jmp .got_x
.use_zero_x:
    xor eax, eax
.got_x:
    movzx esi, word [y_struct]
    cmp ebx, esi
    jae .use_zero_y
    movzx esi, word [y_num + ebx*2]
    jmp .got_y
.use_zero_y:
    xor esi, esi
.got_y:
    
    pushad
    push eax
    push dword output
    call printf
    add esp, 8
    
    push dword plus_msg
    call printf
    add esp, 4
    
    push esi
    push dword output
    call printf
    add esp, 8
    
    push dword equals_msg
    call printf
    add esp, 4
    popad
    
    add al, ch          ; Add previous carry
    mov ch, 0
    add ax, si
    
    jnc .no_carry
    mov ch, 1
    
.no_carry:
    mov word [result_num + ebx*2], ax
    
    pushad
    mov eax, [result_num + ebx*2]
    push eax
    push dword output
    call printf
    add esp, 8
    
    cmp ch, 0
    je .skip_carry_print
    push dword carry_msg
    call printf
    add esp, 4
    movzx eax, ch
    push eax
    push dword output
    call printf
    add esp, 8
    push dword close_msg
    call printf
    add esp, 4
    
.skip_carry_print:
    call print_newline
    popad
    
    inc ebx             ; Move to next position (toward MSB)
    jmp .add_loop
    
.done:
    cmp ch, 0
    je .really_done
    
    inc word [result_struct]
    mov word [result_num + ecx*2], 1
    
.really_done:
    pop ebx
    pop edi
    pop esi
    mov esp, ebp
    pop ebp
    ret

print_multi:
    pushad
    movzx eax, word [edx]
    push eax
    push dword output
    call printf
    add esp, 8
    popad
    sub edx, 2
    dec ecx
    jnz print_multi
    ret

print_newline:
    pushad
    push dword newline
    call printf
    add esp, 4
    popad
    ret

getmulti:
    push ebp
    mov ebp, esp
     
    push dword [stdin]
    push dword 50
    push dword input_buffer
    call fgets
    add esp, 12
    
    xor ecx, ecx          ; Number count
    mov edi, input_buffer
    xor esi, esi          ; Array index
    mov word [current_num], 0
    
.parse_loop:
    movzx eax, byte [edi] ; Get current char
    
    cmp al, 0
    je .store_and_done
    cmp al, 10
    je .store_and_done
    
    cmp al, ' '
    je .next_number
    
    cmp al, '0'          ; Check for hex chars
    jl .skip_char
    cmp al, '9'
    jle .convert_digit
    
    cmp al, 'a'
    jl .skip_char
    cmp al, 'f'
    jg .skip_char
    sub al, 87          ; 'a' -> 10
    jmp .store_digit
    
.convert_digit:
    sub al, '0'
    
.store_digit:
    mov dx, word [current_num]
    shl dx, 4
    or dl, al
    mov word [current_num], dx
    jmp .skip_char
    
.next_number:
    mov dx, word [current_num]
    mov word [x_num + esi], dx
    add esi, 2
    mov word [current_num], 0
    inc ecx
    
.skip_char:
    inc edi
    jmp .parse_loop
    
.store_and_done:
    mov dx, word [current_num]
    cmp dx, 0
    je .parsing_complete
    mov word [x_num + esi], dx
    inc ecx
    
.parsing_complete:
    mov word [x_struct], cx
    mov esp, ebp
    pop ebp
    ret

init_random:
    push ebp
    mov ebp, esp
    push dword 0
    call time
    add esp, 4
    mov edx, eax
    shl edx, 16
    xor eax, edx
    mov [STATE], eax
    mov esp, ebp
    pop ebp
    ret

rand_num:
    push ebp
    mov ebp, esp
    mov eax, [STATE]
    mov ebx, eax
    mov ecx, eax
    shr ecx, 2
    xor eax, ecx
    mov ecx, eax
    shl ecx, 1
    xor eax, ecx
    mov ecx, eax
    shr ecx, 7
    xor eax, ecx
    mov [STATE], eax
    and eax, 0xFFFF
    mov esp, ebp
    pop ebp
    ret

PRmulti_both:
    push ebp
    mov ebp, esp
    push ebx
    push esi

    ; Generate first number
    call rand_num
    and ax, 0x0F
    add ax, 1
    mov word [x_struct], ax
    movzx ecx, ax
    
    xor esi, esi
.generate_loop_x:
    push ecx
    call rand_num
    mov word [x_num + esi], ax
    add esi, 2
    pop ecx
    loop .generate_loop_x

    ; Generate second number
    call rand_num
    and ax, 0x0F
    add ax, 1
    mov word [y_struct], ax
    movzx ecx, ax
    
    xor esi, esi
.generate_loop_y:
    push ecx
    call rand_num
    mov word [y_num + esi], ax
    add esi, 2
    pop ecx
    loop .generate_loop_y

    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret

exit_prog:
    mov esp, ebp
    pop ebp
    ret