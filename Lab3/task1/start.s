section .data
    infile: dd 0        ;stdin
    outfile: dd 1       ;stdout
    charToEnc: db 0     ;stdout

section .rodata
    newline: db 10
    errorMsg: db "Failed to open file", 10
    error_len: equ $ - errorMsg

section .text
extern strlen
global main

main:
    mov edi, [esp + 4]    ;argc
    mov esi, [esp + 8]    ;argv
   
main_loop:
    ;length of argv[i]
    mov eax, [esi]  ; pointing to argv[0]
    push eax
    call strlen     ;eax = strlen(argv[i])

    mov edx, eax    
    mov eax, 4         ; pointing to argv[1]    
    mov ebx, 2      
    pop ecx         
    int 0x80     ;print argv[i]

    mov eax, 4      
    mov ebx, 1      
    mov ecx, newline    
    mov edx, 1      
    int 0x80     ;print a new line     

    mov eax, [esi]  ;argv[i]
    cmp word[eax], "-i" 
    je open_input_file
    cmp word[eax], "-o" 
    je open_output_file
    
next_argument:
    add esi, 4      ;point to the next argument
    dec edi
    jnz main_loop

encode:
    ;read 1 byte from user
    mov eax, 3      
    mov ebx, dword[infile]      
    mov ecx, charToEnc    
    mov edx, 1      
    int 0x80        

    cmp eax, 0
    je exit
    
    cmp byte[charToEnc], 'A'
    jl print
    cmp byte[charToEnc], 'z'
    jg print
    inc byte[charToEnc]
        
print:
    mov eax, 4     
    mov ebx, dword[outfile]      
    mov ecx, charToEnc 
    mov edx, 1  
    int 0x80 

    mov eax, 4
    mov ebx, dword[outfile] 
    mov ecx, newline
    mov edx, 1
    int 0x80
     
    jmp encode

exit:
    mov eax, 1     
    mov ebx, 0      
    int 0x80

open_input_file:
    mov ebx, eax
    add ebx, 2  
    mov eax, 5
    mov ecx, 0  ;read only
    int 0x80
    cmp eax, -1
    je error
    mov dword[infile], eax
    jmp next_argument

open_output_file:
    mov ebx, eax
    add ebx, 2  
    mov eax, 5
    mov ecx, 1101o  ;write, create new file, delete content from file
    mov edx, 644o   ;644 in octa is read/write for user
    int 0x80
    cmp eax, -1
    je error
    mov dword[outfile], eax
    jmp next_argument

error:
    mov eax, 4      
    mov ebx, 2      
    mov ecx, error          
    mov edx, error_len      
    int 0x80        
    mov eax, 1      
    mov ebx, 1     
    int 0x80
