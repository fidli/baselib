#pragma once
#include "util_time.h"
#include "util_sort.cpp"

struct ProfileEntry{
    uint64 callCountTotal;
    float64 timeSpentTotal;
    
    float64 lastStartTime;
};

struct Profile{
    ProfileEntry slots[100];
    const char * names[100];
    int32 slotsUsed;
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
    profile->names[profile->slotsUsed] = name;
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
        const char * name;
        float64 avgTime;
        uint64 totalCount;
        float64 totalTime;
    } entries[ARRAYSIZE(Profile::slots)];
    int32 count;
};

ProfileStats * getCurrentProfileStats(){
    ProfileStats * r = &PUSH(ProfileStats);
    r->count = 0;
    for(int32 i = 0; i < profile->slotsUsed; i++){
        ProfileEntry * entry = &profile->slots[i];
        ProfileStats::Entry * stat = &r->entries[i];
        stat->name = profile->names[i];
        stat->totalCount = entry->callCountTotal;
        stat->totalTime = entry->timeSpentTotal;
        if(stat->totalCount){
            stat->avgTime = stat->totalTime/stat->totalCount;
        }else{
            stat->avgTime = 0;
        }
    }
    r->count = profile->slotsUsed;
    insertSort(r->entries, r->count, [] (ProfileStats::Entry & a, ProfileStats::Entry & b) -> int32{float64 diff = a.avgTime - b.avgTime;if(diff > 0) return -1; return 0;});
    return r;
}
