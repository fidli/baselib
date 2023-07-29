#pragma once
#include "util_time.h"
#include "util_sort.cpp"

#ifndef PROFILE
#define PROFILE 0
#endif

int printf(const char * format, ...);
#if PROFILE

struct ProfileEntry{
    u64 callCountTotal;
    f64 timeSpentTotal;
    f64 timeSpentChildren;
    
    f64 lastStartTime;
    ProfileEntry * parentCaller;
};
#endif

struct Profile{
#if PROFILE
    ProfileEntry slots[100];
    const char * names[100];
    i32 slotsUsed;

    ProfileEntry * lastCaller;
#endif
    f64 startTime;
    f64 endTime;
};

Profile profile;

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
    ProfileDefer(ProfileEntry * entry){
        entry_ = entry;
        entry->lastStartTime = getProcessCurrentTime();
        entry->parentCaller = profile.lastCaller;
        profile.lastCaller = entry;
    }
    ~ProfileDefer(){
        f64 spentTime = getProcessCurrentTime() - entry_->lastStartTime;
        entry_->timeSpentTotal += spentTime; 
        entry_->callCountTotal++; 
        profile.lastCaller = entry_->parentCaller;
        if (entry_->parentCaller != NULL)
        {
            entry_->parentCaller->timeSpentChildren += spentTime;
        }
    }
};

#define PROFILE_SCOPE(name) \
static ProfileEntry * CONCAT(profile_, __LINE__) = createProfileEntry(name); \
ProfileDefer CONCAT(AutoProfileDeferFromLine, __LINE__)(CONCAT(profile_, __LINE__));

#define PROFILE_FUNC PROFILE_SCOPE(__func__)

#else
#define PROFILE_SCOPE(name) 
#define PROFILE_FUNC
#endif

void profileBegin(){
    memset(CAST(void *, profile.slots), 0, sizeof(Profile::slots));
    profile.startTime = getProcessCurrentTime();
}

void profileEnd(){
    profile.endTime = getProcessCurrentTime();

}

void printCurrentProfileStats(){
    int namewidth = 4; // per title column
#if PROFILE
    namewidth = 30;
#endif
    printf("Profile stats:\n");
    printf("| %*s | %20s | %20s | %20s | %20s | %5s |\n", namewidth, "name", "total time [s]", "excl time [s]", "avg time [s]", "avg excl time [s]", "calls");
    f64 totalTimeProfile = profile.endTime - profile.startTime;
#if PROFILE
    for(i32 i = 0; i < profile.slotsUsed; i++){
        ProfileEntry * entry = &profile.slots[i];
        u64 totalCount = entry->callCountTotal;
        f64 totalTime = entry->timeSpentTotal;
        f64 totalExclTime = entry->timeSpentTotal - entry->timeSpentChildren;
        f64 perc = 100.0*totalExclTime / totalTimeProfile;
        f64 avgTime = entry->timeSpentTotal/entry->callCountTotal;
        f64 avgExclTime = totalExclTime/entry->callCountTotal;
        const char * name = profile.names[i];
        printf("| %*s | %20f | %10f (%6.2f%%) | %20f | %20f | %5llu |\n", namewidth, name, totalTime, totalExclTime, perc, avgTime, avgExclTime, totalCount);
    }
#endif
    printf("| %*s | %20s | %10f (100.00%%) | %20s | %20s | %5s |\n", namewidth, "", "", totalTimeProfile, "", "", "");
}
