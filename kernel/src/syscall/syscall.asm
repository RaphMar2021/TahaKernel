global init_syscalls

init_syscalls:
    mov rcx, 0xc0000080
    rdmsr
    or eax, 1
    wrmsr

    mov rcx, 0xc0000081
    rdmsr
    mov edx, 0x00180008
    wrmsr

    mov rcx, 0xc0000084
    mov eax, 0x200
    wrmsr

    ret
