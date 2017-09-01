#ifndef UTIL_RNG_C
#define UTIL_RNG_C

#include "util_time.h"

struct Lcg{
    uint32 state;
};

static Lcg lcgen;

bool initRng(){
    Lcg * gen = &lcgen;
    LocalTime now = getLocalTime();
    gen->state = (uint32)-(now.second * now.minute * now.hour * (1 + getProcessCurrentTime()));
    return true;
}

bool initRng(Lcg * gen){
    LocalTime now = getLocalTime();
    gen->state = (uint32)-(now.second * now.minute * now.hour * (1 + getProcessCurrentTime()));
    return true;
}

uint16 randlcg(Lcg * gen){
    gen->state  = 1103515245 * gen->state + 12345;
    gen->state &= 0xEFFFFFFF; //modulo 31 bits
    return (uint16) (gen->state >> 15);
}

uint16 randlcg(){
    Lcg * gen = &lcgen;
    gen->state  = 1103515245 * gen->state + 12345;
    gen->state &= 0xEFFFFFFF; //modulo 31 bits
    return (uint16) (gen->state >> 15);
}

uint32 randlcgd(){
    uint32 a = randlcg();
    a = a << 16;
    a = a | randlcg();
    return a;
}

#endif