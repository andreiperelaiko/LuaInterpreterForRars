	.file	"main.c"
	.option nopic
	.attribute arch, "rv32i2p1"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.section	.rodata
	.align	2
.LC0:
	.string	"10 + 20 + 30"
	.align	2
.LC1:
	.string	"Tokens count: "
	.align	2
.LC2:
	.string	"INT: "
	.align	2
.LC3:
	.string	"PLUS"
	.text
	.align	2
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-64
	sw	ra,60(sp)
	sw	s0,56(sp)
	addi	s0,sp,64
	lui	a5,%hi(.LC0)
	addi	a5,a5,%lo(.LC0)
	sw	a5,-24(s0)
	lw	a5,-24(s0)
	sw	a5,-36(s0)
	sw	zero,-32(s0)
	li	a5,12
	sw	a5,-28(s0)
	addi	a5,s0,-48
	addi	a4,s0,-36
	mv	a1,a4
	mv	a0,a5
	call	tokenize
	lui	a5,%hi(.LC1)
	addi	a0,a5,%lo(.LC1)
	call	print_string
	lw	a5,-44(s0)
	mv	a0,a5
	call	print_int
	li	a0,10
	call	print_char
	sw	zero,-20(s0)
	j	.L2
.L5:
	addi	a5,s0,-48
	lw	a1,-20(s0)
	mv	a0,a5
	call	get_token
	mv	a4,a0
	mv	a5,a1
	sw	a4,-56(s0)
	sw	a5,-52(s0)
	lw	a4,-56(s0)
	li	a5,2
	bne	a4,a5,.L3
	lui	a5,%hi(.LC2)
	addi	a0,a5,%lo(.LC2)
	call	print_string
	lw	a5,-52(s0)
	mv	a0,a5
	call	print_string
	j	.L4
.L3:
	lw	a4,-56(s0)
	li	a5,1
	bne	a4,a5,.L4
	lui	a5,%hi(.LC3)
	addi	a0,a5,%lo(.LC3)
	call	print_string
.L4:
	li	a0,10
	call	print_char
	lw	a5,-20(s0)
	addi	a5,a5,1
	sw	a5,-20(s0)
.L2:
	lw	a5,-44(s0)
	lw	a4,-20(s0)
	bltu	a4,a5,.L5
	li	a5,0
	mv	a0,a5
	lw	ra,60(sp)
	lw	s0,56(sp)
	addi	sp,sp,64
	jr	ra
	.size	main, .-main
	.ident	"GCC: (GNU) 15.2.0"
	.section	.note.GNU-stack,"",@progbits
