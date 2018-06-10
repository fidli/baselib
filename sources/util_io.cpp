#ifndef UTIL_IO
#define UTIL_IO

#include "util_filesystem.h"
#include "util_string.cpp"

FileHandle getStdOut();
bool writeConsole(const FileContents * source);
int readConsole(char * buffer, uint16 maxlen);
bool initIo();

#ifndef CRT_PRESENT
int printf(const char * format, ...){
    va_list ap;
    va_start(ap, format);
    
    char buffer[1024];
    
    FileContents con;
    con.contents = buffer;
    uint32 successfullyPrinted = printFormatted(ARRAYSIZE(buffer), con.contents, format, ap);
    con.size = strlen(con.contents);
    ASSERT(con.size < 1024);
    va_end(ap);
    
    writeConsole(&con);
    return successfullyPrinted;
}

void print(char * source){
    FileContents con;
    con.contents = source;
    con.size = strlen(con.contents);
    ASSERT(con.size < 1024);
    writeConsole(&con);
}

int scanf(const char * format, ...){
    va_list ap;
    va_start(ap, format);
    
    char buffer[1024];
    int res = readConsole(buffer, 1024);
    ASSERT(res > 0);
    
    uint32 succesfullyScanned = scanFormatted(buffer, format, ap);
    va_end(ap);
    
    return succesfullyScanned;
}
#else
#include <cstdio>
#endif

#endif