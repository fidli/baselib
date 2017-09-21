#include "common.h"
#include "util_time.h"

#ifndef UTIL_FILESYSTEM
#define UTIL_FILESYSTEM

struct FileHandle;
struct LocalTime;

struct FileContents{
    uint32 size;
    char * contents;
};

struct DirectoryContents{
    uint32 count;
    char ** files;
};

bool readFile(const char * path, FileContents * target);
bool saveFile(const char * path, const FileContents * source);
bool appendFile(const char * path, const FileContents * source);
bool readDirectory(const char * path, DirectoryContents * target);
bool fileExists(const char * path);
LocalTime getFileChangeTime(const char * path);

#endif