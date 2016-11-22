#include <Windows.h>
#include "windows_types.h"

#include "common.h"
#include "util_filesystem.cpp"
#include "util_string.cpp"

void readFile(const char * path, FileContents * target){
    HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    ASSERT(SUCCEEDED(file));
    target->size = GetFileSize(file, 0);
    target->contents = &PUSHA(char, target->size);
    ASSERT(ReadFile(file, (void *) target->contents, target->size, 0, 0));

   CloseHandle(file);
}

void saveFile(const char * path, const FileContents * source){
    HANDLE file = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    ASSERT(SUCCEEDED(file));
    WriteFile(file, source->contents, source->size, 0, 0);    
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
