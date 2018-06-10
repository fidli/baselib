#ifndef WINDOWS_TIME
#define WINDOWS_TIME

#include "util_time.h"

static LARGE_INTEGER frequency;
static float32 frequencyF;

bool initTime(){
    int result = QueryPerformanceFrequency(&frequency);
    if(result != 0){
        frequencyF = (float32)frequency.QuadPart;
        return true;
    }
    return false;
}

float32 getProcessCurrentTime(){
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return ((float32)counter.QuadPart / frequencyF);
    
}

uint64 getTick(){
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart;
}

uint64 getTickDivisor(){
    return frequency.QuadPart;
}

float64 translateTickToTime(const uint64 tick, uint64 divisor = getTickDivisor()){
    return (float64)tick / frequencyF;
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