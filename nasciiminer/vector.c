#include "vector.h"

#include <math.h>

double plane3d_Z(vect2f_t *V, vect3f_t *Vn)
{
    return ((Vn->x * V->x + Vn->y * V->y) / Vn->z);
}

double cone3d_Z(vect2f_t *V, vect3f_t *Vn)
{
    double xa = (V->x * V->x) / (Vn->x * Vn->x);
    double yb = (V->y * V->y) / (Vn->y * Vn->y);
    double zz = (xa + yb) / (Vn->z * Vn->z);
    return sqrt(zz);
}
