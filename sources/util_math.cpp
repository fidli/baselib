
#ifndef UTIL_MATH
#define UTIL_MATH

#include "common.h"

#define PI 3.14159265
#define E 2.71828182

//-----------------------------------------------------------------------NUMBERS
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))
#define ABS(a) (((a) < 0) ? -(a) : (a))
#define KRONECKER(a, b) ((a) == (b) ? 1 : 0)

bool32 isOdd(const uint64 a){
    return (a & (uint64) 1);
}

bool aseq(float32 a, float32 b, float32 delta = 0.000001){
    return ABS(a-b) <= delta;
}

bool aseql(float32 test, float32 fixedpoint, float32 delta = 0.000005){
    float32 eps = fixedpoint - test;
    return eps < delta;
}

bool aseqr(float32 test, float32 fixedpoint, float32 delta = 0.000005){
    float32 eps = test - fixedpoint;
    return eps < delta;
}


float32 ceil(float32 value){
    float32 result = (float32)(uint64) ABS(value);
    if(!aseq(result, value)){
        result++;
    }
    return (value < 0) ? -result : result;
}

float32 floor(float32 value){
    return (float32)(int32) value;
    
}

float64 powd(float32 base, int16 power = 2){
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

static float32 subSqrt(float32 value, float32 guess){
    if(aseq(value / guess, guess, 0.000001)){
        return guess;
    }
    return subSqrt(value, (guess + value/guess) / 2);
}

float32 sqrt(float32 value){
    return subSqrt(value, value / 2);
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
    ASSERT(!"fuck");
    return 0;
}

float32 sin(float32){
    ASSERT(!"fuck");
    return 0;
}

float32 cos(float32){
    ASSERT(!"fuck");
    return 0;
}







static float32 agMean(float32 a, float32 b){
    if(aseq(a, b, 0.0000001)) return (b + a) / 2.0;
    return agMean((a+b)/2, sqrt(a*b));
}

float32 ln(float32 number, uint8 precisionBits = 32){
    //Arithmetic-geometric mean approximation
    ASSERT(precisionBits > 0);
    const float32 ln2 = 0.6931471;
    const uint8 m = 18;
    ASSERT(number * powd(2, m) > powd(2, precisionBits / 2));
    float32 meanB = powd(2, 2-m) / number;
    
    return (PI / (2*agMean(1, meanB))) - m * ln2;
}

float32 epow(float32 power){
    if(aseq(power, 0, 0.000000005)) return 1;
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


float32 clamp(float32 originalValue, float32 min, float32 max){
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



//-----------------------------------------------------------------------VERTICES

struct vN{
    float32 * v;
    uint8 size;
};

union dv4{
    struct{
        uint32 x;
        uint32 y;
        uint32 z;
        uint32 w;
    };
    uint32 v[4];
};

union v2{
    struct{
        float32 x;
        float32 y;
    };
    struct{
        float32 pitch;
        float32 yaw;
    };
    float32 v[2];
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


v3 V3(float32 x, float32 y, float32 z){
    v3 res = {x,y,z};
    return res;
}


v2 V2(float32 x, float32 y){
    v2 res = {x,y};
    return res;
}

v4 V4(float32 x, float32 y, float32 z, float32 w){
    v4 result = {x, y, z, w};
    return result;
}

v3 operator+(const v3 & a, const v3 & b){
    v3 result;
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

v2 operator-(const v2 & a, const v2 & b){
    v2 result;
    for(int i = 0; i < ARRAYSIZE(result.v); i++){
        result.v[i] = a.v[i] - b.v[i];
    }
    return result;
}

v2 & operator+=(v2 & a, const v2 & b){
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

v3 & operator+=(v3 & a, const v3 & b){
    for(int i = 0; i < ARRAYSIZE(b.v); i++){
        a.v[i] += b.v[i];
    }
    return a;
}

v3 & operator-=(v3 & a, const v3 & b){
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

v2 operator*(const float32 a, const v2 & b){
    return b * a;
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

float32 dot(v4 a, v4 b){
    return a.x*b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float32 dot(v2 a, v2 b){
    return a.x*b.x + a.y * b.y;
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

float32 length(vN a){
    return sqrt(dot(a,a));
}

float32 length(v4 a){
    return sqrt(dot(a,a));
}

float32 length(v2 a){
    return sqrt(dot(a,a));
}

float32 radAngle(v2 a, v2 b){
    float32 result = atan2(det(a,b), dot(a,b));
    return result;
}

v4 normalize(v4 source){
    v4 result = {};
    float32 len = length(source);
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

v3 normalize(v3 source){
    v3 result = {};
    float32 len = length(source);
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
            float32 minorSign = pow(-1.0f, minorRow + minorCol + 2);
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
struct box{
    v3 lowerCorner;
    v3 upperCorner;
};


float32 degToRad(float32 degAngle){
    return  degAngle * PI / 180.0f;
}

float32 radToDeg(float32 radAngle){
    return radAngle * 180.0f / PI;
}

v4 Quat(v3 axis, float32 radAngle){
    float32 sinHalf = sin(radAngle/2.0f);
    v4 result = {axis.x*sinHalf, axis.y*sinHalf, axis.z*sinHalf, cos(radAngle/2.0f)};
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


mat4 quaternionToMatrix(const v4 & quaternion){
    mat4 result = {};
    
    result.c[0] = 1 - 2*pow(quaternion.y,2) - 2*pow(quaternion.z,2);
    result.c[4] = 2*quaternion.x*quaternion.y - 2*quaternion.w*quaternion.z;
    result.c[8] = 2*quaternion.x*quaternion.z + 2*quaternion.w*quaternion.y;
    
    result.c[1] = 2*quaternion.x*quaternion.y + 2*quaternion.w*quaternion.z;
    result.c[5] = 1 - 2*pow(quaternion.x,2) - 2*pow(quaternion.z,2);
    result.c[9] = 2*quaternion.y*quaternion.z - 2*quaternion.w*quaternion.x;
    
    result.c[2] = 2*quaternion.x*quaternion.z - 2*quaternion.w*quaternion.y;
    result.c[6] = 2*quaternion.y*quaternion.z + 2*quaternion.w*quaternion.x;
    result.c[10] = 1 - 2*pow(quaternion.x,2) - 2*pow(quaternion.y,2);
    result.c[15] = 1;
    
    return result;
}


mat4 rotationYMatrix(float32 radAngle){
    mat4 result = {};
    result.c[0] = 1.0f;
    result.c[5] = cos(radAngle);
    result.c[6] = sin(radAngle);
    result.c[9] = -sin(radAngle);
    result.c[10] = cos(radAngle);
    result.c[15] = 1.0f;
    return result;
}

mat4 rotationXMatrix(float32 radAngle){
    mat4 result = {};
    result.c[0] = cos(radAngle);
    result.c[2] = -sin(radAngle);
    result.c[5] = 1.0f;
    result.c[8] = sin(radAngle);
    result.c[10] = cos(radAngle);
    result.c[15] = 1.0f;
    return result;
}



#endif