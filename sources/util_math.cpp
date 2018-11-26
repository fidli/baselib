#ifndef UTIL_MATH
#define UTIL_MATH

#ifdef PRECISE_MATH
#include <cmath>
#endif

#include "common.h"

#define PI 3.14159265f
#define PI_64 3.1415926535897932f
#define E 2.71828182f

//-----------------------------------------------------------------------NUMBERS
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#ifdef MAX
#undef MAX
#endif
#define MAX(a, b) (((a) < (b)) ? (b) : (a))
#define ABS(a) (((a) < 0) ? -(a) : (a))
#define KRONECKER(a, b) ((a) == (b) ? 1 : 0)


bool32 isOdd(const uint64 a){
    return (a & (uint64) 1);
}

bool aseq(float32 a, float32 b, float32 delta = 0.000001f){
    return ABS(a-b) <= delta;
}

bool aseq64(float64 a, float64 b, float64 delta = 0.000001f){
    return ABS(a-b) <= delta;
}


//is test on the right side of fixedpoint ? is test > fixedpoint?
bool aseql(float32 test, float32 fixedpoint, float32 delta = 0.000005f){
    float32 eps = fixedpoint - test;
    return eps < delta;
}

//is test on the right side of fixedpoint ? is test > fixedpoint?
bool aseql64(float64 test, float64 fixedpoint, float64 delta = 0.000005f){
    float64 eps = fixedpoint - test;
    return eps < delta;
}


//is test on the left side of fixedpoint ? is test < fixedpoint?
bool aseqr(float32 test, float32 fixedpoint, float32 delta = 0.000005f){
    float32 eps = test - fixedpoint;
    return eps < delta;
}

//is test on the left side of fixedpoint ? is test < fixedpoint?
bool aseqr64(float64 test, float64 fixedpoint, float64 delta = 0.000005f){
    float64 eps = test - fixedpoint;
    return eps < delta;
}


#ifndef PRECISE_MATH

float32 ceil(float32 value){
    float32 result = (float32)(uint64) ABS(value);
    if(result != value){
        result++;
    }
    return (value < 0) ? -result : result;
}

float32 floor(float32 value){
    return (float32)(int32) value;
    
}

float32 round(float32 value){
    if((uint32)(value * 10) % 10 >= 5){
        return ceil(value);
    }else{
        return floor(value);
    }
}

float32 powd(float32 base, int16 power = 2){
    //todo square mult
    if(power == 0){
        return 1;
    }
    float32 result = base;
    for(uint16 i = 1; i < ABS(power); i++){
        result *= base;
    }
    return (power < 0) ? (1.0f / result) : result;
}

float64 powd64(float64 base, int16 power = 2){
    //todo square mult
    if(power == 0){
        return 1;
    }
    float64 result = base;
    for(uint16 i = 1; i < ABS(power); i++){
        result *= base;
    }
    return (power < 0) ? (1.0f / result) : result;
}

static float32 subSqrt(float32 value, float32 guess, float32 prec = 0.0001f){
    if(guess != guess){
        //nan
        return guess;
    }
    float32 estimation = value / guess;
    if(aseq(estimation, guess, prec)){
        return (estimation + guess) / 2;
    }
    float32 newGuess = (guess + value/guess) / 2;
    //stack overflow with recursive shittery
    if(aseq(guess, newGuess, prec)){
        return (guess + newGuess) / 2;
    }
    return subSqrt(value, newGuess, prec);
}

static float64 subSqrt64(float64 value, float64 guess, float64 prec = 0.0001f){
    if(guess != guess){
        //nan
        return guess;
    }
    float64 estimation = value / guess;
    if(aseq64(estimation, guess, prec)){
        return (estimation + guess) / 2;
    }
    float64 newGuess = (guess + value/guess) / 2;
    //stack overflow with recursive shittery
    if(aseq64(guess, newGuess, prec)){
        return (guess + newGuess) / 2;
    }
    return subSqrt64(value, newGuess, prec);
}



float32 sqrt(float32 value){
    if(aseq(value, 0)) return 0;
    if(value < 0) INV;
    return subSqrt(value, value / 2, value < 1 ? 0.000001f : 0.01f);
}

float64 sqrt64(float64 value){
    if(aseq64(value, 0)) return 0;
    if(value < 0) INV;
    return subSqrt64(value, value / 2, value < 1 ? 0.000001f : 0.01f);
}

/* e N
float32 sqrt(float32 value){
    ASSERT(value > 0);
    float32 result = 65536;
    while(true){
        float32 candidate = floor((result + floor(value/result))/2);
        if(candidate >= result){
            return candidate;
        }
        result = candidate;
    }
    
    return result;
}
*/


float32 atan2(float32, float32){
    ASSERT(!"implement me");
    return 0;
}

float32 sin(float32 xRad){
    return xRad - powd(xRad,3)/6.0 + powd(xRad,5)/120.0 - powd(xRad,7)/5040.0 +  powd(xRad,9)/362880.0 - powd(xRad,11)/39916800.0 + powd(xRad,13)/6227020800.0 - powd(xRad,15)/1307674368000.0 +  powd(xRad,17)/355687428096000.0 - powd(xRad,19)/1.216451e+17 + powd(xRad,21)/5.1090942e+19;
}

float32 sin64(float64 xRad){
    return xRad - powd64(xRad,3)/6.0 + powd64(xRad,5)/120.0 - powd64(xRad,7)/5040.0 +  powd64(xRad,9)/362880.0 - powd64(xRad,11)/39916800.0 + powd64(xRad,13)/6227020800.0 - powd64(xRad,15)/1307674368000.0 +  powd64(xRad,17)/355687428096000.0 - powd64(xRad,19)/1.216451e+17 + powd64(xRad,21)/5.1090942e+19;
}

float32 cos(float32 xRad){
    return 1 - powd(xRad,2)/2.0 + powd(xRad,4)/24.0 - powd(xRad,6)/720.0 + powd(xRad,8)/40320.0 - powd(xRad,10)/3628800.0 + powd(xRad,12)/479001600.0 - powd(xRad,14)/87178291200.0 + powd(xRad,16)/20922789888000.0 - powd(xRad,18)/6.402373705728e+15 + powd(xRad,20)/2.43290200817664e+18;
}

float64 cos64(float64 xRad){
    return 1 - powd64(xRad,2)/2.0 + powd64(xRad,4)/24.0 - powd64(xRad,6)/720.0 + powd64(xRad,8)/40320.0 - powd64(xRad,10)/3628800.0 + powd64(xRad,12)/479001600.0 - powd64(xRad,14)/87178291200.0 + powd64(xRad,16)/20922789888000.0 - powd64(xRad,18)/6.402373705728e+15 + powd64(xRad,20)/2.43290200817664e+18;
}



float32 acos(float32 cos){
    //https://stackoverflow.com/questions/3380628/fast-arc-cos-algorithm/36387954#36387954
    float32 a = -0.939115566365855f;
    float32 b =  0.9217841528914573f;
    float32 c = -1.2845906244690837f;
    float32 d =  0.295624144969963174f;
    return PI/2 + (a*cos + b * powd(cos,3)) / (1 + c*powd(cos,2) + d*powd(cos, 4));
    
}

float64 acos64(float64 cos){
    //https://stackoverflow.com/questions/3380628/fast-arc-cos-algorithm/36387954#36387954
    float64 a = -0.939115566365855f;
    float64 b =  0.9217841528914573f;
    float64 c = -1.2845906244690837f;
    float64 d =  0.295624144969963174f;
    return PI_64/2 + (a*cos + b * powd64(cos,3)) / (1 + c*powd64(cos,2) + d*powd64(cos, 4));
    
}




static float32 agMean(float32 a, float32 b){
    if(aseq(a, b, 0.0000001f)) return (b + a) / 2.0f;
    return agMean((a+b)/2, sqrt(a*b));
}

float32 ln(float32 number, uint8 precisionBits = 32){
    //Arithmetic-geometric mean approximation
    ASSERT(precisionBits > 0);
    const float32 ln2 = 0.6931471f;
    const uint8 m = 18;
    
    float32 meanB = powd(2, 2-m) / number;
    
    return (PI / (2*agMean(1, meanB))) - m * ln2;
}

float32 epow(float32 power){
    if(aseq(power, 0, 0.000000005f)) return 1;
    //taylor
    float32 abspow = ABS(power) - (uint32) ABS(power);
    float32 result = 1 + abspow + powd(abspow, 2)/2.0f + powd(abspow, 3)/6.0f + powd(abspow, 4)/24.0f + powd(abspow, 5)/120.0f  + powd(abspow, 6)/720.0f + powd(abspow, 7)/5040.0f + powd(abspow, 8)/40320.0f + powd(abspow, 9)/362880.0f + powd(abspow, 10)/3628800.0f + powd(abspow, 11)/39916800.0f + powd(abspow, 12)/479001600.0f +powd(abspow, 13)/6881080200.0f + powd(abspow, 14)/87178291200.0f;
    result *= powd(E, (uint32) ABS(power));
    return (power < 0) ? (1.0 / result) : result;
}

float32 pow(float32 base, float32 power){
    return epow(power * ln(base));
}


float32 log(float32 number, float32 base = 10){
    ASSERT(number != 0);
    float32 temp = ABS(number);
    float32 result = ln(number) / ln(base);
    return (number < 0) ? -result : result;
    
}


float32 clamp(float32 originalValue, float32 min, float32 max){
    if(originalValue < min){
        return min;
    }else if(originalValue > max){
        return max;
    }
    return originalValue;
}


int32 clamp(int32 originalValue, int32 min, int32 max){
    if(originalValue < min){
        return min;
    }else if(originalValue > max){
        return max;
    }
    return originalValue;
}


float32 normalize(float32 value, float32 min, float32 max){
    return (value - min) / (max - min);
}

float32 fmodd(float32 value, uint32 modulus){
    int32 wholePart = (int32) value;
    float32 preResult = (float32)(wholePart % modulus) + (value - wholePart);
    return preResult; 
}

float64 fmodd64(float64 value, uint64 modulus){
    int64 wholePart = (int64) value;
    float64 preResult = (float64)(wholePart % modulus) + (value - wholePart);
    return preResult; 
}
#else


//this is just interface

float32 powd(float32 base, int16 power = 2){
    return (float32)pow((double) base, (double) power);
}

float64 powd64(float64 base, int16 power = 2){
    return (float64)pow(base, (double) power);
}

float64 sqrt64(float64 value){
    return (float64)sqrt((double) value);
}

float32 fmodd(float32 value, uint32 modulus){
    float32 result = (float32) fmod((double) value, (double) modulus);
    return result;
}

float64 fmodd64(float64 value, uint64 modulus){
    float64 result = fmod(value, (double) modulus);
    return result;
}

float64 acos64(float64 cos64){
    return (float64)acos((double)cos64);
}

float64 cos64(float64 radAngle){
    return (float64)cos((double)radAngle);
}

float64 sin64(float64 radAngle){
    return (float64)sin((double)radAngle);
}



#endif

uint8 numlen(int64 number){
    int result = 1;
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
    float32 * v;
    uint8 size;
};

union dv4{
    struct{
        int32 x;
        int32 y;
        int32 z;
        int32 w;
    };
    int32 v[4];
};

union v2{
    struct{
        float32 x;
        float32 y;
    };
    struct{
        float32 a;
        float32 b;
    };
    struct{
        float32 pitch;
        float32 yaw;
    };
    float32 v[2];
};

union v2_64{
    struct{
        float64 x;
        float64 y;
    };
    struct{
        float64 a;
        float64 b;
    };
    struct{
        float64 pitch;
        float64 yaw;
    };
    float64 v[2];
};


union dv2{
    struct{
        int32 x;
        int32 y;
    };
    struct{
        int32 pitch;
        int32 yaw;
    };
    int32 v[2];
};

union v3{
    struct{
        float32 x;
        float32 y;
        float32 z;
    };
    struct{
        float32 r;
        float32 g;
        float32 b;
    };
    struct{
        float32 right;
        float32 forward;
        float32 upward;
    };
    float32 v[3];
};

union v3_64{
    struct{
        float64 x;
        float64 y;
        float64 z;
    };
    struct{
        float64 r;
        float64 g;
        float64 b;
    };
    struct{
        float64 right;
        float64 forward;
        float64 upward;
    };
    float64 v[3];
};

union dv3{
    struct{
        int32 x;
        int32 y;
        int32 z;
    };
    struct{
        int32 r;
        int32 g;
        int32 b;
    };
    struct{
        int32 right;
        int32 forward;
        int32 upward;
    };
    int32 v[3];
};

union dv3_64{
    struct{
        int64 x;
        int64 y;
        int64 z;
    };
    struct{
        int64 r;
        int64 g;
        int64 b;
    };
    struct{
        int64 right;
        int64 forward;
        int64 upward;
    };
    int64 v[3];
};

union v4{
    struct{
        float32 x;
        float32 y;
        float32 z;
        float32 w;
    };
    struct{
        float32 r;
        float32 g;
        float32 b;
        float32 a;
    };
    float32 v[4];
};



union v4_64{
    struct{
        float64 x;
        float64 y;
        float64 z;
        float64 w;
    };
    struct{
        float64 r;
        float64 g;
        float64 b;
        float64 a;
    };
    float64 v[4];
};


v3 V3(float32 x, float32 y, float32 z){
    v3 res = {x,y,z};
    return res;
}

v3_64 V3_64(float64 x, float64 y, float64 z){
    v3_64 res = {x,y,z};
    return res;
}

dv3_64 DV3_64(int64 x, int64 y, int64 z){
    dv3_64 res = {x,y,z};
    return res;
}


v2 V2(float32 x, float32 y){
    v2 res = {x,y};
    return res;
}

v2_64 V2_64(float64 x, float64 y){
    v2_64 res = {x,y};
    return res;
}

dv2 DV2(int32 x, int32 y){
    dv2 res = {x, y};
    return res;
}

v4 V4(float32 x, float32 y, float32 z, float32 w){
    v4 result = {x, y, z, w};
    return result;
}

v4_64 V4_64(float64 x, float64 y, float64 z, float64 w){
    v4_64 result = {x, y, z, w};
    return result;
}

v3 operator+(const v3 & a, const v3 & b){
    v3 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] + b.v[i];
    }
    return result;
}

v3_64 operator+(const v3_64 & a, const v3_64 & b){
    v3_64 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] + b.v[i];
    }
    return result;
}


v2 operator+(const v2 & a, const v2 & b){
    v2 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] + b.v[i];
    }
    return result;
}

dv2 operator+(const dv2 & a, const dv2 & b){
    dv2 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] + b.v[i];
    }
    return result;
}

v2 operator+(const v2 & a, const dv2 & b){
    v2 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] + b.v[i];
    }
    return result;
}

v2 operator+(const dv2 & a, const v2 & b){
    return b + a;
}

v2 operator-(const v2 & a, const v2 & b){
    v2 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] - b.v[i];
    }
    return result;
}

v2_64 operator-(const v2_64 & a, const v2_64 & b){
    v2_64 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] - b.v[i];
    }
    return result;
}

dv2 operator-(const dv2 & a, const dv2 & b){
    dv2 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] - b.v[i];
    }
    return result;
}

dv2 operator-(const dv2 & a){
    dv2 result;
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
    v3 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] - b.v[i];
    }
    return result;
}

v3_64 operator-(const v3_64 & a, const v3_64 & b){
    v3_64 result;
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

dv3_64 & operator+=(dv3_64 & a, const dv3_64 & b){
    for(int i = 0; i < ARRAYSIZE(b.v); i++){
        a.v[i] += b.v[i];
    }
    return a;
}

v3_64 operator/(const dv3_64 & a, const float64 b){
    v3_64 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = (float64) a.v[i] / b;
    }
    return result;
}

v3_64 operator*(const dv3_64 & a, const float64 b){
    v3_64 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] * b;
    }
    return result;
}

dv3_64 operator*(const dv3_64 & a, const int32 b){
    dv3_64 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] * b;
    }
    return result;
}

dv3_64 operator+(const dv3_64 & a, const dv3_64 & b){
    dv3_64 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] + b.v[i];
    }
    return result;
}

dv3_64 operator-(const dv3_64 & a, const dv3_64 & b){
    dv3_64 result;
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


bool operator==(const v3 & a, const v3 & b){
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

v2 operator*(const v2 & b, const float32 a){
    v2 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a*b.v[i];
    }
    return result;
}

v2 operator*(const v2 & b, const int32 a){
    v2 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = ((float32)a)*b.v[i];
    }
    return result;
}

v2 operator*(const float32 a, const v2 & b){
    return b * a;
}

v2 operator*(const int32 a, const v2 & b){
    return b * a;
}


dv2 operator*(const dv2 & b, const int32 a){
    dv2 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a*b.v[i];
    }
    return result;
}

dv2 operator*(const int32 a, const dv2 & b){
    return b * a;
}

v2 operator*=(v2 & v, const float32 a){
    v.x *= a;
    v.y *= a;
    return v;
}

v2 operator*(const dv2 & b, const float32 a){
    v2 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a*(float32)b.v[i];
    }
    return result;
}

v2 operator*(const float32 a, const dv2 & b){
    return b * a;
}

v2 dv2Tov2(const dv2 & a){
    v2 result = {(float32)a.x, (float32)a.y};
    return result;
}

dv2 v2Todv2(const v2 & a){
    dv2 result = {(int32)a.x, (int32)a.y};
    return result;
}

v3 operator*(const v3 & b, const float32 a){
    v3 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a*b.v[i];
    }
    return result;
}

v3 operator*(const float32 a, const v3 & b){
    return b * a;
}

v3_64 operator*(const v3_64 & b, const float64 a){
    v3_64 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a*b.v[i];
    }
    return result;
}

v3_64 operator*(const float64 a, const v3_64 & b){
    return b * a;
}


vN operator*(const vN & b, const float32 a){
    vN result = b;
    for(int i = 0; i < b.size; i++){
        result.v[i] = a*b.v[i];
    }
    return result;
}

vN operator*(const float32 a, const vN & b){
    return b * a;
}


v3 lerp(const v3 * origin, const v3 * target, float32 coeficient){
    return *origin + (*target-*origin) * coeficient;
}

v2 lerp(const v2 * origin, const v2 * target, float32 coeficient){
    return *origin + (*target-*origin) * coeficient;
}



float32 dot(v3 a, v3 b){
    return a.x*b.x + a.y * b.y + a.z * b.z;
}

float64 dot64(v3_64 a, v3_64 b){
    return a.x*b.x + a.y * b.y + a.z * b.z;
}

float32 dot(v4 a, v4 b){
    return a.x*b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float64 dot64(v4_64 a, v4_64 b){
    return a.x*b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float32 dot(v2 a, v2 b){
    return a.x*b.x + a.y * b.y;
}

float64 dot64(v2_64 a, v2_64 b){
    return a.x*b.x + a.y * b.y;
}

float32 dot(dv2 a, dv2 b){
    return (float32)(a.x*b.x + a.y * b.y);
}

float32 dot(vN a, vN b){
    ASSERT(a.size == b.size);
    float32 result = 0;
    for(int i = 0; i < a.size; i++){
        result += a.v[i] * b.v[i];
    }
    return result;
}


float32 det(v2 a, v2 b){
    return a.x*b.y - b.x*a.y;
}

float32 length(v3 a){
    return sqrt(dot(a,a));
}

float64 length64(v3_64 a){
    return sqrt64(dot64(a,a));
}

float32 length(vN a){
    return sqrt(dot(a,a));
}

float32 length(v4 a){
    return sqrt(dot(a,a));
}

float64 length64(v4_64 a){
    return sqrt64(dot64(a,a));
}

float32 length(v2 a){
    return sqrt(dot(a,a));
}

float64 length64(v2_64 a){
    return sqrt64(dot64(a,a));
}


float32 length(dv2 a){
    return sqrt(dot(a,a));
}

float32 radAngleFull(v2 a, v2 b){
    float32 result = atan2(det(a,b), dot(a,b));
    return result;
}

float32 radAngle(v2 a, v2 b){
    float32 cos = dot(a,b) / (length(a) * length(b));
    return acos(cos);
}

float64 radAngle64(v2_64 a, v2_64 b){
    float64 cos = dot64(a,b) / (length64(a) * length64(b));
    return acos64(cos);
}

v4 normalize(v4 source){
    v4 result = {};
    float32 len = length(source);
    for(int i = 0; i < ARRAYSIZE(source.v); i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}

v4_64 normalize64(v4_64 source){
    v4_64 result = {};
    float64 len = length64(source);
    for(int i = 0; i < ARRAYSIZE(source.v); i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}

v2 normalize(v2 source){
    v2 result = {};
    float32 len = length(source);
    for(int i = 0; i < ARRAYSIZE(source.v); i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}

v2 normalize(dv2 source){
    v2 result = {};
    v2 cast = {(float32)source.x, (float32)source.y};
    float32 len = length(cast);
    for(int i = 0; i < ARRAYSIZE(cast.v); i++){
        result.v[i] = cast.v[i] / len;
    }
    return result;
}

v3 normalize(v3 source){
    v3 result = {};
    float32 len = length(source);
    for(int i = 0; i < ARRAYSIZE(source.v); i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}

v3_64 normalize64(v3_64 source){
    v3_64 result = {};
    float64 len = length64(source);
    for(int i = 0; i < ARRAYSIZE(source.v); i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}

vN normalize(vN source){
    vN result = source;
    float32 len = length(source);
    for(int i = 0; i < source.size; i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}



v3 hadamard(const v3 & A, const v3 & B){
    return V3(A.x * B.x, A.y * B.y, A.z * B.z);
}

float32 sum(const vN * A){
    float32 result = 0;
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

v2 rotate(const dv2 & point, const float32 radAngle){
    v2 result;
    float32 cosA = cos(radAngle);
    float32 sinA = sin(radAngle);
    result.x = cosA * point.x - sinA * point.y;
    result.y = sinA * point.x + cosA * point.y;
    return result;
}

//-----------------------------------------------------------------------MATRICES

struct matNM{
    float32 * c;
    uint16 width;
    uint16 height;
};


union mat4{
    float32 cells[4][4];
    float32 c[16];
};

union mat4_64{
    float64 cells[4][4];
    float64 c[16];
};


union mat3{
    float32 cells[3][3];
    float32 c[9];
};

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
    
    ASSERT((uint16)result->size == matrix->width);
    ASSERT((uint16)vector->size == matrix->height);
    
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

mat4 operator*(const mat4 & matrix, const float32 alfa){
    mat4 result;
    for(int cellIndex = 0; cellIndex < ARRAYSIZE(matrix.c); cellIndex++){
        result.c[cellIndex] = alfa * matrix.c[cellIndex];
    }
    return result;
}

mat4 operator*(const float32 alfa, const mat4 & matrix){
    return matrix * alfa;
}



float32 determinant(const mat3 * matrix){
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
    
    float32 originalDeterminant = 0;
    for(int minorRow = 0; minorRow < ARRAYSIZE(minors.cells); minorRow++){
        for(int minorCol = 0; minorCol < ARRAYSIZE(minors.cells); minorCol++){
            mat3 tempMatrix = {};
            uint8 tempIndex = 0;
            for(int originalMatrixRow = 0; originalMatrixRow < ARRAYSIZE(originalMatrix->cells); originalMatrixRow++){
                for(int originalMatrixCol = 0; originalMatrixCol < ARRAYSIZE(originalMatrix->cells); originalMatrixCol++){
                    if(originalMatrixRow != minorRow && originalMatrixCol != minorCol){
                        tempMatrix.c[tempIndex++] = originalMatrix->cells[originalMatrixRow][originalMatrixCol];
                    }
                }
            }
            float32 minorSign = pow(-1.0f, (float32)(minorRow + minorCol + 2));
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

float32 degToRad(float32 degAngle){
    return  degAngle * PI / 180.0f;
}

float64 degToRad64(float64 degAngle){
    return  degAngle * PI_64 / 180.0f;
}

float32 radToDeg(float32 radAngle){
    return radAngle * 180.0f / PI;
}

float64 radToDeg64(float64 radAngle){
    return radAngle * 180.0f / PI_64;
}

v4 Quat(v3 axis, float32 radAngle){
    float32 sinHalf = sin(radAngle/2.0f);
    v4 result = {axis.x*sinHalf, axis.y*sinHalf, axis.z*sinHalf, cos(radAngle/2.0f)};
    return result;
}

v4_64 Quat64(v3_64 axis, float64 radAngle){
    float64 sinHalf = sin64(radAngle/2.0f);
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


mat4 rotationXMatrix(float32 radAngle){
    mat4 result = {};
    result.c[0] = 1.0f;
    result.c[5] = cos(radAngle);
    result.c[6] = -sin(radAngle);
    result.c[9] = sin(radAngle);
    result.c[10] = cos(radAngle);
    result.c[15] = 1.0f;
    return result;
}

mat4 rotationZMatrix(float32 radAngle){
    mat4 result = {};
    result.c[0] = cos(radAngle);
    result.c[1] = -sin(radAngle);
    result.c[4] = sin(radAngle);
    result.c[5] = -cos(radAngle);
    result.c[10] = 1.0f;
    result.c[15] = 1.0f;
    return result;
}

mat4 rotationYMatrix(float32 radAngle){
    mat4 result = {};
    result.c[0] = cos(radAngle);
    result.c[2] = sin(radAngle);
    result.c[5] = 1.0f;
    result.c[8] = -sin(radAngle);
    result.c[10] = cos(radAngle);
    result.c[15] = 1.0f;
    return result;
}

mat4_64 rotationXMatrix64(float64 radAngle){
    mat4_64 result = {};
    result.c[0] = 1.0f;
    result.c[5] = cos64(radAngle);
    result.c[6] = -sin64(radAngle);
    result.c[9] = sin64(radAngle);
    result.c[10] = cos64(radAngle);
    result.c[15] = 1.0f;
    return result;
}

mat4_64 rotationZMatrix64(float64 radAngle){
    mat4_64 result = {};
    result.c[0] = cos64(radAngle);
    result.c[1] = -sin64(radAngle);
    result.c[4] = sin64(radAngle);
    result.c[5] = -cos64(radAngle);
    result.c[10] = 1.0f;
    result.c[15] = 1.0f;
    return result;
}

mat4_64 rotationYMatrix64(float64 radAngle){
    mat4_64 result = {};
    result.c[0] = cos64(radAngle);
    result.c[2] = sin64(radAngle);
    result.c[5] = 1.0f;
    result.c[8] = -sin64(radAngle);
    result.c[10] = cos64(radAngle);
    result.c[15] = 1.0f;
    return result;
}



#endif