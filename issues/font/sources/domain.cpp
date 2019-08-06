#ifndef CRT_PRESENT
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
#endif

#define EXPORT_FUNCTION extern "C" __declspec(dllexport)


#include "common.h"
#include "mem.h"

#include "util_mem.h"

#include "domain_input.h"
#include "util_image.cpp"

#include "util_filesystem.cpp"
#include "util_time.h"
#include "platform/windows_filesystem.cpp"
#include "platform/windows_time.cpp"

EXPORT_FUNCTION void init(Memory & platformMem)
{
	mem = platformMem;
    initTime();   

    FileContents fileContents = {};
    readFile("C:\\Windows\\Fonts\\SourceSansPro-Regular.otf", &fileContents);
    // PARse
    if(!strncmp(fileContents.contents, "ttcf", 4))
    {
        //NOTE(fidli): this is font collection
        INV;
    }
    else if(!strncmp(fileContents.contents, "OTTO", 4))
    {
        uint16 numTables = *((uint16*)(fileContents.contents + 4));
        uint32 tableOffset = 12;
        for(int tableIndex = 0; tableIndex < numTables; tableIndex++)
        {
            char * tableHead = fileContents.contents + tableOffset;
            char * dataStart = tableHead + 16;
            uint32 tableLength = *((uint32*)(tableHead + 12));

            tableOffset += tableLength + 16;
        }
    }
}

EXPORT_FUNCTION void input(const Input * input)
{

}

EXPORT_FUNCTION void iteration(float32 dt)
{
}

EXPORT_FUNCTION void render(Image * renderingTarget)
{

}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpReserved)
{
    return 1;
}

BOOL WINAPI _DllMainCRTStartup(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpReserved)
{
    return 1;
    return DllMain(hinstDLL,fdwReason,lpReserved);
}
