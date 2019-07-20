#ifndef LINUX_DLL
#define LINUX_DLL

#include <dlfcn.h>

#include "linux_filesystem.cpp"

#define DEFINEDLLFUNC(RP,NM, ...) \
typedef RP(* NM##FuncType)(__VA_ARGS__); \
static NM##FuncType NM = NULL;

#define OBTAINDLLFUNC(HNDL, FNC) \
FNC = (FNC##FuncType) dlsym(HNDL, #FNC)


bool hasDllChangedAndReloaded(FileWatchHandle * target, void ** lib, void (*customWait)(void) = NULL){
    if(hasFileChanged(target)){
        if(customWait != NULL){
            customWait();
        }
        if(*lib != 0){
            dlclose(*lib);
            *lib = 0;
        }
        *lib = dlopen(target->path, RTLD_NOW | RTLD_GLOBAL);
        if(*lib != 0){
            return true;
        }else{
            //printf("error,  %s\n", dlerror());
        }
    }
    return false;
}


#endif

