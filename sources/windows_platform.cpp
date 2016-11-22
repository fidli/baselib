#include <Windows.h>
extern "C"{
    int _fltused;
};
#include "windows_types.h"


#define PERSISTENT_MEM MEGABYTE(10)
#define TEMP_MEM MEGABYTE(350)
#define STACK_MEM MEGABYTE(500)

#include "common.h"
#include "windows_filesystem.cpp"

inline void alertError(const char * text, HWND window = NULL){
    ASSERT(!"fuck");
}

inline float64 getCurrentTime(){
	LARGE_INTEGER counter;
	LARGE_INTEGER frequency;
	if(QueryPerformanceCounter(&counter) == 0){
		alertError("Performance counter fucked up. (No microtime for you).");
		return 0.0f;
	};
	if(QueryPerformanceFrequency(&frequency) == 0){
		alertError("Performance frequency fucked up. (No microtime for you).");
		return 0.0f;
	};

  return ((float64)counter.QuadPart / (float64)frequency.QuadPart);
}

#include "domaincode.cpp"


int main(char ** argv, int argc) {
    LPVOID memoryStart = VirtualAlloc(NULL, TEMP_MEM + PERSISTENT_MEM + STACK_MEM, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);


    if (memoryStart) {
        initMemory(memoryStart);
        
        run();


	   				if (!VirtualFree(memoryStart, 0, MEM_RELEASE)) {
								alertError("Failed to free memory");
		    			}

						
						}
						return 0;


}


void __stdcall mainCRTStartup(){
    ExitProcess(main(0,0));
}