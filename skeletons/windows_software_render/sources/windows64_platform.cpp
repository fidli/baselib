#ifndef CRT_PRESENT
extern "C" void * __cdecl memset(void *, int, size_t);
#pragma intrinsic(memset)
extern "C" void * __cdecl memcpy(void *, const void *, size_t);
#pragma intrinsic(memcpy)

extern "C"{
#pragma function(memset)
    void * memset(void * dest, int c, size_t count)
    {
        char * bytes = (char *) dest;
        while (count--)
        {
            (*bytes) = (char) c;
            (*bytes++);
        }
        return dest;
    }
    
#pragma function(memcpy)
    void *memcpy(void *dest, const void *src, size_t count)
    {
        char *dest8 = (char *)dest;
        const char *src8 = (const char *)src;
        while (count--)
        {
            *dest8++ = *src8++;
        }
        return dest;
    }
}
extern "C"{
    int _fltused;
};
#endif
#include <Windows.h>
#include "platform/windows_types.h"

#include "mem.h"

#include "common.h"
#include "util_mem.h"

#include "platform/windows_time.cpp"
#include "platform/windows_dll.cpp"

#include "domain_input.h"
#include "windows64_input.cpp"

#include "util_image.cpp"
#include "util_conv.cpp"
#include "util_log.cpp"

#include "platform/windows_io.cpp"


#define FRAME_TIME 0.033f

DEFINEDLLFUNC(void, init, Memory &);
DEFINEDLLFUNC(void, input, const Input *);
DEFINEDLLFUNC(void, iteration, float32);
DEFINEDLLFUNC(void, render, Image *);

struct DrawContext
{
    uint32 * drawbuffer;
    Image drawBitmapData;
    Image originalBitmapData;
    BITMAPINFO drawinfo;
    HDC  backbufferDC;
    HBITMAP DIBSection;
    uint32 width;
    uint32 height;
};


struct Context
{
    HINSTANCE hInstance;
    HWND window;
    DrawContext renderer;
    Image renderingTarget;
	bool keepRunning;
	
	Input input;
};

Context * context;

LRESULT
CALLBACK
WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_SIZE:
		{
            if(context->renderer.DIBSection)
			{
				DeleteObject(context->renderer.DIBSection);
			}
			else
			{
				context->renderer.backbufferDC = CreateCompatibleDC(NULL);
			}
			WORD width = (WORD)lParam;
			WORD height = (WORD) (lParam >> sizeof(WORD) * 8);
			context->renderer.drawinfo = {
					{
						sizeof(BITMAPINFOHEADER),
						width,
						-height,
						1,
						32,
						BI_RGB,
						0,
						0,
						0,
						0,
						0
					},
				0
			};
			context->renderer.DIBSection = CreateDIBSection(context->renderer.backbufferDC, &context->renderer.drawinfo, DIB_RGB_COLORS, (void**) &context->renderer.drawbuffer, NULL, NULL);
			
			context->renderer.height = height;
			context->renderer.width = width;
        }
        break;
        case WM_PAINT:
		{
            PAINTSTRUCT paint;
            HDC dc = BeginPaint(window, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int width = paint.rcPaint.right - paint.rcPaint.left;
            int height = paint.rcPaint.bottom - paint.rcPaint.top;
            
            StretchDIBits(dc, x, y, width, height, 0, 0, width, height, context->renderer.drawbuffer, &context->renderer.drawinfo, DIB_RGB_COLORS, SRCCOPY);
            
            EndPaint(context->window, &paint);
        }
		break;
        case WM_CLOSE:
        case WM_DESTROY:
        {
            context->keepRunning = false;
            return 0;
        }
		break;
		default:
		{
			return handleInput(window, message, wParam, lParam);
		}
    }
    return DefWindowProc (window, message, wParam, lParam);
}

static
inline
int
main(LPWSTR * argvW, int argc){
    
    LPVOID memoryStart = VirtualAlloc(NULL, TEMP_MEM + PERSISTENT_MEM + STACK_MEM, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    bool memory = memoryStart != NULL;
    
    if(!memory)
	{
        ASSERT(!"Failed to init memory");
        return 0;
    }
    initMemory(memoryStart);
    context = &PPUSH(Context);
    context->hInstance = GetModuleHandle(NULL);
    context->keepRunning = true;
    context->renderer.drawBitmapData = {};
    bool initLogSuccess = initTime() && initIo();
	if(!initLogSuccess)
	{
		ASSERT(!"Failed to init logging");
		return 0;
	}
	
	LOG(default, privileges, "Dropping privileges");
    bool privilegesSuccess = false;
	// NOTE(fidli): Dropping privileges
	{
		DWORD result = ERROR_SUCCESS;
		HANDLE processToken  = NULL;
		if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &processToken))
		{
			DWORD privsize = 0;
			GetTokenInformation(processToken, TokenPrivileges, NULL, 0, &privsize);
			TOKEN_PRIVILEGES * priv = (TOKEN_PRIVILEGES *) &PUSHA(byte, privsize);
			if(GetTokenInformation(processToken, TokenPrivileges, priv , privsize, &privsize))
			{
				for(DWORD i = 0; i < priv->PrivilegeCount; ++i )
				{ 
					priv->Privileges[i].Attributes = SE_PRIVILEGE_REMOVED;
				}
				if(AdjustTokenPrivileges(processToken, TRUE, priv, NULL, NULL, NULL) == 0)
				{
					result = GetLastError();
					LOGE(default, privileges, "Failed to disable a privlege. Err: %d", result);
				}
			}
			else
			{
				result = GetLastError();
				LOGE(default, privileges, "Failed to obtain a token. Err: %d", result);
			}
			//POP;
		}
		else
		{
			result = GetLastError();
			LOGE(default, privileges, "Failed to sopen a token. Err: %d", result);
		}
		CloseHandle(processToken);
		privilegesSuccess = result == ERROR_SUCCESS;
		LOG(default, privileges, "Dropping privileges success: %d", privilegesSuccess);
	}
	
	LOG(default, args, "Parsing CLI args");
	char ** argv = &PPUSHA(char *, argc);
	char ** argvUTF8 = &PPUSHA(char *, argc);
    bool argvSuccess = false;
	// NOTE(fidli): obtaining CLI arguments
	{
		bool success = true;
		for(int i = 0; i < argc && success; i++)
		{
			int toAlloc = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, NULL, 0, NULL, NULL);
			success &= toAlloc != 0;
			argvUTF8[i] = &PPUSHA(char, toAlloc);
			int res = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, argvUTF8[i], toAlloc, NULL, NULL);
			success &= res != 0;
			uint32 finalLen;
			//this is more or equal to real needed
			argv[i] = &PPUSHA(char, toAlloc);
			if(convUTF8toAscii((byte *)argvUTF8[i], toAlloc, &argv[i], &finalLen) != 0)
			{
				LOGW(default, args, "Argument is not fully ascii compatible - those characters were replaced by '_'. Please use simple ASCII parameter values");
			}
		}
		argvSuccess = success;
		LOG(default, args, "Parsing CLI args success: %d", argvSuccess);
    }
    
	// NOTE(fidli): creating window & rendering target
	bool windowSuccess = false;
	LOG(default, drawingContext, "Creating drawing context");
	{
		context->renderingTarget.info.bitsPerSample = 8;
		context->renderingTarget.info.samplesPerPixel = 4;
		context->renderingTarget.info.interpretation = BitmapInterpretationType_ARGB;
		context->renderingTarget.info.origin = BitmapOriginType_TopLeft;
		
		WNDCLASSEX style = {};
		style.cbSize = sizeof(WNDCLASSEX);
		style.style = CS_OWNDC;
		style.lpfnWndProc = WindowProc;
		style.hInstance = context->hInstance;
		style.lpszClassName = "MainClass";
		if(RegisterClassEx(&style) != 0)
		{
			context->window = CreateWindowEx(NULL,
											 "MainClass", "App", WS_OVERLAPPEDWINDOW | WS_SIZEBOX,
											 CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, context->hInstance, NULL);
			windowSuccess = context->window != NULL;
		}
		else
		{
			LOGE(default, drawingContext, "Failed to register window class");
		}
		LOG(default, drawingContext, "Creating drawing context success: %d", windowSuccess);
	}
	LOG(default, setup, "Initing modules");
    bool initSuccess = true;
	LOG(default, setup, "Initing modules success: %d", initSuccess);
    
    HMODULE domainLib = 0;
    FileWatchHandle domainLibWatch;
    
	LOG(default, hotload, "Watching domain.dll");
	// NOTE(fidli): hotloading
    bool watchSuccess = watchFile("domain.dll", &domainLibWatch);
	LOG(default, hotload, "Watching domain.dll success: %d", watchSuccess);
    
	// NOTE(fidli): all initialisation was a success
    if(initSuccess && watchSuccess && windowSuccess	&& argvSuccess && privilegesSuccess && memory)
	{
        ShowWindow(context->window, SW_SHOWMAXIMIZED);
        bool wasDomainInit = false;
        while (context->keepRunning)
		{
            
            float32 frameStartTime = getProcessCurrentTime();
            
            if(hasDllChangedAndReloaded(&domainLibWatch, &domainLib))
			{
				OBTAINDLLFUNC(domainLib, init);
				OBTAINDLLFUNC(domainLib, input);
				OBTAINDLLFUNC(domainLib, iteration);
				OBTAINDLLFUNC(domainLib, render);
                               
                if(domainLib == NULL)
				{
                    input = NULL;
					iteration = NULL;
					render = NULL;
					init = NULL;
                }
				else
				{
					if(!wasDomainInit && init != NULL)
					{
						init(mem);
						wasDomainInit = true;
					}
				}
            }
            
            context->input = {};
            
            MSG msg;
            while(PeekMessage(&msg, context->window, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            
            // NOTE(fidli): handle domain input here
			if(input != NULL)
			{
				input(&context->input);
			}
			// NOTE(fidli): handle domain loop here
			if(iteration != NULL)
			{
				iteration(FRAME_TIME);
			}
			
            context->renderingTarget.info.width = context->renderer.width;
            context->renderingTarget.info.height = context->renderer.height;
            context->renderingTarget.data = (byte *)context->renderer.drawbuffer;
                      
			// NOTE(fidli): handle domain rendering here
			if(render != NULL)
			{
				render(&context->renderingTarget);
			}
            float32 endFrameTime = getProcessCurrentTime();
            float32 timeTaken = endFrameTime-frameStartTime;
            
			//30 fps
            if(timeTaken < FRAME_TIME)
			{
                Sleep(1000*(DWORD)(FRAME_TIME - timeTaken));
            }
            InvalidateRect(context->window, NULL, TRUE);
                       
        }
        
        
    }
	else
	{
		LOGE(default, setup, "Not everything set-up correctly");
        ASSERT(false);
    }
    
    if (!VirtualFree(memoryStart, 0, MEM_RELEASE)) {
        ASSERT(!"Failed to free memory");
    }
    
    return 0;
}



int mainCRTStartup(){
    int argc = 0;
    LPWSTR * argv =  CommandLineToArgvW(GetCommandLineW(), &argc);
    
    int result = main(argv,argc);
    LocalFree(argv);
    ExitProcess(result);
}

