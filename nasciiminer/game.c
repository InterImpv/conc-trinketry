#include "game.h"
#include "error.h"
#include "rand.h"
#include "vector.h"
#include <curses.h>
#include <stdint.h>
#include <stdio.h>

/*
COLOR_BLACK   0
COLOR_RED     1
COLOR_GREEN   2
COLOR_YELLOW  3
COLOR_BLUE    4
COLOR_MAGENTA 5
COLOR_CYAN    6
COLOR_WHITE   7
*/

#define CPAIR_BLACK_ON_BLACK   (1)

#define CPAIR_BLACK_ON_RED     (2)
#define CPAIR_BLACK_ON_GREEN   (3)
#define CPAIR_BLACK_ON_YELLOW  (4)
#define CPAIR_BLACK_ON_BLUE    (5)
#define CPAIR_BLACK_ON_MAGENTA (6)
#define CPAIR_BLACK_ON_CYAN    (7)
#define CPAIR_BLACK_ON_WHITE   (8)

#define CPAIR_RED_ON_BLACK     (9)
#define CPAIR_GREEN_ON_BLACK   (10)
#define CPAIR_YELLOW_ON_BLACK  (11)
#define CPAIR_BLUE_ON_BLACK    (12)
#define CPAIR_MAGENTA_ON_BLACK (13)
#define CPAIR_CYAN_ON_BLACK    (14)
#define CPAIR_WHITE_ON_BLACK   (15)

static const char *get_color_str(int col)
{
	switch (col) {
	case COLOR_BLACK:
		return "black";
	case COLOR_RED:
		return "red";
	case COLOR_GREEN:
		return "green";
	case COLOR_YELLOW:
		return "yellow";
	case COLOR_BLUE:
		return "blue";
	case COLOR_MAGENTA:
		return "magenta";
	case COLOR_CYAN:
		return "cyan";
	case COLOR_WHITE:
		return "white";

	default:
		return "unknown";
	}
}

err_t palette_set_color(palette_t *this, size_t n, int fore, int back)
{
	RETURN_IF_NULL(this, RET_ERR);
	if (n >= GAME_MAX_COLORS)
		return RET_ERR;
	/* all ok */
	this->color[n].fore = fore;
	this->color[n].back = back;
	init_pair(n + 1, fore, back);
	return RET_OK;
}

int palette_lookup_pair(palette_t *this, int fore, int back)
{
	int pair = 0;
	RETURN_IF_NULL(this, pair);
	/* all ok */
	for (size_t i = 0; i < GAME_MAX_COLORS; i++) {
		if (this->color[i].fore == fore && this->color[i].back == back) {
			pair = i + 1;
			break;
		}
	}
	return pair;
}

palette_color_t *palette_lookup_color(palette_t *this, int pair)
{
	RETURN_IF_NULL(this, &this->color[0]);
	if (pair >= GAME_MAX_COLORS)
		return &this->color[0];
	return &this->color[pair];
}

err_t tile_set(tile_t *this, char symbol, uint32_t color)
{
	RETURN_IF_NULL(this, RET_ERR);
	this->symbol = symbol;
	this->color_pair = color;
	return RET_OK;
}

err_t tile_copy(tile_t *dst, const tile_t *src)
{
	RETURN_IF_NULL(dst, RET_ERR);
	RETURN_IF_NULL(src, RET_ERR);
	memcpy(dst, src, sizeof(*dst));
	return RET_OK;
}

err_t map_init(map_t *this)
{
	RETURN_IF_NULL(this, RET_ERR);
	if (this->data) {
		free(this->data);
	}
	memset(this, 0, sizeof(*this));
	/* all ok */
	this->initialized = true;
	return RET_OK;
}

err_t map_alloc(map_t *this, vect2i_t *vsize)
{
	RETURN_IF_NULL(this, RET_ERR);
	/* check size */
	if (vsize->x > MAP_DIM_MAX_SIZE_X || vsize->y > MAP_DIM_MAX_SIZE_Y) {
		return RET_ERR;
	}
	/* allocate */
	map_set_size(this, vsize);
	this->data = malloc(vsize->x * vsize->y * sizeof(this->data[0]));
	/* check */
	if (!this->data) {
		return RET_ERR;
	}
	/* all ok */
	return RET_OK;
}

err_t map_delete(map_t *this)
{
	RETURN_IF_NULL(this, RET_ERR);
	if (this->data) {
		free(this->data);
	}
	map_set_size(this, &(vect2i_t){0, 0});
	return RET_OK;
}

err_t map_set_size(map_t *this, vect2i_t *size)
{
	RETURN_IF_NULL(this, RET_ERR);
	this->size_x = size->x;
	this->size_y = size->y;
	return RET_OK;
}

err_t map_set_pos(map_t *this, int32_t x, int32_t y)
{
	RETURN_IF_NULL(this, RET_ERR);
	this->pos.x = x;
	this->pos.y = y;
	return RET_OK;
}

tile_t *map_get_tile(map_t *this, int32_t x, int32_t y)
{
	if (x < 0 || x >= this->size_x || y < 0 || y >= this->size_y)
		return NULL;
	/* get tile */
	return &this->data[y * this->size_x + x];
}

err_t map_set_tile(map_t *this, int32_t x, int32_t y, const tile_t *tile)
{
	tile_t *t = map_get_tile(this, x, y);
	RETURN_IF_NULL(t, RET_ERR);
	/* set tile */
	tile_copy(&this->data[y * this->size_x + x], tile);
	return RET_OK;
}

err_t map_fill_region(map_t *this, region2i_t *region, const tile_t *tile)
{
	RETURN_IF_NULL(this, RET_ERR);
	for (int32_t y = region->y; y < region->y + region->h; y++) {
		for (int32_t x = region->x; x < region->x + region->w; x++) {
			map_set_tile(this, x, y, tile);
		}
	}
	/* all ok */
	return RET_OK;
}

err_t entity_init(entity_t *this)
{
	RETURN_IF_NULL(this, RET_ERR);
	memset(this, 0, sizeof(*this));
	this->tile.id = 0;
	this->tile.color_pair = CPAIR_RED_ON_BLACK;
	this->hp = -1;
	strcpy(this->name, "unknown");
	/* all ok */
	this->initialized = true;
	return RET_OK;
}

err_t entity_move(entity_t *this, vect2i_t *dt, game_t *game)
{
	RETURN_IF_NULL(this, RET_ERR);
	RETURN_IF_NULL(dt, RET_ERR);
	RETURN_IF_NULL(game, RET_ERR);
	tile_t *target = game_map_get_tile(game, this->pos.x + dt->x, this->pos.y + dt->y);
	if (target && !target->flags.is_solid) {
		this->prev_pos.x = this->pos.x;
		this->prev_pos.y = this->pos.y;
		this->pos.x = this->pos.x + dt->x;
		this->pos.y = this->pos.y + dt->y;
	}
	/* all ok */
	return RET_OK;
}

static err_t game_init_tile_colors(game_t *this)
{
	if (!has_colors()) {
		fprintf(this->log, "no colors\n");
		return RET_ERR;
	}
	start_color();
	for (int fore = 0; fore < GAME_COLORS; fore++) {
		for (int back = 0; back < GAME_COLORS; back++) {
			int n = fore * GAME_COLORS + back;
			palette_set_color(&this->palette, n, fore, back);
			fprintf(this->log, "color pair %i = %s, %s\n", n + 1, get_color_str(fore), get_color_str(back));
		}
	}
	/* all ok */
	return RET_OK;
}

static err_t game_add_tile(game_t *this, const char *name, tile_t *def)
{
	if (this->tile_def.count >= GAME_MAX_TILES)
		return RET_ERR;
	/* add */
	def->id = this->tile_def.count;
	strncpy(this->tile_def.name[this->tile_def.count], name, TILE_NAME_SIZE - 1);
	memcpy(&this->tile_def.tile[this->tile_def.count], def, sizeof(this->tile_def.tile[0]));
	this->tile_def.count++;
	return RET_OK;
}

static err_t game_init_tile_dict(game_t *this)
{
	/* player entity */
	this->player.tile.id = -1;
	this->player.tile.color.fore = COLOR_RED;
	this->player.tile.color.back = COLOR_BLACK;
	this->player.tile.color_pair = palette_lookup_pair(&this->palette, this->player.tile.color.fore, this->player.tile.color.back);
	this->player.tile.symbol = '@';
	this->player.tile.simulation = TILE_SIM_NONE;
	this->player.tile.flags.is_solid = true;
	this->player.tile.flags.attr_bold = true;
	this->player.hp = -1;
	strcpy(this->player.name, "player");
	/* tiles */
	tile_t tile;
	/* - air */
	memset(&tile, 0, sizeof(tile));
	tile.color.fore = COLOR_WHITE;
	tile.color.back = COLOR_BLACK;
	tile.color_pair = palette_lookup_pair(&this->palette, tile.color.fore, tile.color.back);
	tile.symbol = ' ';
	tile.simulation = TILE_SIM_NONE;
	tile.flags.is_solid = false;
	game_add_tile(this, "air", &tile);
	this->tile_empty = &this->tile_def.tile[0]; // special empty tile
	/* - solid wall */
	memset(&tile, 0, sizeof(tile));
	tile.color.fore = COLOR_BLACK;
	tile.color.back = COLOR_WHITE;
	tile.color_pair = palette_lookup_pair(&this->palette, tile.color.fore, tile.color.back);
	tile.symbol = ' ';
	tile.simulation = TILE_SIM_NONE;
	tile.flags.is_solid = true;
	tile.flags.attr_bold = true;
	game_add_tile(this, "stone", &tile);
	/* - sand wall */
	memset(&tile, 0, sizeof(tile));
	tile.color.fore = COLOR_BLACK;
	tile.color.back = COLOR_YELLOW;
	tile.color_pair = palette_lookup_pair(&this->palette, tile.color.fore, tile.color.back);
	tile.symbol = ' ';
	tile.simulation = TILE_SIM_FALLING;
	tile.flags.is_solid = true;
	game_add_tile(this, "sand", &tile);
	/* - water */
	memset(&tile, 0, sizeof(tile));
	tile.color.fore = COLOR_CYAN;
	tile.color.back = COLOR_BLUE;
	tile.color_pair = palette_lookup_pair(&this->palette, tile.color.fore, tile.color.back);
	tile.symbol = '~';
	tile.simulation = TILE_SIM_LIQUID;
	tile.flags.is_solid = false;
	game_add_tile(this, "water", &tile);
	/* - gas */
	memset(&tile, 0, sizeof(tile));
	tile.color.fore = COLOR_MAGENTA;
	tile.color.back = COLOR_BLACK;
	tile.color_pair = palette_lookup_pair(&this->palette, tile.color.fore, tile.color.back);
	tile.symbol = '*';
	tile.simulation = TILE_SIM_GAS;
	tile.flags.is_solid = false;
	game_add_tile(this, "gas", &tile);
	/* print */
	for (size_t i = 0; i < this->tile_def.count; i++) {
		fprintf(this->log, "tile [%u]:\n", this->tile_def.tile[i].id);
		fprintf(this->log, " -> name = %s\n", this->tile_def.name[i]);
		fprintf(this->log, " -> symbol = '%c'\n", this->tile_def.tile[i].symbol);
		fprintf(this->log, " -> fore   = %i\n", this->tile_def.tile[i].color.fore);
		fprintf(this->log, " -> back   = %i\n", this->tile_def.tile[i].color.back);
		fprintf(this->log, " -> pair   = %i\n", this->tile_def.tile[i].color_pair);
	}
	/* all ok */
	return RET_OK;
}

static err_t game_init_curses(game_t *this)
{
	RETURN_IF_NULL(this, RET_ERR);
	/* init curses */
	initscr();
	noecho();
	timeout(this->config.tick_rate);
	keypad(stdscr, TRUE);
	curs_set(0);
	clear();
	fprintf(this->log, "curses initialized (tr=%i)\n", this->config.tick_rate);
	/* all ok */
	return RET_OK;
}

err_t game_init(game_t *this, game_config_t *config)
{
	RETURN_IF_NULL(this, RET_ERR);
	RETURN_IF_NULL(config, RET_ERR);
	memset(this, 0, sizeof(*this));
	memcpy(&this->config, config, sizeof(this->config));
	/* open log file */
	this->log = fopen("log.txt", "w");
	RETURN_IF_NULL(this->log, RET_ERR);
	/* game info */
	game_init_curses(this);
	urand_init(&this->urand);
	entity_init(&this->player);
	game_init_tile_colors(this);
	game_init_tile_dict(this);
	game_set_ui_pos(this, this->config.map_size.x + 1, 0);
	/* init objects */
	map_init(&this->map);
	/* all ok */
	this->initialized = true;
	return RET_OK;
}

err_t game_free(game_t *this)
{
	RETURN_IF_NULL(this, RET_ERR);
	urand_close(&this->urand);
	map_delete(&this->map);
	fclose(this->log);
	/* all ok */
	return RET_OK;
}

err_t game_set_ui_pos(game_t *this, int32_t x, int32_t y)
{
	RETURN_IF_NULL(this, RET_ERR);
	this->ui_pos.x = x;
	this->ui_pos.y = y;
	/* all ok */
	return RET_OK;
}

err_t game_load(game_t *this, const char *filename)
{
	RETURN_IF_NULL(this, RET_ERR);
	FILE *fd_save = fopen(filename, "r");
	RETURN_IF_NULL(fd_save, RET_ERR);
	/* make header */
	game_save_hdr_t hdr = { 0 };
	/* read header */
	fread(&hdr, sizeof(hdr), 1, fd_save);
	/* load config */
	this->config.map_size.x = hdr.config.map_size.x;
	this->config.map_size.y = hdr.config.map_size.y;
	fprintf(this->log, "version = 0x%08X\n", hdr.version);
	fprintf(this->log, "map_size = (%i, %i)\n", hdr.config.map_size.x, hdr.config.map_size.y);
	fprintf(this->log, "map_size = (%i, %i)\n", this->config.map_size.x, this->config.map_size.y);
	/* load map */
	game_map_alloc(this);
	fprintf(this->log, "map_size = (%lu, %lu)\n", this->map.size_x, this->map.size_y);
	for (int32_t y = 0; y < this->map.size_y; y++) {
		tile_t *row = game_map_get_tile(this, 0, y);
		fread(row, sizeof(*row), this->map.size_x, fd_save);
	}
	/* load player */
	fread(&this->player, sizeof(this->player), 1, fd_save);
	/* all ok */
	fclose(fd_save);
	return RET_OK;
}

err_t game_save(game_t *this, const char *filename)
{
	RETURN_IF_NULL(this, RET_ERR);
	FILE *fd_save = fopen(filename, "w");
	RETURN_IF_NULL(fd_save, RET_ERR);
	/* make header */
	game_save_hdr_t hdr = { 0 };
	hdr.version = VERSION;
	memcpy(&hdr.config, &this->config, sizeof(hdr.config));
	/* write header */
	fwrite(&hdr, sizeof(hdr), 1, fd_save);
	/* write map */
	for (int32_t y = 0; y < this->map.size_y; y++) {
		tile_t *row = game_map_get_tile(this, 0, y);
		fwrite(row, sizeof(*row), this->map.size_x, fd_save);
	}
	/* write player */
	fwrite(&this->player, sizeof(this->player), 1, fd_save);
	/* close */
	fclose(fd_save);
	/* all ok */
	return RET_OK;
}

err_t game_map_alloc(game_t *this)
{
	RETURN_IF_NULL(this, RET_ERR);
	map_alloc(&this->map, &this->config.map_size);
	region2i_t screen = { 0 };
	screen.x = 0;
	screen.y = 0;
	screen.h = this->config.map_size.y;
	screen.w = this->config.map_size.x;
	map_fill_region(&this->map, &screen, this->tile_empty);
	/* all ok */
	return RET_OK;
}

inline tile_t *game_map_get_tile(game_t *this, int32_t x, int32_t y)
{
	return map_get_tile(&this->map, x, y);
}

inline err_t game_map_set_tile(game_t *this, int32_t x, int32_t y, const tile_t *tile)
{
	return map_set_tile(&this->map, x, y, tile);
}

err_t game_map_generate_default(game_t *this)
{
	RETURN_IF_NULL(this, RET_ERR);
	for (size_t y = 0; y < this->config.map_size.y; y++) {
		for (size_t x = 0; x < this->config.map_size.x; x++) {
			size_t r = urand_i32(&this->urand, 0, this->tile_def.count);
			game_map_set_tile(this, x, y, &this->tile_def.tile[r]);
		}
	}
	/* all ok */
	return RET_OK;
}

err_t game_map_generate_noise(game_t *this)
{
	RETURN_IF_NULL(this, RET_ERR);
	for (size_t y = 0; y < this->config.map_size.y; y++) {
		for (size_t x = 0; x < this->config.map_size.x; x++) {
			size_t r = urand_i32(&this->urand, 0, this->tile_def.count);
			game_map_set_tile(this, x, y, &this->tile_def.tile[r]);
		}
	}
	/* all ok */
	return RET_OK;
}

err_t game_map_generate_test(game_t *this, uint32_t type)
{
	RETURN_IF_NULL(this, RET_ERR);
	region2i_t screen = { 0 };
	switch (type) {
	case 0:
		/* refill with air */
		screen.x = 0;
		screen.y = 0;
		screen.h = this->config.map_size.y;
		screen.w = this->config.map_size.x;
		map_fill_region(&this->map, &screen, this->tile_empty);
		/* make sand */
		screen.x = 0;
		screen.y = 0;
		screen.h = 3;
		screen.w = this->config.map_size.x;
		map_fill_region(&this->map, &screen, &this->tile_def.tile[2]);
		/* make platform */
		screen.x = this->config.map_size.x / 2 - (this->config.map_size.x / 2) / 2;
		screen.y = 8;
		screen.h = 1;
		screen.w = this->config.map_size.x / 2;
		map_fill_region(&this->map, &screen, &this->tile_def.tile[1]);
		break;

	default:
		break;
	}
	/* all ok */
	return RET_OK;
}

err_t game_map_basic_smooth(game_t *this, size_t iterations)
{
	for (size_t k = 0; k < iterations; k++) {
		for (size_t y = 0; y < this->config.map_size.y; y++) {
			for (size_t x = 0; x < this->config.map_size.x; x++) {
				int32_t v = 0;
				tile_t *curr = game_map_get_tile(this, x, y);
				tile_t *new = curr;
				tile_t *ut = game_map_get_tile(this, x, y - 1);
				tile_t *dt = game_map_get_tile(this, x, y + 1);
				tile_t *lt = game_map_get_tile(this, x - 1, y);
				tile_t *rt = game_map_get_tile(this, x + 1, y);
				/* select random neighbour */
				v = urand_i32(&this->urand, 0, 4);
				switch (v) {
				case 0:
					new = ut;
					break;
				case 1:
					new = ut;
					break;
				case 2:
					new = ut;
					break;
				case 3:
					new = ut;
					break;
				
				default:
					break;
				}
				/* clear tiles surrounded by air */
				if ((!ut || !ut->flags.is_solid) && (!dt || !dt->flags.is_solid) &&
					(!lt || !lt->flags.is_solid) && (!rt || !rt->flags.is_solid))
				{
					tile_copy(curr, this->tile_empty);
				}
				/* clear tiles surrounded by solid */
				if ((!ut || ut->flags.is_solid) && (!dt || dt->flags.is_solid) &&
					(!lt || lt->flags.is_solid) && (!rt || rt->flags.is_solid))
				{
					tile_copy(curr, new);
				}
			}
		}
	}
	return RET_OK;
}

static err_t game_map_tick_tile(game_t *this, tile_t *tile, int32_t x, int32_t y)
{
	tile_t tmp = { 0 };
	tile_t *utile = game_map_get_tile(this, x, y - 1);
	tile_t *dtile = game_map_get_tile(this, x, y + 1);
	tile_t *htile = NULL;
	tile_t *dhtile = NULL;
	if (urand_i32(&this->urand, 0, 2)) {
		htile = game_map_get_tile(this, x - 1, y);
		dhtile = game_map_get_tile(this, x - 1, y + 1);
	} else {
		htile = game_map_get_tile(this, x + 1, y);
		dhtile = game_map_get_tile(this, x + 1, y + 1);
	}
	/* simulate by type */
	switch (tile->simulation) {
	case TILE_SIM_FALLING:
		/* check if tile under current is solid, if not check horizontal */
		if (dtile && !dtile->flags.is_solid) {
			tile_copy(&tmp, dtile);
			tile_copy(dtile, tile);
			tile_copy(tile, &tmp);
		} else {
			if (dhtile && !dhtile->flags.is_solid) {
				tile_copy(&tmp, dhtile);
				tile_copy(dhtile, tile);
				tile_copy(tile, &tmp);
			}
		}
		break;
	case TILE_SIM_LIQUID:
		/* check if tile under current is solid, if not check horizontal */
		if (dtile && !dtile->flags.is_solid && dtile->id != tile->id) {
			tile_copy(&tmp, dtile);
			tile_copy(dtile, tile);
			tile_copy(tile, &tmp);
		} else {
			if (htile && !htile->flags.is_solid && htile->id != tile->id) {
				tile_copy(&tmp, htile);
				tile_copy(htile, tile);
				tile_copy(tile, &tmp);
			}
		}
		break;
	case TILE_SIM_GAS:
		/* check if tile above current is solid, if not check horizontal */
		if (utile && !utile->flags.is_solid && utile->id != tile->id) {
			tile_copy(&tmp, utile);
			tile_copy(utile, tile);
			tile_copy(tile, &tmp);
		} else {
			if (htile && !htile->flags.is_solid && htile->id != tile->id) {
				tile_copy(&tmp, htile);
				tile_copy(htile, tile);
				tile_copy(tile, &tmp);
			}
		}
		break;

	default:
		break;
	}
	return RET_OK;
}

err_t game_map_tick_tiles(game_t *this)
{
	RETURN_IF_NULL(this, RET_ERR);
	for (int32_t y = this->config.map_size.y - 1; y >= 0; y--) {
		for (int32_t x = this->config.map_size.x - 1; x >= 0; x--) {
			tile_t *t = game_map_get_tile(this, x, y);
			if (!t)
				continue;
			/* set display flags */
			game_map_tick_tile(this, t, x, y);
		}
	}
	/* all ok */
	return RET_OK;
}

err_t game_map_display(game_t *this)
{
	RETURN_IF_NULL(this, RET_ERR);
	for (int32_t y = 0; y < this->config.map_size.y; y++) {
		for (int32_t x = 0; x < this->config.map_size.x; x++) {
			tile_t *t = game_map_get_tile(this, x, y);
			if (!t)
				continue;
			/* set display flags */
			int32_t flags = 0;
			if (t->flags.attr_blink) {
				flags |= A_BLINK;
			};
			if (t->flags.attr_dim) {
				flags |= A_DIM;
			};
			if (t->flags.attr_bold) {
				flags |= A_BOLD;
			};
			mvaddch(this->map.pos.y + y, this->map.pos.x + x, t->symbol | flags | COLOR_PAIR(t->color_pair));
		}
	}
	/* all ok */
	return RET_OK;
}

err_t game_entity_display(game_t *this, entity_t *ent)
{
	RETURN_IF_NULL(this, RET_ERR);
	/* get tile at player position */
	tile_t *t = game_map_get_tile(this, ent->pos.x, ent->pos.y);
	/* set display flags */
	int32_t flags = 0;
	if (ent->tile.flags.attr_blink || (t && t->flags.attr_blink)) {
		flags |= A_BLINK;
	};
	if (ent->tile.flags.attr_dim || (t && t->flags.attr_dim)) {
		flags |= A_DIM;
	};
	if (ent->tile.flags.attr_bold || (t && t->flags.attr_bold)) {
		flags |= A_BOLD;
	};
	/* construct entity color by taking tile background color */
	int tile_color = COLOR_BLACK;
	if (t) {
		tile_color = t->color.back;
	}
	int color_pair = palette_lookup_pair(&this->palette, ent->tile.color.fore, tile_color);
	mvaddch(this->map.pos.y + ent->pos.y, this->map.pos.x + ent->pos.x, ent->tile.symbol | flags | COLOR_PAIR(color_pair));
	/* all ok */
	return RET_OK;
}

static err_t game_ui_display(game_t *this)
{
	RETURN_IF_NULL(this, RET_ERR);
	move(this->ui_pos.y, this->ui_pos.x);
	clrtoeol();
	mvprintw(this->ui_pos.y, this->ui_pos.x, "tile %lu %s", this->build_tile_id, this->tile_def.name[this->build_tile_id]);
	/* all ok */
	return RET_OK;
}

err_t game_promt_input(game_t *this, int *key, const char *prompt_str)
{
	RETURN_IF_NULL(this, RET_ERR);
	RETURN_IF_NULL(key, RET_ERR);
	timeout(-1);
	move(this->ui_pos.y, this->ui_pos.x);
	clrtoeol();
	mvprintw(this->ui_pos.y, this->ui_pos.x, prompt_str);
	*key = getch();
	timeout(this->config.tick_rate);
	/* all ok */
	return RET_OK;
}

static err_t game_action_place(game_t *this, int32_t x, int32_t y, tile_t *tile)
{
	err_t ret = RET_OK;
	int key = '.';
	vect2i_t pos = { x, y };
	game_promt_input(this, &key, "which direction?");
	switch (key) {
	case KEY_UP:
	case 'w':
		pos.y--;
		break;
	case KEY_DOWN:
	case 's':
		pos.y++;
		break;
	case KEY_LEFT:
	case 'a':
		pos.x--;
		break;
	case KEY_RIGHT:
	case 'd':
		pos.x++;
		break;

	default:
		ret = RET_ERR;
		break;
	}
	if (ret == RET_OK) {
		game_map_set_tile(this, pos.x, pos.y, tile);
	}
	return ret;
}

static err_t game_handle_input(game_t *this, int key)
{
	err_t ret = RET_OK;
	/* local vars */
	tile_t *move_tile = NULL;
	/* key switch */
	switch (key) {
	case KEY_UP:
	case 'w':
		entity_move(&this->player, &(vect2i_t){0, -1}, this);
		break;
	case KEY_DOWN:
	case 's':
		entity_move(&this->player, &(vect2i_t){0, 1}, this);
		break;
	case KEY_LEFT:
	case 'a':
		entity_move(&this->player, &(vect2i_t){-1, 0}, this);
		break;
	case KEY_RIGHT:
	case 'd':
		entity_move(&this->player, &(vect2i_t){1, 0}, this);
		break;

	case 'r':
		game_map_generate_noise(this);
		game_map_basic_smooth(this, 100);
		break;

	case 'z':
		game_action_place(this, this->player.pos.x, this->player.pos.y, &this->tile_def.tile[this->build_tile_id]);
		break;
	case 'x':
		game_action_place(this, this->player.pos.x, this->player.pos.y, this->tile_empty);
		break;
	case '+':
		this->build_tile_id++;
		if (this->build_tile_id >= this->tile_def.count) {
			this->build_tile_id = 0;
		}
		break;
	case '-':
		this->build_tile_id--;
		if (this->build_tile_id < 0) {
			this->build_tile_id = this->tile_def.count - 1;
		}
		break;
	
	default:
		ret = RET_ERR;
		break;
	}
	return ret;
}

static err_t game_handle_logic(game_t *this)
{
	game_map_tick_tiles(this);
	return RET_OK;
}

static err_t game_handle_render(game_t *this)
{
	game_map_display(this);
	game_entity_display(this, &this->player);
	game_ui_display(this);
	return RET_OK;
}

err_t game_loop(game_t *this)
{
	/* game loop */
	int key = '.';
	do {
		game_handle_input(this, key);
		game_handle_logic(this);
		game_handle_render(this);
		refresh();
	} while ((key = getch()) != 'q');
	/* game quit */
	return RET_OK;
}