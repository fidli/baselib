#ifndef UTIL_TIME
#define UTIL_TIME


struct LocalTime{
    u16 day;
    u16 month;
    u16 year;
    
    u16 hour;
    u16 minute;
    u16 second;
    
    u16 millisecond;
};

static bool transitionYear(const u16 year){
    return (year % 4) == 0 || (year / 100) * 100 == year;
}

bool isLocalTimeValid(const LocalTime * time){
    bool clock = time->hour <= 23 && time->minute <= 59 && time->second <= 59;
    
    bool date = time->month >= 1 && time->month <= 12 && time->day <= 31 && time->day >= 1 && time->year < 10000;
    bool sense = ((time->month & 1) && time->month < 7 && time->day <= 30) || ((time->month & 1) && time->month > 7 && time->day <= 30) || 
        (!(time->month & 1) && time->month != 2) || (time->month == 2 && transitionYear(time->year) && time->day <= 29) || (time->month == 2 && !transitionYear(time->year) && time->day <= 28);
    
    return clock && date && sense;
}

bool operator!=(const LocalTime a, const LocalTime b){
    return
        a.day != b.day ||
        a.month != b.month ||
        a.year != b.year ||
        a.hour != b.hour ||
        a.minute != b.minute ||
        a.second != b.second ||
        a.millisecond != b.millisecond;
}


u64 getTick();

f64 translateTickToTime(const u64 tick, const u64 tickDivisor);

u64 getTickDivisor();

f64 getProcessCurrentTime();


LocalTime getLocalTime();

#endif
