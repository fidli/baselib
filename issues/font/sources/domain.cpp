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

struct OpenTypeFont
{
    struct
    {
        uint16 segCountX2;
        uint16 segCount;
        uint16 searchRange;
        uint16 entrySelector;
        uint16 rangeShift;
        uint16 * endCode;
        uint16 * startCode;
        int16 * idDelta;
        uint16 * idRangeOffset;
        uint16 * glyphIdArray;
    } cmap;
};

uint16 searchGlyphIndex(OpenTypeFont * font, char * utf8Code)
{
    return font->cmap.glyphIdArray[0];
    for(int segmentIndex = 0; segmentIndex < font->cmap.segCount; segmentIndex++)
    {
    }
}

EXPORT_FUNCTION void init(Memory & platformMem)
{
	mem = platformMem;
    initTime();   

    FileContents fileContents = {};
    readFile("C:\\Windows\\Fonts\\SourceSansPro-Regular.otf", &fileContents);
    // PARse
    OpenTypeFont font;
    if(!strncmp(fileContents.contents, "ttcf", 4))
    {
        //NOTE(fidli): this is font collection
        INV;
    }
    else if(!strncmp(fileContents.contents, "OTTO", 4))
    {
        skipBytes(&fileContents, 4);
        uint16 numTables = readUint16(&fileContents);
        skipBytes(&fileContents, 6);
        for(int tableIndex = 0; tableIndex < numTables; tableIndex++)
        {
            char * tableTag = fileContents.contents + fileContents.head;
            skipBytes(&fileContents, 8);
            uint32 tableOffset = readUint32(&fileContents);
            uint32 tableLength = readUint32(&fileContents);
            if(!strncmp("cmap", tableTag, 4))
            {
                FileContents tableContents = fileContents;
                tableContents.contents += tableOffset;
                tableContents.head = 0;
                tableContents.size = tableLength;

                skipBytes(&tableContents, 2);
                uint16 numSubtables = readUint16(&tableContents);
                for(int subtableIndex = 0; subtableIndex < numSubtables; subtableIndex++)
                {
                    uint16 platformId = readUint16(&tableContents);
                    uint16 encodingId = readUint16(&tableContents);
                    uint32 subtableOffset = readUint32(&tableContents);
                    if(platformId == 3 && encodingId == 1)
                    {
                        // NOTE(fidli): using format 4 of table
                        FileContents subtableContents = tableContents;
                        subtableContents.contents += subtableOffset;
                        subtableContents.head = 0;
                        
                        uint16 format = readUint16(&subtableContents);
                        ASSERT(format == 4);
                        subtableContents.size = readUint16(&subtableContents);
                        skipBytes(&subtableContents, 2);
                        font.cmap.segCountX2 = readUint16(&subtableContents);
                        // https://docs.microsoft.com/en-us/typography/opentype/spec/cmap
                        // NOTE(fidli): this is character code -> glyph index
                        font.cmap.segCount = font.cmap.segCountX2 / 2;
                        skipBytes(&subtableContents, 6);
                        font.cmap.endCode = CAST(uint16 *, subtableContents.contents + subtableContents.head);
                        skipBytes(&subtableContents, font.cmap.segCountX2 + 2);
                        font.cmap.startCode = CAST(uint16 *, subtableContents.contents + subtableContents.head);
                        skipBytes(&subtableContents, font.cmap.segCountX2*3);
                        font.cmap.glyphIdArray = CAST(uint16 *, subtableContents.contents + subtableContents.head);
                        
                    }
                }
            }
        }
    }
    uint16 glyphIndex = searchGlyphIndex(&font, "I");
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
