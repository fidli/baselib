#include "common.h"

#ifndef UTIL_FILESYSTEM_H
#define UTIL_FILESYSTEM_H

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
void readDirectory(const char * path, DirectoryContents * target);

#endif