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
    uint32 size;
    char * contents;
};

struct DirectoryContents{
    uint32 count;
    char ** files;
};

bool getFileChangeTime(const char * path, LocalTime * result);

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