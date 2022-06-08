# STARTUP
__rst:		movi r6, __SP
		lw r6, r6, 0
		movi r7, main
		jalr r7, r7

__SP:		.fill 0x7fff

# DATA
# array address
arr:	.fill 0x3000
size:	.fill 16

main:	movi r1, arr		# int16 *arr = 0x0000
	lw r1, r1, 0
	movi r2, size		# int16 n = 16
	lw r2, r2, 0

	movi r3, 1		# t1 = 1
	movi r4, 1		# t2 = 1

	sw r3, r1, 0		# arr[0] = t1
	addi r1, r1, 1		# arr++

	sw r4, r1, 0		# arr[1] = t2
	addi r1, r1, 1		# arr++

loop:	beq r2, r0, end		# while (size != 0) {

	add r5, r3, r4		# next = t1 + t2
	sw r5, r1, 0		# arr[i] = next

	lw r4, r1, 0		# t2 = arr[i]

	nand r1, r1, r1		#
	addi r1, r1, 1		#
	nand r1, r1, r1		#
	lw r3, r1, 0		# t1 = arr[i - 1]

	addi r1, r1, 2		# arr++

	nand r2, r2, r2		#
	addi r2, r2, 1		#
	nand r2, r2, r2		# size--
	beq r0, r0, loop	# }

end:	beq r0, r0, end		# while(1) { }
