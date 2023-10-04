#pragma once
#include "util_time.h"
#include "util_sort.cpp"

#ifndef PROFILE
#define PROFILE 1
#endif

bool initProfileDone = false;

int printf(const char * format, ...);
#if PROFILE

struct ProfileEntry{
    u64 callCountTotal;
    u64 timeSpentExclusive;
    u64 timeSpentInclusive;
    u64 bytesProcessed;
};

struct ProfileDefer;
#endif


struct Profile{
#if PROFILE
    ProfileEntry slots[100];
    const char * names[100];
    i32 slotsUsed;

    ProfileDefer * lastCaller;
    f64 period;
#endif
    u64 startTime;
    u64 endTime;
};

Profile profile;

bool initProfile(){
    ASSERT(initTimeDone);
    f64 start = getProcessCurrentTime();
    u64 tscStart = __rdtsc();
    f64 end = start + 0.01f;

    f64 now;
    do {
       now = getProcessCurrentTime(); 
    } while(now < end);
    
    u64 tscEnd = __rdtsc();

    profile.period = (now - start)/CAST(f64, tscEnd - tscStart);
    initProfileDone = true;
    return true;
}

#if PROFILE

ProfileEntry * createProfileEntry(const char * name){
    ProfileEntry * result = &profile.slots[profile.slotsUsed];
    memset(CAST(void *, result), 0, sizeof(ProfileEntry));
    // NOTE(fidli): this mush be compile known or persistent string, which is the usual case
    profile.names[profile.slotsUsed] = name;
    profile.slotsUsed++;
    return result;
}

struct ProfileDefer{
    ProfileEntry * entry_;
    u64 lastStartTime;
    u64 lastInclusiveTime;
    ProfileDefer * parentCaller;

    ProfileDefer(ProfileEntry * entry, u64 size = 0){
        entry_ = entry;
        parentCaller = profile.lastCaller;
        lastInclusiveTime = entry->timeSpentInclusive;
        profile.lastCaller = this;
        lastStartTime = __rdtsc();
        entry->bytesProcessed += size;
    }
    ~ProfileDefer(){
        u64 spentTime = __rdtsc() - lastStartTime;

        entry_->timeSpentExclusive += spentTime; 
        entry_->timeSpentInclusive = lastInclusiveTime + spentTime;
        entry_->callCountTotal++; 

        if (parentCaller != NULL)
        {
            parentCaller->entry_->timeSpentExclusive -= spentTime;
        }
        profile.lastCaller = parentCaller;
    }
};

#define PROFILE_SCOPE(name, ...) \
static ProfileEntry * CONCAT(profile_, __LINE__) = createProfileEntry(name); \
ProfileDefer CONCAT(AutoProfileDeferFromLine, __LINE__)(CONCAT(profile_, __LINE__), __VA_ARGS__);

#define PROFILE_FUNC(...) PROFILE_SCOPE(__func__, __VA_ARGS__)

#define PROFILE_BYTES(bytes) ASSERT(profile.lastCaller != NULL); profile.lastCaller->entry_->bytesProcessed += bytes;
#else
#define PROFILE_SCOPE(name) 
#define PROFILE_FUNC(...)
#define PROFILE_BYTES(bytes)
#endif

void profileBegin(){
    memset(CAST(void *, profile.slots), 0, sizeof(Profile::slots));
    profile.startTime = __rdtsc();
}

void profileEnd(){
    profile.endTime = __rdtsc();

}

void printCurrentProfileStats(){
    ASSERT(initProfileDone);
#if PROFILE
    int namewidth = 30; // per title column
    printf("Profile stats:\n");
    printf("| %*s | %10s | %20s | %10s | %10s | %14s | %12s | %5s |\n", namewidth, "name", "total [s]", "excl [s]", "thr [MB/s]", "avg [s]", "avg thr [MB/s]", "avg excl [s]", "calls");
    f64 totalTimeProfile = CAST(f64, profile.endTime - profile.startTime) * profile.period;
    for(i32 i = 0; i < profile.slotsUsed; i++){
        ProfileEntry * entry = &profile.slots[i];
        u64 totalCount = entry->callCountTotal;
        f64 totalTimeIncl = CAST(f64, entry->timeSpentInclusive)*profile.period;
        f64 totalExclTime = CAST(f64, entry->timeSpentExclusive)*profile.period;
        f64 perc = 100.0*totalExclTime / totalTimeProfile;
        f64 avgTime = totalTimeIncl/entry->callCountTotal;
        f64 avgExclTime = totalExclTime/entry->callCountTotal;
        f64 thr = (CAST(f64, entry->bytesProcessed)/CAST(f64, MEGABYTE(1))) / totalTimeIncl;
        f64 avgThr = thr / totalCount;
        const char * name = profile.names[i];
        printf("| %*s | %10f | %10f (%6.2f%%) | %10.3f | %10f | %14.3f | %12f | %5llu |\n", namewidth, name, totalTimeIncl, totalExclTime, perc, thr, avgTime, avgThr, avgExclTime, totalCount);
    }
    printf("| %*s | %10s | %10f (100.00%%) | %10s | %10s | %14s | %12s | %5s |\n", namewidth, "", "", totalTimeProfile, "", "", "", "", "");
#endif
}
