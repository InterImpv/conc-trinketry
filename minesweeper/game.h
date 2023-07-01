#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum map_tile {
    ZERO = '0',     // white
    ONE = '1',      // blue 
    TWO = '2',      // green
    THREE = '3',    // yellow
    FOUR = '4',     // magenta
    FIVE = '5',     // red
    SIX = '6',      // red
    SEVEN = '7',    // yellow
    EIGHT = '8',    // cyan
    MINE = '*',
    FLAGGED = 'P',
    EMPTY = '.',
    ERROR = 'X',    // red
    UNKNOWN = '#',
} tile_t;

typedef enum game_status {
    ONGOING = 0,
    LOSE = 1,
    WIN = 2
} status_t;

typedef struct tile_map {
    tile_t **t;
    size_t size;
} tilemap_t;

typedef struct game_engine {
    tilemap_t *remap;
    tilemap_t *immap;
    size_t size;
    int mine_count;
    int curs_x;
    int curs_y;
    status_t status;
    bool is_debug;
    bool is_first;
    bool use_color;
} minesweeper_t;

/* alloc */
minesweeper_t *ms_alloc(const size_t mapsize);
/* dealloc */
void ms_free(minesweeper_t *game);

/* init */
const int ms_init(minesweeper_t *game, const uint32_t mine_count);
void ms_maps_zero(minesweeper_t *game);
void ms_maps_test(minesweeper_t *game);

/* map logic */
const bool tm_check_map(tilemap_t *map, const int x, const int y);
const tile_t tm_get_tile(tilemap_t *map, const int x, const int y);
const int32_t tm_get_tile_color(const tile_t tile);
void tm_change_tile(tilemap_t *map, const int x, const int y, const tile_t tile);

const int32_t ms_total_flags(minesweeper_t *game);
const int32_t ms_total_mines(minesweeper_t *game);

/* map print */
void ms_draw_remap(minesweeper_t *game, const int x, const int y);
void ms_draw_immap(minesweeper_t *game, const int x, const int y);
    /* DEBUG BEGIN */
void ms_remap_show(minesweeper_t *game);
    /* DEBUG END */

/* game logic */
const tile_t ms_inc_tile(minesweeper_t *game, const int x, const int y);
void ms_reveal_at(minesweeper_t *game, const int x, const int y);
void ms_poke(minesweeper_t *game);
void ms_flag(minesweeper_t *game);
void ms_place_mines(minesweeper_t *game);
void ms_show_mines(minesweeper_t *game);
void ms_check_win(minesweeper_t *game);

const bool ms_check_curs(minesweeper_t *game, const int x, const int y);
void ms_curs_l(minesweeper_t *game);
void ms_curs_r(minesweeper_t *game);
void ms_curs_u(minesweeper_t *game);
void ms_curs_d(minesweeper_t *game);

