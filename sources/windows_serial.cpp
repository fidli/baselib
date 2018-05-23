#ifndef WINDOWS_SERIAL
#define WINDOWS_SERIAL
#include "common.h"
#include "util_time.h"

struct SerialHandle{
    HANDLE handle;
};

bool openHandle(const char * addr, SerialHandle * result){
    HANDLE serial  = CreateFile(addr, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
    if(serial != INVALID_HANDLE_VALUE){
        DCB settings;
        ASSERT(GetCommState(serial, &settings));
        settings.BaudRate = 9600;
        settings.fBinary = 1;
        settings.fParity = 0;
        settings.fOutxCtsFlow = 0;
        settings.fOutxDsrFlow = 0;
        settings.fDtrControl = 0;
        settings.fDsrSensitivity = 0;
        settings.fTXContinueOnXoff = 0;
        settings.fOutX = 0;
        settings.fInX = 0;
        settings.fErrorChar = 0;
        settings.fRtsControl = 0;
        settings.fAbortOnError = 0;
        //settings.XonLim = 32768;
        //settings.XoffLim = 8196;
        settings.ByteSize = 8;
        settings.Parity = 0;
        settings.StopBits = ONESTOPBIT;
        settings.XonChar = 17;
        settings.XoffChar = 19;
        settings.ErrorChar = 0;
        settings.EofChar = 0;
        settings.EvtChar = 0;
        ASSERT(SetCommState(serial, &settings));
        result->handle = serial;
        return true;
    }
    return false;
}

bool setBaudRate(SerialHandle * target, int32 rate){
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


int32 writeSerial(SerialHandle * target, const char * buffer, uint32 length){
    DWORD written;
    OVERLAPPED result = {};
    DWORD writeOp = WriteFile(target->handle, buffer, length, NULL, &result);
    if(writeOp || (!writeOp && GetLastError() == ERROR_IO_PENDING)){
        while(!GetOverlappedResult(target->handle, &result, &written, false)){};
        return written;
    }
    return -1;
    
}

int32 readSerial(SerialHandle * source, char * buffer, uint32 maxRead, float32 timeout){
    DWORD read;
    OVERLAPPED result = {};
    DWORD readOp = ReadFile(source->handle, buffer, maxRead, NULL, &result);
    if(timeout == -1){
        if(readOp || (!readOp && GetLastError() == ERROR_IO_PENDING)){
            while(!GetOverlappedResult(source->handle, &result, &read, false)){};
            return read;
        }
    }else{
        if(readOp || (!readOp && GetLastError() == ERROR_IO_PENDING)){
            float32 start = getProcessCurrentTime();
            do{
                if(GetOverlappedResult(source->handle, &result, &read, false)){
                    return read;
                }
            }while(getProcessCurrentTime() - start < timeout);
            ASSERT(CancelIoEx(source->handle, &result));
            return 0;
        }
    }
    return -1;
}



#endif