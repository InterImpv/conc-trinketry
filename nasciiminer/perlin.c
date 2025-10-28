#include "perlin.h"
#include "error.h"
#include "rand.h"

err_t perlin_init(perlin_t *this, rng_t *rng)
{
    RETURN_IF_NULL(this, RET_ERR);
    memset(this, 0, sizeof(*this));
    
    return RET_OK;
}

double perlin_noise(perlin_t *this, double x, double y, double z)
{
    int32_t dx = (int32_t)floor(x) & 0xFF;
    int32_t dy = (int32_t)floor(y) & 0xFF;
    int32_t dz = (int32_t)floor(z) & 0xFF;

    x = x - floor(x);
    y = y - floor(y);
    z = z - floor(z);

    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

    int32_t A  = this->pv[dx] + dy;
    int32_t AA = this->pv[A] + dz;
    int32_t AB = this->pv[A + 1] + dz;
    int32_t B  = this->pv[dx + 1] + dy;
    int32_t BA = this->pv[B] + dz;
    int32_t BB = this->pv[B + 1] + dz;

    double grad_AA0 = grad(this->pv[AA], x, y, z);
    double grad_BA0 = grad(this->pv[BA], x - 1, y, z);
    double grad_AB0 = grad(this->pv[AB], x, y - 1, z);
    double grad_BB0 = grad(this->pv[BB], x - 1, y - 1, z);

    double grad_AA1 = grad(this->pv[AA + 1], x, y, z - 1);
    double grad_BA1 = grad(this->pv[BA + 1], x - 1, y, z - 1);
    double grad_AB1 = grad(this->pv[AB + 1], x, y - 1, z - 1);
    double grad_BB1 = grad(this->pv[BB + 1], x - 1, y - 1, z - 1);

    double lerpAABA0 = lerp(u, grad_AA0, grad_BA0);
    double lerpABBB0 = lerp(u, grad_AB0, grad_BB0);

    double lerpAABA1 = lerp(u, grad_AA1, grad_BA1);
    double lerpABBB1 = lerp(u, grad_AB1, grad_BB1);

    double v = lerp(w, lerp(v, lerpAABA0, lerpABBB0), lerp(v, lerpAABA1, lerpABBB1));

    return ((v + 1.0) / 2.0);
}