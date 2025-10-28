#ifndef __MATRIX_H__
#define __MATRIX_H__

#include "error.h"
#include "vector.h"

err_t mm_fill_plane(double *m, vect2i_t *size, vect2f_t *bound, vect3f_t *plane);
err_t mm_fill_rand(double *m, vect2i_t *size);
err_t mm_invert_by_max(double *m, vect2i_t *size);
err_t mm_mirror(double *m, vect2i_t *size);
err_t mm_normalize(double *m, vect2i_t *size);
err_t mm_apply_mask(double *m, double *mask, vect2i_t *size);

err_t mm_print(double *m, vect2i_t *size);

#endif
