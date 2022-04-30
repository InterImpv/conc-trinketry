start:	add r1, r0, r0		# int16 *arr = 0x0000
		addi r1, r1, 0		#
		add r2, r0, r0		# int16 n = 16
		addi r2, r2, 16		#

		add r3, r0, r0		# int16 term1 = 0
		add r4, r0, r0		# int16 term2 = 0
		add r5, r0, r0		# int16 i = 0

		addi r3, r3, 0		# term1 = 0
		addi r4, r4, 1		# term2 = 1
		sw r4, r1, 0		# *arr = term2

		addi r1, r1, 1		# arr++

loop:	beq r5, r2, end		# while(i != n) {

		add r4, r4, r3		# term2 = term2 + term1

		sw r4, r1, 0		# *arr = term2
		lw r3, r1, -1		# term1 = *(--arr)

		addi r1, r1, 1		# arr++
		addi r5, r5, 1		# i++

		beq r0, r0, loop	# }

end:	beq r0, r0, end		# while(1) { }
