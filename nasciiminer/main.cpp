#include <curses.h>
#include "engine.h"

#include <thread>

int main(void)
{
	engine_t m_engine;
	m_engine.map_generate(24);

	std::thread render_th(&engine_t::render_frame, &m_engine);
	std::thread simulate_th(&engine_t::simulate_frame, &m_engine);
	render_th.join();
	simulate_th.join();

	endwin();
	return 0;
}