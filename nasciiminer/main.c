#include "error.h"
#include "utils.h"
#include "vector.h"
#include "matrix.h"

#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>

#define SIZE_X  (80UL)
#define SIZE_Y  (24UL)
#define SIZE_XY (SIZE_X * SIZE_Y)

#define TICKS_PER_SECOND (60UL)
#define UPDATE_MS		 (1000UL / TICKS_PER_SECOND)

int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "invalid argument count %i!\n", argc);
		exit(-1);
	}
	/* get map size */
	vect2i_t size = { 0 };
	size.x = atoi(argv[1]);
	size.y = atoi(argv[2]);
	if (size.x == 0 || size.x > SIZE_X) {
		fprintf(stderr, "invalid X size %i!\n", size.x);
		exit(-1);
	}
	if (size.y == 0 || size.y > SIZE_Y) {
		fprintf(stderr, "invalid Y size %i!\n", size.y);
		exit(-1);
	}
	/* init game config */
	game_config_t game_config = { 0 };
	game_config.tick_rate = UPDATE_MS;
	game_config.map_size.x = size.x;
	game_config.map_size.y = size.y;
	/* init game */
	game_t game = { 0 };
	if (game_init(&game, &game_config) != RET_OK) {
		fprintf(stderr, "init failed!\n");
		goto exit;
	}
	if (game_load(&game, "save.dat") != RET_OK) {
		game_map_alloc(&game);
		game_map_generate_noise(&game);
		game_map_basic_smooth(&game, 100);
	}
	map_set_pos(&game.map, 0, 0);
	/* loop */
	game_loop(&game);
	/* end game */
	game_save(&game, "save.dat");
exit:
	game_free(&game);
	endwin();

	return 0;
}

