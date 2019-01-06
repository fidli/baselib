#ifndef COMMON_H
#define COMMON_H


#ifdef RELEASE
#define ASSERT 
#else
#ifndef ASSERT
#define ASSERT(expression) if(!(expression)) {LOGE(default, common, "ASSERT failed on line %d file %s\n", __LINE__, __FILE__);*(int *)0 = 0;}
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




#endif