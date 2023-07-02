
#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <argp.h>
#include <ncurses.h>

const char *argp_program_version = "minesweeper - v1.0";
static char doc[] = "minesweeper - the game";

static struct argp_option global_options[] = {
    { "size", 's', "NUM", 0, "Specify how large play area should be" },
    { "mines", 'm', "NUM", 0, "Specify how many mines there will be" },
    { "debug", 'd', 0, 0, "Debug mode" },
    { 0 }
};

struct arguments {
    int map_size;
    int mine_count;
    bool is_debug;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *args = (struct arguments *)state->input;
    
    switch (key)
    {
    case 's':
        args->map_size = atoi(arg);
        
        if (args->map_size <= 1 || args->map_size > 100) {
            fprintf(stderr, "ERR: map size should be in [1, 100]\n");
            argp_usage(state);
        }
        break;

    case 'm':
        args->mine_count = atoi(arg);
        
        if (args->mine_count < 1) {
            fprintf(stderr, "ERR: amount of mines should be >= 1\n");
            argp_usage(state);
        }
        break;

    case 'd':
        args->is_debug = TRUE;
        break;
        
    case ARGP_KEY_END:
        if ((args->map_size == 0 || args->mine_count == 0) && args->is_debug == FALSE)
            argp_usage(state);
        break;
        
    default:
        return ARGP_ERR_UNKNOWN;
        break;
    };
    
    return 0;
}

int main(int argc, char **argv)
{
    struct argp argp = {
        global_options, parse_opt, 0, doc
    };
    
    struct arguments game_vars = {
        0, 0, FALSE
    };

    /* timer struct */
    stimer_t stimer;
    gt_reset(&stimer);
    /* parse argv and set flags */
    argp_parse(&argp, argc, argv, 0, 0, &game_vars);
    /* init curses */
    initscr();
    noecho();
    timeout(100);
    keypad(stdscr, TRUE);
    /* use pseudorandom */
    srand(time(NULL));
    /* init game */
    minesweeper_t *game;
    /* check game mode */
    if (game_vars.is_debug) {
        game = ms_alloc(10);
        ms_init(game, game_vars.mine_count);
        game->is_debug = TRUE;
    } else {
        game = ms_alloc(game_vars.map_size);
        ms_init(game, game_vars.mine_count);
    }

    int key = '.';
    do {
        if (!game->status == ONGOING)
            continue;
        /* main loop */
        curs_set(0);
        clear();
        /* process user input */
        switch (key) {
        case KEY_UP:
            ms_curs_u(game);
            break;
        case KEY_DOWN:
            ms_curs_d(game);
            break;
        case KEY_LEFT:
            ms_curs_l(game);
            break;
        case KEY_RIGHT:
            ms_curs_r(game);
            break;
        case 'z':
            if (game->is_first)
                gt_start(&stimer);
            ms_poke(game);
            break;
        case 'x':
            if (game->is_first)
                gt_start(&stimer);
            ms_flag(game);
            break;

        default:
            break;
        }
        /* display time */
        gt_advance(&stimer);
        mvprintw(0, 0, "%02u:%02u", stimer.mins, stimer.secs);
        /* count mines */
        int mines_total = ms_total_mines(game);
        int mines_left =  mines_total - ms_total_flags(game);
        mvprintw(0, 6, "%i/%i", mines_left, mines_total);
        /* draw current map state */
        ms_draw_immap(game, 0, 1);

        if (game_vars.is_debug) {
            /* print real map */
            ms_draw_remap(game, game->size + 1, 1);
            /* print debug information */
            mvprintw(1, 2 * game->size + 2, "x: %i, y: %i", game->curs_x, game->curs_y);
            mvprintw(2, 2 * game->size + 2, "k: 0x%08x %i", key, key);
            mvprintw(3, 2 * game->size + 2, "s: %u", game->status);
            mvprintw(4, 2 * game->size + 2, "f: %u", game->is_first);
            mvprintw(5, 2 * game->size + 2, "c: %u", game->use_color);
        }

        /* move cursor to current position */
        move(game->curs_y + 1, game->curs_x);
        /* check victory conditions */
        ms_check_win(game);

        if (game->status == LOSE) {
            mvprintw(game->size + 2, 0, "You've lost!");
            gt_stop(&stimer);
        } else if (game->status == WIN) {
            mvprintw(game->size + 2, 0, "You've won!");
            gt_stop(&stimer);
        }

        /* redraw screen */
        curs_set(1);
        refresh();
    } while ((key = getch()) != 'q');

    /* end game */
    endwin();
    ms_free(game);

    return 0;
}
