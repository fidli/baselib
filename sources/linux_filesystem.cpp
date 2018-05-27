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

bool readFile(const char * path, FileContents * target){
    FileHandle file;
    file.handle = fopen(path, "r");
    file.descriptor = fileno(file.handle);
    ASSERT(file.descriptor  != -1);
    if(file.descriptor == -1){
        return false;
    }
    int size = getFileSizeAndGoBackToBeginning(&file);
    target->size = size;
    target->contents = &PUSHA(char, target->size);
    ssize_t res = fread(target->contents, 1, target->size, file.handle);
    ASSERT(res == target->size);
    if(res != target->size){
        return false;
    }
    fclose(file.handle);
    return true;
}

bool writeFile(FileHandle * target, const FileContents * source){
    INV;
}

bool saveFile(const char * path, const FileContents * source){
    INV;
}

bool appendFile(const char * path, const FileContents * source){
    INV;
}

bool readDirectory(const char * path, DirectoryContents * target){
    INV;
}


extern LocalTime sysToLocal(const timespec * timespec);

bool getFileChangeTime(const char * path, LocalTime * result){
    struct stat attr;
    int res = stat(path, &attr);
    if(res == -1) return false;
    
    timespec wrap;
    wrap.tv_sec = attr.st_mtime;
    wrap.tv_nsec = 0;
    
    *result = sysToLocal(&wrap);
    
    return true;
}


#endif