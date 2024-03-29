#pragma once
#define _CRT_SECURE_NO_WARNINGS // using unsafe stds
//NOTE(AK): ---------------- instead of cstd
extern "C" void * __cdecl memset(void *, int, size_t);
extern "C" void * __cdecl memcpy(void *, const void *, size_t);
extern "C" int __cdecl memcmp(const void *, const void *, size_t);
#pragma function(memset)
#pragma function(memcpy)
#pragma function(memcmp)

extern "C"{
    void * memset(void * dest, int c, size_t count)
    {
        char * bytes = (char *) dest;
        while (count--)
        {
            (*bytes) = (char) c;
            (*bytes++);
        }
        return dest;
    }
    
    void *memcpy(void *dest, const void *src, size_t count)
    {
        char *dest8 = (char *)dest;
        const char *src8 = (const char *)src;
        while (count--)
        {
            *dest8++ = *src8++;
        }
        return dest;
    }

    int memcmp(const void *lhs, const void *rhs, size_t count)
    {
        const unsigned char *lhs8 = (const unsigned char *)lhs;
        const unsigned char *rhs8 = (const unsigned char *)rhs;
        int diff = 0;
        
        while (count-- && diff == 0)
        {
            diff = *lhs8 - *rhs8;
            lhs8++;
            rhs8++;
        }
        return diff;
    }

    int _fltused;
};

//NOTE(AK): --------------- end of instead of cstd

