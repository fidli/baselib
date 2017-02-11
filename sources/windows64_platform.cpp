#include <Windows.h>
extern "C"{
    int _fltused;
};
#include "windows_types.h"


#define PERSISTENT_MEM MEGABYTE(1)
#define TEMP_MEM MEGABYTE(500)
#define STACK_MEM MEGABYTE(1)

#include "common.h"
#include "util_mem.h"

#include "domaincode.cpp"

static inline DWORD jettisonAllPrivileges() {
    DWORD result = ERROR_SUCCESS;
    HANDLE processToken  = NULL;
    if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &processToken)){
        TOKEN_PRIVILEGES * priv = (TOKEN_PRIVILEGES *) mem.temp;
        DWORD privsize;
        if(GetTokenInformation(processToken, TokenPrivileges, priv , EFFECTIVE_TEMP_MEM_SIZE, &privsize) > 0){
            for(DWORD i = 0; i < priv->PrivilegeCount; ++i ){
                priv->Privileges[i].Attributes = SE_PRIVILEGE_REMOVED;
            }
            if(AdjustTokenPrivileges(processToken, TRUE, priv, NULL, NULL, NULL) == 0){
                result = GetLastError();
            }
        }else{
            result = GetLastError();
        }
    }else{
        result = GetLastError();
    }
    CloseHandle(processToken);
    return result;
}


static inline int main(LPWSTR * argvW, int argc) {
    
    
    
    LPVOID memoryStart = VirtualAlloc(NULL, TEMP_MEM + PERSISTENT_MEM + STACK_MEM, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    
    
    if (memoryStart) {
        initMemory(memoryStart);
        
        ASSERT(jettisonAllPrivileges() == ERROR_SUCCESS);
        
        ASSERT(initTime());
        ASSERT(initRng());
        
        char ** argv = &PUSHA(char *, argc);
        
        for(int i = 0; i < argc; i++){
            int toAlloc = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, NULL, 0, NULL, NULL);
            ASSERT(toAlloc != 0);
            argv[i] = &PUSHA(char, toAlloc);
            ASSERT(WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, argv[i], toAlloc, NULL, NULL) != 0);
        }
        
        
        run();
        
        
        
        if (!VirtualFree(memoryStart, 0, MEM_RELEASE)) {
            ASSERT(!"Failed to free memory");
        }
        
        
    }
    return 0;
    
    
}


void __stdcall mainCRTStartup(){
    int argc = 0;
    LPWSTR * argv =  CommandLineToArgvW(GetCommandLineW(), &argc);
    int result = main(argv,argc);
    LocalFree(argv);
    ExitProcess(result);
}