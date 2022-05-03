#include "instructions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ncurses.h>

/* MAIN */
int main(int argc, char **argv)
{
	mem_unit *memory = mem_create();
	mem_init(memory);

	/* TODO: make unified memory space with read/write protections */
	mem_fill_rom(memory, "asm/hello.o");
	mem_fill_ram(memory, "asm/helloram.o");

	reg_unit *registers = reg_create();
	reg_init(registers);

	initscr();
	noecho();
	curs_set(FALSE);

	uint32_t key = '.';
	instr_t *instruction = instr_create();
	instr_decode(instruction, 0x0000);
	while ((key = getch()) != 'x') {
		/* get input */
		switch (key)
		{
		case 'q':
			dec_rom_ptr(memory);
			break;
		case 'a':
			inc_rom_ptr(memory);
			break;
		case 'w':
			dec_ram_ptr(memory);
			break;
		case 's':
			inc_ram_ptr(memory);
			break;
		case '.':
			instr_decode(instruction, instr_fetch(memory, registers));
			instr_exec(instruction, registers, memory);
			break;
		};

		/* draw to 80 x 24 terminal
		 * TODO: handle resize actions */
		clear();
		draw_rom(memory);
		draw_ram(memory);
		draw_regs(registers);
		draw_instr(instruction);
		draw_stdout(memory);
		refresh();
	}
	instr_free(instruction);

	mem_free(memory);
	reg_free(registers);

	endwin();

	return 0;
}

