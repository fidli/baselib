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

#define KILOBYTE(n) (((nint)1024)*(n))
#define MEGABYTE(n) (((nint)1024)*((nint)KILOBYTE((n))))
#define GIGABYTE(n) (((nint)1024)*((nint)MEGABYTE((n))))

#endif