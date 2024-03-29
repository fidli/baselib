#ifndef WINDOWS_TIME
#define WINDOWS_TIME

#include "util_time.h"

static LARGE_INTEGER frequency;
static f64 frequencyF;

bool initTime(){
    initTimeDone = true;
    int result = QueryPerformanceFrequency(&frequency);
    if(result != 0){
        frequencyF = CAST(f64, frequency.QuadPart);
        return true;
    }
    return false;
}

f64 getProcessCurrentTime(){
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (CAST(f64, counter.QuadPart) / frequencyF);
    
}

u64 getTick(){
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart;
}

u64 getTickDivisor(){
    return frequency.QuadPart;
}

f64 translateTickToTime(const u64 tick, u64 divisor = getTickDivisor()){
    return (f64)tick / divisor;
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
