#ifndef WINDOWS_SERIAL
#define WINDOWS_SERIAL

#include <Windows.h>

struct SerialHandle{
    HANDLE handle;
};

bool openHandle(const char * addr, SerialHandle * result){
    HANDLE serial  = CreateFile(addr, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
    if(serial != INVALID_HANDLE_VALUE){
        result->handle = serial;
        return true;
    }
    return false;
}

bool setRate(SerialHandle * target, int32 rate){
    DCB settings;
    if(GetCommState(target->handle, &settings)){
        if(settings.BaudRate != rate){
            settings.BaudRate = rate;
            bool result = SetCommState(target->handle, &settings);
            return result;
        }
        return true;
    }
    return false;
}

bool clearSerialPort(SerialHandle * target){
    return PurgeComm(target->handle, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
}

#endif