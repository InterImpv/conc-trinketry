#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* you can use <sys/random.h> to rewrite this*/

/* random module */
typedef struct rnd_s {
    FILE *_file;
} rnd_t;

rnd_t *rnd_create(void);
enum GEN_ERR rnd_init(rnd_t *rnd);

enum GEN_ERR rnd_free(rnd_t *rnd);

uint32_t rnd_rand(rnd_t *sys_rand, uint32_t min, uint32_t max);
