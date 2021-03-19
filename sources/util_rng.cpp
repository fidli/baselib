#ifndef UTIL_RNG_C
#define UTIL_RNG_C

#include "util_time.h"

struct Lcg{
    u32 state;
};

static Lcg lcgen;

bool initRng(){
    Lcg * gen = &lcgen;
    LocalTime now = getLocalTime();
    gen->state = (u32)-(now.second * now.minute * now.hour * (1 + getProcessCurrentTime()));
    return true;
}

bool initRng(Lcg * gen){
    LocalTime now = getLocalTime();
    gen->state = (u32)-(now.second * now.minute * now.hour * (1 + getProcessCurrentTime()));
    return true;
}



uint16 randlcg(Lcg * gen){
    gen->state  = 1103515245 * gen->state + 12345;
    gen->state &= 0xEFFFFFFF; //modulo 31 bits
    return (u16) (gen->state >> 15);
}

uint16 randlcg(){
    Lcg * gen = &lcgen;
    gen->state  = 1103515245 * gen->state + 12345;
    gen->state &= 0xEFFFFFFF; //modulo 31 bits
    return (u16) (gen->state >> 15);
}

uint32 randlcgd(){
    u32 a = randlcg();
    a = a << 16;
    a = a | randlcg();
    return a;
}

uint16 randlcgRange(u16 spanSize){
    return (u16)(((f32) randlcg() / (u16) -1) * spanSize);
}

#endif