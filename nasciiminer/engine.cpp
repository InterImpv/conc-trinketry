#include "engine.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <thread>
#include <ncurses.h>

#define TERMX_MIN 40
#define TERMY_MIN 12
static const char *TERMERR_SMALL = "Terminal too small!";

/* POINT BEGIN */
point32_t::point32_t(void)
{
	this->x = 0;
	this->y = 0;
}

point32_t::point32_t(const uint32_t x, const uint32_t y)
{
	this->x = x;
	this->y = y;
}

point32_t::point32_t(const point32_t &p)
{
	this->x = p.x;
	this->y = p.y;
}
/* POINT END */

/* ENGINE BEGIN */

engine_t::engine_t(void)
{
	/* curses init */
	initscr();
	noecho();
	curs_set(0);
	timeout(-1);

	this->key = '.';
}

void engine_t::map_generate(uint64_t size)
{
	if (size > 576)
		size = 576;

	for (uint64_t i = 0; i < size; i++) {
		this->map.push_back(std::string(size, '#'));
	}
}

void engine_t::render_map(void)
{
	uint64_t line = 1;
	for (auto it : this->map) {
		mvaddnstr(line, 0, it.c_str(), this->scrsize.x);

		line++;
		if (line > scrsize.y)
			break;
	}
}

void engine_t::render_frame(void)
{
	uint32_t scrx = 0, scry = 0;
	getmaxyx(stdscr, scry, scrx);
	this->scrsize = point32_t(scrx, scry);

	while (this->key != 'q') {
		getmaxyx(stdscr, scry, scrx);
		this->scrsize = point32_t(scrx, scry);

		/* temporary check*/
		if (scrx < TERMX_MIN || scry < TERMY_MIN) {
			clear();
			mvprintw(scry / 2, scrx / 2 - std::strlen(TERMERR_SMALL) / 2, TERMERR_SMALL);
			refresh();
			std::this_thread::sleep_for(std::chrono::milliseconds(33));
			continue;
		}

		/* draw loop BEGIN */
		clear();
		
		mvprintw(0, 0, "k: 0x%08x %d", this->key, this->key);
		this->render_map();

		refresh();
		/* draw loop END */
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}
}

void engine_t::simulate_frame(void)
{
	while ((this->key = getch()) != 'q') {
		// switch (key) {
		// case 'w':
		// 	this->player_move_up();
		// 	break;
		// case 's':
		// 	this->player_move_down();
		// 	break;
		// case 'a':
		// 	this->player_move_left();
		// 	break;
		// case 'd':
		// 	this->player_move_right();
		// 	break;
		// case 'z':
		// 	this->inc_degree_step();
		// 	break;
		// case 'x':
		// 	this->dec_degree_step();
		// 	break;
		// }
	}
}
/* ENGINE END */
