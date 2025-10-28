#ifndef __UTILS_H__
#define __UTILS_H__

#include "error.h"

double lerp_ip(double t, double a, double b);
double lerp_pp(double t, double a, double b);
double lerp(double t, double a, double b);

double find_maxf(double *arr, size_t size);

#endif
