#ifndef LINUX_TIME
#define LINUX_TIME

#include "util_time.h"

#include <time.h>

static timespec clock_;


static f32 startTime;

bool initTime(){
    startTime = getProcessCurrentTime();
    return true;
}


float32 getProcessCurrentTime(){
    clock_gettime(CLOCK_MONOTONIC, &clock_);
    return ((float32)clock_.tv_sec) + clock_.tv_nsec / 1.0e9 - startTime;
}

LocalTime sysToLocal(const timespec * timespec){
    
    
    tm * time = localtime (&timespec->tv_sec);
    
    LocalTime result = {};
    result.year = time->tm_year + 1900;
    result.month = time->tm_mon + 1;
    result.day = time->tm_mday;
    
    result.hour = time->tm_hour;
    result.minute = time->tm_min;
    result.second = time->tm_sec;
    result.millisecond = timespec->tv_nsec / 1.0e6;
    return result;
}


LocalTime getLocalTime(){
    
    LocalTime result = {};
    
    
    timespec spec;
    
    clock_gettime(CLOCK_REALTIME, &spec);
    result = sysToLocal(&spec);
    
    return result;
}

#endif
