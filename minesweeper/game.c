#include "game.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ncurses.h>

/* COLOR PAIRS */
#define PAIR_DEF 0
#define PAIR_REDb 1
#define PAIR_GREb 2
#define PAIR_BLUb 3
#define PAIR_YELb 4
#define PAIR_MAGb 5
#define PAIR_CYAb 6
#define PAIR_bRED 7
#define PAIR_bGRE 8

/* ALLOCATION */
static tile_t **ms_alloc_map(const size_t mapsize)
{
    tile_t **map = (tile_t **)malloc(sizeof(*map) * mapsize);
    assert(map != NULL);

    for (size_t i = 0; i < mapsize; i++) {
        map[i] = (tile_t *)malloc(sizeof(*map[i]) * mapsize);
        assert(map[i] != NULL);    // check col
    }
    return map;
}

static tilemap_t *ms_alloc_tilemap(const size_t mapsize)
{
    tilemap_t *tilemap = (tilemap_t *)malloc(sizeof(*tilemap));
    assert(tilemap != NULL);

    tilemap->size = mapsize;
    tilemap->t = ms_alloc_map(tilemap->size);

    return tilemap;
}

minesweeper_t *ms_alloc(const size_t mapsize)
{
    minesweeper_t *ret = NULL;
    ret = (minesweeper_t *)malloc(sizeof(*ret));
    assert(ret != NULL);

    ret->remap = ms_alloc_tilemap(mapsize);
    ret->immap = ms_alloc_tilemap(mapsize);

    return ret;
}

void ms_free(minesweeper_t *game)
{
    if (!game)
        return;

    for (size_t i = 0; i < game->size; i++)
        free(game->immap->t[i]);
    free(game->immap->t);
    free(game->immap);

    for (size_t i = 0; i < game->size; i++)
        free(game->remap->t[i]);
    free(game->remap->t);
    free(game->remap);

    free(game);
}

/* INIT */
int ms_init(minesweeper_t *game, const uint32_t mine_count)
{
    if (!game)
        return -1;

    game->size = game->remap->size;
    game->mine_count = mine_count;
    game->curs_x = game->size / 2;
    game->curs_y = game->size / 2;
    game->scr_start_x = 0;
    game->scr_start_y = 0;
    game->status = ONGOING;
    game->is_first = TRUE;
    game->use_color = FALSE;

    if (has_colors()) {
        game->use_color = TRUE;
        start_color();
        /* numbered cells */
        init_pair(PAIR_REDb, COLOR_RED, COLOR_BLACK);
        init_pair(PAIR_GREb, COLOR_GREEN, COLOR_BLACK);
        init_pair(PAIR_BLUb, COLOR_BLUE, COLOR_BLACK);
        init_pair(PAIR_YELb, COLOR_YELLOW, COLOR_BLACK);
        init_pair(PAIR_MAGb, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(PAIR_CYAb, COLOR_CYAN, COLOR_BLACK);
        /* for detonated mine */
        init_pair(PAIR_bRED, COLOR_BLACK, COLOR_RED);
        /* for flagged cell */
        init_pair(PAIR_bGRE, COLOR_BLACK, COLOR_GREEN);
    }
    ms_maps_zero(game);

    return 0;
}

void ms_maps_zero(minesweeper_t *game)
{
    if (!game)
        return;

    for (size_t i = 0; i < game->size; i++) {
        for (size_t j = 0; j < game->size; j++) {
            game->remap->t[i][j] = ZERO;
            game->immap->t[i][j] = UNKNOWN;
        }
    }
}

void ms_maps_test(minesweeper_t *game)
{
    if (!game)
        return;

    ms_maps_zero(game);

    /* magic pattern to show every number */
    game->remap->t[1][1] = MINE;
    game->remap->t[1][2] = MINE;
    game->remap->t[1][3] = MINE;
    game->remap->t[1][4] = MINE;

    game->remap->t[2][4] = MINE;

    game->remap->t[3][0] = MINE;
    game->remap->t[3][1] = MINE;
    game->remap->t[3][2] = MINE;
    game->remap->t[3][3] = MINE;
    game->remap->t[3][4] = MINE;

    game->remap->t[1][6] = MINE;
    game->remap->t[2][6] = MINE;
    game->remap->t[3][6] = MINE;
    game->remap->t[1][7] = MINE;
    game->remap->t[3][7] = MINE;
    game->remap->t[1][8] = MINE;
    game->remap->t[2][8] = MINE;
    game->remap->t[3][8] = MINE;
}

bool ms_check_screen_size(minesweeper_t *game)
{
    if (!game)
        return FALSE;

    int sx = 0, sy = 0;
    int cx = 0, cy = 0;

    const char *errstr = "Screen too small!";
    const size_t errstrn = strlen(errstr);

    while (sy <= game->size + 2 || sx < MIN_SCREEN_X) {
        getmaxyx(stdscr, sy, sx);
        cx = (sx - errstrn) / 2;
        cy = sy / 2;

        clear();
        mvprintw(cy, cx, errstr);
        refresh();
    }
    return TRUE;
}

void ms_recalc_screen(minesweeper_t *game)
{
    if (!game)
        return;

    int sx = 0, sy = 0;
    getmaxyx(stdscr, sy, sx);

    const int min = ((sx < MIN_SCREEN_X) ? MIN_SCREEN_X : game->size);

    game->scr_start_x = (sx - min) / 2;
    game->scr_start_y = (sy - game->size) / 2;
}

void ms_sprint_centerx(minesweeper_t *game, const int y, const char *str)
{
    if (!game || !str)
        return;

    const int cx = game->scr_start_x + (game->size - strlen(str)) / 2;
    const int cy = game->scr_start_y + y;//game->size + 1;

    mvprintw(cy, cx, str);
}

/* MAP LOGIC */
bool tm_check_map(tilemap_t *map, const int x, const int y)
{
    if ((x >= 0 && x <= (int)map->size - 1) &&
        (y >= 0 && y <= (int)map->size - 1)) {
        return TRUE;
    }
    return FALSE;
}

tile_t tm_get_tile(tilemap_t *map, const int x, const int y)
{
    if (tm_check_map(map, x, y))
        return map->t[y][x];
    return UNKNOWN;
}

int16_t tm_get_tile_color(const tile_t tile)
{
    switch(tile) {
    /* red */
    case FIVE:
    case SIX:
    case ERROR:
        return PAIR_REDb;
    /* green */
    case TWO:
        return PAIR_GREb;
    /* blue */
    case ONE:
        return PAIR_BLUb;
    /* yellow */
    case SEVEN:
    case THREE:
        return PAIR_YELb;
    /* magenta */
    case FOUR:
        return PAIR_MAGb;
    /* cyan */
    case EIGHT:
        return PAIR_CYAb;
    /* black-on-green */
    case FLAGGED:
        return PAIR_bGRE;

    default:
        return PAIR_DEF;
    }
}

void tm_change_tile(tilemap_t *map, const int x, const int y, const tile_t tile)
{
    if (tm_check_map(map, x, y))
        map->t[y][x] = tile;
}

int ms_total_flags(minesweeper_t *game)
{
    int ret = 0;
    if (!game)
        return ret;

    for (size_t i = 0; i < game->size; i++)
        for (size_t j = 0; j < game->size; j++)
            if (tm_get_tile(game->immap, j, i) == FLAGGED)
                ret++;
    
    return ret;
}

int ms_total_mines(minesweeper_t *game)
{
    int ret = 0;
    if (!game)
        return ret;

    for (size_t i = 0; i < game->size; i++)
        for (size_t j = 0; j < game->size; j++)
            if (tm_get_tile(game->remap, j, i) == MINE)
                ret++;
    
    return ret;
}

/* MAP PRINT */
static inline void tm_draw_gen(const int x, const int y, const char ch)
{
    mvaddch(y, x, ch);
}

static inline void tm_draw_col(const int x, const int y, const char ch, const int pair)
{
    attron(COLOR_PAIR(pair));
    mvaddch(y, x, ch);
    attroff(COLOR_PAIR(pair));
}

void ms_draw_remap(minesweeper_t *game, const int x, const int y)
{
    if (!game)
        return;

    for (size_t i = 0; i < game->size; i++) {
        for (size_t j = 0; j < game->size; j++) {
            /* check color mode */
            if (game->use_color) {
                /* get tile & it's color */
                tile_t tile = tm_get_tile(game->remap, j, i);
                int color = tm_get_tile_color(tile);
                /* draw color */
                tm_draw_col(
                    game->scr_start_x + j + x,
                    game->scr_start_y + i + y,
                    game->remap->t[i][j], color
                );
            } else {
                /* draw mono */
                tm_draw_gen(
                    game->scr_start_x + j + x,
                    game->scr_start_y + i + y,
                    game->remap->t[i][j]
                );
            }
        }
    }
}

void ms_draw_immap(minesweeper_t *game, const int x, const int y)
{
    if (!game)
        return;

    for (size_t i = 0; i < game->size; i++) {
        for (size_t j = 0; j < game->size; j++) {
            /* check color mode */
            if (game->use_color) {
                /* get tile & it's color */
                tile_t tile = tm_get_tile(game->immap, j, i);
                int color = tm_get_tile_color(tile);
                /* if lost then highlight the mine at a cursor position */
                if (tile == MINE && ms_check_curs(game, j, i))
                    color = PAIR_bRED;
                /* draw color */
                tm_draw_col(
                    game->scr_start_x + j + x,
                    game->scr_start_y + i + y,
                    game->immap->t[i][j],
                    color
                );
            } else {
                /* draw mono */
                tm_draw_gen(
                    game->scr_start_x + j + x,
                    game->scr_start_y + i + y,
                    game->immap->t[i][j]
                );
            }
        }
    }
}

void ms_remap_show(minesweeper_t *game)
{
    if (!game)
        return;

    for (size_t i = 0; i < game->remap->size; i++)
        for (size_t j = 0; j < game->remap->size; j++)
            game->remap->t[i][j] = ms_inc_tile(game, j, i);
}

/* GAME LOGIC */
tile_t ms_inc_tile(minesweeper_t *game, const int x, const int y)
{
    if (!game)
        return ZERO;

    if (tm_get_tile(game->remap, x, y) == MINE)
        return MINE;

    uint8_t ret = 0;
    for (int i = -1; i < 2; i++)
        for (int j = -1; j < 2; j++)
            if (tm_get_tile(game->remap, x + j, y + i) == MINE)
                ret++;

    return (tile_t)(ret + '0');
}

void ms_reveal_at(minesweeper_t *game, const int x, const int y)
{
    if (!game)
        return;

    /* check bound */
    if (!tm_check_map(game->immap, x, y))
        return;

    /* stop at anything but unknown cells */
    if (tm_get_tile(game->immap, x, y) != UNKNOWN)
        return;

    tile_t tile = ms_inc_tile(game, x, y);
    // fprintf(stderr, "count check: %d, %d\n", x, y);

    /* recursively reveal unknown empty cells */
    if (tile != ZERO && tile != MINE && tile != FLAGGED) {
        tm_change_tile(game->immap, x, y, tile);
        return;
    } else if (tile == ZERO) {
        tm_change_tile(game->immap, x, y, EMPTY);
        ms_reveal_at(game, x, y - 1);       // N
        ms_reveal_at(game, x + 1, y - 1);   // NE
        ms_reveal_at(game, x + 1, y);       // E
        ms_reveal_at(game, x + 1, y + 1);   // SE
        ms_reveal_at(game, x, y + 1);       // S
        ms_reveal_at(game, x - 1, y + 1);   // SW
        ms_reveal_at(game, x - 1, y);       // W
        ms_reveal_at(game, x - 1, y - 1);   // NW
    }
}

void ms_poke(minesweeper_t *game)
{
    if (!game)
        return;

    if (game->is_first) {
        ms_place_mines(game);
        ms_remap_show(game);
        game->is_first = FALSE;
    }
    
    tile_t tile = tm_get_tile(game->remap, game->curs_x, game->curs_y);

    switch (tile) {
    case MINE:
        game->status = LOSE;
        ms_show_mines(game);
        break;
    
    default:
        ms_reveal_at(game, game->curs_x, game->curs_y);
        break;
    }
}

void ms_flag(minesweeper_t *game)
{
    if (!game)
        return;

    if (game->is_first) {
        ms_place_mines(game);
        ms_remap_show(game);
        game->is_first = FALSE;
    }
    
    tile_t tile = tm_get_tile(game->immap, game->curs_x, game->curs_y);

    switch (tile) {
    case UNKNOWN:
        tm_change_tile(game->immap, game->curs_x, game->curs_y, FLAGGED);
        break;
    case FLAGGED:
        tm_change_tile(game->immap, game->curs_x, game->curs_y, UNKNOWN);
        break;
    
    default:
        break;
    }
}

void ms_place_mines(minesweeper_t *game)
{
    if (!game)
        return;

    if (game->is_debug) {
        ms_maps_test(game);
        return;
    }
        
    uint32_t x = rand() % game->remap->size;
    uint32_t y = rand() % game->remap->size;

    uint32_t to_place = game->mine_count;
    uint32_t placed = 0;
    uint32_t free_space = game->size * game->size;

    while (to_place > 0 && placed + 1 < free_space) {
        if (tm_get_tile(game->remap, x, y) != MINE &&
            !ms_check_curs(game, x, y)) {
            tm_change_tile(game->remap, x, y, MINE);
            to_place--;
            placed++;
        }
        x = rand() % game->remap->size;
        y = rand() % game->remap->size;
    }
}

void ms_show_mines(minesweeper_t *game)
{
    if (!game)
        return;

    for (size_t i = 0; i < game->size; i++) {
        for (size_t j = 0; j < game->size; j++) {
            /* show mines & check for invalid flags */
            if (tm_get_tile(game->remap, j, i) == MINE) {
                /* show all mines that are not flagged & the one that detonated */
                if (tm_get_tile(game->immap, j, i) != FLAGGED ||
                    ms_check_curs(game, j, i))
                    tm_change_tile(game->immap, j, i, MINE);
            } else if (tm_get_tile(game->immap, j, i) == FLAGGED) {
                tm_change_tile(game->immap, j, i, ERROR);
            }
        }
    }
}

void ms_check_win(minesweeper_t *game)
{
    /* victory conditions: 
        - no unknown cells;
        - no false-flags;
        - all mines flagged; */

    for (size_t i = 0; i < game->size; i++) {
        for (size_t j = 0; j < game->size; j++) {
            tile_t immap_tile = tm_get_tile(game->immap, j, i);
            tile_t remap_tile = tm_get_tile(game->remap, j, i);

            /* if any unknowns then continue */
            if (immap_tile == UNKNOWN)
                return;
            /* if any cell that is not mined is flagged then continue */
            if (remap_tile != MINE && immap_tile == FLAGGED)
                return;
            /* if any mine is not flagged then continue */
            if (remap_tile == MINE && immap_tile != FLAGGED)
                return;
        }
    }
    game->status = WIN;
    return;
}

/* CURSOR FUNCTIONS */
inline bool ms_check_curs(minesweeper_t *game, const int x, const int y)
{
    return (x == game->curs_x && y == game->curs_y);
}

void ms_curs_l(minesweeper_t *game)
{
    if (game && (game->curs_x > 0))
        game->curs_x--;
}

void ms_curs_r(minesweeper_t *game)
{
    if (game && (game->curs_x < (int)game->size - 1))
        game->curs_x++;
}

void ms_curs_u(minesweeper_t *game)
{
    if (game && (game->curs_y > 0))
        game->curs_y--;
}

void ms_curs_d(minesweeper_t *game)
{
    if (game && (game->curs_y < (int)game->size - 1))
        game->curs_y++;
}

/* BASIC TIMER */
void gt_reset(stimer_t *timer)
{
    if (!timer)
        return;
    
    timer->start.tv_sec = 0;
    timer->start.tv_usec = 0;
    timer->curr.tv_sec = 0;
    timer->curr.tv_usec = 0;
    timer->delta = 0;
    timer->secs = 0;
    timer->mins = 0;
    timer->is_paused = TRUE;
}

void gt_start(stimer_t *timer)
{
    if (timer->is_paused) {
        timer->is_paused = FALSE;
        gettimeofday(&timer->start, NULL);  // needed
        gettimeofday(&timer->curr, NULL);   // do not care
    }
}

void gt_stop(stimer_t *timer)
{
    timer->is_paused = TRUE;
}

static inline time_t gt_get_secs(stimer_t *timer)
{
    return (timer->delta % 60);
}

static inline time_t gt_get_mins(stimer_t *timer)
{
    return (timer->delta / 60);
}

void gt_advance(stimer_t *timer)
{
    if (timer->is_paused)
        return;
    /* do not bother if over 99 */
    if (timer->mins > 99) {
        timer->secs = 99;
        timer->mins = 99;
        return;
    }
    /* read current time & find delta */
    gettimeofday(&timer->curr, NULL);
    timer->delta = timer->curr.tv_sec - timer->start.tv_sec;
    /* update displays */
    timer->secs = gt_get_secs(timer);
    timer->mins = gt_get_mins(timer);
}