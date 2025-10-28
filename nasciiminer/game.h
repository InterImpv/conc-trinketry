#ifndef __GAME_H__
#define __GAME_H__

#include "error.h"
#include "vector.h"
#include "region.h"
#include "rand.h"
#include <stddef.h>
#include <stdint.h>

#define VERSION (1UL)

#define MAP_DIM_MAX_SIZE_X (80UL)
#define MAP_DIM_MAX_SIZE_Y (24UL)

#define ENTITY_NAME_SIZE (32UL)
#define TILE_NAME_SIZE   (32UL)

#define GAME_MAX_TILES  (256UL)
#define GAME_COLORS     (8UL)
#define GAME_MAX_COLORS (GAME_COLORS * GAME_COLORS)

typedef struct palette_color {
	int fore;
	int back;
} palette_color_t;

typedef struct palette {
	palette_color_t color[GAME_MAX_COLORS];
} palette_t;

typedef enum {
	TILE_SIM_NONE = 0,
	TILE_SIM_FALLING,
	TILE_SIM_LIQUID,
	TILE_SIM_GAS,
} tile_sim_t;

typedef struct map_tile_flags {
	uint32_t is_solid : 1;
	uint32_t attr_blink : 1;
	uint32_t attr_dim : 1;
	uint32_t attr_bold : 1;
} tile_flags_t;

typedef struct map_tile {
	uint32_t id;
	palette_color_t color;
	int color_pair;
	char symbol;
	tile_sim_t simulation;
	tile_flags_t flags;
} tile_t;

typedef struct game_map {
	bool initialized;
	/* screen pos */
	vect2i_t pos;
	/* data */
	size_t size_x;
	size_t size_y;
	tile_t *data;
} map_t;

typedef struct game_entity {
	bool initialized;
	/* data */
	tile_t tile;
	vect2i_t pos;
	vect2i_t prev_pos;
	int32_t hp;
	char name[ENTITY_NAME_SIZE];
} entity_t;

typedef struct game_controller_configuration {
	int32_t tick_rate;
	vect2i_t map_size;
} game_config_t;

typedef struct game_tile_definitions {
	size_t count;
	char name[GAME_MAX_TILES][TILE_NAME_SIZE];
	tile_t tile[GAME_MAX_TILES];
} game_tile_def_t;

typedef struct game_controller {
	bool initialized;
	game_config_t config;
	palette_t palette;
	FILE *log;
	/* ui */
	vect2i_t ui_pos;
	/* data */
	urand_t urand;
	int32_t build_tile_id;
	entity_t player;
	map_t map;
	/* tile dictionary */
	tile_t *tile_empty;
	game_tile_def_t tile_def;
} game_t;

typedef struct game_save_file_header {
	uint32_t version;
	game_config_t config;
} __attribute__((packed)) game_save_hdr_t;

err_t palette_set_color(palette_t *this, size_t n, int fore, int back);
int palette_lookup_pair(palette_t *this, int fore, int back);
palette_color_t *palette_lookup_color(palette_t *this, int pair);

err_t tile_set(tile_t *this, char symbol, uint32_t color);
err_t tile_copy(tile_t *dst, const tile_t *src);

err_t map_init(map_t *this);
err_t map_alloc(map_t *this, vect2i_t *vsize);
err_t map_delete(map_t *this);
err_t map_set_size(map_t *this, vect2i_t *size);
err_t map_set_pos(map_t *this, int32_t x, int32_t y);
tile_t *map_get_tile(map_t *this, int32_t x, int32_t y);
err_t map_set_tile(map_t *this, int32_t x, int32_t y, const tile_t *tile);
err_t map_fill_region(map_t *this, region2i_t *region, const tile_t *tile);

err_t entity_init(entity_t *this);
err_t entity_move(entity_t *this, vect2i_t *dt, game_t *game);

err_t game_init(game_t *this, game_config_t *config);
err_t game_free(game_t *this);

err_t game_set_ui_pos(game_t *this, int32_t x, int32_t y);

err_t game_load(game_t *this, const char *filename);
err_t game_save(game_t *this, const char *filename);

err_t game_map_alloc(game_t *this);
tile_t *game_map_get_tile(game_t *this, int32_t x, int32_t y);
err_t game_map_set_tile(game_t *this, int32_t x, int32_t y, const tile_t *tile);

err_t game_map_generate_default(game_t *this);
err_t game_map_generate_noise(game_t *this);
err_t game_map_generate_test(game_t *this, uint32_t type);

err_t game_map_basic_smooth(game_t *this, size_t iterations);

err_t game_map_tick_tiles(game_t *this);
err_t game_map_display(game_t *this);

err_t game_entity_display(game_t *this, entity_t *ent);

err_t game_loop(game_t *this);

#endif
