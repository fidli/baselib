#pragma once
#include "util_time.h"
i32 logErrorCount;
#include <stdarg.h>
#ifndef RELEASE
#include "util_string.cpp"
#include "util_io.cpp"
#else
#include "math_macros.h"
#include "mem_structs.h"
#ifndef CRT_PRESENT
u32 snprintf(char * target, nint limit, const char * format, ...);
#endif
inline void * allocate(PersistentStackAllocator * allocator, u64 bytes);
nint strnlen(const char * target, nint limit);
u32 printFormatted(u32 maxprint, char * target, const char * format, va_list ap);
int printf(const char * format, ...);

u8 numlen(i64 number);
f64 powd64(f64 base, i16 power);
f32 powd(f32 base, i16 power);

bool appendFile(const char * path, char * data, u32 length);
bool createEmptyFile(const char * path);
#endif
//NOTE(AK):
/*

Usage:
I want to be able to just call these in various places with possibly 0 configuration

E.g.

LOGW(render, "Rendered only part of text '%s', canvas is too small", text);
LOG(network, "Client connected. Assigned ID: '%d', id);

It is obvious that there must be a target, where to log - PopUp message?, Console?, Custom Console? File?

Should resources define that?
What if resources want to log to multiple loggers?
What if I want to log under resource but to only one target and not all?

create loggers first?
pseudo:

//the name should be globally unique, have and array of loggers, init them per domain
void createLogger("console", file/console, other spec...);

LOGW(console, render, "Rendered only part of text '%s', canvas is too small", text);

*/

enum LogLevel{
    LogLevel_Error,
    LogLevel_Warning,
    LogLevel_Notice
};

enum LogTarget{
    LogTarget_Invalid,
    
    LogTarget_Cli,
    LogTarget_File,
    LogTarget_Status,
    LogTarget_Console,
    
    LogTarget_Count
};

struct LoggerInfo{
    LogLevel level;
    LogTarget target;
    union{
        struct{
            char path[255];
        } file;
        struct{
            char lastStatus[255];
            f32 lastStatusTime;
        } status;
        struct{
            char lastStates[100][255];
            f32 lastStatusTime;
            i32 count;
            i32 size;
            i32 head;
        } console;
    };
};

struct Loggers{
    char messagebuffer[1024];
    char formatbuffer[1024];
    LocalTime lt;
    
    LoggerInfo loggers[20];
    char loggerNames[20][20];
    u8 loggerCount;
};

static Loggers * loggers;

bool initLog(){
    loggers = ((Loggers *) allocate(&mem.persistent, sizeof(Loggers)));
    return true;
}
// NOTE(fidli): stupid linux hak
#define VA_ARGS(...) , ##__VA_ARGS__
#define LOGE(loggerName, resourceName, message, ...) log(STRINGIFY(loggerName), LogLevel_Error, STRINGIFY(resourceName), (message) VA_ARGS(__VA_ARGS__)); logErrorCount++;
#define LOGW(loggerName, resourceName, message, ...) log(STRINGIFY(loggerName), LogLevel_Warning, STRINGIFY(resourceName), (message) VA_ARGS( __VA_ARGS__));
#define LOG(loggerName, resourceName, message, ...) log(STRINGIFY(loggerName), LogLevel_Notice, STRINGIFY(resourceName), (message) VA_ARGS(__VA_ARGS__));

char map[3] = {'E', 'W', 'N'};

void log(const char * loggerName, LogLevel level, const char * resourceName, const char * format, ...){
    va_list ap;    
    va_start(ap, format);
    char format2[] = "[%02hu.%02hu.%04hu %02hu:%02hu:%02hu][%s][%c] %s\r\n";
    loggers->lt = getLocalTime();
    snprintf(loggers->formatbuffer, ARRAYSIZE(loggers->formatbuffer), format2, loggers->lt.day, loggers->lt.month, loggers->lt.year, loggers->lt.hour, loggers->lt.minute, loggers->lt.second, resourceName, map[CAST(i32, level)], format);
#if CRT_PRESENT
    vsnprintf(loggers->messagebuffer, ARRAYSIZE(loggers->messagebuffer), loggers->formatbuffer, ap);
#else
    printFormatted(ARRAYSIZE(loggers->messagebuffer), loggers->messagebuffer, loggers->formatbuffer, ap);
#endif
    va_end(ap);
    LoggerInfo * info = NULL;
    //TODO(AK): Hash table this
    for(i32 i = 0; i < ARRAYSIZE(loggers->loggerNames); i++){
        if(!strncmp(loggerName, loggers->loggerNames[i], ARRAYSIZE(loggers->loggerNames[0]))){
            info = &loggers->loggers[i];
        }
    }
    if(!info && !strncmp(loggerName, "default", 7)){
        //STD output
        printf("%1024s", loggers->messagebuffer);
        return;
    }else if(info){
        if(info->level >= level){
            switch(info->target){
                case LogTarget_Cli:{
                    printf("%1024s", loggers->messagebuffer);
                }break;
                case LogTarget_File:{
                    appendFile(info->file.path, loggers->messagebuffer, strnlen(loggers->messagebuffer, 1024));
                }break;
                case LogTarget_Status:{
                    strncpy(info->status.lastStatus, strstr(loggers->messagebuffer, "] ")+2, 255);
                    info->status.lastStatusTime = getProcessCurrentTime();
                }break;
                case LogTarget_Console:{
                    i32 newIndex = (info->console.head + 1) % info->console.size;
                    strncpy(info->console.lastStates[info->console.head], loggers->messagebuffer, 255);
                    info->console.lastStatusTime = getProcessCurrentTime();
                    info->console.head = newIndex;
                    info->console.count = MIN(info->console.size, info->console.count+1);
                }break;
                case LogTarget_Count:
                case LogTarget_Invalid:
                default:{
#ifndef RELEASE
                    //ASSERT
                    *(int *)0 = 0;
#endif
                }break;
            }
        }
    }else{
		// NOTE(fidli): logger with name loggerName not found
#ifndef RELEASE
        //ASSERT
        *(int *)0 = 0;
#endif
    }
    
}

i32 getLoggerStatusCount(const char * loggerName){
    for(i32 i = 0; i < ARRAYSIZE(loggers->loggerNames); i++){
        if(!strncmp(loggerName, loggers->loggerNames[i], ARRAYSIZE(loggers->loggerNames[0]))){
            if(loggers->loggers[i].target == LogTarget_Status){
                return 1;
            }else if(loggers->loggers[i].target == LogTarget_Console){
                return loggers->loggers[i].console.count;
            }else{
#ifndef RELEASE
        INV
#endif
            }
        }
    }
    return 0;
}

const char * getLoggerStatus(const char * loggerName, i32 recentMessageIndex = 0){
    for(i32 i = 0; i < ARRAYSIZE(loggers->loggerNames); i++){
        if(!strncmp(loggerName, loggers->loggerNames[i], ARRAYSIZE(loggers->loggerNames[0]))){
            if(loggers->loggers[i].target == LogTarget_Status){
#ifndef RELEASE
                ASSERT(recentMessageIndex == 0);
#endif
                return loggers->loggers[i].status.lastStatus;
            }else if(loggers->loggers[i].target == LogTarget_Console){
                i32 index = (loggers->loggers[i].console.head - 1 - recentMessageIndex + loggers->loggers[i].console.size) % loggers->loggers[i].console.size;
                return loggers->loggers[i].console.lastStates[index];
            }else{
#ifndef RELEASE
        INV
#endif
            }
        }
    }
    return NULL;
}

f32 getLoggerStatusTime(const char * loggerName){
    for(i32 i = 0; i < ARRAYSIZE(loggers->loggerNames); i++){
        if(!strncmp(loggerName, loggers->loggerNames[i], ARRAYSIZE(loggers->loggerNames[0]))){
            if(loggers->loggers[i].target == LogTarget_Status){
                return loggers->loggers[i].status.lastStatusTime;
            }else if(loggers->loggers[i].target == LogTarget_Console){
                return loggers->loggers[i].console.lastStatusTime;
            }
        }
    }
    return 0;
}

bool createStatusLogger(const char * loggerName, LogLevel level){
    if(loggers->loggerCount >= ARRAYSIZE(loggers->loggers)){
        return false;
    }
    for(i32 i = 0; i < ARRAYSIZE(loggers->loggerNames); i++){
        if(!strncmp(loggerName, loggers->loggerNames[i], ARRAYSIZE(loggers->loggerNames[0]))){
            return false;
        }
    }
    strncpy(loggers->loggerNames[loggers->loggerCount], loggerName, ARRAYSIZE(loggers->loggerNames[0]));
    loggers->loggers[loggers->loggerCount].target = LogTarget_Status;
    memset(loggers->loggers[loggers->loggerCount].status.lastStatus, 0, ARRAYSIZE(loggers->loggers[loggers->loggerCount].status.lastStatus));
    loggers->loggers[loggers->loggerCount].level = level;
    loggers->loggerCount++;
    return true;
}

bool createConsoleLogger(const char * loggerName, LogLevel level){
    if(loggers->loggerCount >= ARRAYSIZE(loggers->loggers)){
        return false;
    }
    for(i32 i = 0; i < ARRAYSIZE(loggers->loggerNames); i++){
        if(!strncmp(loggerName, loggers->loggerNames[i], ARRAYSIZE(loggers->loggerNames[0]))){
            return false;
        }
    }
    strncpy(loggers->loggerNames[loggers->loggerCount], loggerName, ARRAYSIZE(loggers->loggerNames[0]));
    loggers->loggers[loggers->loggerCount].target = LogTarget_Console;
    memset(loggers->loggers[loggers->loggerCount].console.lastStates, 0, ARRAYSIZE(loggers->loggers[loggers->loggerCount].console.lastStates) * ARRAYSIZE(loggers->loggers[loggers->loggerCount].console.lastStates[0]));
    loggers->loggers[loggers->loggerCount].console.size = ARRAYSIZE(loggers->loggers[loggers->loggerCount].console.lastStates);
    loggers->loggers[loggers->loggerCount].console.head = 0;
    loggers->loggers[loggers->loggerCount].console.count = 0;
    loggers->loggers[loggers->loggerCount].level = level;
    loggers->loggerCount++;
    return true;

}

bool createFileLogger(const char * loggerName, LogLevel level, const char * path){
    if(loggers->loggerCount >= ARRAYSIZE(loggers->loggers)){
        return false;
    }
    for(i32 i = 0; i < ARRAYSIZE(loggers->loggerNames); i++){
        if(!strncmp(loggerName, loggers->loggerNames[i], ARRAYSIZE(loggers->loggerNames[0]))){
            return false;
        }
    }
    bool result = createEmptyFile(path);
    if(result){
        strncpy(loggers->loggerNames[loggers->loggerCount], loggerName, ARRAYSIZE(loggers->loggerNames[0]));
        strncpy(loggers->loggers[loggers->loggerCount].file.path, path, ARRAYSIZE(loggers->loggers[loggers->loggerCount].file.path));
        loggers->loggers[loggers->loggerCount].target = LogTarget_File;
        loggers->loggers[loggers->loggerCount].level = level;
        loggers->loggerCount++;
    }
    return result;
}
