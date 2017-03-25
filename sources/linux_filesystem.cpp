#ifndef LINUX_FILESYSTEM
#define LINUX_FILESYSTEM

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

struct FileHandle{
    FILE * handle;
    int descriptor;
};

static inline int getFileSizeAndGoBackToBeginning(FileHandle * handle){
    fseek(handle->handle, 0L, SEEK_END);
    int byteSize = ftell(handle->handle);
    rewind(handle->handle);
    return byteSize;
}

void readFile(const char * path, FileContents * target){
    FileHandle file;
    file.handle = fopen(path, "r");
    file.descriptor = fileno(file.handle);
    ASSERT(file.descriptor  != -1);
    int size = getFileSizeAndGoBackToBeginning(&file);
    target->size = size;
    target->contents = &PUSHA(char, target->size);
    ssize_t res = fread(target->contents, 1, target->size, file.handle);
    ASSERT(res == target->size);
    fclose(file.handle);
}

bool writeFile(FileHandle * target, const FileContents * source){
    INV;
}

void saveFile(const char * path, const FileContents * source){
    INV;
}

void appendFile(const char * path, const FileContents * source){
    INV;
}

void readDirectory(const char * path, DirectoryContents * target){
    INV;
}

#endif