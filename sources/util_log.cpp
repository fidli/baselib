#pragma once

#include "util_time.h"
#ifndef RELEASE
#include "util_string.cpp"
#include "util_io.cpp"
#else
#include "math_macros.h"
uint32 snprintf(char * target, nint limit, const char * format, ...);
uint32 printFormatted(uint32 maxprint, char * target, const char * format, va_list ap);
int printf(const char * format, ...);

uint8 numlen(int64 number);
float64 powd64(float64 base, int16 power);
float32 powd(float32 base, int16 power);

bool appendFile(const char * path, char * data, uint32 length);
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
    
    LogTarget_Console,
    LogTarget_File,
    
    LogTarget_Count
};

struct LoggerInfo{
    LogLevel level;
    LogTarget target;
    union{
        struct{
            char path[255];
        }file;
    };
};



struct Loggers{
    char messagebuffer[1024];
    char formatbuffer[1024];
    LocalTime lt;
    
    LoggerInfo loggers[20];
    char loggerNames[20][20];
    uint8 loggerCount;
};

static Loggers loggers;

#define LOGE(loggerName, resourceName, message, ...) log(STRINGIFY(loggerName), LogLevel_Error, STRINGIFY(resourceName), (message), __VA_ARGS__);
#define LOGW(loggerName, resourceName, message, ...) log(STRINGIFY(loggerName), LogLevel_Warning, STRINGIFY(resourceName), (message), __VA_ARGS__);
#define LOG(loggerName, resourceName, message, ...) log(STRINGIFY(loggerName), LogLevel_Notice, STRINGIFY(resourceName), (message), __VA_ARGS__);

char map[3] = {'E', 'W', 'N'};

void log(char * loggerName, LogLevel level, char * resourceName, char * format, ...){
    va_list ap;    
    va_start(ap, format);
    char format2[] = "[%02hu.%02hu.%04hu %02hu:%02hu:%02hu][%s][%c] %s\r\n";
    loggers.lt = getLocalTime();
    snprintf(loggers.formatbuffer, ARRAYSIZE(loggers.formatbuffer), format2, loggers.lt.day, loggers.lt.month, loggers.lt.year, loggers.lt.hour, loggers.lt.minute, loggers.lt.second, resourceName, map[CAST(int32, level)], format);
#if CRT_PRESENT
    vsnprintf(loggers.messagebuffer, ARRAYSIZE(loggers.messagebuffer), loggers.formatbuffer, ap);
#else
    printFormatted(ARRAYSIZE(loggers.messagebuffer), loggers.messagebuffer, loggers.formatbuffer, ap);
#endif
    va_end(ap);
    LoggerInfo * info = NULL;
    //TODO(AK): Hash table this
    for(int32 i = 0; i < ARRAYSIZE(loggers.loggerNames); i++){
        if(!strncmp(loggerName, loggers.loggerNames[i], ARRAYSIZE(loggers.loggerNames[0]))){
            info = &loggers.loggers[i];
        }
    }
    if(!info && !strncmp(loggerName, "default", 7)){
        //STD output
        printf("%1024s", loggers.messagebuffer);
        return;
    }else if(info){
        if(info->level >= level){
            switch(info->target){
                case LogTarget_Console:{
                    printf("%1024s", loggers.messagebuffer);
                }break;
                case LogTarget_File:{
                    appendFile(info->file.path, loggers.messagebuffer, strlen(loggers.messagebuffer));
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

bool createFileLogger(const char * loggerName, LogLevel level, const char * path){
    if(loggers.loggerCount >= ARRAYSIZE(loggers.loggers)){
        return false;
    }
    for(int32 i = 0; i < ARRAYSIZE(loggers.loggerNames); i++){
        if(!strncmp(loggerName, loggers.loggerNames[i], ARRAYSIZE(loggers.loggerNames[0]))){
            return false;
        }
    }
    bool result = createEmptyFile(path);
    if(result){
        strncpy(loggers.loggerNames[loggers.loggerCount], loggerName, ARRAYSIZE(loggers.loggerNames[0]));
        strncpy(loggers.loggers[loggers.loggerCount].file.path, path, ARRAYSIZE(loggers.loggers[loggers.loggerCount].file.path));
        loggers.loggers[loggers.loggerCount].target = LogTarget_File;
        loggers.loggers[loggers.loggerCount].level = level;
        loggers.loggerCount++;
    }
    return result;
}
