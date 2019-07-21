#pragma once

#include "util_string.cpp"
#include "util_io.cpp"
#include "util_time.h"

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
    LogLevel_Error = (int)'E',
    LogLevel_Warning = (int)'W',
    LogLevel_Notice = (int)'N'
};

enum LogTarget{
    LogTarget_Invalid,
    
    LogTarget_Console,
    
    LogTarget_Count
};

struct LoggerInfo{
    LogLevel level;
    LogTarget target;
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

#define xstr(a) str(a)
#define str(a) #a

#define LOGE(loggerName, resourceName, message, ...) log(xstr(loggerName), LogLevel_Error, xstr(resourceName), (message), __VA_ARGS__);
#define LOGW(loggerName, resourceName, message, ...) log(xstr(loggerName), LogLevel_Warning, xstr(resourceName), (message), __VA_ARGS__);
#define LOG(loggerName, resourceName, message, ...) log(xstr(loggerName), LogLevel_Notice, xstr(resourceName), (message), __VA_ARGS__);


void log(char * loggerName, LogLevel level, char * resourceName, char * format, ...){
    va_list ap;    
    va_start(ap, format);
    char format2[] = "[%02hu.%02hu.%04hu %02hu:%02hu:%02hu][%s][%c] %s\r\n";
    loggers.lt = getLocalTime();
    snprintf(loggers.formatbuffer, ARRAYSIZE(loggers.formatbuffer), format2, loggers.lt.day, loggers.lt.month, loggers.lt.year, loggers.lt.hour, loggers.lt.minute, loggers.lt.second, resourceName, (char) level, format);
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
        if(info->level <= level){
            switch(info->target){
                case LogTarget_Console:{
                    printf("%1024s", loggers.messagebuffer);
                }break;
                default:{
                    //ASSERT
                    *(int *)0 = 0;
                }break;
            }
        }
    }else{
		// NOTE(fidli): logger with name loggerName not found
        // ASSERT
        *(int *)0 = 0;
    }
    
}