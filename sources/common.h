#ifndef COMMON_H
#define COMMON_H

#ifdef RELEASE
#define ASSERT
#else
#define ASSERT(expression) if(!(expression)) {*(int *)0 = 0;}
#endif

#define INV ASSERT(!"FUCK");

#define ARRAYSIZE(array) sizeof(array) /sizeof(*(array))

#define KILOBYTE(n) 1024*n
#define MEGABYTE(n) 1024*KILOBYTE(n)
#define GIGABYTE(n) 1024*MEGABYTE(n)

#endif