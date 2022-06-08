#include "cmdi.h"
#include "gen-err.h"

#include <iostream>
#include <cstring>
#include <ncurses.h>

#define TERMX_MIN 100
#define TERMY_MIN 24
static const char *TERMERR_SMALL = "Terminal too small!";

int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cerr << "ERR " << E_ARG << ": no file given\n";
		std::cerr << "Usage:\t./main <object-file>\n";
		return E_ARG;
	}
	
	mem_unit memory = mem_unit();
	memory.reset();
	if (memory.fill(argv[1]) != E_OK)
		return E_IO;

	reg_unit registers = reg_unit();
	registers.reset();

	ctrl_unit control = ctrl_unit();
	if (control.set_mem(&memory) != E_OK || control.set_reg(&registers) != E_OK)
		return E_IO;

	initscr();
	noecho();
	curs_set(FALSE);
	keypad(stdscr, TRUE);

	size_t scrx, scry;
	uint32_t key = '.';
	do {
		getmaxyx(stdscr, scry, scrx);
		if (scrx < TERMX_MIN || scry < TERMY_MIN) {
			clear();
			mvprintw(scry / 2, scrx / 2 - strlen(TERMERR_SMALL) / 2, TERMERR_SMALL);
			refresh();
			continue;
		}

		switch (key) {
		case 'u':
			memory.dec_rom_ptr();
			break;
		case 'j':
			memory.inc_rom_ptr();
			break;
		case 'i':
			memory.dec_ram_ptr();
			break;
		case 'k':
			memory.inc_ram_ptr();
			break;
		case 'r':
			registers.reset();
			break;
		case '\n':
			control.getline();
			control.parseio();
			break;
		case '.':
			control.fetch();
			control.decode();
			control.execute();
			break;
		case ',':
			control.cmd_exetobreak();
			break;
		};

		clear();
		memory.draw();
		registers.draw();
		control.draw();
		refresh();

	} while ( (key = getch()) != 'q');
	endwin();

	return E_OK;
}