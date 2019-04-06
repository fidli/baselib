#ifndef UTIL_CONFIG
#define UTIL_CONFIG

#include "util_filesystem.cpp"

bool loadConfig(const char * file, bool (*lineCallback)(const char *, void ** context), void * initialContext = NULL){
    FileContents contents = {};
    if(readFile(file, &contents)){
        
        char line[1024];
        
        while(getNextLine(&contents, line, ARRAYSIZE(line))){
            uint32 localOffset = 0;
            while(line[localOffset] == ' ' || line[localOffset] == '\t'){
                localOffset++;
            }
            //skip commentary or empty line
			if(line[localOffset] == '#' || line[localOffset] == 0) continue;
            if(!lineCallback(line+localOffset, &initialContext)){
                POP; // read file pushes
                return false;
            }
            
        }
        POP; //read file pushesh
        return true;
    }
    return false;
}



#endif
