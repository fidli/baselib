#ifndef WINDOWS_DLL
#define WINDOWS_DLL

#include "windows_filesystem.cpp"

#define DEFINEDLLFUNC(RP,NM, ...) \
typedef RP(CALLBACK * NM##FuncType)(__VA_ARGS__); \
NM##FuncType NM;

#define OBTAINDLLFUNC(HNDL, FNC) \
FNC = (FNC##FuncType) GetProcAddress(HNDL, #FNC)


bool hasDllChangedAndReloaded(FileWatchHandle * target, HMODULE * lib, void (*customWait)(void) = NULL){
    if(hasFileChanged(target)){
        
        char pathCopy[256];
        sprintf(pathCopy, "%250s.copy", target->path);
        if(customWait != NULL){
            customWait();
        }
        if(*lib != 0){
            FreeLibrary(*lib);
        }
        while(CopyFile(target->path, pathCopy, false) <= 0);
        *lib = LoadLibrary(pathCopy);
        if(*lib != 0){
            return true;
        }else{
            return false;
        }
    }
    return false;
}


#endif

