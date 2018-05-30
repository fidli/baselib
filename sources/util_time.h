#ifndef UTIL_TIME
#define UTIL_TIME


struct LocalTime{
    uint16 day;
    uint16 month;
    uint16 year;
    
    uint16 hour;
    uint16 minute;
    uint16 second;
    
    uint16 millisecond;
};

static bool transitionYear(const uint16 year){
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


uint64 getTick();

float64 translateTickToTime();

float32 getProcessCurrentTime();


LocalTime getLocalTime();

#endif