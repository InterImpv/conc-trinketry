#include "cmdi.h"
#include "winpos.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ncurses.h>

/* MAIN */
int main(int argc, char **argv)
{
	mem_unit *memory = mem_create();
	if (memory == NULL) {
		fprintf(stderr, "ERR %d: no memory available\n", -E_INIT);
		goto dealloc_mems;
	}
	mem_reset(memory);
	if (mem_fill(memory, "asm/hello.o") == E_IO) {
		goto dealloc_mems;
	}

	reg_unit *registers = reg_create();
	if (registers == NULL) {
		fprintf(stderr, "ERR %d: no memory available\n", -E_INIT);
		goto dealloc_regs;
	}
	reg_reset(registers);

	instr_t *instruction = instr_create();
	if (instruction == NULL) {
		fprintf(stderr, "ERR %d: no memory available\n", -E_INIT);
		goto dealloc_instr;
	}
	instr_decode(instruction, 0x0000);

	cmd_t *cmdunit = cmd_create();
	if (instruction == NULL) {
		fprintf(stderr, "ERR %d: no memory available\n", -E_INIT);
		goto dealloc_cmd;
	}
	cmd_init(cmdunit, registers, memory);

	/* "Upon successful completion, initscr() returns a pointer to
	 * stdscr. Otherwise, it does not return." */
	if (initscr() == NULL) {
		fprintf(stderr, "ERR %d: curses init fail\n", -E_INIT);
		goto dealloc_cmd;
	}
	noecho();
	curs_set(FALSE);
	keypad(stdscr, TRUE);

	uint32_t key = '.';
	do {
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
		case 'r':
			reg_reset(registers);
			break;
		case '\n':
			cmd_getline(cmdunit);
			break;
		case '.':
			instr_decode(instruction, instr_fetch(memory, registers));
			instr_exec(instruction, registers, memory);
			break;
		};

		/* draw to 80 x 24 terminal
		 * TODO: handle resize actions */
		clear();
		draw_mem(memory);
		draw_regs(registers);
		draw_instr(instruction);
		draw_stdout(memory);
		draw_cmdiobuf(cmdunit);
		refresh();
	} while ((key = getch()) != 'x');
	endwin();

dealloc_cmd:
	cmd_free(cmdunit);

dealloc_instr:
	instr_free(instruction);

dealloc_regs:
	reg_free(registers);

dealloc_mems:
	mem_free(memory);

	return 0;
}

