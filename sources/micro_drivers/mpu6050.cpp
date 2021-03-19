#ifndef MPU6050
#define MPU6050


float mpu6050_g = 9.8196f;
double mpu6050_g64 = mpu6050_g;

enum GyroPrecision{
    GyroPrecision_250,
    GyroPrecision_500,
    GyroPrecision_1000,
    GyroPrecision_2000
};

enum AccPrecision{
    AccPrecision_2,
    AccPrecision_4,
    AccPrecision_8,
    AccPrecision_16
};

#pragma pack(push, 1)
struct MPU6050Settings{
    GyroPrecision gyroPrecision;
    AccPrecision accPrecision;
    u16 sampleRate;
};
#pragma pack(pop)


struct MPU6050Handle{
    //platform, lib specific handles
    int fd;
    //mpu6050 specific stuff
    MPU6050Settings settings;
};


#ifndef SERVER

#include "wiringPi.h"
#include "wiringPiI2C.h"

//replace these two with whatever I2C implementation that you fancy
void write8Reg(MPU6050Handle * handle, int reg, u8 data){
    wiringPiI2CWriteReg8(handle->fd, reg, (int)data);
}
uint8 read8Reg(MPU6050Handle * handle, int reg){
    return (u8) wiringPiI2CReadReg8(handle->fd, reg);
}

#else
void write8Reg(MPU6050Handle * handle, int reg, u8 data){
    INV;
}
uint8 read8Reg(MPU6050Handle * handle, int reg){
    INV;
    return 0;
}

#endif

#define MPU6050_REGISTER_SAMPLE_RATE 0x19
#define MPU6050_REGISTER_CONFIG 0x1A
#define MPU6050_REGISTER_GYRO_CONFIG 0x1B
#define MPU6050_REGISTER_ACCEL_CONFIG 0x1C
#define MPU6050_REGISTER_FIFO_EN 0x23
#define MPU6050_REGISTER_USER_CONTROL 0x6A
#define MPU6050_REGISTER_PWR_MGMT_1 0x6B
#define MPU6050_REGISTER_PWR_MGMT_2 0x6C

#define MPU6050_REGISTER_FIFO_COUNT_H 0x72
#define MPU6050_REGISTER_FIFO_COUNT_L 0x73
#define MPU6050_REGISTER_FIFO_R_W 0x74

#define MPU6050_REGISTER_ACCEL_XOUT_H 0x3B
#define MPU6050_REGISTER_ACCEL_XOUT_L 0x3C
#define MPU6050_REGISTER_ACCEL_YOUT_H 0x3D
#define MPU6050_REGISTER_ACCEL_YOUT_L 0x3E
#define MPU6050_REGISTER_ACCEL_ZOUT_H 0x3F
#define MPU6050_REGISTER_ACCEL_ZOUT_L 0x40


void mpu6050_resetFifo(MPU6050Handle * handle){
    u8 currentReg = read8Reg(handle, MPU6050_REGISTER_USER_CONTROL);
    write8Reg(handle, MPU6050_REGISTER_USER_CONTROL, currentReg | 4);
    u8 reg;
    while((reg = read8Reg(handle, MPU6050_REGISTER_USER_CONTROL)) != currentReg){
#if DEBUG
        printf("reseting device fifo  %hhu\n", reg);
        sleep(1);
#endif
    }
}

//resets all the flags to default state
void mpu6050_reset(MPU6050Handle * handle){
    write8Reg(handle, MPU6050_REGISTER_PWR_MGMT_1, 128);
    //wait untill device resets
    //64 is default value of this register
    u8 reg;
    while((reg = read8Reg(handle, MPU6050_REGISTER_PWR_MGMT_1)) != 64){
#if DEBUG
        printf("reseting device  %hhu\n", reg);
        sleep(1);
#endif
    }
}



void mpu6050_setup(MPU6050Handle * handle, const MPU6050Settings settings){
    
    //turn on power cycling for fifo settings
    write8Reg(handle, MPU6050_REGISTER_PWR_MGMT_1, 32);
    
    //use fifo buffer, no slaves, reset FIFO
    write8Reg(handle, MPU6050_REGISTER_USER_CONTROL, 64);
    
    u8 reg;
    while((reg = read8Reg(handle, MPU6050_REGISTER_USER_CONTROL)) != 64){
#if DEBUG
        printf("reseting fifo %hhu\n", reg);
#endif
    }
    
    //clean the reg
    write8Reg(handle, MPU6050_REGISTER_PWR_MGMT_2, 0);
    //no sleep, no temperature X axis gyroscope as clock
    write8Reg(handle, MPU6050_REGISTER_PWR_MGMT_1, 8+1);
    
    
    //fifo mask, what to put in fifo, gyro x y z, acc x y z in order of Hbyte, Lbyte, 12 bytes per sample, acc values first then gyro ones
    write8Reg(handle, MPU6050_REGISTER_FIFO_EN, 64+32+16+8);
    //no FSYNC, NO lfp
    write8Reg(handle, MPU6050_REGISTER_CONFIG, 0);
    //sample rate = (8khz / (settings->sampleRate+1))
    u8 param = (8000 / settings.sampleRate) - 1;
    write8Reg(handle, MPU6050_REGISTER_SAMPLE_RATE, param);
    
    //sensitivity
    write8Reg(handle, MPU6050_REGISTER_ACCEL_CONFIG, settings.accPrecision << 3);
    write8Reg(handle, MPU6050_REGISTER_GYRO_CONFIG, settings.gyroPrecision << 3);
    
    
    
}

uint16 mpu6050_fifoCount(MPU6050Handle * handle){
    u8 high = read8Reg(handle, MPU6050_REGISTER_FIFO_COUNT_H);
    u8 low = read8Reg(handle, MPU6050_REGISTER_FIFO_COUNT_L);
    return (((u16) high) << 8) + low;
}

uint8 mpu6050_readFifoByte(MPU6050Handle * handle){
    return read8Reg(handle, MPU6050_REGISTER_FIFO_R_W);
}


uint16 mpu6050_getAccDivisor(const MPU6050Settings * setting){
    switch(setting->accPrecision){
        case AccPrecision_2:{
            return 16384; 
        }break;
        case AccPrecision_4:{
            return 8192;
        }break;
        case AccPrecision_8:{
            return 4096;
        }break;
        case AccPrecision_16:{
            return 2048;
        }break;
        default:{
            INV;
        }break;
    }
    return 0;
}

float32 mpu6050_getGyroDivisor(const MPU6050Settings * setting){
    switch(setting->gyroPrecision){
        case GyroPrecision_250:{
            return 131; 
        }break;
        case GyroPrecision_500:{
            return 65.5f;
        }break;
        case GyroPrecision_1000:{
            return 32.8f;
        }break;
        case GyroPrecision_2000:{
            return 16.4f;
        }break;
        default:{
            INV;
        }break;
    }
    return 0;
}

int16 mpu6050_getGyroDivisorTimes10(const MPU6050Settings * setting){
    switch(setting->gyroPrecision){
        case GyroPrecision_250:{
            return 1310; 
        }break;
        case GyroPrecision_500:{
            return 655;
        }break;
        case GyroPrecision_1000:{
            return 328;
        }break;
        case GyroPrecision_2000:{
            return 164;
        }break;
        default:{
            INV;
        }break;
    }
    return 0;
}


void mpu6050_acc16_float32(const MPU6050Settings setting, const i16 x, const i16 y, const i16 z, f32 * result_x, f32 * result_y,float32 * result_z){
    f32 attun = 1.0f / mpu6050_getAccDivisor(&setting);
    *result_x = attun * x;
    *result_y = attun * y;
    *result_z = attun * z;
}

void mpu6050_acc16_float64(const MPU6050Settings setting, const i16 x, const i16 y, const i16 z, f64 * result_x, f64 * result_y,float64 * result_z){
    f64 attun = 1.0f / mpu6050_getAccDivisor(&setting);
    *result_x = attun * x;
    *result_y = attun * y;
    *result_z = attun * z;
}

void mpu6050_acc32_float32(const MPU6050Settings setting, const i32 x, const i32 y, const i32 z, f32 * result_x, f32 * result_y,float32 * result_z){
    f32 attun = 1.0f / mpu6050_getAccDivisor(&setting);
    
    *result_x = attun * x;
    *result_y = attun * y;
    *result_z = attun * z;
}

void mpu6050_acc32_float64(const MPU6050Settings setting, const i32 x, const i32 y, const i32 z, f64 * result_x, f64 * result_y,float64 * result_z){
    f64 attun = 1.0f / mpu6050_getAccDivisor(&setting);
    *result_x = attun * x;
    *result_y = attun * y;
    *result_z = attun * z;
}


void mpu6050_gyro16_float32(const MPU6050Settings setting, const i16 x, const i16 y, const i16 z, f32 * result_x, f32 * result_y, f32 * result_z){
    f32 attun = 1.0f / mpu6050_getGyroDivisor(&setting);
    *result_x = attun * x;
    *result_y = attun * y;
    *result_z = attun * z;
}

void mpu6050_gyro16_float64(const MPU6050Settings setting, const i16 x, const i16 y, const i16 z, f64 * result_x, f64 * result_y, f64 * result_z){
    f64 attun = 1.0f / mpu6050_getGyroDivisor(&setting);
    *result_x = attun * x;
    *result_y = attun * y;
    *result_z = attun * z;
}


void mpu6050_gyro32_float32(const MPU6050Settings setting, const i32 x, const i32 y, const i32 z, f32 * result_x, f32 * result_y, f32 * result_z){
    f32 attun = 1.0f / mpu6050_getGyroDivisor(&setting);
    *result_x = attun * x;
    *result_y = attun * y;
    *result_z = attun * z;
}

void mpu6050_gyro32_float64(const MPU6050Settings setting, const i32 x, const i32 y, const i32 z, f64 * result_x, f64 * result_y, f64 * result_z){
    f64 attun = 1.0f / mpu6050_getGyroDivisor(&setting);
    *result_x = attun * x;
    *result_y = attun * y;
    *result_z = attun * z;
}


float32 mpu6050_getTimeDelta(const u16 sampleRate){
    return 1.0f / sampleRate;
}

float64 mpu6050_getTimeDelta64(const u16 sampleRate){
    return 1.0f / sampleRate;
}

#endif