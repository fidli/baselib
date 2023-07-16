#pragma once
#include "util_time.h"
#include "util_sort.cpp"

int printf(const char * format, ...);

struct ProfileEntry{
    u64 callCountTotal;
    f64 timeSpentTotal;
    f64 timeSpentChildren;
    
    f64 lastStartTime;
    ProfileEntry * parentCaller;
};

struct Profile{
    ProfileEntry slots[100];
    char names[100][30];
    i32 slotsUsed;

    ProfileEntry * lastCaller;
};

Profile profile;

ProfileEntry * createProfileEntry(const char * name){
    ProfileEntry * result = &profile.slots[profile.slotsUsed];
    memset(CAST(void *, result), 0, sizeof(ProfileEntry));
    // NOTE(fidli): this mush be compile known or persistent string, which is the usual case
    strncpy(profile.names[profile.slotsUsed], name, ARRAYSIZE(profile.names[0]));
    profile.slotsUsed++;
    return result;
}

void profileStart(ProfileEntry * target){
    target->lastStartTime = getProcessCurrentTime();
    target->parentCaller = profile.lastCaller;
    profile.lastCaller = target;
}

void profileEnd(){
    ProfileEntry * target = profile.lastCaller;
    ASSERT(target != NULL);
    f64 spentTime = getProcessCurrentTime() - target->lastStartTime;
    target->timeSpentTotal += spentTime; 
    target->callCountTotal++; 
    profile.lastCaller = target->parentCaller;
    if (target->parentCaller != NULL)
    {
        target->parentCaller->timeSpentChildren += spentTime;
    }

}

#define PROFILE_START(name) \
{\
    static ProfileEntry * profile = createProfileEntry(name); \
    profileStart(profile);  \
}

#define PROFILE_END() profileEnd()

struct ProfileDefer{
    ProfileEntry * entry_;
    ProfileDefer(ProfileEntry * entry){
        entry_ = entry;
        profileStart(entry_);
    }
    ~ProfileDefer(){
        profileEnd();
    }
};

#define PROFILE_SCOPE(name) \
static ProfileEntry * CONCAT(profile_, __LINE__) = createProfileEntry(name); \
ProfileDefer CONCAT(AutoProfileDeferFromLine, __LINE__)(CONCAT(profile_, __LINE__));

#define PROFILE_FUNC PROFILE_SCOPE(__func__)

void profileClearStats(){
    memset(CAST(void *, profile.slots), 0, sizeof(ProfileEntry)*profile.slotsUsed);
}

struct ProfileStats{
    struct Entry{
        u64 totalCount;
        f64 totalTime;
        f64 totalExclTime;
        f64 avgTime;
        f64 avgExclTime;
        const char * name;
    } entries[ARRAYSIZE(Profile::slots)];
    i32 count;
};

inline ProfileStats * getCurrentProfileStats(){
    ProfileStats * r = &PUSH(ProfileStats);
    i32 count = 0;
    
    for(i32 i = 0; i < profile.slotsUsed; i++){
        ProfileEntry * entry = &profile.slots[i];
        if(entry->callCountTotal){
            ProfileStats::Entry * stat = &r->entries[count];
            stat->totalCount = entry->callCountTotal;
            stat->totalTime = entry->timeSpentTotal;
            stat->totalExclTime = entry->timeSpentTotal - entry->timeSpentChildren;
            stat->avgTime = entry->timeSpentTotal/entry->callCountTotal;
            stat->avgExclTime = stat->totalExclTime/entry->callCountTotal;
            stat->name = profile.names[i];
            count++;
        }
    }
    r->count = count;
    return r;
}

void printCurrentProfileStats(){
    ProfileStats * stats = getCurrentProfileStats();
    int namewidth = 0;
    for(i32 i = 0; i < stats->count; i++){
        int s = CAST(int, strlen(stats->entries[i].name));
        if (s > namewidth){
            namewidth = s;
        }
    }
    printf("Profile stats:\n");
    printf("| %*s | %20s | %20s | %20s | %20s | %5s |\n", namewidth, "name", "total time [s]", "excl time [s]", "avg time [s]", "avg excl time [s]", "calls");
    for(i32 i = 0; i < stats->count; i++){
        printf("| %*s | %20f | %20f | %20f | %20f | %5llu |%c", namewidth, stats->entries[i].name, stats->entries[i].totalTime, stats->entries[i].totalExclTime, stats->entries[i].avgTime, stats->entries[i].avgExclTime, stats->entries[i].totalCount, i == stats->count - 1 ? '\0' : '\n');
    }
}
