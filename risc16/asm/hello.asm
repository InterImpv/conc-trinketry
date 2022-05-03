start:	movi r1, 0x0080		# char *stdout = 0x0080
		movi r2, 0x0001		# char *str = "Hello, World!"
		movi r3, 0x0000		# uint16 str_size = @0x0000

		lw r3, r3, 0		# str_size = *(uint16 *)str_size

loop:	beq r3, r0, end		# while (str_size != 0) {

		lw r4, r2, 0		# *stdout = *str
		sw r4, r1, 0

		addi r1, r1, 1		# stdout++
		addi r2, r2, 1		# str++

		nand r3, r3, r3		# str_size--;
		addi r3, r3, 1
		nand r3, r3, r3

		beq r0, r0, loop	# }

end:	beq r0, r0, end		# while(1) { }

