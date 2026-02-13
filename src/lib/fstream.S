.text
.globl open
.globl close
.globl read
.globl write
.globl lseek

open:
    li a1, 0
    li a7, 1024
    ecall
    ret

close:
    li a7, 57
    ecall
    ret

read:
    li a7, 63
    ecall
    ret

write:
    li a7, 64
    ecall 
    ret

lseek:
   li a7, 62
   ecall 
   ret