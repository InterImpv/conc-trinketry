/* random module */

static const char *pth_random = "/dev/urandom";

#include "rand.h"

#include <limits.h>

rnd_t *rnd_create(void)
{
    rnd_t *rnd = NULL;
    rnd = malloc(sizeof(*rnd));
    
    assert(NULL != rnd && "ERR NOMEM");

    return rnd;
}

enum GEN_ERR rnd_init(rnd_t *rnd)
{
    if (NULL == rnd)
        return E_ARG;

    rnd->_file = fopen(pth_random, "rb");

    if (NULL == rnd->_file) {
        fprintf(stderr, "ERR %d: could not open \"%s\"\n", -E_IO, pth_random);
        return E_IO;
    }

    return E_OK;
}

enum GEN_ERR rnd_free(rnd_t *rnd)
{
    if (NULL == rnd)
        return E_ARG;

    if (NULL != rnd->_file)
        fclose(rnd->_file);
    
    free(rnd);

    return E_OK;
}

static uint32_t rnd_get32(rnd_t *rnd)
{        
    assert(NULL != rnd && NULL != rnd->_file);

    uint32_t result = UINT_MAX;
    fread(&result, sizeof(uint32_t), 1, rnd->_file);

    return result;
}

uint32_t rnd_rand(rnd_t *rnd, uint32_t min, uint32_t max)
{
    assert(NULL != rnd && NULL != rnd->_file);

    return (min + rnd_get32(rnd) % max);
}
