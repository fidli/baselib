#ifndef WINDOWS_IO
#define WINDOWS_IO

#include "windows_filesystem.cpp"

#include "util_io.cpp"

static FileHandle consoleOut;
static FileHandle consoleIn;

bool initIo(){
    consoleOut.handle = GetStdHandle(STD_OUTPUT_HANDLE);
    consoleIn.handle = GetStdHandle(STD_INPUT_HANDLE);
    return (consoleOut.handle != INVALID_HANDLE_VALUE && consoleOut.handle != NULL) && (consoleIn.handle != INVALID_HANDLE_VALUE && consoleIn.handle != NULL);
}

int readConsole(char * buffer, u16 maxsize){
    //< check whether redirection requires read file instead of 
    FlushConsoleInputBuffer(consoleIn.handle);
    bool done = false;
    int size = 0;
    
    while(!done){
        WaitForSingleObject(consoleIn.handle, INFINITE);
        DWORD amount;
        ASSERT(GetNumberOfConsoleInputEvents(consoleIn.handle, &amount) > 0);
        INPUT_RECORD * events = &PUSHA(INPUT_RECORD, amount);
        DWORD read;
        ASSERT(ReadConsoleInput(consoleIn.handle, events, amount, &read) > 0);
        
        for(DWORD i = 0; i < read; i++){
            if(events[i].EventType == KEY_EVENT && events[i].Event.KeyEvent.wRepeatCount == 1 && events[i].Event.KeyEvent.bKeyDown){
                char readChar = events[i].Event.KeyEvent.uChar.AsciiChar;
                if(readChar == '\r'){
                    done = true;
                    if(size == maxsize){
                        POP;
                        return 0;
                    }
                    buffer[size++] = '\0';
                    printf("\r\n");
                    break;
                }else if(readChar == (char) 8 || (readChar >= ' ' && readChar <= '~')){
                    if(size == maxsize){
                        POP;
                        return 0;
                    }
                    if(readChar != (char) 8){
                        buffer[size++] = readChar;
                        printf("%c", readChar);
                    }else{
                        if(size > 0){
                            size--;
                            
                            DWORD written;
                            CONSOLE_SCREEN_BUFFER_INFO info;
                            ASSERT(GetConsoleScreenBufferInfo(consoleOut.handle, &info) > 0);
                            COORD newpos = info.dwCursorPosition;
                            newpos.X -= 1;
                            ASSERT(SetConsoleCursorPosition(consoleOut.handle, newpos) > 0);
                            ASSERT(FillConsoleOutputCharacter(consoleOut.handle, ' ', 1, newpos, &written) > 0);
                            ASSERT(written == 1);
                        }
                    }
                }
            }
        }
        POP;
    }
    
    return size;
    
}
bool writeConsole(const FileContents * source){
    bool result = writeFile(&consoleOut, source);
    ASSERT(result);
    return result;
}

#endif
