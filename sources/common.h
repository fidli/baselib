#ifndef COMMON_H
#define COMMON_H

#ifdef RELEASE
#define ASSERT 
#else
#ifndef ASSERT
#define ASSERT(expression) if(!(expression)) {*(int *)0 = 0;}
#endif
#endif

#define INV ASSERT(!"FUCK");

#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif
#define ARRAYSIZE(array) (sizeof(array) /sizeof(*(array)))

#define KILOBYTE(n) ((size_t)1024)*(n)
#define MEGABYTE(n) ((size_t)1024)*((size_t)KILOBYTE((n)))
#define GIGABYTE(n) ((size_t)1024)*((size_t)MEGABYTE((n)))

#endif