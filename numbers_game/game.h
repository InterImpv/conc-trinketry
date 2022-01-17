#include "rand.h"

#include <stdbool.h>
#include <string.h>

typedef struct glob_flags {
    uint8_t is_newgame : 1;
    uint8_t is_loadgame : 1;
} gflags_t;

enum GAME_STATUS {
    GAME_NEW,
    GAME_LOAD
};

enum SAV_PARAM {
    SAV_WINCNT = 0,
    SAV_TOTGAM = 1,
    SAV_BOUNDR = 2,
    SAV_GUESSV = 3
};

typedef struct game_struct {
    rnd_t *rand_gen;
    gflags_t flags;
    const char *savefile;
    
    /* used for specific signals */
    uint32_t status;
        
    uint32_t score;
    uint32_t game_count;
    
    uint32_t number;
    uint32_t boundary;
} game_t;

game_t *game_create(void);
enum GEN_ERR game_init(game_t *engine, uint32_t boundary, enum GAME_STATUS save);
enum GEN_ERR game_play(game_t *engine, uint32_t guess);
enum GEN_ERR game_free(game_t *engine);
