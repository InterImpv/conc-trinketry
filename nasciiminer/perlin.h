#ifndef __ERROR_H__
#define __ERROR_H__

#include "error.h"
#include "rand.h"

#define PERLIN_VECTOR_SIZE (256UL)

typedef struct perlin_noise {
    bool initialized;
    int32_t pv[PERLIN_VECTOR_SIZE];
} perlin_t;

#endif
