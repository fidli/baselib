#ifndef UTIL_RNG_C
#define UTIL_RNG_C

#include "util_time.h";

static struct Lcg{
    uint32 state;
};

static Lcg lcgen;

bool initRng(){
    LocalTime now = getLocalTime();
    lcgen.state = - (now.second * now.minute * now.hour * (1 + getProcessCurrentTime()));
    return true;
}

uint16 randlcg(){
    lcgen.state  = 1103515245 * lcgen.state + 12345;
    lcgen.state &= 0xEFFFFFFF; //modulo 31 bits
    return (uint16) (lcgen.state >> 15);
}

uint32 randlcgd(){
    uint32 a = randlcg();
    a = a << 16;
    a = a | randlcg();
    return a;
}

#endif