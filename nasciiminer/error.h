#ifndef __ERROR_H__
#define __ERROR_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <limits.h>
#include <float.h>
#include <math.h>
#include <string.h>

#include <ncurses.h>

typedef enum {
    RET_OK = 0,
    RET_ERR,
    RET_BUSY
} err_t;

#define RETURN_IF_NULL(expr, retval) if(!(expr)) return (retval)

#endif
