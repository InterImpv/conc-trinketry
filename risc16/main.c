#include "instructions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* MAIN */
int main(int argc, char **argv)
{
	mem_unit *memory = mem_create();
	mem_init(memory);
	mem_fill(memory, "asm/fibb.o");

	/* magic numbers */
	uint32_t clk_cycles = 11 + 7 * 16;

	reg_unit *registers = reg_create();
	reg_init(registers);

	instr_t *instruction = instr_create();
	for (uint32_t i = 0; i < clk_cycles; i++) {
		instr_dec_fill(instruction, instr_fetch(memory, registers));
		instr_exec(instruction, registers, memory);

		print_instr(instruction);
		print_regs(registers);
		print_mem(memory);
	}
	instr_free(instruction);

	mem_free(memory);
	reg_free(registers);

	return 0;
}

