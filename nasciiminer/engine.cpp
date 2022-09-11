#include "engine.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <bits/stdc++.h>

#include <iostream>
#include <memory>
#include <thread>
#include <ncurses.h>
#include <vector>

#define TERMX_MIN 40
#define TERMY_MIN 12
static const char *TERMERR_SMALL = "Terminal too small!";

static const uint64_t clamp_ui64(const uint64_t val, const uint64_t min, const uint64_t max)
{
	const uint64_t ret = (val < min) ? min : val;
	return (ret > max) ? max : ret;
}

static const bool in_range(const double val, const double min, const double max)
{
	return (val >= min && val <= max) ? true : false;
}

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
	timeout(50);

	this->key = '.';
	this->currlayer = 1;
	this->layers = 0;
	this->perlin = PerlinNoise();
}

void engine_t::inc_layer(void)
{
	currlayer = clamp_ui64(++currlayer, 1, layers);
}

void engine_t::dec_layer(void)
{
	currlayer = clamp_ui64(--currlayer, 1, layers);
}

void engine_t::map_generate_layer(void)
{
	//double pf = 1;
	double sf = 0.1;

	std::vector<std::string> layer;
	for (uint64_t y = 0; y < map_h; y++) {
		std::string line;
		for (uint64_t x = 0; x < map_w; x++) {
			double pn = perlin.noise((double)(x * sf), (double)(y * sf), (double)(layers * sf));
			char ch = ' ';

			if (in_range(pn, 0.75, 1)) {
				ch = ' ';
			} else if (in_range(pn, 0.5, 0.75)) {
				ch = '.';
			} else if (in_range(pn, 0.25, 0.5)) {
				ch = 'O';
			} else {
				ch = '@';
			}
			line.push_back(ch);
		}
		layer.push_back(line);
	}
	map.push_back(layer);
	layers++;
}

void engine_t::map_generate_fill(const char ch)
{
	std::vector<std::string> layer;
	for (uint64_t y = 0; y < map_h; y++) {
		std::string line;
		for (uint64_t x = 0; x < map_w; x++) {
			line.push_back(ch);
		}
		layer.push_back(line);
	}
	map.push_back(layer);
	layers++;
}

void engine_t::map_generate(const uint64_t w, const uint64_t h, const uint64_t z)
{
	this->map_w = clamp_ui64(w, 8, 80);
	this->map_h = clamp_ui64(h, 4, 24);
	this->map_z = clamp_ui64(z, 1, 64);

	this->map_generate_fill('X');
	for (uint64_t i = 0; i < this->map_z ; i++) {
		this->map_generate_layer();
	}
	this->map_generate_fill('X');
}

void engine_t::render_map(void)
{
	for (uint64_t y = 0; y < map[currlayer - 1].size(); y++) {
		for (uint64_t x = 0; x < map[currlayer - 1][y].size(); x++) {
			mvaddch(y + 1, x, map[currlayer - 1][y][x]);
		}
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
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}
		/* draw loop BEGIN */
		auto start = std::chrono::high_resolution_clock::now();
		clear();
		
		this->render_map();

		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed = end - start;
		mvprintw(0, 0, "f: %.1f", 1 / elapsed.count());
		mvprintw(0, 20, "k: 0x%08x %d", this->key, this->key);
		mvprintw(0, 40, "cl: %d, ml: %d", this->currlayer, this->layers);

		refresh();
		/* draw loop END */
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

void engine_t::simulate_frame(void)
{
	while ((this->key = getch()) != 'q') {
		switch (key) {
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
		case 'z':
			this->dec_layer();
			break;
		case 'x':
			this->inc_layer();
			break;
		}
	}
}
/* ENGINE END */
