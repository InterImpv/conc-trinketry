#include "utils.h"

#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <float.h>
#include <math.h>

double lerp_ip(double t, double a, double b)
{ 
	return a + t * (b - a);
}

double lerp_pp(double t, double a, double b)
{ 
	return (1 - t) * a + t * b; 
}

double lerp(double t, double a, double b)
{ 
	return lerp_ip(t, a, b); 
}

double fade(double t)
{
	return (t * t * t * (t * (t * 6 - 15) + 10));
}

double grad(int32_t hash, double x, double y, double z)
{
	int32_t h = hash & 0xF;
	double u = ((h < 8) ? x : y);
	double v = ((h < 4) ? y : ((h == 12 || h == 14) ? x : z));
	return (((h & 0x1) == 0) ? u : -u) + (((h & 0x2) == 0) ? v : -v);
}

double find_maxf(double *arr, size_t size)
{
	double max = DBL_MIN;
	for (size_t i = 0; i < size; i++) {
		if (arr[i] > max) {
			max = arr[i];
		}
	}
	return max;
}
