#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdint.h>

typedef struct vector_2d_int32 {
    int32_t x;
    int32_t y;
} vect2i_t;

typedef struct vector_3d_int32 {
    int32_t x;
    int32_t y;
    int32_t z;
} vect3i_t;

typedef struct vector_2d_double {
    double x;
    double y;
} vect2f_t;

typedef struct vector_3d_double {
    double x;
    double y;
    double z;
} vect3f_t;

double plane3d_Z(vect2f_t *V, vect3f_t *Vn);
double cone3d_Z(vect2f_t *V, vect3f_t *Vn);

#endif
