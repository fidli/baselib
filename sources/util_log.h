#ifndef UTIL_LOG_H
#define UTIL_LOG_H

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
    LogLevel_Invalid,
    
    LogLevel_Error,
    LogLevel_Warning,
    LogLevel_Notice,
    
    LogLevel_Count,
};

struct LoggerInfo{
    
};



struct Loggers{
    char logbuffer[1024];
    LoggerInfo loggers[20];
    char * loggerNames[20][20];
    uint8 loggerCount;
};

static Loggers loggers;

#define LOGE(loggerName, resourceName, message, ...) log(#(loggerName), LogLevel_Error, #(resource), (message), __VA_ARGS__);
#define LOGW(loggerName, resourceName, message, ...) log(#(loggerName), LogLevel_Warning, #(resource), (message), __VA_ARGS__);
#define LOG(loggerName, resourceName, message, ...) log(#(loggerName), LogLevel_Notice, #(resource), (message), __VA_ARGS__);


void log(char * loggerName, LogLevel level, char * resourceName, char * format, ...){
    va_list ap;    
    va_start(ap, format);
    printFormatted(ARRAYSIZE(loggers.logbuffer), logbuffer, format, ap);
    va_end(ap);
    
}

#define LOGL(level, resource, message,...)  lt = getLocalTime(); sprintf(logbuffer, "[%02hu.%02hu.%04hu %02hu:%02hu:%02hu] %900s\r\n", lt.day, lt.month, lt.year, lt.hour, lt.minute, lt.second, (message)); printf(logbuffer, __VA_ARGS__);



#endif