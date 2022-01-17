#include "game.h"

/* game handler */
static const char *pth_savefile = "data.sav";  // the only save file for now
static const uint32_t MIN_BOUNDARY = 2;

static enum GEN_ERR __game_load_param(game_t *engine, uint32_t pno, uint32_t pval)
{
    assert(NULL != engine);
    
    switch (pno)
    {
    case SAV_WINCNT:
        engine->score = pval;
        return E_OK;
        break;
    case SAV_TOTGAM:
        engine->game_count = pval;
        return E_OK;
        break;
    case SAV_BOUNDR:
        engine->boundary = pval;
        return E_OK;
        break;
    case SAV_GUESSV:
        engine->number = pval;
        return E_OK;
        break;
    
    default:
        return E_ARG;
        break;
    };
    
    return E_ARG;
}

static enum GEN_ERR __game_roll_new(game_t *engine)
{
    assert(NULL != engine);

    engine->number = rnd_rand(engine->rand_gen, 1, engine->boundary);
    
    return E_OK;
}

static enum GEN_ERR __game_save(game_t *engine)
{
    assert(NULL != engine);
        
    if (engine->status != E_OK) {
        fprintf(stderr, "ERR %d: No savefile loaded, nothing to save\n", -E_IO);
        return E_IO;
    }
        
    FILE *save = fopen(engine->savefile, "wb");
    assert(NULL != save);
        
    fwrite(&engine->score, sizeof(engine->score), 1, save);
    fwrite(&engine->game_count, sizeof(engine->game_count), 1, save);
    fwrite(&engine->boundary, sizeof(engine->boundary), 1, save);
    fwrite(&engine->number, sizeof(engine->number), 1, save);
        
    fclose(save);
    
    return E_OK;
}

static enum GEN_ERR __game_new(game_t *engine, uint32_t boundary)
{
    assert(NULL != engine);
    
    if (boundary <= 1) {
        fprintf(stderr, "ERR %d: Invalid boundary %u, set to 2\n", -E_ARG, boundary);
        boundary = MIN_BOUNDARY;
        engine->status = E_ARG;
    }

    engine->score = 0;
    engine->game_count = 0;
    engine->boundary = boundary;
    
    engine->savefile = pth_savefile;

    __game_roll_new(engine);
    
    engine->status = E_OK;
    
    return E_OK;
}

static enum GEN_ERR __game_load(game_t *engine)
{
    assert(NULL != engine);
        
    engine->score = -1;
    engine->game_count = -1;
    engine->boundary = -1;
    engine->number = -1;
    
    engine->savefile = pth_savefile;
    
    /* save information */
    FILE *save = NULL;
    uint32_t valbuf = -1;
    uint32_t check_it = 0;      // ~ checksum, but not really
        
    /* open save */
    save = fopen(engine->savefile, "rb");
    if (NULL == save) {
        engine->status = E_IO;
        fprintf(stderr, "ERR %d: Could not open savefile\n", -engine->status);
        
        return engine->status;
    }
    
    /* read save information */
    for (uint32_t i = SAV_WINCNT; !feof(save); i++) {
        fread(&valbuf, sizeof(uint32_t), 1, save);       
        __game_load_param(engine, i, valbuf);
        
        check_it = i;
    }
    
    if (check_it != 4) {
        fclose(save);
        engine->status = E_SAVE;
        fprintf(stderr, "ERR %d: Corrupt savefile\n", -engine->status);
        
        return engine->status;
    }
    
    fclose(save);
    
    engine->status = E_OK;
    return E_OK;
}

game_t *game_create(void)
{
    game_t *engine = NULL;
    engine = malloc(sizeof(*engine));
    
    assert(engine != NULL);
    
    return engine;
}

enum GEN_ERR game_init(game_t *engine, uint32_t boundary, enum GAME_STATUS save)
{
    assert(engine != NULL);
 
    /* init random generator */
    engine->rand_gen = rnd_create();
    if (rnd_init(engine->rand_gen) != E_OK) {
        engine->status = E_INIT;
        fprintf(stderr, "ERR %d: RNG init failure\n", -engine->status);
        
        return engine->status;
    }
    
    /* either start anew or load default save */
    if (GAME_NEW == save) { 
        __game_new(engine, boundary);
        
        if (engine->status != E_OK)
            engine->status = E_INIT;    // boundary is incorrect
    } else if (GAME_LOAD == save) {
        if (__game_load(engine) != E_OK) {
            engine->status = E_INIT;    // savefile error
            return engine->status;
        }
    } else {
        fprintf(stderr, "ERR %d: Unknown init parameter\n", -E_UKW);
    }
    
    __game_save(engine);

    return E_OK;
}

enum GEN_ERR game_free(game_t *engine)
{
    assert(engine != NULL);
                
    rnd_free(engine->rand_gen);
    free(engine);

    return E_OK;
}

enum GEN_ERR game_play(game_t *engine, uint32_t guess)
{
    assert(engine != NULL);
        
    if (engine->status == E_SAVE) // save was not loaded, data is incorrect
        return E_SAVE;            // will break if @status was reset before
        
    if (NULL != engine->rand_gen) {
        engine->game_count++;
        
        if (guess == 0) {
            printf("!0\n");
        } else if (guess > engine->number) {
            printf("<%u\n", guess);
        } else if (guess < engine->number) {
            printf(">%u\n", guess);
        } else {
            __game_roll_new(engine);
            engine->score++;
            printf("+1: %u / %u\n", engine->score, engine->game_count);
        }
    } else {
        engine->status = E_INIT;
        fprintf(stderr, "ERR %d: Init error\n", engine->status);
        
        return engine->status;
    }
    
    __game_save(engine);

    return E_OK;
}

