start:	addi r1, r1, 10		# m = 10
	addi r2, r2, 16		# n = 16

	add r1, r1, r2		# m = m + n

	nand r3, r2, r2		# r3 = -n
	addi r3, r3, 1		#
	add r2, r1, r3		# n = m - n

	nand r3, r2, r2		# r3 = -n
	addi r3, r3, 1		#
	add r1, r1, r3		# m = m - n

end:	beq r0, r0, end		# while(1) { }
