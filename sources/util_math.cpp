
#ifndef UTIL_MATH
#define UTIL_MATH

#include "common.h"

#define PI 3.14159265

//-----------------------------------------------------------------------VERTICES

union dv4{
    struct{
        Uint32 x;
        Uint32 y;
        Uint32 z;
        Uint32 w;
    };
    Uint32 v[4];
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
    for(int i = 0; i < arraySize(result.v); i++){
        result.v[i] = a.v[i] + b.v[i];
    }
    return result;
}

v2 operator+(const v2 & a, const v2 & b){
    v2 result;
    for(int i = 0; i < arraySize(result.v); i++){
        result.v[i] = a.v[i] + b.v[i];
    }
    return result;
}

v2 operator-(const v2 & a, const v2 & b){
    v2 result;
    for(int i = 0; i < arraySize(result.v); i++){
        result.v[i] = a.v[i] - b.v[i];
    }
    return result;
}

v2 & operator+=(v2 & a, const v2 & b){
    for(int i = 0; i < arraySize(b.v); i++){
        a.v[i] += b.v[i];
    }
    return a;
}

v3 operator-(const v3 & a, const v3 & b){
    v3 result;
    for(int i = 0; i < arraySize(result.v); i++){
        result.v[i] = a.v[i] - b.v[i];
    }
    return result;
}

v3 & operator+=(v3 & a, const v3 & b){
    for(int i = 0; i < arraySize(b.v); i++){
        a.v[i] += b.v[i];
    }
    return a;
}

v3 & operator-=(v3 & a, const v3 & b){
    for(int i = 0; i < arraySize(b.v); i++){
        a.v[i] -= b.v[i];
    }
    return a;
}

bool operator==(const v3 & a, const v3 & b){
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

v2 operator*(const v2 & b, const float32 a){
    v2 result;
    for(int i = 0; i < arraySize(result.v); i++){
        result.v[i] = a*b.v[i];
    }
    return result;
}

v2 operator*(const float32 a, const v2 & b){
    return b * a;
}

v3 operator*(const v3 & b, const float32 a){
    v3 result;
    for(int i = 0; i < arraySize(result.v); i++){
        result.v[i] = a*b.v[i];
    }
    return result;
}

v3 operator*(const float32 a, const v3 & b){
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

float32 det(v2 a, v2 b){
    return a.x*b.y - b.x*a.y;
}

float32 length(v3 a){
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
    for(int i = 0; i < arraySize(source.v); i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}

v2 normalize(v2 source){
    v2 result = {};
    float32 len = length(source);
    for(int i = 0; i < arraySize(source.v); i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}

v3 normalize(v3 source){
    v3 result = {};
    float32 len = length(source);
    for(int i = 0; i < arraySize(source.v); i++){
        result.v[i] = source.v[i] / len;
    }
    return result;
}



v3 hadamard(const v3 & A, const v3 & B){
    return V3(A.x * B.x, A.y * B.y, A.z * B.z);
}

//-----------------------------------------------------------------------MATRICES

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
    for(int cellIndex = 0; cellIndex < arraySize(matrix.c); cellIndex++){
        result.c[cellIndex] = alfa * matrix.c[cellIndex];
    }
    return result;
}

mat4 operator*(const float32 alfa, const mat4 & matrix){
    return matrix * alfa;
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
    for(int originalMatrixRow = 0; originalMatrixRow < arraySize(originalMatrix->cells); originalMatrixRow++){
        for(int originalMatrixCol = 0; originalMatrixCol < arraySize(originalMatrix->cells); originalMatrixCol++){
            result.cells[originalMatrixCol][originalMatrixRow] = originalMatrix->cells[originalMatrixRow][originalMatrixCol];
        }
    }
    return result;
}

mat4 transpose(const mat4 * originalMatrix){
    mat4 result = {};
    for(int originalMatrixRow = 0; originalMatrixRow < arraySize(originalMatrix->cells); originalMatrixRow++){
        for(int originalMatrixCol = 0; originalMatrixCol < arraySize(originalMatrix->cells); originalMatrixCol++){
            result.cells[originalMatrixCol][originalMatrixRow] = originalMatrix->cells[originalMatrixRow][originalMatrixCol];
        }
    }
    return result;
}

mat4 inverseMatrix(const mat4 * originalMatrix){
    
    
    mat4 minors = {};
    
    float32 originalDeterminant = 0;
    for(int minorRow = 0; minorRow < arraySize(minors.cells); minorRow++){
        for(int minorCol = 0; minorCol < arraySize(minors.cells); minorCol++){
            mat3 tempMatrix = {};
            Uint8 tempIndex = 0;
            for(int originalMatrixRow = 0; originalMatrixRow < arraySize(originalMatrix->cells); originalMatrixRow++){
                for(int originalMatrixCol = 0; originalMatrixCol < arraySize(originalMatrix->cells); originalMatrixCol++){
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
    assert(originalDeterminant != 0);
    return (1.0f/ originalDeterminant) * adjugate;
    
}


//-----------------------------------------------------------------------NUMBERS

int64 pow(int64 base, uint8 power){
    //todo square mult
    
    int64 result = base;
    for(uint8 i = 1; i < power; i++){
        result *= base;
    }
    return result;
}


float64 log(float64 number, uint64 base = 10){
//more robust and faster?
    float64 temp = number;
    float64 result = 0;
    while(temp > 1.0f){
        result++;
        temp /= base;
    }
    //todo the ity bity part
    ASSERT(number > base);
    return result;
    
}

uint8 numlen(int64 number){
    ASSERT(number >= 0);
    int result = 1;
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



#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#endif