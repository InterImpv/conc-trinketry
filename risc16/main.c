//#include "instructions.h"
#include "modules.h"
#include "winpos.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ncurses.h>

#define IO_BUF_SIZE 33

/* MAIN */
int main(int argc, char **argv)
{
	mem_unit *memory = mem_create();
	if (memory == NULL) {
		fprintf(stderr, "ERR %d: no memory available\n", -E_INIT);
		goto dealloc_mems;
	}
	mem_reset(memory);
	mem_fill(memory, "asm/hello.o");

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

	/* "Upon successful completion, initscr() returns a pointer to
	 * stdscr. Otherwise, it does not return." */
	if (initscr() == NULL) {
		fprintf(stderr, "ERR %d: curses init fail\n", -E_INIT);
		goto dealloc_instr;
	}
	noecho();
	curs_set(FALSE);

	uint32_t key = '.';
	char io_buf[IO_BUF_SIZE] = "";
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
		case 'i':		// sometimes invalid ptr error happens, dunno
			echo();
			curs_set(TRUE);

			mvaddch(Y_SCANIN, X_SCANIN - 1, '>');
			mvgetnstr(Y_SCANIN, X_SCANIN, io_buf, IO_BUF_SIZE - 1);

			curs_set(FALSE);
			noecho();
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
		/* io */
		mvprintw(Y_SCANIN - 1, X_SCANIN, "%s", io_buf);
		refresh();
	} while ((key = getch()) != 'x');
	endwin();

dealloc_instr:
	instr_free(instruction);

dealloc_regs:
	reg_free(registers);

dealloc_mems:
	mem_free(memory);

	return 0;
}

