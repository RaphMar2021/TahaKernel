global jmp_ring3

jmp_ring3:
    mov $0x23, %rax
    mov %ax, %ds
    mov %ax, %es
    mov %rax, %ss
    pushq $0x23         ; CS (Code Segment) for user mode
    pushq %rdi          ; Placeholder for RDI (user-space function argument)
    pushq %rbx          ; Saved user-mode register
    pushq %rbp          ; Saved user-mode register
    pushq %r12          ; Saved user-mode register
    pushq %r13          ; Saved user-mode register
    pushq %r14          ; Saved user-mode register
    pushq %r15          ; Saved user-mode register
    pushfq              ; Save RFLAGS
    popq %rax           ; Get RFLAGS into RAX
    or $0x200, %rax     ; Set IF (interrupt flag) in RFLAGS
    pushq %rax          ; Save modified RFLAGS
    pushq %rcx          ; Return address for user mode
    pushq %rdx          ; Return address for user mode
    pushq %rsi          ; Return address for user mode
    lea user_mode_entry(%rip), %rax
    pushq %rax          ; Return address for user mode

    mov $0x1b, %rax     ; Set user-mode data segment
    mov %ax, %fs
    mov %ax, %gs

    iretq

user_mode_entry:
    ret
