#ifndef UTIL_CONFIG
#define UTIL_CONFIG

#include "util_filesystem.cpp"

bool loadConfig(const char * file, bool (*lineCallback)(const uint64, const char *, void ** context), void * initialContext = NULL){
    FileContents contents = {};
    if(readFile(file, &contents)){
        
        char line[1024];
        u64 lineIndex = 0;
        while(getNextLine(&contents, line, ARRAYSIZE(line))){
            u32 localOffset = 0;
            while(line[localOffset] == ' ' || line[localOffset] == '\t'){
                localOffset++;
            }
            //skip commentary or empty line
			if(line[localOffset] == '#' || line[localOffset] == 0) continue;
            if(!lineCallback(lineIndex, line+localOffset, &initialContext)){
                POP; // read file pushes
                return false;
            }
            lineIndex++;
        }
        POP; //read file pushesh
        return true;
    }
    return false;
}



#endif
