#pragma once


#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif
#define ARRAYSIZE(array) (sizeof((array)) / sizeof(*(array)))

#define KILOBYTE(n) (((nint)1024)*(n))
#define MEGABYTE(n) (((nint)1024)*((nint)KILOBYTE((n))))
#define GIGABYTE(n) (((nint)1024)*((nint)MEGABYTE((n))))


#define INT32_MAX 0x7FFFFFFF
#define INT8_MAX 0x7F

#define STRINGIFY(a) str(a)
#define str(a) #a
#define cncat(a, b) a ## b
#define CONCAT(a, b) cncat(a, b)

#define CAST(type, what) ((type)(what))

#define SWAP(a, b) { auto tmp = a; a = b; b = tmp;}

#ifdef RELEASE
#pragma message("RELEASE MODE")
#include "util_log.cpp"
#define ASSERT(expression) if(!(expression)) {LOGE(default, assertion, "ASSERT(%s) failed. In %s() in file %s:%u", #expression, __func__, __FILE__, __LINE__);}
#else
#ifndef ASSERT
#define ASSERT(expression) if(!(expression)) {*(int *)0 = 0;}
#endif
#endif

#define INV ASSERT(!"INVALID PROGRAM PATH");
