# STARTUP
__rst:		movi r6, __SP
		lw r6, r6, 0
		movi r7, main
		jalr r7, r7

__SP:		.fill 0x7fff

# DATA
# Hello, World!
stdout:		.fill 0x2000
str_size:	.fill 14
str_addr:	.fill 72
.fill 101
.fill 108
.fill 108
.fill 111
.fill 44
.fill 32
.fill 87
.fill 111
.fill 114
.fill 108
.fill 100
.fill 33
.fill 32

# PROGRAM
main:		movi r1, stdout			# char *stdout = @stdout
		lw r1, r1, 0

		movi r4, 2			# uint16 i = 2
loop:		beq r4, r0, end			# while (i != 0) {

		movi r2, str_addr		# char *str = "Hello, World!"
		movi r3, str_size		# uint16 str_size = @str_size
		lw r3, r3, 0

		movi r7, mem_cp_in
		jalr r7, r7

		nand r4, r4, r4			#
		addi r4, r4, 1			#
		nand r4, r4, r4			# i--

		beq r0, r0, loop

end:		beq r0, r0, end			# while(1) { }

# FUNCTIONS
# mem_cp:
#	$r1 = destination pointer
#	$r2 = source pointer
#	$r3 = n of words
mem_cp_in:	addi r6, r6, 1			# push r4 onto stack
		sw r4, r6, 0			# sp++

mem_cp_l1:	beq r3, r0, mem_cp_ret	# while (str_size != 0) {

		lw r4, r2, 0			# *stdout = *str
		sw r4, r1, 0

		addi r1, r1, 1			# stdout++
		addi r2, r2, 1			# str++

		nand r3, r3, r3			# str_size--
		addi r3, r3, 1
		nand r3, r3, r3

		beq r0, r0, mem_cp_l1	# }

mem_cp_ret:	lw r4, r6, 0			# pop r4 from stack
		nand r6, r6, r6			#
		addi r6, r6, 1			#
		nand r6, r6, r6			# sp--

		jalr r7, r7
