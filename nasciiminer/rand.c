#include "error.h"
#include "rand.h"
#include <stdio.h>

#define PATH_LINUX_RNG "/dev/urandom"

urand_t *urand_create(void)
{
    urand_t *this = NULL;
    this = malloc(sizeof(*this));
    return this;
}

err_t urand_free(urand_t *this)
{
    RETURN_IF_NULL(this, RET_ERR);
    urand_close(this);
    free(this);
    return RET_OK;
}

err_t urand_init(urand_t *this)
{
    RETURN_IF_NULL(this, RET_ERR);
    this->file = fopen(PATH_LINUX_RNG, "rb");
    if (!this->file) {
        fprintf(stderr, "ERR %d: could not open \"%s\"\n", RET_ERR, PATH_LINUX_RNG);
        return RET_ERR;
    }
    return RET_OK;
}

err_t urand_close(urand_t *this)
{
    RETURN_IF_NULL(this, RET_ERR);
    if (this->file) {
        fclose(this->file);
    }
    return RET_OK;
}
err_t urand_read(urand_t *this, void *ptr, size_t n)
{
    RETURN_IF_NULL(this, RET_ERR);
    RETURN_IF_NULL(this->file, RET_ERR);
    fread(ptr, n, 1, this->file);
    return RET_OK;
}

int8_t urand_i8(urand_t *this, int8_t min, int8_t max)
{
    RETURN_IF_NULL(this, RET_ERR);
    uint8_t v = 0;
    urand_read(this, &v, sizeof(v));
    return (min + v % max);
}

int32_t urand_i32(urand_t *this, int32_t min, int32_t max)
{
    RETURN_IF_NULL(this, RET_ERR);
    uint32_t v = 0;
    urand_read(this, &v, sizeof(v));
    return (min + v % max);
}