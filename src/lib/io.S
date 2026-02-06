.text

.globl print_char
print_char:
    li      a7, 11
    ecall
    ret

.globl print_string
print_string:
    li      a7, 4
    ecall
    ret

.globl print_int
print_int:
    li      a7, 1
    ecall
    ret
