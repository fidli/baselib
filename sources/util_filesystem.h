#include "common.h"
#include "util_time.h"

#ifndef UTIL_FILESYSTEM
#define UTIL_FILESYSTEM

struct FileHandle;


struct FileWatchHandle{
    char path[256];
    LocalTime lastChangeTime;
};

struct FileContents{
    //internal, do not use
    uint32 head;
    
    //end of internal
    uint32 size;
    char * contents;
};

struct DirectoryContents{
    uint32 count;
    char ** files;
};

bool getFileChangeTime(const char * path, LocalTime * result);

bool getFileSize(const char * path, int32 * result);

bool getNextLine(FileContents * contents, char * line, uint32 linelen){
    char format[30];
    uint32 res = snprintf(format, 30, "%%%u[^\r\n]", linelen-1);
    ASSERT(res == 1);
    if(sscanf(contents->contents + contents->head, format, line) == 1){
        contents->head += strlen(line);
        char trail = contents->contents[contents->head];
        while((trail == '\r' || trail == '\n') && trail != '\0'){
            contents->head++;
            trail = contents->contents[contents->head];
        }
        
        return true;
    }
    return false;
}

bool watchFile(const char * path, FileWatchHandle * result){
    if(getFileChangeTime(path, &result->lastChangeTime)){
        strncpy(result->path, path, ARRAYSIZE(FileWatchHandle::path));
        result->lastChangeTime = {};
        return true;
    }
    return false;
}

bool hasFileChanged(FileWatchHandle * target){
    LocalTime newTime;
    if(getFileChangeTime(target->path, &newTime)){
        if(newTime != target->lastChangeTime){
            target->lastChangeTime = newTime;
            return true;
        }
    }
    return false;
}


bool readFile(const char * path, FileContents * target);
bool saveFile(const char * path, const FileContents * source);
bool appendFile(const char * path, const FileContents * source);
bool readDirectory(const char * path, DirectoryContents * target);
bool fileExists(const char * path);


#endif