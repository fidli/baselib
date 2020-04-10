#pragma once
#define PI 3.14159265f
#define PI_64 3.1415926535897932f
#define E 2.71828182f

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#ifdef MAX
#undef MAX
#endif
#define MAX(a, b) (((a) < (b)) ? (b) : (a))
#define ABS(a) (((a) < 0) ? -(a) : (a))
#define KRONECKER(a, b) ((a) == (b) ? 1 : 0)
#define SGN(a) ((a) == 0 ? 0 : ((a) > 0 ? 1 : -1))
