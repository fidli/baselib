#pragma once
#include "util_time.h"

struct ProfileEntry{
    uint64 callCountTotal;
    float64 timeSpentTotal;
    
    float64 lastStartTime;

    const char * name;
    ProfileEntry * parent;
};

struct Profile{
    ProfileEntry slots[100];
    int32 slotsUsed;

    ProfileEntry * currentEntry;
};

Profile * profile;

bool initProfile(){
    profile = &PPUSH(Profile);
    profile->slotsUsed = 0;
    profile->currentEntry = NULL;
    return true;
}

ProfileEntry * createProfileEntry(const char * name){
    ProfileEntry * result = &profile->slots[profile->slotsUsed++];
    memset(CAST(void *, result), 0, sizeof(ProfileEntry));
    // NOTE(fidli): this mush be compile known or persistent string, which is the usual case
    result->name = name;
    return result;
}

void profileStart(ProfileEntry * target){
    target->lastStartTime = getProcessCurrentTime();
    target->parent = profile->currentEntry;
    profile->currentEntry = target;
}

void profileEnd(ProfileEntry * target){
    target->timeSpentTotal += getProcessCurrentTime() - target->lastStartTime;
    target->callCountTotal++; 
    profile->currentEntry = target->parent;
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

struct ProfileStats{
    struct Entry{
        const char * name;
        int32 level;
        float32 percOfParent;
        float64 avgTime;
        uint64 totalCount;
        float64 totalTime;
    } entries[ARRAYSIZE(Profile::slots)];
    int32 count;
    int32 maxLevel;
};

void constructProfileStatsTreeRec(ProfileEntry * parent, ProfileStats * stats, int32 level){
    for(int32 i = 0; i < profile->slotsUsed; i++){
        ProfileEntry * entry = &profile->slots[i];
        if(entry->parent == parent){
            ProfileStats::Entry * stat = &stats->entries[stats->count];
            stat->name = entry->name;
            stat->level = level;
            stat->totalTime = entry->timeSpentTotal;
            stat->totalCount = entry->callCountTotal;
            stat->avgTime = stat->totalTime/stat->totalCount;
            if(parent && parent->timeSpentTotal > 0.0001){
                stat->percOfParent = entry->timeSpentTotal/parent->timeSpentTotal * 100;
            }else{
                stat->percOfParent = 0;
            }
            stats->count++;
            if(level > stats->maxLevel){
                stats->maxLevel = level;
            }
            constructProfileStatsTreeRec(entry, stats, level+1);
        }
    }
}

ProfileStats * getCurrentProfileStats(){
    ProfileStats * r = &PUSH(ProfileStats);
    r->count = 0;
    r->maxLevel = 0;
    constructProfileStatsTreeRec(NULL, r, 0);
    ASSERT(r->count == profile->slotsUsed);
    return r;
}
