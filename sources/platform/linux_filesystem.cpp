#ifndef LINUX_FILESYSTEM
#define LINUX_FILESYSTEM

#include "string.h"
#include <stdio.h>
#include "util_filesystem.cpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>



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
    if(!file.handle)
    {
        return false;
    }
    file.descriptor = fileno(file.handle);
    ASSERT(file.descriptor  != -1);
    if(file.descriptor == -1){
        return false;
    }
    int size = getFileSizeAndGoBackToBeginning(&file) + 1;
    target->head = 0;
    target->size = size;
    target->contents = &PUSHA(char, target->size);
    ssize_t res = fread(target->contents, 1, target->size-1, file.handle);
    ASSERT(res == target->size - 1);
    if(res != target->size - 1){
        return false;
    }
    target->contents[target->size-1] = '\0';
    fclose(file.handle);
    return true;
}

bool writeFile(FileHandle * target, const FileContents * source){
    INV;
}

bool saveFile(const char * path, const FileContents * source){
    FILE * f = fopen(path, "w");
    if(f){
        size_t written = fwrite(CAST(void *, source->contents), 1, source->size, f);
        fclose(f);
        return written == source->size;
    }
    return false;
}

bool appendFile(const char * path, const FileContents * source){
    FILE * f = fopen(path, "a");
    if(f){
        size_t written = fwrite(CAST(void *, source->contents), 1, source->size, f);
        fclose(f);
        return written == source->size;
    }
    return false;
}

bool readDirectory(const char * path, DirectoryContents * target){
    DIR *dir = opendir(path);
    if(dir){
        dirent * entry;
        target->count = 0;
        target->files = &PUSHA(char*, 255);
        while((entry = readdir(dir)) != NULL){
            if((entry->d_name[0] != '.' && entry->d_name[1] != 0) && (entry->d_name[0] != '.' && entry->d_name[1] != '.' && entry->d_name[2] != 0)){
                target->files[target->count] = &PUSHA(char, 255);
                strncpy(target->files[target->count], entry->d_name, 255);
                target->count++;
            }
        }
        closedir(dir);
        return true;
    }
    return false;
}

bool deleteFile(const char * path){
    return remove(path) == 0;
}

bool moveFile(const char * oldPath, const char * newPath){
    return rename(oldPath, newPath) == 0;
}

bool fileExists(const char * path){
    struct stat attr;
    int res = stat(path, &attr);
    if(res == -1) return false;
    return true;
}

bool dirExists(const char * path){
    return fileExists(path);
}

bool createDirectory(const char * path){
    return dirExists(path) || mkdir(path, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH | S_IWOTH) == 0;
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
