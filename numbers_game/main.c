#include "game.h"

#include <argp.h>

const char *argp_program_version = "numguess - v0.1";
static char doc[] = "numguess - a game where you guess numbers";

struct arguments {
    uint32_t guess;
    uint32_t boundary;
    gflags_t flags;
};
 
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *args = state->input;
    
    switch (key)
    {
    case 'n':
        args->boundary = atoi(arg);
        args->flags.is_newgame = true;;
        
        if (args->boundary <= 1) {
            fprintf(stderr, "ERR %d: boundary should be > 1\n", -E_ARG);
            argp_usage(state);
        }
        
        break;
    case 't':
        args->guess = atoi(arg);
        args->flags.is_loadgame = true;
        
        if (args->guess == 0) {
            fprintf(stderr, "ERR %d: guess should be >= 1\n", -E_ARG);
            argp_usage(state);
        }
            
        break;
        
    /* argc checking */
    case ARGP_KEY_ARG:
        if (state->argc >= 2)
            argp_usage(state);
        break;
        
    case ARGP_KEY_END:
        if (state->argc < 2)
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
    int retval = E_OK;

    struct argp_option options[] = {
        { "new", 'n', "NUM", 0, "Start a new game, with a set boundary [2, NUM]" },
        { "try", 't', "NUM", 0, "Play the game by guessing with [1, NUM]" },
        { 0 }
    };
    
    struct argp argp = {
        options, parse_opt, 0, doc
    };
    
    struct arguments game_vars = {
        0, 0,
        { false, false }
    };
        
    /* parse argv and set flags */
    argp_parse(&argp, argc, argv, 0, 0, &game_vars);
    
    /* start game */
    game_t *game = game_create();
    game->flags = game_vars.flags;  // set only after parse
    
    if (game->flags.is_newgame && !game->flags.is_loadgame) {
        retval = game_init(game, game_vars.boundary, GAME_NEW);
    } else if (!game->flags.is_newgame && game->flags.is_loadgame) {
        retval = game_init(game, game_vars.boundary, GAME_LOAD);
        
        if (retval == E_OK) 
            retval = game_play(game, game_vars.guess); 
    } else if (game->flags.is_newgame && game->flags.is_loadgame) {
        retval = game_init(game, game_vars.boundary, GAME_NEW);
        retval = game_init(game, game_vars.boundary, GAME_LOAD);
        
        if (retval == E_OK) 
            retval = game_play(game, game_vars.guess); 
    } else {
        retval = E_ARG;
        fprintf(stderr, "ERR %d: Invalid option given\n", retval);
    }
    game_free(game);
    
    return retval;
}
