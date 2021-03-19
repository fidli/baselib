#pragma once
#include "util_time.h"
#include "util_sort.cpp"

struct ProfileEntry{
    u64 callCountTotal;
    f64 timeSpentTotal;
    
    f64 lastStartTime;
};

struct Profile{
    ProfileEntry slots[100];
    char names[100][30];
    i32 slotsUsed;
};

Profile * profile;

bool initProfile(){
    profile = &PPUSH(Profile);
    profile->slotsUsed = 0;
    return true;
}

ProfileEntry * createProfileEntry(const char * name){
    ProfileEntry * result = &profile->slots[profile->slotsUsed];
    memset(CAST(void *, result), 0, sizeof(ProfileEntry));
    // NOTE(fidli): this mush be compile known or persistent string, which is the usual case
    strncpy(profile->names[profile->slotsUsed], name, ARRAYSIZE(profile->names[0]));
    profile->slotsUsed++;
    return result;
}

void profileStart(ProfileEntry * target){
    target->lastStartTime = getProcessCurrentTime();
}

void profileEnd(ProfileEntry * target){
    target->timeSpentTotal += getProcessCurrentTime() - target->lastStartTime;
    target->callCountTotal++; 
}

#define PROFILE_START(name) \
static ProfileEntry * profile_##name = createProfileEntry(#name); \
profileStart(profile_##name);

#define PROFILE_END(name) \
profileEnd(profile_##name);

struct ProfileDefer{
    ProfileEntry * entry_;
    ProfileDefer(ProfileEntry * entry){
        entry_ = entry;
        profileStart(entry_);
    }
    ~ProfileDefer(){
        profileEnd(entry_);
    }
};

#define PROFILE_SCOPE(name) \
static ProfileEntry * profile_##name = createProfileEntry(#name); \
ProfileDefer CONCAT(AutoProfileDeferFromLine, __LINE__)(profile_##name);

void profileClearStats(){
    memset(CAST(void *, profile->slots), 0, sizeof(ProfileEntry)*profile->slotsUsed);
}

struct ProfileStats{
    struct Entry{
        u64 totalCount;
        f64 totalTime;
        f64 avgTime;
        const char * name;
    } entries[ARRAYSIZE(Profile::slots)];
    i32 count;
};

inline ProfileStats * getCurrentProfileStats(){
    ProfileStats * r = &PUSH(ProfileStats);
    i32 count = 0;
    
    for(i32 i = 0; i < profile->slotsUsed; i++){
        ProfileEntry * entry = &profile->slots[i];
        if(entry->callCountTotal){
            ProfileStats::Entry * stat = &r->entries[count];
            stat->totalCount = entry->callCountTotal;
            stat->totalTime = entry->timeSpentTotal;
            stat->avgTime = entry->timeSpentTotal/entry->callCountTotal;
            stat->name = profile->names[i];
            count++;
        }
    }
    r->count = count;
    mergeSort(&r->entries[0], r->count, [] (ProfileStats::Entry & a, ProfileStats::Entry & b) -> i32{f64 diff = a.avgTime - b.avgTime;if(diff < 0) return 1; return -1;});
    return r;
}
