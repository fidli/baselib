#pragma once

#ifdef RELEASE
#define ASSERT 
#else
#ifndef ASSERT
#define ASSERT(expression) if(!(expression)) {/*LOGE(default, common, "ASSERT failed on line %d file %s\n", __LINE__, __FILE__);*/*(int *)0 = 0;}
#endif
#endif



#define INV ASSERT(!"INVALID PROGRAM PATH");

#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif
#define ARRAYSIZE(array) (sizeof((array)) / sizeof(*(array)))

#define KILOBYTE(n) (((nint)1024)*(n))
#define MEGABYTE(n) (((nint)1024)*((nint)KILOBYTE((n))))
#define GIGABYTE(n) (((nint)1024)*((nint)MEGABYTE((n))))


#define INT32_MAX 0x7FFFFFFF;
#define INT8_MAX 0x7F;

#define STRINGIFY(a) str(a)
#define str(a) #a
#define cncat(a, b) a ## b
#define CONCAT(a, b) cncat(a, b)