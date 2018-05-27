#ifndef LINUX_SERIAL
#define LINUX_SERIAL

#include <termios.h>
#include <unistd.h>

#include "util_serial.h"
#include "linux_time.cpp"

struct SerialHandle{
    int handle;
};

bool openHandle(const char * addr, SerialHandle * result){
    result->handle = open(addr, O_RDWR | O_ASYNC);
    if(result->handle != -1){
        termios setting = {};
        if(tcgetattr(result->handle, &setting) == -1){
            closeHandle(result);
            return false;
        }
        setting.c_iflag = IGNBRK | CS8 | CLOCAL;
        setting.c_oflag = IGNBRK | CS8 | CLOCAL;
        
        //nonblocking
        setting.c_cc[VMIN] = 0;
        setting.c_cc[VTIME] = 0;
        
        if(tcsetattr(result->handle, TCSANOW, &setting) == -1)
        {
            closeHandle(result);
            return false;
        }
    }
    return result->handle != -1;
}

bool setBaudRate(SerialHandle * target, int32 rate){
    speed_t rateConst;
    if(rate == 50){
        rateConst = B50;
    }else if(rate == 75){
        rateConst = B75;
    }else if(rate == 110){
        rateConst = B110;
    }else if(rate == 134){
        rateConst = B134;
    }else if(rate == 150){
        rateConst = B150;
    }else if(rate == 200){
        rateConst = B200;
    }else if(rate == 300){
        rateConst = B300;
    }else if(rate == 600){
        rateConst = B600;
    }else if(rate == 1200){
        rateConst = B1200;
    }else if(rate == 1800){
        rateConst = B1800;
    }else if(rate == 2400){
        rateConst = B2400;
    }else if(rate == 4800){
        rateConst = B4800;
    }else if(rate == 9600){
        rateConst = B9600;
    }else if(rate == 19200){
        rateConst = B19200;
    }else if(rate == 38400){
        rateConst = B38400;
    }else if(rate == 57600){
        rateConst = B57600;
    }else if(rate == 115200){
        rateConst = B115200;
    }else if(rate == 230400){
        rateConst = B230400;
    }else{
        return false;
    }
    termios setting = {};
    if(tcgetattr(target->handle, &setting) == -1) return false;
    if(cfsetspeed(&setting, rateConst) == -1) return false;
    return tcsetattr(target->handle, TCSANOW, &setting) != -1;
}

bool clearSerialPort(SerialHandle * target){
    return tcflush(target->handle, TCIOFLUSH) != -1;
}


int32 writeSerial(SerialHandle * target, const char * buffer, uint32 length){
    ssize_t written = 0;
    while(written != length){
        ssize_t subRes = write(target->handle, buffer, length);
        if(subRes == -1 && errno != EAGAIN) return -1;
        written += subRes;
    }
    return written;
    
}

int32 readSerial(SerialHandle * source, char * buffer, uint32 maxRead, float32 timeout){
    ssize_t readBytes = 0;
    float32 startTime = getProcessCurrentTime();
    while(readBytes != maxRead){
        ssize_t subRes = read(source->handle, buffer, maxRead - readBytes);
        if(subRes == -1 && errno != EAGAIN) return -1;
        readBytes += subRes;
        if((getProcessCurrentTime() - startTime) > timeout) break;
    }
    return readBytes;
    
}

bool isHandleOpened(const SerialHandle * target){
    return target->handle != -1;
}

bool closeHandle(SerialHandle * target){
    if(close(target->handle) == 0){
        target->handle = -1;
        return true;
    }
    return false;
}


#endif

