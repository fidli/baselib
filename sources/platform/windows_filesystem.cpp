#ifndef WINDOWS_FILESYSTEM
#define WINDOWS_FILESYSTEM

#include "util_filesystem.cpp"

struct FileHandle{
    HANDLE handle;
};

bool getFileSize(const char * path, uint32 * result){
    HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(file ==  INVALID_HANDLE_VALUE) return false;
    DWORD r = GetFileSize(file, 0);
    if(r == INVALID_FILE_SIZE) return false;
    *result = r;
    CloseHandle(file);
    return true;
}

bool readFile(const char * path, FileContents * target){
    HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(file ==  INVALID_HANDLE_VALUE) return false;
    if(target->size == 0){
        target->size = GetFileSize(file, 0);
        target->contents = &PUSHA(char, target->size);
    }
    if(!ReadFile(file, (void *) target->contents, target->size, 0, 0)){
        CloseHandle(file);
        return false;
    }
    target->head = 0;
    CloseHandle(file);
    return true;
}

bool fileExists(const char * path){
    DWORD res = GetFileAttributes(path); 
    return res != INVALID_FILE_ATTRIBUTES && !(res & FILE_ATTRIBUTE_DIRECTORY);
}

bool pathExists(const char * path){
    DWORD res = GetFileAttributes(path); 
    return res != INVALID_FILE_ATTRIBUTES;
}

bool dirExists(const char * path){
    DWORD res = GetFileAttributes(path); 
    return res != INVALID_FILE_ATTRIBUTES && (res & FILE_ATTRIBUTE_DIRECTORY);
}

bool writeFile(FileHandle * target, const FileContents * source){
    return WriteFile(target->handle, source->contents, source->size, 0, 0) > 0;    
}

bool saveFile(const char * path, const FileContents * source){
    HANDLE file = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if(file == INVALID_HANDLE_VALUE){
        return false;
    }
    FileHandle handle;
    handle.handle = file;
    bool result = writeFile(&handle, source);
    CloseHandle(file);
    return result;
}

bool appendFile(const char * path, const FileContents * source){
    HANDLE file = CreateFile(path, FILE_APPEND_DATA, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if(file == INVALID_HANDLE_VALUE){
        return false;
    };
    FileHandle handle;
    handle.handle = file;
    bool result = writeFile(&handle, source);
    CloseHandle(file);
    return result;
}

bool readDirectory(const char * path, DirectoryContents * target){
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
    
    
    HANDLE file = FindFirstFile(fullpath, &result);
    POP;
    if(file == INVALID_HANDLE_VALUE){
        return false;
    }
    
    target->count = 0;
	PUSHI;
    target->files = &PUSHA(char*, 255);
    do{
        //strcmp & copy
        if((result.cFileName[0] != '.' && result.cFileName[1] != 0) && (result.cFileName[0] != '.' && result.cFileName[1] != '.' && result.cFileName[2] != 0)){
            target->files[target->count] = &PUSHA(char, 255);
            uint16 i = 0;
            for(; result.cFileName[i] != 0; i++){
                if(i == 255) return false;
                target->files[target->count][i] = result.cFileName[i];
            }
            target->files[target->count][i] = 0;
            target->count++;
        }
    }
    while(FindNextFile(file, &result));
    
    return true;
    
}

extern LocalTime sysToLocal(const SYSTEMTIME * time);

bool getFileChangeTime(const char * path, LocalTime * result){
    HANDLE file = CreateFile(path, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(file != INVALID_HANDLE_VALUE){
        
        FILETIME filetime;
        BOOL success = GetFileTime(file, NULL, NULL, &filetime);
        if(success != 0){
            
            SYSTEMTIME systime;
            success = FileTimeToSystemTime(&filetime, &systime);
            if(success != 0){
                *result = sysToLocal(&systime);
            }
        }
        CloseHandle(file);
        
        return success != 0;
        
    }
    return false;
}

bool deleteFile(const char * path){
    return DeleteFile(path) != 0;
}

bool moveFile(const char * oldPath, const char * newPath){
    return MoveFile(oldPath, newPath) != 0;
}

bool createDirectory(const char * path){
    bool r = CreateDirectory(path, NULL);
    if(!r){
        r = GetLastError() == 183; // already exists
        // TODO(fidli): some dirs do not exist on the way - create them before
    }
    return r;
}

#endif
