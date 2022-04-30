#include "instructions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* MAIN */
int main(int argc, char **argv)
{
	mem_unit memory;
	memory.ram_ptr = 0;
	memset(memory.rom, 0, MEM_SIZE * sizeof(uint16_t));
	memset(memory.ram, 0, MEM_SIZE * sizeof(uint16_t));

	/* fibbonachi sequence */
	memory.rom[0] = 0x0400;
	memory.rom[1] = 0x2480;
	memory.rom[2] = 0x0800;
	memory.rom[3] = 0x2910;
	memory.rom[4] = 0x0C00;
	memory.rom[5] = 0x1000;
	memory.rom[6] = 0x1400;
	memory.rom[7] = 0x2D80;
	memory.rom[8] = 0x3201;
	memory.rom[9] = 0x9080;
	memory.rom[10] = 0x2481;
	memory.rom[11] = 0xD506;
	memory.rom[12] = 0x1203;
	memory.rom[13] = 0x9080;
	memory.rom[14] = 0xACFF;
	memory.rom[15] = 0x2481;
	memory.rom[16] = 0x3681;
	memory.rom[17] = 0xC079;
	memory.rom[18] = 0xC07F;

	uint32_t clk_cycles = 11 + 7 * 16;		// magic numbers

	reg_unit registers;
	registers.pc = 0;
	memset(registers.rx, 0, N_OF_REGS * sizeof(uint16_t));

	instr_t *instruction = instr_create();
	for (uint32_t i = 0; i < clk_cycles; i++) {
		instr_dec_fill(instruction, instr_fetch(&memory, &registers));
		instr_exec(instruction, &registers, &memory);

		print_instr(instruction);
		print_regs(&registers);
		print_mem(&memory);
	}
	instr_free(instruction);

	return 0;
}

