#ifndef UTIL_CONFIG
#define UTIL_CONFIG

#include "util_string.cpp"
#include "util_filesystem.h"

bool loadConfig(const char * file, bool (*lineCallback)(const char *)){
    FileContents contents;
    if(readFile(file, &contents)){
        uint32 offset = 0;
        char line[1024];
        while(sscanf(contents.contents + offset, "%[^\r\n]", line) == 1){
            offset += strlen(line);
            char trail = contents.contents[offset];
            while((trail == '\r' || trail == '\n') && trail != '\0'){
                offset++;
                trail = contents.contents[offset];
            }
            if(!lineCallback(line)){
                POP;
                return false;
            }
            
        }
        POP;
        return true;
    }
    return false;
}



#endif
