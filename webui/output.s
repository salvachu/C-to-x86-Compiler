.data
print_fmt_ld: .string "%ld \n"
print_fmt_d: .string "%d \n"

.text
.globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $24, %rsp
    movq $0, -8(%rbp)
    movq $0, -16(%rbp)
    movq $0, -24(%rbp)
    movq $1, %rax
    movl %eax, -8(%rbp)
    movq $10, %rax
    movl %eax, -16(%rbp)
    movq $1000000, %rax
    movq %rax, -24(%rbp)
    movq $20, %rax
    movl %eax, -8(%rbp)
    movl -8(%rbp), %eax
    movl %eax, %esi
    leaq print_fmt_d(%rip), %rdi
    movl $0, %eax
    call printf@PLT
    movl -16(%rbp), %eax
    movl %eax, %esi
    leaq print_fmt_d(%rip), %rdi
    movl $0, %eax
    call printf@PLT
    movq -24(%rbp), %rax
    movq %rax, %rsi
    leaq print_fmt_ld(%rip), %rdi
    movl $0, %eax
    call printf@PLT
    movq $0, %rax
    movq %rax, %r15
    jmp .end_main
.end_main:
    movl $0, %eax
    leave
    ret
.section .note.GNU-stack,"",@progbits
