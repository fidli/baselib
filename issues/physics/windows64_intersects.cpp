extern "C" void * __cdecl memset(void *, int, size_t);
#pragma intrinsic(memset)
extern "C" void * __cdecl memcpy(void *, const void *, size_t);
#pragma intrinsic(memcpy)

extern "C"{
#pragma function(memset)
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
    
#pragma function(memcpy)
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
}
extern "C"{
    int _fltused;
};



#include <Windows.h>

#include "windows_types.h"
#include "common.h"


#define PERSISTENT_MEM MEGABYTE(1)
#define TEMP_MEM MEGABYTE(1)
#define STACK_MEM MEGABYTE(101)

#include "util_mem.h"
#include "util_physics.cpp"
#include "windows_time.cpp"
#include "util_string.cpp"

struct Context{
    HINSTANCE hInstance;
};

Context context;


static inline int main(LPWSTR * argvW, int argc) {
    
    
    LPVOID memoryStart = VirtualAlloc(NULL, TEMP_MEM + PERSISTENT_MEM + STACK_MEM, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    bool memory = memoryStart != NULL;
    
    if(!memory){
        ASSERT(false);
        return 0;
    }
    
    initMemory(memoryStart);
    initTime();
    initRng();
    
    //spheres
    {
        Sphere_64 A;
        A.radius = 1;
        A.origin = V3_64(0,0,0);
        
        Sphere_64 B;
        B.radius = 1;
        B.origin = V3_64(1,0,0);
        
        Box_64 result;
        bool doIntersect = intersectSpheresAABB64(&A, &B, &result);
        int breakmeman = 0;
    }
    
    //segments
    {
        Segment1D_64 A;
        A.a = 0;
        A.b = 2;
        
        Segment1D_64 B;
        B.b = 1;
        B.a = 3;
        
        Segment1D_64 result;
        bool doIntersect = intersect1DSegments64(&A, &B, &result);
        int breakmeman = 0;
    }
    
    //AABBs
    {
        Box_64 A;
        A.lowerCorner = V3_64(-0.5, -0.5, -0.5);
        A.upperCorner = V3_64(0.5, 0.5, 0.5);
        
        Box_64 B;
        B.lowerCorner = V3_64(0, 0, 0);
        B.upperCorner = V3_64(1, 1, 1);
        
        Box_64 C;
        C.lowerCorner = V3_64(0, 0, 1);
        C.upperCorner = V3_64(1, 1, 2);
        
        
        Box_64 result;
        bool doIntersect = intersectBoxes64(&A, &B, &result);
        int breakmeman = 0;
        
        doIntersect = intersectBoxes64(&A, &C, &result);
        breakmeman = 0;
    }
    
    
    if (!VirtualFree(memoryStart, 0, MEM_RELEASE)) {
        //more like log it
        ASSERT(!"Failed to free memory");
    }
    
    return 0;
}



int mainCRTStartup(){
    int argc = 0;
    LPWSTR * argv =  CommandLineToArgvW(GetCommandLineW(), &argc);
    int result = main(argv,argc);
    LocalFree(argv);
    ExitProcess(result);
}



