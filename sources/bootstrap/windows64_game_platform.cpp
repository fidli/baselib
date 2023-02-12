#include "platform/windows_nocrt.h"
#include <Windows.h>
#include "platform/windows_types.h"

#include "settings.h"

#include "common.h"
#include "util_mem.h"
#include "util_profile.cpp"

//NOTE(AK): this-app instance
HINSTANCE hInstance;
HWND window;

#include "platform/windows_time.cpp"
#include "platform/windows_filesystem.cpp"
#include "platform/windows_io.cpp"
#include "platform/windows_env.cpp"
#include "util_log.cpp"
#include "util_string.cpp"
#include "util_conv.cpp"
#include "util_data.cpp"
#include "util_font.cpp"
#include "platform/windows_opengl.cpp"
#include "util_opengl.cpp"
#include "platform/windows_imgui.cpp"

//NOTE(AK): these are set after memory initialisation
struct Platform;
Platform * platform;
struct OpenGL;
OpenGL * gl;

#include "platform_game_func.cpp"
#include "game.cpp"
//NOTE(AK): these are set after memory initialisation
Game * game;
#include "platform_game_render.cpp"

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
    if(!guiHandleInputWin(message, wParam, lParam)){
        switch(message)
        {
            case WM_NCMOUSEMOVE:{
                platform->mouseOut = true;
                return 0;
            }break;
            case WM_MOUSEMOVE:{
                i16 x = lParam;
                i16 y = lParam >> 16;
                platform->input.mouse.pos = DV2(x, y);
                platform->mouseOut = false;
            }break;
            case WM_SIZING:{
                RECT * dims = (RECT *) lParam;
                platform->resolution.x = dims->right - dims->left;
                platform->resolution.y = dims->bottom - dims->top;
                return TRUE;
            }break;
            case WM_SIZE:{
                platform->resolution.y = (i16) (lParam >> 16);
                platform->resolution.x = (i16) (lParam);
                return 0;
            }break;
            case WM_CLOSE:
            case WM_DESTROY:{
                    gameExit();
                    return 0;
            } break;
            case WM_KEYDOWN:{
            }break;
            case WM_KEYUP:{
            }break;
        }
    }
    return DefWindowProc (hWnd, message, wParam, lParam);
}

static inline int main(LPWSTR * argvW, int argc) {
    LPVOID memoryStart = VirtualAlloc(NULL, TEMP_MEM + PERSISTENT_MEM, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    
    if (memoryStart) {
        //START OF INITING ROUTINES
        initMemory(memoryStart);
        {
            HANDLE processToken  = NULL;
            if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &processToken))
            {
                DWORD privsize = 0;
                GetTokenInformation(processToken, TokenPrivileges, NULL, 0, &privsize);
                TOKEN_PRIVILEGES * priv = (TOKEN_PRIVILEGES *) &PUSHA_SCOPE(byte, privsize);
                if(GetTokenInformation(processToken, TokenPrivileges, priv , privsize, &privsize))
                {
                    for(DWORD i = 0; i < priv->PrivilegeCount; ++i )
                    { 
                        priv->Privileges[i].Attributes = SE_PRIVILEGE_REMOVED;
                    }
                    if(AdjustTokenPrivileges(processToken, TRUE, priv, NULL, NULL, NULL) == 0)
                    {
                        VirtualFree(memoryStart, 0, MEM_RELEASE);
                        return 1;
                    }
                }
            }else{
                VirtualFree(memoryStart, 0, MEM_RELEASE);
                return 1;
            }
            CloseHandle(processToken);
        }
        HANDLE instance = CreateMutexEx(NULL, "Global_"  GAME_UNIQUE_NAME "_game", 0, 0);
        if(instance == NULL){
            HWND runningEd = FindWindow("GameMainClass", GAME_UNIQUE_NAME);
            if(runningEd == NULL){
                MessageBox(NULL, "Other instance of game is running (maybe crashed, kill in task manager and check error log)", "Already running", MB_OK);
                VirtualFree(memoryStart, 0, MEM_RELEASE);
                return 1;
            }else{
                ShowWindow(runningEd, SW_SHOWNORMAL);
                SetForegroundWindow(runningEd);
                VirtualFree(memoryStart, 0, MEM_RELEASE);
                return 0;
            }
        }

        bool initSuccess = true;
        char * executableDir = &PPUSHA(char, 255);
        GetModuleFileNameA(NULL, executableDir, 255);
        for(i32 i = strlen(executableDir)-1; i >= 0; i--){
            if(executableDir[i] == '\\'){
                executableDir[i] = '\0';
                break;
            }
        }
        char * lastRunDir = &PPUSHA(char, 255);
        snprintf(lastRunDir, 255, "%s\\last_run\\", executableDir);
        char * saveDir = &PPUSHA(char, 255);
        snprintf(saveDir, 255, "%s\\save\\", executableDir);
        bool firstRun = !dirExists(lastRunDir) && !dirExists(saveDir);
        initSuccess &= createDirectory(lastRunDir) && createDirectory(saveDir);
        if(!initSuccess){
            VirtualFree(memoryStart, 0, MEM_RELEASE);
            return 1;
        }
        initSuccess &= initLog();
        if(!initSuccess){
            VirtualFree(memoryStart, 0, MEM_RELEASE);
            return 1;
        }
        char lastExitPath[255];
        snprintf(lastExitPath, 255, "%s\\last_exit.txt", lastRunDir);
        char * currentLogPath = &PPUSHA(char, 255);
        snprintf(currentLogPath, 255, "%s\\current_log.txt", lastRunDir);
        char * badLogPath = &PPUSHA(char, 255);
        snprintf(badLogPath, 255, "%s\\bad_log.txt", lastRunDir);
        char * lastLogPath = &PPUSHA(char, 255);
        snprintf(lastLogPath, 255, "%s\\last_log.txt", lastRunDir);
        
        initSuccess &= createFileLogger("default", LogLevel_Notice, currentLogPath) && createStatusLogger("status", LogLevel_Notice) && createStatusLogger("status_big", LogLevel_Notice) && createConsoleLogger("console", LogLevel_Notice);
        bool lastExitWasOk = false;
        {
            bool lastExitStatus = fileExists(lastExitPath);
            if(lastExitStatus){
                FileContents lastExitFileContents = {};
                readFile(lastExitPath, &lastExitFileContents);
                i32 res = - 1;
                i32 ec = 1;
                snscanf(lastExitFileContents.contents, lastExitFileContents.size, "%d %d", &res, &ec);
                if(lastExitFileContents.size > 10){
                    lastExitFileContents.contents[10] = '\0';
                }
                LOG(default, last_run, "Last exit file: %s. Scanned %d %d", lastExitFileContents.contents, res, ec); 
                lastExitStatus = res == 0 && ec == 0;
            }
            lastExitWasOk = (lastExitStatus && !fileExists(badLogPath)) || (firstRun);
            if(!lastExitWasOk){
                if(!fileExists(currentLogPath) && !fileExists(badLogPath)){
                    FileContents c = {};
                    c.size = KILOBYTE(1);
                    c.contents = &PUSHA_SCOPE(char, c.size);
                    strncpy(c.contents, "No previous log was present", c.size);
                    saveFile(badLogPath, &c);
                }else if(fileExists(currentLogPath) && !fileExists(badLogPath) && !moveFile(currentLogPath, badLogPath)){
                    MessageBox(NULL, "There was an error. Either clear logs and start fresh, or contact a developer.", "Too bad", MB_OK);
                    VirtualFree(memoryStart, 0, MEM_RELEASE);
                    return 1;
                }else{
#ifdef RELEASE
                    MessageBox(NULL, "Last run had an error. You can continue, but please contact a developer to solve it.", "Not good, but whatever", MB_OK);
#endif
                }
            }
        }
        
        char ** argv = &PPUSHA(char *, argc);
        char ** argvUTF8 = &PPUSHA(char *, argc);
        //command line arguments parsing
        if(initSuccess){
            for(int i = 0; initSuccess && i < argc; i++){
                int toAlloc = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, NULL, 0, NULL, NULL);
                initSuccess &= toAlloc != 0;
                if(initSuccess){
                    argv[i] = &PPUSHA(char, toAlloc);
                    initSuccess &= WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, argv[i], toAlloc, NULL, NULL) != 0;
                    ASSERT(toAlloc != 0);
                    if(initSuccess){
                        argvUTF8[i] = &PPUSHA(char, toAlloc);
                        initSuccess &= WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, argvUTF8[i], toAlloc, NULL, NULL) != 0;
                        if(initSuccess){
                            u32 finalLen;
                            argv[i] = &PPUSHA(char, toAlloc);
                            if(convUTF8toAscii((byte *)argvUTF8[i], toAlloc, &argv[i], &finalLen) != 0){
                                LOGW(default, common, "An argument %d is not fully ascii compatible - those characters were replaced by '_'. Please use simple ASCII parameter values\n", i);
                            }
                        }
                    }
                }
            }
        }
        //memory assignation
        platform = &PPUSH(Platform);
        platform->lastRunDir = lastRunDir;
        platform->lastExitPath = lastExitPath;
        platform->badLogPath = badLogPath;
        platform->saveDir = saveDir;
        game = &PPUSH(Game);
        gl = &PPUSH(OpenGL);
        
        //modules initalization
        initSuccess &= initSuccess && initTime();
        LOG(default, startup, "Init time success: %u", initSuccess);
        initSuccess &= initSuccess && initProfile();
        LOG(default, startup, "Init profile success: %u", initSuccess);
        
        //creating window
        WNDCLASSEX style = {};
        style.cbSize = sizeof(WNDCLASSEX);
        style.style = CS_OWNDC | CS_DBLCLKS;
        style.lpfnWndProc = WindowProc;
        style.hInstance = hInstance;
        style.lpszClassName = "GameMainClass";
        initSuccess &= initSuccess && RegisterClassEx(&style) != 0;
        if(initSuccess){
            window = CreateWindowEx(NULL,
                                    "GameMainClass", GAME_UNIQUE_NAME, WS_OVERLAPPEDWINDOW | WS_SIZEBOX,
                                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
            initSuccess &= window != NULL;
        }
        LOG(default, startup, "Init window success: %u", initSuccess);
        //Open gl initialisation
        //is used later, also when deleting
        HGLRC gameGlContext;
        HDC dc;
        //creating drawing context, so that we can use opengl
        if(initSuccess){
            PIXELFORMATDESCRIPTOR contextFormat = {
                sizeof(PIXELFORMATDESCRIPTOR),
                1,
                PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
                PFD_TYPE_RGBA,
                32, //32 bits per color
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                24, //24 bit z buffer
                0, //0 bit stencil buffer (these two values must not exceed 32 bit)
                0,
                0,
                0,
                0
            };
            
            dc = GetDC(window);
            int pixelFormat = ChoosePixelFormat(dc, &contextFormat);//windows matches our wishes
            initSuccess &= pixelFormat != 0;
            //setting the wished parameters upon our window
            initSuccess &= initSuccess && SetPixelFormat(dc, pixelFormat, &contextFormat) == TRUE;
            if(initSuccess){
                //load gl fucntions, 
                initSuccess &= initSuccess & initOpenGL(dc);
                // vsync disable
                wglSwapIntervalEXT(0);
            }
            //creating opengl context
            if(initSuccess){
                gameGlContext = wglCreateContext(dc);
                initSuccess &= gameGlContext != NULL;
                wglMakeCurrent(dc, gameGlContext);
            }
            
        }
        //end of opengl init
        LOG(default, startup, "Init opengl success: %u", initSuccess);
        HCURSOR cursor = (HCURSOR) LoadImage(hInstance, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE);
        //END OF INITIALISATION ROUTINES
        

        if(initSuccess){
            //show window and get resolution
            {
                LOG(default, startup, "Showing window");
                ShowWindow(window, 1);
            }
            
            initGameRender();
            initSuccess &= initSuccess && guiInit("resources\\fonts\\lib-mono\\lib-mono.bmp", "resources\\fonts\\lib-mono\\lib-mono.fnt");
            LOG(default, startup, "Init gui success: %u", initSuccess);
        }
        if(initSuccess){
            // CURSORS 
            FileContents imageFile = {};
            const char * fullPath = "resources\\hud\\cursor.bmp";
            bool r = readFile(fullPath, &imageFile);
            ASSERT(r);
            Image image;
            r &= decodeBMP(&imageFile, &image);
            ASSERT(r);
            ASSERT(image.info.interpretation == BitmapInterpretationType_RGBA || BitmapInterpretationType_RGB);
            if(image.info.origin == BitmapOriginType_BottomLeft){
                r &= flipY(&image);
                ASSERT(r);
            }
            GLint interp;
            if(image.info.interpretation == BitmapInterpretationType_RGBA){
                interp = GL_RGBA;
            }else if(image.info.interpretation == BitmapInterpretationType_RGB){
                interp = GL_RGB;
            }
            ASSERT(image.info.origin == BitmapOriginType_TopLeft);
            glGenTextures(1, &platform->render.cursorTexture);
            glBindTexture(GL_TEXTURE_2D, platform->render.cursorTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, interp, image.info.width, image.info.height, 0, interp, GL_UNSIGNED_BYTE, image.data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            POP;
        }
        bool r = true;
        r &= compileShaders(___sources_opengl_shaders_game_vert, ___sources_opengl_shaders_game_vert_len, ___sources_opengl_shaders_game_frag, ___sources_opengl_shaders_game_frag_len, &gl->game.vertexShader, &gl->game.fragmentShader, &gl->game.program);
        ASSERT(r);
        initGameShader();
        LOG(default, shaders, "Game shaders loaded");
        //let the game do its init routines
        platformGameInit();

        f64 currentTime = getProcessCurrentTime();
        platform->appRunning = true;
        platform->fps = 1;
        platform->frameIndex = 0;

        f32 accumulator = 0;
        Timer * fpsTimer = platformGetTimer(platformAddTimer(1));
        u64 lastFpsFrameIndex = 0;
        bool wasShowProfile = platform->showProfile;
        //THE GAME LOOP
        while (platform->appRunning) {
            if(!wasShowProfile && platform->showProfile){
                profileClearStats();
                platform->framesRenderedSinceLastProfileClear = 0;
            }
            wasShowProfile = platform->showProfile;
            if(fpsTimer->ticked){
                platform->fps = CAST(f32, platform->frameIndex - lastFpsFrameIndex) / (1.0f + fpsTimer->progressAbsolute);
                lastFpsFrameIndex = platform->frameIndex;
            }
            f64 newTime = getProcessCurrentTime();
            f64 frameTime = newTime - currentTime;
#define ANIMATION_STEP 0.02f
            accumulator += frameTime;
            currentTime = newTime;
            FLUSH;
            platformHotloadAssets();
            //END OF HOTLOADING
            
            //HANDLE INPUT
            auto prevMouse = platform->input.mouse;
            platform->input = {};
            platform->input.mouse = prevMouse;
            MSG msg;
            while(PeekMessage(&msg, window, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
             
            if(gameHandleMetaStuff()){
                gamePreFixedStep(frameTime);
                while(accumulator > ANIMATION_STEP){
                    gameFixedStep(ANIMATION_STEP);
                    accumulator -= ANIMATION_STEP;
                }
                gameStep(frameTime);
            }
            
            if(platform->mouseOut){
                if(GetCursor() == NULL){
                    cursor = SetCursor(cursor);
                }
            }else{
                if(GetCursor() != NULL){
                    cursor = SetCursor(NULL);
                }
            }
            render(frameTime);
            bool r = SwapBuffers(dc) == TRUE;
            ASSERT(r);
            if(!r){
                platform->appRunning = false;
            }
            //END OF RENDER
            //
            
            // advance timers
            for(i32 i = 0; i < platform->timersCount; i++){
                Timer * t = &platform->timers[i];
                t->progressAbsolute += frameTime;
                t->ticked = false;
                while(t->progressAbsolute > t->period){
                    t->progressAbsolute -= t->period;
                    t->ticked = true;
                }
                t->progressNormalized = t->progressAbsolute / t->period;
            }
            platform->frameIndex++;
            platform->framesRenderedSinceLastProfileClear++;
        }
        LOG(default, common, "Quitting main loop");
        //END OF MAIN LOOP
        if(!game->lastRunWasBad){
            deleteFile(lastLogPath);
            if(!moveFile(currentLogPath, lastLogPath)){
                VirtualFree(memoryStart, 0, MEM_RELEASE);
                return 1;
            }
            char con[10] = {};
            FileContents c;
            c.size = ARRAYSIZE(con);
            c.contents = con;
            snprintf(con, c.size, "0 %d", logErrorCount);
            saveFile(lastExitPath, &c);
        }

        //shaders and stuff
        glDeleteShader(gl->game.fragmentShader);
        glDeleteShader(gl->game.vertexShader);
        glDeleteProgram(gl->game.program);
        
        guiFinalize();
        //end of shaders and stuff
        
        //cleanup
        DestroyWindow(window);
        UnregisterClass("GameMainClass", hInstance);
        wglDeleteContext(gameGlContext);
        
        VirtualFree(memoryStart, 0, MEM_RELEASE);
        
    }else{
        // NO MEMORY
    }
    return 0;
}


/**
This is the entry point
*/
void __stdcall WinMainCRTStartup(){
    int argc = 0;
    LPWSTR * argv =  CommandLineToArgvW(GetCommandLineW(), &argc);
    hInstance = GetModuleHandle(NULL);
    int result = main(argv,argc);
    LocalFree(argv);
    ExitProcess(result);
}
