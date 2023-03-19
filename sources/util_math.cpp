#pragma once

#ifdef PRECISE_MATH
#include <cmath>
#include "math.h"
#endif

#include "common.h"
#include "math_macros.h"

//-----------------------------------------------------------------------NUMBERS

b32 isOdd(const u64 a){
    return (a & (u64) 1);
}

bool aseq(f32 a, f32 b, f32 delta = 0.000001f){
    return ABS(a-b) <= delta;
}

bool aseq64(f64 a, f64 b, f64 delta = 0.000001f){
    return ABS(a-b) <= delta;
}


//is test on the right side of fixedpoint ? is test > fixedpoint?
bool aseql(f32 test, f32 fixedpoint, f32 delta = 0.000005f){
    f32 eps = fixedpoint - test;
    return eps < delta;
}

//is test on the right side of fixedpoint ? is test > fixedpoint?
bool aseql64(f64 test, f64 fixedpoint, f64 delta = 0.000005f){
    f64 eps = fixedpoint - test;
    return eps < delta;
}


//is test on the left side of fixedpoint ? is test < fixedpoint?
bool aseqr(f32 test, f32 fixedpoint, f32 delta = 0.000005f){
    f32 eps = test - fixedpoint;
    return eps < delta;
}

//is test on the left side of fixedpoint ? is test < fixedpoint?
bool aseqr64(f64 test, f64 fixedpoint, f64 delta = 0.000005f){
    f64 eps = test - fixedpoint;
    return eps < delta;
}


#ifndef PRECISE_MATH

f32 ceil(f32 value){
    i64 i = CAST(i64, value);
    f32 result = CAST(f32, i);
    if(result != value && value >= 0){
        result++;
    }
    return result;
}

f32 floor(f32 value){
    i64 i = CAST(i64, value);
    f32 result = CAST(f32, i);
    if(result != value && value <= 0){
        result--;
    }
    return result;
    
}

f32 round(f32 value){
    if((u32)(value * 10) % 10 >= 5){
        return ceil(value);
    }else{
        return floor(value);
    }
}

f32 powd(f32 base, i16 power = 2){
    //todo square mult
    if(power == 0){
        return 1;
    }
    f32 result = base;
    for(u16 i = 1; i < ABS(power); i++){
        result *= base;
    }
    return (power < 0) ? (1.0f / result) : result;
}

f64 powd64(f64 base, i16 power = 2){
    //todo square mult
    if(power == 0){
        return 1;
    }
    f64 result = base;
    for(u16 i = 1; i < ABS(power); i++){
        result *= base;
    }
    return (power < 0) ? (1.0f / result) : result;
}

static f32 subSqrt(f32 value, f32 guess, f32 prec = 0.0001f){
    if(guess != guess){
        //nan
        return guess;
    }
    f32 estimation = value / guess;
    if(aseq(estimation, guess, prec)){
        return (estimation + guess) / 2;
    }
    f32 newGuess = (guess + value/guess) / 2;
    //NOTE(AK): stack overflow with too much recursive calls, this cuts off the tail when then solution is close enough
    if(aseq(guess, newGuess, prec)){
        return (guess + newGuess) / 2;
    }
    return subSqrt(value, newGuess, prec);
}

static f64 subSqrt64(f64 value, f64 guess, f64 prec = 0.0001f){
    if(guess != guess){
        //nan
        return guess;
    }
    f64 estimation = value / guess;
    if(aseq64(estimation, guess, prec)){
        return (estimation + guess) / 2;
    }
    f64 newGuess = (guess + value/guess) / 2;
    //NOTE(AK): stack overflow with too much recursive calls, this cuts off the tail when then solution is close enough
    if(aseq64(guess, newGuess, prec)){
        return (guess + newGuess) / 2;
    }
    return subSqrt64(value, newGuess, prec);
}



f32 sqrt(f32 value){
    if(aseq(value, 0)) return 0;
    if(value < 0) INV;
    return subSqrt(value, value / 2, value < 1 ? 0.000001f : 0.01f);
}

f64 sqrt64(f64 value){
    if(aseq64(value, 0)) return 0;
    if(value < 0) INV;
    return subSqrt64(value, value / 2, value < 1 ? 0.000001f : 0.01f);
}

/* e N
f32 sqrt(f32 value){
    ASSERT(value > 0);
    f32 result = 65536;
    while(true){
        f32 candidate = floor((result + floor(value/result))/2);
        if(candidate >= result){
            return candidate;
        }
        result = candidate;
    }
    
    return result;
}
*/


f32 atan2(f32, f32){
    ASSERT(!"implement me");
    return 0;
}

f32 sin(f32 xRad){
    f32 result = CAST(f32, xRad - powd(xRad,3)/6.0 + powd(xRad,5)/120.0 - powd(xRad,7)/5040.0 +  powd(xRad,9)/362880.0 - powd(xRad,11)/39916800.0 + powd(xRad,13)/6227020800.0 - powd(xRad,15)/1307674368000.0 +  powd(xRad,17)/355687428096000.0 - powd(xRad,19)/1.216451e+17 + powd(xRad,21)/5.1090942e+19);
    ASSERT(xRad <= 2*PI && xRad >= -2*PI);
    return result;
}

f32 sin64(f64 xRad){
    ASSERT(xRad <= 2*PI && xRad >= 0);
    return CAST(f32, xRad - powd64(xRad,3)/6.0 + powd64(xRad,5)/120.0 - powd64(xRad,7)/5040.0 +  powd64(xRad,9)/362880.0 - powd64(xRad,11)/39916800.0 + powd64(xRad,13)/6227020800.0 - powd64(xRad,15)/1307674368000.0 +  powd64(xRad,17)/355687428096000.0 - powd64(xRad,19)/1.216451e+17 + powd64(xRad,21)/5.1090942e+19);
}

f32 cos(f32 xRad){
    f32 result = CAST(f32, 1 - powd(xRad,2)/2.0 + powd(xRad,4)/24.0 - powd(xRad,6)/720.0 + powd(xRad,8)/40320.0 - powd(xRad,10)/3628800.0 + powd(xRad,12)/479001600.0 - powd(xRad,14)/87178291200.0 + powd(xRad,16)/20922789888000.0 - powd(xRad,18)/6.402373705728e+15 + powd(xRad,20)/2.43290200817664e+18);
    ASSERT(xRad <= 2*PI && xRad >= -2*PI);
    return result;
}

f64 cos64(f64 xRad){
    ASSERT(xRad <= 2*PI && xRad >= 0);
    return 1 - powd64(xRad,2)/2.0 + powd64(xRad,4)/24.0 - powd64(xRad,6)/720.0 + powd64(xRad,8)/40320.0 - powd64(xRad,10)/3628800.0 + powd64(xRad,12)/479001600.0 - powd64(xRad,14)/87178291200.0 + powd64(xRad,16)/20922789888000.0 - powd64(xRad,18)/6.402373705728e+15 + powd64(xRad,20)/2.43290200817664e+18;
}

f32 tan(f32 rad){
	ASSERT(!"something better");
	return sin(rad) / cos(rad);
}


f32 acos(f32 cos){
    //https://stackoverflow.com/questions/3380628/fast-arc-cos-algorithm/36387954#36387954
    f32 a = -0.939115566365855f;
    f32 b =  0.9217841528914573f;
    f32 c = -1.2845906244690837f;
    f32 d =  0.295624144969963174f;
    return PI/2 + (a*cos + b * powd(cos,3)) / (1 + c*powd(cos,2) + d*powd(cos, 4));
    
}

f64 acos64(f64 cos){
    //https://stackoverflow.com/questions/3380628/fast-arc-cos-algorithm/36387954#36387954
    f64 a = -0.939115566365855f;
    f64 b =  0.9217841528914573f;
    f64 c = -1.2845906244690837f;
    f64 d =  0.295624144969963174f;
    return PI_64/2 + (a*cos + b * powd64(cos,3)) / (1 + c*powd64(cos,2) + d*powd64(cos, 4));
    
}




static f32 agMean(f32 a, f32 b){
    if(aseq(a, b, 0.0000001f)) return (b + a) / 2.0f;
    return agMean((a+b)/2, sqrt(a*b));
}

f32 ln(f32 number, u8 precisionBits = 32){
    //Arithmetic-geometric mean approximation
    ASSERT(precisionBits > 0);
    const f32 ln2 = 0.6931471f;
    const u8 m = 18;
    
    f32 meanB = powd(2, 2-m) / number;
    
    return (PI / (2*agMean(1, meanB))) - m * ln2;
}

f32 epow(f32 power){
    if(aseq(power, 0, 0.000000005f)) return 1;
    //taylor
    f32 abspow = ABS(power) - (u32) ABS(power);
    f32 result = 1 + abspow + powd(abspow, 2)/2.0f + powd(abspow, 3)/6.0f + powd(abspow, 4)/24.0f + powd(abspow, 5)/120.0f  + powd(abspow, 6)/720.0f + powd(abspow, 7)/5040.0f + powd(abspow, 8)/40320.0f + powd(abspow, 9)/362880.0f + powd(abspow, 10)/3628800.0f + powd(abspow, 11)/39916800.0f + powd(abspow, 12)/479001600.0f +powd(abspow, 13)/6881080200.0f + powd(abspow, 14)/87178291200.0f;
    result *= powd(CAST(f32, E), (u16) ABS(power));
    return (power < 0) ? (1.0f / result) : result;
}

f32 pow(f32 base, f32 power){
    return epow(power * ln(base));
}


f32 log(f32 number, f32 base = 10){
    ASSERT(number != 0);
    f32 result = ln(number) / ln(base);
    return (number < 0) ? -result : result;
    
}


f32 clamp(f32 originalValue, f32 min, f32 max){
    if(originalValue < min){
        return min;
    }else if(originalValue > max){
        return max;
    }
    return originalValue;
}


i32 clamp(i32 originalValue, i32 min, i32 max){
    if(originalValue < min){
        return min;
    }else if(originalValue > max){
        return max;
    }
    return originalValue;
}


f32 normalize(f32 value, f32 min, f32 max){
    return (value - min) / (max - min);
}

f32 fmodd(f32 value, u32 modulus){
    i32 wholePart = (i32) value;
    f32 preResult = (f32)(wholePart % modulus) + (value - wholePart);
    return preResult; 
}

f64 fmodd64(f64 value, u64 modulus){
    i64 wholePart = (i64) value;
    f64 preResult = (f64)(wholePart % modulus) + (value - wholePart);
    return preResult; 
}

f32 fmod(f32 value, f32 modulus){
    f32 divided = value / modulus;
    f32 remain = divided - ((f32)(i32)divided);
    return modulus*remain;
}

f64 fmod64(f64 value, f64 modulus){
    f64 divided = value / modulus;
    f64 remain = divided - ((f64)(i64)divided);
    return modulus*remain;
}
#else


//this is just interface

f32 powd(f32 base, i16 power = 2){
    return (f32)pow((double) base, (double) power);
}

f64 powd64(f64 base, i16 power = 2){
    return (f64)pow(base, (double) power);
}

f64 sqrt64(f64 value){
    return (f64)sqrt((double) value);
}

f32 fmodd(f32 value, u32 modulus){
    f32 result = (f32) fmod((double) value, (double) modulus);
    return result;
}

f64 fmodd64(f64 value, u64 modulus){
    return result;
}

f64 acos64(f64 cos64){
    return (f64)acos((double)cos64);
}

f64 cos64(f64 radAngle){
    return (f64)cos((double)radAngle);
}

f64 sin64(f64 radAngle){
    return (f64)sin((double)radAngle);
}



#endif

u8 numlen(i64 number){
    u8 result = 1;
    if(number < 0){
        result++;
    }
    number = ABS(number);
    while(number > 9){
        number /= 10;
        result++;
    }
    return result;
}


//-----------------------------------------------------------------------VERTICES

struct vN{
    f32 * v;
    u8 size;
};

union dv4{
    struct{
        i32 x;
        i32 y;
        i32 z;
        i32 w;
    };
    i32 v[4];
};

union v2{
    struct{
        f32 x;
        f32 y;
    };
    struct{
        f32 a;
        f32 b;
    };
    struct{
        f32 pitch;
        f32 yaw;
    };
    f32 v[2];
};

union v2_64{
    struct{
        f64 x;
        f64 y;
    };
    struct{
        f64 a;
        f64 b;
    };
    struct{
        f64 pitch;
        f64 yaw;
    };
    f64 v[2];
};


union dv2{
    struct{
        i32 x;
        i32 y;
    };
    struct{
        i32 pitch;
        i32 yaw;
    };
    i32 v[2];
};

union v3{
    struct{
        f32 x;
        f32 y;
        f32 z;
    };
    struct{
        f32 r;
        f32 g;
        f32 b;
    };
    struct{
        f32 right;
        f32 forward;
        f32 upward;
    };
    f32 v[3];
};

union v3_64{
    struct{
        f64 x;
        f64 y;
        f64 z;
    };
    struct{
        f64 r;
        f64 g;
        f64 b;
    };
    struct{
        f64 right;
        f64 forward;
        f64 upward;
    };
    f64 v[3];
};

union dv3{
    struct{
        i32 x;
        i32 y;
        i32 z;
    };
    struct{
        i32 r;
        i32 g;
        i32 b;
    };
    struct{
        i32 right;
        i32 forward;
        i32 upward;
    };
    i32 v[3];
};

union dv3_64{
    struct{
        i64 x;
        i64 y;
        i64 z;
    };
    struct{
        i64 r;
        i64 g;
        i64 b;
    };
    struct{
        i64 right;
        i64 forward;
        i64 upward;
    };
    i64 v[3];
};

union v4{
    struct{
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    struct{
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
    f32 v[4];
};



union v4_64{
    struct{
        f64 x;
        f64 y;
        f64 z;
        f64 w;
    };
    struct{
        f64 r;
        f64 g;
        f64 b;
        f64 a;
    };
    f64 v[4];
};


v3 V3(f32 x, f32 y, f32 z){
    v3 res = {x,y,z};
    return res;
}

v3_64 V3_64(f64 x, f64 y, f64 z){
    v3_64 res = {x,y,z};
    return res;
}

dv3_64 DV3_64(i64 x, i64 y, i64 z){
    dv3_64 res = {x,y,z};
    return res;
}

v2 V2(const dv2 & v){
    v2 res = {CAST(f32, v.x), CAST(f32, v.y)};
    return res;
}

v2 V2(f32 x, f32 y){
    v2 res = {x,y};
    return res;
}

v2_64 V2_64(f64 x, f64 y){
    v2_64 res = {x,y};
    return res;
}

v2 operator/=(v2 & a, f32 b){
    for(int i = 0; i < ARRAYSIZE(a.v); i++){
        a.v[i] /= b;
    }
    return a;
}

v2 operator-=(v2 & a, v2 b){
    for(int i = 0; i < ARRAYSIZE(a.v); i++){
        a.v[i] -= b.v[i];
    }
    return a;
}

dv2 DV2(i32 x, i32 y){
    dv2 res = {x, y};
    return res;
}

v4 V4(f32 x, f32 y, f32 z, f32 w){
    v4 result = {x, y, z, w};
    return result;
}

v4_64 V4_64(f64 x, f64 y, f64 z, f64 w){
    v4_64 result = {x, y, z, w};
    return result;
}

v3 operator+(const v3 & a, const v3 & b){
    v3 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] + b.v[i];
    }
    return result;
}

v3 operator-(const v3 & a){
    v3 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = -a.v[i];
    }
    return result;
}

v3_64 operator+(const v3_64 & a, const v3_64 & b){
    v3_64 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] + b.v[i];
    }
    return result;
}

dv3_64 operator/=(dv3_64 & a, i64 b){
    for(int i = 0; i < ARRAYSIZE(a.v); i++){
        a.v[i] /= b;
    }
    return a;
}


v2 operator+(const v2 & a, const v2 & b){
    v2 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] + b.v[i];
    }
    return result;
}

dv2 operator+(const dv2 & a, const dv2 & b){
    dv2 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] + b.v[i];
    }
    return result;
}

v2 operator+(const v2 & a, const dv2 & b){
    v2 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] + b.v[i];
    }
    return result;
}

v2 operator+(const dv2 & a, const v2 & b){
    return b + a;
}

v2 operator-(const v2 & a, const v2 & b){
    v2 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] - b.v[i];
    }
    return result;
}

v2_64 operator-(const v2_64 & a, const v2_64 & b){
    v2_64 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] - b.v[i];
    }
    return result;
}

dv2 operator-(const dv2 & a, const dv2 & b){
    dv2 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] - b.v[i];
    }
    return result;
}



dv2 operator-(const dv2 & a){
    dv2 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = -a.v[i];
    }
    return result;
}

v2 operator-(const v2 & a){
    v2 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = -a.v[i];
    }
    return result;
}

v2 & operator+=(v2 & a, const v2 & b){
    for(int i = 0; i < ARRAYSIZE(b.v); i++){
        a.v[i] += b.v[i];
    }
    return a;
}

dv2 & operator+=(dv2 & a, const dv2 & b){
    for(int i = 0; i < ARRAYSIZE(b.v); i++){
        a.v[i] += b.v[i];
    }
    return a;
}

v3 operator-(const v3 & a, const v3 & b){
    v3 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] - b.v[i];
    }
    return result;
}

v3_64 operator-(const v3_64 & a, const v3_64 & b){
    v3_64 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] - b.v[i];
    }
    return result;
}

v3 & operator+=(v3 & a, const v3 & b){
    for(int i = 0; i < ARRAYSIZE(b.v); i++){
        a.v[i] += b.v[i];
    }
    return a;
}

v3_64 & operator+=(v3_64 & a, const v3_64 & b){
    for(int i = 0; i < ARRAYSIZE(b.v); i++){
        a.v[i] += b.v[i];
    }
    return a;
}


dv3 & operator+=(dv3 & a, const dv3 & b){
    for(int i = 0; i < ARRAYSIZE(b.v); i++){
        a.v[i] += b.v[i];
    }
    return a;
}

v2 & operator+=(v2 & a, const dv2 & b){
    for(int i = 0; i < ARRAYSIZE(b.v); i++){
        a.v[i] += CAST(f32, b.v[i]);
    }
    return a;
}

dv3_64 & operator+=(dv3_64 & a, const dv3_64 & b){
    for(int i = 0; i < ARRAYSIZE(b.v); i++){
        a.v[i] += b.v[i];
    }
    return a;
}

dv2 operator/(const dv2 & a, const i32 b){
    dv2 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] / b;
    }
    return result;
}

v2 operator/(const v2 & a, const f32 b){
    v2 result= {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] / b;
    }
    return result;
}

v3_64 operator/(const dv3_64 & a, const f64 b){
    v3_64 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = (f64) a.v[i] / b;
    }
    return result;
}

v3_64 operator*(const dv3_64 & a, const f64 b){
    v3_64 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] * b;
    }
    return result;
}

dv3_64 operator*(const dv3_64 & a, const i32 b){
    dv3_64 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] * b;
    }
    return result;
}

dv3_64 operator+(const dv3_64 & a, const dv3_64 & b){
    dv3_64 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] + b.v[i];
    }
    return result;
}

dv3_64 operator-(const dv3_64 & a, const dv3_64 & b){
    dv3_64 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] -  b.v[i];
    }
    return result;
}

v3 & operator-=(v3 & a, const v3 & b){
    for(int i = 0; i < ARRAYSIZE(b.v); i++){
        a.v[i] -= b.v[i];
    }
    return a;
}

v3_64 & operator-=(v3_64 & a, const v3_64 & b){
    for(int i = 0; i < ARRAYSIZE(b.v); i++){
        a.v[i] -= b.v[i];
    }
    return a;
}

dv2 & operator-=(dv2 & a, const dv2 & b){
    for(int i = 0; i < ARRAYSIZE(b.v); i++){
        a.v[i] -= b.v[i];
    }
    return a;
}

dv3_64 & operator-=(dv3_64 & a, const dv3_64 & b){
    for(int i = 0; i < ARRAYSIZE(b.v); i++){
        a.v[i] -= b.v[i];
    }
    return a;
}

bool operator==(const dv2 & a, const dv2 & b){
    return a.x == b.x && a.y == b.y;
}

bool operator!=(const dv2 & a, const dv2 & b){
    return !(a == b);
}

bool operator==(const v3 & a, const v3 & b){
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

v2 operator*(const v2 & b, const f32 a){
    v2 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a*b.v[i];
    }
    return result;
}

v2 operator*(const v2 & b, const i32 a){
    v2 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = ((f32)a)*b.v[i];
    }
    return result;
}

v2 operator*(const f32 a, const v2 & b){
    return b * a;
}

v2 operator*(const i32 a, const v2 & b){
    return b * a;
}


dv2 operator*(const dv2 & b, const i32 a){
    dv2 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a*b.v[i];
    }
    return result;
}

dv2 operator*(const i32 a, const dv2 & b){
    return b * a;
}

v2 operator*=(v2 & v, const f32 a){
    v.x *= a;
    v.y *= a;
    return v;
}

v2 operator*(const dv2 & b, const f32 a){
    v2 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a*(f32)b.v[i];
    }
    return result;
}

v2 operator*(const f32 a, const dv2 & b){
    return b * a;
}

v2 dv2Tov2(const dv2 & a){
    v2 result = {(f32)a.x, (f32)a.y};
    return result;
}

dv2 v2Todv2(const v2 & a){
    dv2 result = {(i32)a.x, (i32)a.y};
    return result;
}

v3 operator*(const v3 & b, const f32 a){
    v3 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a*b.v[i];
    }
    return result;
}

v3 operator*(const f32 a, const v3 & b){
    return b * a;
}

v3_64 operator*(const v3_64 & b, const f64 a){
    v3_64 result = {};
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a*b.v[i];
    }
    return result;
}

v3_64 operator*(const f64 a, const v3_64 & b){
    return b * a;
}


vN operator*(const vN & b, const f32 a){
    vN result = b;
    for(int i = 0; i < b.size; i++){
        result.v[i] = a*b.v[i];
    }
    return result;
}

vN operator*(const f32 a, const vN & b){
    return b * a;
}

f32 lerp(const f32 origin, const f32 target, f32 coeficient){
    return origin + (target-origin) * coeficient;
}

v3 lerp(const v3 * origin, const v3 * target, f32 coeficient){
    return *origin + (*target-*origin) * coeficient;
}

v2 lerp(const v2 * origin, const v2 * target, f32 coeficient){
    return *origin + (*target-*origin) * coeficient;
}

v2 lerp(const dv2 * origin, const dv2 * target, f32 coeficient){
    return *origin + (*target-*origin) * coeficient;
}


f32 dot(v3 a, v3 b){
    return a.x*b.x + a.y * b.y + a.z * b.z;
}

f64 dot64(v3_64 a, v3_64 b){
    return a.x*b.x + a.y * b.y + a.z * b.z;
}

f32 dot(v4 a, v4 b){
    return a.x*b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

f64 dot64(v4_64 a, v4_64 b){
    return a.x*b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

f32 dot(v2 a, v2 b){
    return a.x*b.x + a.y * b.y;
}

f64 dot64(v2_64 a, v2_64 b){
    return a.x*b.x + a.y * b.y;
}

u64 dot64(dv3_64 a, dv3_64 b){
    return a.x*b.x + a.y * b.y + b.z * b.z;
}

f32 dot(dv2 a, dv2 b){
    return (f32)(a.x*b.x + a.y * b.y);
}

f32 dot(vN a, vN b){
    ASSERT(a.size == b.size);
    f32 result = 0;
    for(int i = 0; i < a.size; i++){
        result += a.v[i] * b.v[i];
    }
    return result;
}


f32 det(v2 a, v2 b){
    return a.x*b.y - b.x*a.y;
}

f32 length(v3 a){
    return sqrt(dot(a,a));
}

f64 length64(v3_64 a){
    return sqrt64(dot64(a,a));
}

f64 length64(dv3_64 a){
    return sqrt64((f64)dot64(a,a));
}

f32 length(vN a){
    return sqrt(dot(a,a));
}

f32 length(v4 a){
    return sqrt(dot(a,a));
}

f64 length64(v4_64 a){
    return sqrt64(dot64(a,a));
}

f32 length(v2 a){
    return sqrt(dot(a,a));
}

f64 length64(v2_64 a){
    return sqrt64(dot64(a,a));
}


f32 length(dv2 a){
    return sqrt(dot(a,a));
}

f32 radAngleFull(v2 a, v2 b){
    f32 result = atan2(det(a,b), dot(a,b));
    return result;
}

f32 radAngle(v2 a, v2 b){
    f32 cos = dot(a,b) / (length(a) * length(b));
    return acos(cos);
}

f64 radAngle64(v2_64 a, v2_64 b){
    f64 cos = dot64(a,b) / (length64(a) * length64(b));
    return acos64(cos);
}

v4 normalize(v4 source){
    v4 result = {};
    f32 len = length(source);
    for(int i = 0; i < ARRAYSIZE(source.v); i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}

v4_64 normalize64(v4_64 source){
    v4_64 result = {};
    f64 len = length64(source);
    for(int i = 0; i < ARRAYSIZE(source.v); i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}

v2 normalize(v2 source){
    v2 result = {};
    f32 len = length(source);
    for(int i = 0; i < ARRAYSIZE(source.v); i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}

v2 normalize(dv2 source){
    v2 result = {};
    v2 cast = {(f32)source.x, (f32)source.y};
    f32 len = length(cast);
    for(int i = 0; i < ARRAYSIZE(cast.v); i++){
        result.v[i] = cast.v[i] / len;
    }
    return result;
}

v3 normalize(v3 source){
    v3 result = {};
    f32 len = length(source);
    for(int i = 0; i < ARRAYSIZE(source.v); i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}

v3_64 normalize64(v3_64 source){
    v3_64 result = {};
    f64 len = length64(source);
    for(int i = 0; i < ARRAYSIZE(source.v); i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}

v3_64 normalize64(dv3_64 source){
    v3_64 result = {};
    f64 len = length64(source);
    for(int i = 0; i < ARRAYSIZE(source.v); i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}

vN normalize(vN source){
    vN result = source;
    f32 len = length(source);
    for(int i = 0; i < source.size; i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}



v3 hadamard(const v3 & A, const v3 & B){
    return V3(A.x * B.x, A.y * B.y, A.z * B.z);
}

dv3_64 hadamard64(const dv3_64 & A, const dv3_64 & B){
    return DV3_64(A.x * B.x, A.y * B.y, A.z * B.z);
}

v3_64 hadamard64(const v3_64 & A, const v3_64 & B){
    return V3_64(A.x * B.x, A.y * B.y, A.z * B.z);
}

f32 sum(const vN * A){
    f32 result = 0;
    for(int i = 0; i < A->size; i++){
        result += A->v[i];
    }
    return result;
}

dv2 translate(const dv2 & point, const dv2 & translation){
    dv2 result;
    result.x = point.x + translation.x;
    result.y = point.y + translation.y;
    return result;
}

v2 rotate(const dv2 & point, const f32 radAngle){
    v2 result;
    f32 cosA = cos(radAngle);
    f32 sinA = sin(radAngle);
    result.x = cosA * point.x - sinA * point.y;
    result.y = sinA * point.x + cosA * point.y;
    return result;
}

v2 slerp(const v2 * origin, const v2 * target, f32 coeficient){
    if (length(*origin) < 0.00005f || length(*target) < 0.00005f) {
        return *origin;
    }
    f32 angle = acos(dot(normalize(*origin), normalize(*target)));
    f32 sinAngle = sin(angle);
    return (sin((1-coeficient)*angle)/sinAngle)*(*origin) + (sin(coeficient*angle)/sinAngle)*(*target);
}


//-----------------------------------------------------------------------MATRICES

struct matNM{
    f32 * c;
    u16 width;
    u16 height;
};


union mat4{
    f32 cells[4][4];
    f32 c[16];
};

union mat4_64{
    f64 cells[4][4];
    f64 c[16];
};


union mat3{
    f32 cells[3][3];
    f32 c[9];
};

mat3 operator*(const mat3 & A, const mat3 & B){
    mat3 result = {};
    
    for(int matrixCol = 0; matrixCol < 3; matrixCol++){
        for(int matrixRow = 0; matrixRow < 3; matrixRow++){
            f32 val = 0;
            for(int i = 0; i < 3; i++){
                val += A.c[matrixRow*3 + i] * B.c[i*3 + matrixCol]; 
            }
            result.c[matrixRow*3 + matrixCol] = val;
        }
    }
    return result;
}
v2 operator*(const mat3 & matrix, const v2 & vector){
    v3 originalVector = V3(vector.x, vector.y, 1);
    
    v3 resultVector = {};
    
    for(int matrixRow = 0; matrixRow < 3; matrixRow++){
        for(int vectorMember = 0; vectorMember < 3; vectorMember++){
            resultVector.v[matrixRow] += matrix.c[3*matrixRow + vectorMember] * originalVector.v[vectorMember];
        }
    }
    return V2(resultVector.x, resultVector.y);
}

v3 operator*(const mat4 & matrix, const v3 & vector){
    v4 originalVector = V4(vector.x, vector.y, vector.z, 1);
    
    v4 resultVector = {};
    
    for(int matrixRow = 0; matrixRow < 4; matrixRow++){
        for(int vectorMember = 0; vectorMember < 4; vectorMember++){
            resultVector.v[matrixRow] += matrix.c[4*matrixRow + vectorMember] * originalVector.v[vectorMember];
        }
    }
    return V3(resultVector.x, resultVector.y, resultVector.z);
}

v3_64 operator*(const mat4_64 & matrix, const v3_64 & vector){
    v4_64 originalVector = V4_64(vector.x, vector.y, vector.z, 1);
    
    v4_64 resultVector = {};
    
    for(int matrixRow = 0; matrixRow < 4; matrixRow++){
        for(int vectorMember = 0; vectorMember < 4; vectorMember++){
            resultVector.v[matrixRow] += matrix.c[4*matrixRow + vectorMember] * originalVector.v[vectorMember];
        }
    }
    return V3_64(resultVector.x, resultVector.y, resultVector.z);
}

void mul(const vN * vector, const matNM * matrix, vN * result){
    
    ASSERT((u16)result->size == matrix->width);
    ASSERT((u16)vector->size == matrix->height);
    
    for(int matrixCol = 0; matrixCol < matrix->width; matrixCol++){
        result->v[matrixCol] = 0;
        for(int vectorMember = 0; vectorMember < vector->size; vectorMember++){
            result->v[matrixCol] += matrix->c[matrixCol*matrix->height +vectorMember] * vector->v[vectorMember];
        }
    }
    
}



v3 operator*(const mat4 & matrix, const v4 & originalVector){
    v4 resultVector = {};
    
    for(int matrixRow = 0; matrixRow < 4; matrixRow++){
        for(int vectorMember = 0; vectorMember < 4; vectorMember++){
            resultVector.v[matrixRow] += matrix.c[4*matrixRow + vectorMember] * originalVector.v[vectorMember];
        }
    }
    return V3(resultVector.x, resultVector.y, resultVector.z) * (1.0f / resultVector.w);
}

v3_64 operator*(const mat4_64 & matrix, const v4_64 & originalVector){
    v4_64 resultVector = {};
    
    for(int matrixRow = 0; matrixRow < 4; matrixRow++){
        for(int vectorMember = 0; vectorMember < 4; vectorMember++){
            resultVector.v[matrixRow] += matrix.c[4*matrixRow + vectorMember] * originalVector.v[vectorMember];
        }
    }
    return V3_64(resultVector.x, resultVector.y, resultVector.z) * (1.0f / resultVector.w);
}

mat4 operator*(const mat4 & matrix, const f32 alfa){
    mat4 result = {};
    for(int cellIndex = 0; cellIndex < ARRAYSIZE(matrix.c); cellIndex++){
        result.c[cellIndex] = alfa * matrix.c[cellIndex];
    }
    return result;
}

mat4 operator*(const f32 alfa, const mat4 & matrix){
    return matrix * alfa;
}



f32 determinant(const mat3 * matrix){
    return matrix->cells[0][0] * matrix->cells[1][1] * matrix->cells[2][2]
        + matrix->cells[0][1] * matrix->cells[1][2] * matrix->cells[2][0]
        + matrix->cells[1][0] * matrix->cells[2][1] * matrix->cells[0][2]
    
        - matrix->cells[0][2] * matrix->cells[1][1] * matrix->cells[2][0]
        - matrix->cells[0][1] * matrix->cells[1][0] * matrix->cells[2][2]
        - matrix->cells[0][0] * matrix->cells[2][1] * matrix->cells[1][2];
}

mat3 transpose(const mat3 * originalMatrix){
    mat3 result = {};
    for(int originalMatrixRow = 0; originalMatrixRow < ARRAYSIZE(originalMatrix->cells); originalMatrixRow++){
        for(int originalMatrixCol = 0; originalMatrixCol < ARRAYSIZE(originalMatrix->cells); originalMatrixCol++){
            result.cells[originalMatrixCol][originalMatrixRow] = originalMatrix->cells[originalMatrixRow][originalMatrixCol];
        }
    }
    return result;
}

mat4 transpose(const mat4 * originalMatrix){
    mat4 result = {};
    for(int originalMatrixRow = 0; originalMatrixRow < ARRAYSIZE(originalMatrix->cells); originalMatrixRow++){
        for(int originalMatrixCol = 0; originalMatrixCol < ARRAYSIZE(originalMatrix->cells); originalMatrixCol++){
            result.cells[originalMatrixCol][originalMatrixRow] = originalMatrix->cells[originalMatrixRow][originalMatrixCol];
        }
    }
    return result;
}

mat4 inverseMatrix(const mat4 * originalMatrix){
    
    
    mat4 minors = {};
    
    f32 originalDeterminant = 0;
    for(int minorRow = 0; minorRow < ARRAYSIZE(minors.cells); minorRow++){
        for(int minorCol = 0; minorCol < ARRAYSIZE(minors.cells); minorCol++){
            mat3 tempMatrix = {};
            u8 tempIndex = 0;
            for(int originalMatrixRow = 0; originalMatrixRow < ARRAYSIZE(originalMatrix->cells); originalMatrixRow++){
                for(int originalMatrixCol = 0; originalMatrixCol < ARRAYSIZE(originalMatrix->cells); originalMatrixCol++){
                    if(originalMatrixRow != minorRow && originalMatrixCol != minorCol){
                        tempMatrix.c[tempIndex++] = originalMatrix->cells[originalMatrixRow][originalMatrixCol];
                    }
                }
            }
            f32 minorSign = pow(-1.0f, (f32)(minorRow + minorCol + 2));
            minors.cells[minorRow][minorCol] = minorSign * determinant(&tempMatrix);
            if(minorRow == 0){
                originalDeterminant += minors.cells[minorRow][minorCol];
            }
            
            
        }
    }
    
    mat4 adjugate = transpose(&minors);
    ASSERT(originalDeterminant != 0);
    return (1.0f/ originalDeterminant) * adjugate;
    
}


//-----------------------------------------------------------------------GEOMETRY

f32 degToRad(f32 degAngle){
    return  degAngle * PI / 180.0f;
}

f64 degToRad64(f64 degAngle){
    return  degAngle * PI_64 / 180.0f;
}

f32 radToDeg(f32 radAngle){
    return radAngle * 180.0f / PI;
}

f64 radToDeg64(f64 radAngle){
    return radAngle * 180.0f / PI_64;
}

v4 Quat(v3 axis, f32 radAngle){
    f32 sinHalf = sin(radAngle/2.0f);
    v4 result = {axis.x*sinHalf, axis.y*sinHalf, axis.z*sinHalf, cos(radAngle/2.0f)};
    return result;
}

v4_64 Quat64(v3_64 axis, f64 radAngle){
    f64 sinHalf = sin64(radAngle/2.0f);
    v4_64 result = {axis.x*sinHalf, axis.y*sinHalf, axis.z*sinHalf, cos64(radAngle/2.0f)};
    return result;
}

v4 operator*(const v4 & quaternion1, const v4 & quaternion2){
    v4 result;
    result.x = quaternion1.w * quaternion2.x + quaternion1.x * quaternion2.w + quaternion1.y * quaternion2.z - quaternion1.z * quaternion2.y;
    result.y = quaternion1.w * quaternion2.y + quaternion1.y * quaternion2.w + quaternion1.z * quaternion2.x - quaternion1.x * quaternion2.z;
    result.z = quaternion1.w * quaternion2.z + quaternion1.z * quaternion2.w + quaternion1.x * quaternion2.y - quaternion1.y * quaternion2.x;
    result.w = quaternion1.w * quaternion2.w - quaternion1.x * quaternion2.x - quaternion1.y * quaternion2.y - quaternion1.z * quaternion2.z;
    return result;
}

v4_64 operator*(const v4_64 & quaternion1, const v4_64 & quaternion2){
    v4_64 result;
    result.x = quaternion1.w * quaternion2.x + quaternion1.x * quaternion2.w + quaternion1.y * quaternion2.z - quaternion1.z * quaternion2.y;
    result.y = quaternion1.w * quaternion2.y + quaternion1.y * quaternion2.w + quaternion1.z * quaternion2.x - quaternion1.x * quaternion2.z;
    result.z = quaternion1.w * quaternion2.z + quaternion1.z * quaternion2.w + quaternion1.x * quaternion2.y - quaternion1.y * quaternion2.x;
    result.w = quaternion1.w * quaternion2.w - quaternion1.x * quaternion2.x - quaternion1.y * quaternion2.y - quaternion1.z * quaternion2.z;
    return result;
}


mat4 quaternionToMatrixT(const v4 & quaternion){
    mat4 result = {};
    
    result.c[0] = 1 - 2*powd(quaternion.y,2) - 2*powd(quaternion.z,2);
    result.c[4] = 2*quaternion.x*quaternion.y - 2*quaternion.w*quaternion.z;
    result.c[8] = 2*quaternion.x*quaternion.z + 2*quaternion.w*quaternion.y;
    
    result.c[1] = 2*quaternion.x*quaternion.y + 2*quaternion.w*quaternion.z;
    result.c[5] = 1 - 2*powd(quaternion.x,2) - 2*powd(quaternion.z,2);
    result.c[9] = 2*quaternion.y*quaternion.z - 2*quaternion.w*quaternion.x;
    
    result.c[2] = 2*quaternion.x*quaternion.z - 2*quaternion.w*quaternion.y;
    result.c[6] = 2*quaternion.y*quaternion.z + 2*quaternion.w*quaternion.x;
    result.c[10] = 1 - 2*powd(quaternion.x,2) - 2*powd(quaternion.y,2);
    result.c[15] = 1;
    
    return result;
}

mat4 quaternionToMatrix(const v4 & quaternion){
    mat4 result = {};
    
    result.c[0] = 1 - 2*powd(quaternion.y,2) - 2*powd(quaternion.z,2);
    result.c[1] = 2*quaternion.x*quaternion.y - 2*quaternion.w*quaternion.z;
    result.c[2] = 2*quaternion.x*quaternion.z + 2*quaternion.w*quaternion.y;
    
    result.c[4] = 2*quaternion.x*quaternion.y + 2*quaternion.w*quaternion.z;
    result.c[5] = 1 - 2*powd(quaternion.x,2) - 2*powd(quaternion.z,2);
    result.c[6] = 2*quaternion.y*quaternion.z - 2*quaternion.w*quaternion.x;
    
    result.c[8] = 2*quaternion.x*quaternion.z - 2*quaternion.w*quaternion.y;
    result.c[9] = 2*quaternion.y*quaternion.z + 2*quaternion.w*quaternion.x;
    result.c[10] = 1 - 2*powd(quaternion.x,2) - 2*powd(quaternion.y,2);
    result.c[15] = 1;
    
    return result;
}


mat4_64 quaternionToMatrix64(const v4_64 & quaternion){
    mat4_64 result = {};
    
    result.c[0] = 1 - 2*powd64(quaternion.y,2) - 2*powd64(quaternion.z,2);
    result.c[1] = 2*quaternion.x*quaternion.y - 2*quaternion.w*quaternion.z;
    result.c[2] = 2*quaternion.x*quaternion.z + 2*quaternion.w*quaternion.y;
    
    result.c[4] = 2*quaternion.x*quaternion.y + 2*quaternion.w*quaternion.z;
    result.c[5] = 1 - 2*powd64(quaternion.x,2) - 2*powd64(quaternion.z,2);
    result.c[6] = 2*quaternion.y*quaternion.z - 2*quaternion.w*quaternion.x;
    
    result.c[8] = 2*quaternion.x*quaternion.z - 2*quaternion.w*quaternion.y;
    result.c[9] = 2*quaternion.y*quaternion.z + 2*quaternion.w*quaternion.x;
    result.c[10] = 1 - 2*powd64(quaternion.x,2) - 2*powd64(quaternion.y,2);
    result.c[15] = 1;
    
    return result;
}

// 0 1 2
// 3 4 5
// 6 7 8
mat3 translationMatrix(const v2 offset){
    mat3 result = {};
    result.c[0] = 1.0f;
    result.c[2] = offset.x;
    result.c[4] = 1.0f;
    result.c[5] = offset.y;
    result.c[8] = 1.0f;
    return result;
}

// 0 1 2
// 3 4 5
// 6 7 8
mat3 scalingMatrix(const v2 scale){
    mat3 result = {};
    result.c[0] = scale.x;
    result.c[4] = scale.y;
    result.c[8] = 1.0f;
    return result;
}

// 0 1 2
// 3 4 5
// 6 7 8
mat3 rotationYMatrix3(f32 radAngle){
    mat3 result = {};
    result.c[0] = cos(radAngle);
    result.c[2] = sin(radAngle);
    result.c[4] = 1.0f;
    result.c[8] = 1.0f;
    return result;
}

// 0 1 2
// 3 4 5
// 6 7 8
mat3 ortoProjectionMatrix(v2 worldResolution, dv2 pxResolution)
{
    ASSERT(worldResolution.x >= worldResolution.y);
    f32 displayAspectRatio = CAST(f32, pxResolution.x) / pxResolution.y;
    f32 desiredWorldResolution = worldResolution.x / displayAspectRatio;
    if (desiredWorldResolution < worldResolution.y){
        f32 enlarge = worldResolution.y / desiredWorldResolution;
        worldResolution.x *= enlarge;
    }
    else{
        worldResolution.y = desiredWorldResolution;
    }
    v2 worldTopLeft = V2(-worldResolution.x/2.0f, worldResolution.y/2.0f);
    v2 worldBotRight = V2(+worldResolution.x/2.0f, -worldResolution.y/2.0f);
    mat3 result = {};
    result.c[0] = (2.0f / (worldBotRight.x - worldTopLeft.x));
    result.c[2] = -1.0f * ((worldBotRight.x + worldTopLeft.x)/(worldBotRight.x - worldTopLeft.x));
    result.c[4] = 2.0f / (worldTopLeft.y - worldBotRight.y);
    result.c[5] = -1.0f * ((worldTopLeft.y + worldBotRight.y)/(worldTopLeft.y - worldBotRight.y));
    result.c[8] = 1.0f;
    return result;
}


mat4 translationMatrix(const v3 offset){
    mat4 result = {};
    result.c[0] = 1.0f;
    result.c[12] = offset.x;
    result.c[5] = 1.0f;
    result.c[13] = offset.y;
    result.c[10] = 1.0f;
    result.c[14] = offset.z;
    result.c[15] = 1.0f;
    return result;
}

mat4 rotationXMatrix(f32 radAngle){
    mat4 result = {};
    result.c[0] = 1.0f;
    result.c[5] = cos(radAngle);
    result.c[6] = -sin(radAngle);
    result.c[9] = sin(radAngle);
    result.c[10] = cos(radAngle);
    result.c[15] = 1.0f;
    return result;
}

mat4 rotationZMatrix(f32 radAngle){
    mat4 result = {};
    result.c[0] = cos(radAngle);
    result.c[1] = -sin(radAngle);
    result.c[4] = sin(radAngle);
    result.c[5] = -cos(radAngle);
    result.c[10] = 1.0f;
    result.c[15] = 1.0f;
    return result;
}

mat4 rotationYMatrix(f32 radAngle){
    mat4 result = {};
    result.c[0] = cos(radAngle);
    result.c[2] = sin(radAngle);
    result.c[5] = 1.0f;
    result.c[8] = -sin(radAngle);
    result.c[10] = cos(radAngle);
    result.c[15] = 1.0f;
    return result;
}

mat4_64 rotationXMatrix64(f64 radAngle){
    mat4_64 result = {};
    result.c[0] = 1.0f;
    result.c[5] = cos64(radAngle);
    result.c[6] = -sin64(radAngle);
    result.c[9] = sin64(radAngle);
    result.c[10] = cos64(radAngle);
    result.c[15] = 1.0f;
    return result;
}

mat4_64 rotationZMatrix64(f64 radAngle){
    mat4_64 result = {};
    result.c[0] = cos64(radAngle);
    result.c[1] = -sin64(radAngle);
    result.c[4] = sin64(radAngle);
    result.c[5] = -cos64(radAngle);
    result.c[10] = 1.0f;
    result.c[15] = 1.0f;
    return result;
}

mat4_64 rotationYMatrix64(f64 radAngle){
    mat4_64 result = {};
    result.c[0] = cos64(radAngle);
    result.c[2] = sin64(radAngle);
    result.c[5] = 1.0f;
    result.c[8] = -sin64(radAngle);
    result.c[10] = cos64(radAngle);
    result.c[15] = 1.0f;
    return result;
}
