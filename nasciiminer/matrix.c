#include "matrix.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <float.h>
#include <math.h>

static char f2ch(double x)
{
    x = fabs(x);
    if (x <= 0.0) {
        return ' ';
    } else if (x > 0.0 && x <= 0.25) {
        return '.';
    } else if (x > 0.25 && x <= 0.5) {
        return ':';
    } else if (x > 0.5 && x <= 0.75) {
        return '#';
    } else {
        return '@';
    }
}

err_t mm_fill_plane(double *m, vect2i_t *size, vect2f_t *bound, vect3f_t *plane)
{
    /* fill */
    vect2f_t xy = { 0 };
    double dx = 0, dy = 0;
    for (size_t y = 0; y < size->y; y++) {
        dy = (double)y / (double)size->y;
        for (size_t x = 0; x < size->x; x++) {
            dx = (double)x / (double)size->x;
            xy.x = dx, xy.y = dy;
            m[y * size->x + x] = plane3d_Z(&xy, plane);
        }
    }
    return RET_OK;
}

err_t mm_fill_rand(double *m, vect2i_t *size)
{
    /* fill */
    for (size_t y = 0; y < size->y; y++) {
        for (size_t x = 0; x < size->x; x++) {
            m[y * size->x + x] = rand();
        }
    }
    return RET_OK;
}

// err_t mm_rebound(double *m, vect2i_t *size, vect2f_t *bound)
// {
//     /* fill */
//     for (size_t y = 0; y < size->y; y++) {
//         for (size_t x = 0; x < size->x; x++) {
//             m[y * size->x + x] = plane3d_Z(&xy, plane);
//         }
//     }
//     return RET_OK;
// }

err_t mm_invert_by_max(double *m, vect2i_t *size)
{
    /* fill */
    double max = find_maxf(m, size->x * size->y);
    for (size_t y = 0; y < size->y; y++) {
        for (size_t x = 0; x < size->x; x++) {
            m[y * size->x + x] = max - m[y * size->x + x];
        }
    }
    return RET_OK;
}

err_t mm_mirror(double *m, vect2i_t *size)
{
    /* fill */
    for (size_t y = 0; y < size->y; y++) {
        for (size_t x = 0; x < size->x; x++) {
            m[y * size->x + x] = -m[y * size->x + x];
        }
    }
    return RET_OK;
}

err_t mm_normalize(double *m, vect2i_t *size)
{
    /* fill */
    double max = find_maxf(m, size->x * size->y);
    for (size_t y = 0; y < size->y; y++) {
        for (size_t x = 0; x < size->x; x++) {
            m[y * size->x + x] = m[y * size->x + x] / max;
        }
    }
    return RET_OK;
}

err_t mm_apply_mask(double *m, double *mask, vect2i_t *size)
{
    /* fill */
    for (size_t y = 0; y < size->y; y++) {
        for (size_t x = 0; x < size->x; x++) {
            m[y * size->x + x] = m[y * size->x + x] * mask[y * size->x + x];
        }
    }
    return RET_OK;
}

err_t mm_print(double *m, vect2i_t *size)
{
    printf("(%i, %i) =\n", size->x, size->y);
    for (size_t y = 0; y < size->y; y++) {
        for (size_t x = 0; x < size->x; x++) {
            double v = m[y * size->x + x];
            printf("%c", f2ch(v));
        }
        printf("\n");
    }
    return RET_OK;
}
