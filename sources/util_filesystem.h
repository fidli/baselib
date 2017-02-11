#include "common.h"

#ifndef UTIL_FILESYSTEM
#define UTIL_FILESYSTEM

extern struct FileHandle;

struct FileContents{
    uint32 size;
    char * contents;
};

struct DirectoryContents{
    uint32 count;
    char ** files;
};

void readFile(const char * path, FileContents * target);
void saveFile(const char * path, const FileContents * source);
void appendFile(const char * path, const FileContents * source);
void readDirectory(const char * path, DirectoryContents * target);

#endif