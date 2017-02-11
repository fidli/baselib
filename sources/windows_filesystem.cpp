#ifndef WINDOWS_FILESYSTEM
#define WINDOWS_FILESYSTEM

#include <Windows.h>
#include "windows_types.h"

#include "common.h"
#include "util_mem.h"
#include "util_filesystem.h"
#include "util_string.cpp"

struct FileHandle{
    HANDLE handle;
};

void readFile(const char * path, FileContents * target){
    HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    ASSERT(SUCCEEDED(file));
    target->size = GetFileSize(file, 0);
    target->contents = &PUSHA(char, target->size);
    ASSERT(ReadFile(file, (void *) target->contents, target->size, 0, 0));
    
    CloseHandle(file);
}

bool writeFile(FileHandle * target, const FileContents * source){
    return WriteFile(target->handle, source->contents, source->size, 0, 0) > 0;    
}

void saveFile(const char * path, const FileContents * source){
    HANDLE file = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    ASSERT(SUCCEEDED(file));
    FileHandle handle;
    handle.handle = file;
    writeFile(&handle, source);
    CloseHandle(file);
}

void appendFile(const char * path, const FileContents * source){
    HANDLE file = CreateFile(path, FILE_APPEND_DATA, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    ASSERT(SUCCEEDED(file));
    FileHandle handle;
    handle.handle = file;
    writeFile(&handle, source);
    CloseHandle(file);
}

void readDirectory(const char * path, DirectoryContents * target){
    WIN32_FIND_DATA result;
    HANDLE file = FindFirstFile(strcat(path, "\\*"), &result);
    ASSERT(file != INVALID_HANDLE_VALUE);
    target->count = 0;
    target->files = &PUSHA(char*, 255);
    do{
        
        if(strcmp(".", result.cFileName) && strcmp("..", result.cFileName)){
            target->files[target->count] = &PUSHS(char, 255);
            strcpy(target->files[target->count], result.cFileName);
            target->count++;
        }
    }
    while(FindNextFile(file, &result));
    
    
    
}

#endif