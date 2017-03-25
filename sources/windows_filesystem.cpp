#ifndef WINDOWS_FILESYSTEM
#define WINDOWS_FILESYSTEM



#include "util_filesystem.h"


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
    
    //pathlen
    uint32 pathlen = 0;
    for(;path[pathlen] != 0; pathlen++){
    }
    //strcat
    char append[] = "\\*";
    //pathlen and array size both include terminating symbol
    char * fullpath = &PUSHA(char, pathlen + ARRAYSIZE(append) - 1);
    uint32 index = 0;
    while(path[index] != '\0'){
        fullpath[index] = path[index];
        index++;
    }
    uint32 index2 = 0;
    while(append[index2] != '\0'){
        fullpath[index + index2] = append[index2];
        index2++;
    }
    fullpath[index + index2] = '\0';
    POP;
    
    HANDLE file = FindFirstFile(fullpath, &result);
    ASSERT(file != INVALID_HANDLE_VALUE);
    target->count = 0;
    target->files = &PUSHA(char*, 255);
    do{
        //strcmp & copy
        if((result.cFileName[0] != '.' && result.cFileName[1] != 0) && (result.cFileName[0] != '.' && result.cFileName[1] != '.' && result.cFileName[2] != 0)){
            target->files[target->count] = &PUSHS(char, 255);
            uint16 i = 0;
            for(; result.cFileName[i] != 0; i++){
                ASSERT(i < 255);
                target->files[target->count][i] = result.cFileName[i];
            }
            target->files[target->count][i] = 0;
            target->count++;
        }
    }
    while(FindNextFile(file, &result));
    
    
    
}

#endif