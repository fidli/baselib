#ifndef UTIL_TIME
#define UTIL_TIME

struct Timer{
    f64 period;
    f64 progressAbsolute;
    f64 progressNormalized;
    bool ticked;
};

static Timer timers[255];
static i32 timersCount;

struct LocalTime{
    u16 day;
    u16 month;
    u16 year;
    
    u16 hour;
    u16 minute;
    u16 second;
    
    u16 millisecond;
};

bool initTimeDone = false;

bool initTime();

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

Timer* addTimer(f64 tick){
    Timer * t = &timers[timersCount++];
    memset(CAST(void*, t), 0, sizeof(Timer));
    t->period = tick;
    return t;
}

void advanceTimers(f64 dt){
    for(i32 i = 0; i < timersCount; i++){
        Timer * t = &timers[i];
        t->progressAbsolute += dt;
        t->ticked = false;
        while(t->progressAbsolute > t->period){
            t->progressAbsolute -= t->period;
            t->ticked = true;
        }
        t->progressNormalized = t->progressAbsolute / t->period;
    }
}

#endif
