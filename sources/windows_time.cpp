#ifndef WINDOWS_TIME
#define WINDOWS_TIME

#include "util_time.h"

static LARGE_INTEGER frequency;

bool initTime(){
    return QueryPerformanceFrequency(&frequency) != 0;
}

float32 getProcessCurrentTime(){
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return ((float32)counter.QuadPart / (float32)frequency.QuadPart);
    
}

LocalTime sysToLocal(const SYSTEMTIME * time){
    LocalTime result = {};
    result.year = time->wYear;
    result.month = time->wMonth;
    result.day = time->wDay;
    
    result.hour = time->wHour;
    result.minute = time->wMinute;
    result.second = time->wSecond;
    result.millisecond = time->wMilliseconds;
    return result;
}


LocalTime getLocalTime(){
    
    LocalTime result = {};
    
    SYSTEMTIME time;
    GetLocalTime(&time);
    
    result = sysToLocal(&time);
    
    return result;
}


bool setLocalTime(const LocalTime * source){
    SYSTEMTIME time = {};
    
    time.wYear = source->year;
    time.wMonth = source->month;
    time.wDay = source->day;
    
    time.wHour = source->hour;
    time.wMinute = source->minute ;
    time.wSecond = source->second;
    
    return SetLocalTime(&time) > 0;
}

        #endif