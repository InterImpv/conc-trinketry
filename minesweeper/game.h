#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

#define MIN_SCREEN_X 10

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
    WIN = 2,
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
    int scr_start_x;
    int scr_start_y;
    status_t status;
    bool is_debug;
    bool is_first;
    bool use_color;
} minesweeper_t;

typedef struct game_timer {
    struct timeval start;
    struct timeval curr;
    time_t delta;
    time_t secs;
    time_t mins;
    bool is_paused;
} stimer_t;

/* alloc */
minesweeper_t *ms_alloc(const size_t mapsize);
/* dealloc */
void ms_free(minesweeper_t *game);

/* init */
int ms_init(minesweeper_t *game, const uint32_t mine_count);
void ms_maps_zero(minesweeper_t *game);
void ms_maps_test(minesweeper_t *game);
bool ms_check_screen_size(minesweeper_t *game);
void ms_recalc_screen(minesweeper_t *game);
void ms_sprint_centerx(minesweeper_t *game, const int y, const char *str);

/* map logic */
bool tm_check_map(tilemap_t *map, const int x, const int y);
tile_t tm_get_tile(tilemap_t *map, const int x, const int y);
int16_t tm_get_tile_color(const tile_t tile);
void tm_change_tile(tilemap_t *map, const int x, const int y, const tile_t tile);

int ms_total_flags(minesweeper_t *game);
int ms_total_mines(minesweeper_t *game);

/* map print */
void ms_draw_remap(minesweeper_t *game, const int x, const int y);
void ms_draw_immap(minesweeper_t *game, const int x, const int y);
    /* DEBUG BEGIN */
void ms_remap_show(minesweeper_t *game);
    /* DEBUG END */

/* game logic */
tile_t ms_inc_tile(minesweeper_t *game, const int x, const int y);
void ms_reveal_at(minesweeper_t *game, const int x, const int y);
void ms_poke(minesweeper_t *game);
void ms_flag(minesweeper_t *game);
void ms_place_mines(minesweeper_t *game);
void ms_show_mines(minesweeper_t *game);
void ms_check_win(minesweeper_t *game);

bool ms_check_curs(minesweeper_t *game, const int x, const int y);
void ms_curs_l(minesweeper_t *game);
void ms_curs_r(minesweeper_t *game);
void ms_curs_u(minesweeper_t *game);
void ms_curs_d(minesweeper_t *game);

/* basic timer */
void gt_reset(stimer_t *timer);
void gt_start(stimer_t *timer);
void gt_stop(stimer_t *timer);
void gt_advance(stimer_t *timer);