#ifndef __RAND_H__
#define __RAND_H__

#include "error.h"

/* you can use <sys/random.h> to rewrite this*/

/* random module */
typedef struct system_random {
    FILE *file;
} urand_t;

typedef struct system_random {
    FILE *file;
} urand_t;

typedef enum {
    RNG_VAR_STDLIB = 0,
    RNG_VAR_URAND,
} rng_variant_t;

typedef struct random_engine {
    bool initialized;
    rng_variant_t variant;
    union {
        stdlib
    } engine;
} rng_t;

urand_t *urand_create(void);
err_t urand_init(urand_t *this);
err_t urand_close(urand_t *this);
err_t urand_free(urand_t *this);
int8_t urand_i8(urand_t *this, int8_t min, int8_t max);
int32_t urand_i32(urand_t *this, int32_t min, int32_t max);

#endif