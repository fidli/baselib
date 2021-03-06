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

bool setBaudRate(SerialHandle * target, i32 rate){
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


i32 writeSerial(SerialHandle * target, const char * buffer, u32 length){
    DWORD written;
    OVERLAPPED result = {};
    result.Offset =  0xFFFFFFFF;
    result.OffsetHigh =  0xFFFFFFFF;
    DWORD writeOp = WriteFile(target->handle, buffer, length, NULL, &result);
    if(writeOp || (!writeOp && GetLastError() == ERROR_IO_PENDING)){
        if(GetOverlappedResultEx(target->handle, &result, &written, INFINITE, false) != 0){
            return written;
        }
        //cancel the result to avoid memory rewriting
        BOOL cancelRes = CancelIoEx(target->handle, &result);
        ASSERT(cancelRes || GetLastError() == ERROR_NOT_FOUND);
        return -1;
    }
    return -1;
    
}

//@Hack @Robustness make this dynamic?
OVERLAPPED trashPool[50];
int trashIndex = 0;

void writeSerialQuick(SerialHandle * target, const char * buffer, u32 length){
    OVERLAPPED * trash = &trashPool[trashIndex];
    trashIndex = (trashIndex + 1) % ARRAYSIZE(trashPool);
    trash->Offset =  0xFFFFFFFF;
    trash->OffsetHigh =  0xFFFFFFFF;
    WriteFile(target->handle, buffer, length, NULL, trash);
}


i32 readSerial(SerialHandle * source, char * buffer, u32 maxRead, f32 timeout){
    DWORD read;
    OVERLAPPED result = {};
    DWORD readOp = ReadFile(source->handle, buffer, maxRead, NULL, &result);
    if(timeout == -1){
        if(readOp || (!readOp && GetLastError() == ERROR_IO_PENDING)){
            if(GetOverlappedResultEx(source->handle, &result, &read, INFINITE, false) != 0){
                return read;
            }
            //cancel the result to avoid memory rewriting
            BOOL cancelRes = CancelIoEx(source->handle, &result);
            ASSERT(cancelRes || GetLastError() == ERROR_NOT_FOUND);
            return 0;
        }
    }else{
        if(readOp || (!readOp && GetLastError() == ERROR_IO_PENDING)){
            if(GetOverlappedResultEx(source->handle, &result, &read, (DWORD)(timeout*1000), false) != 0){
                return read;
            }
            //cancel the result to avoid memory rewriting
            BOOL cancelRes = CancelIoEx(source->handle, &result);
            ASSERT(cancelRes || GetLastError() == ERROR_NOT_FOUND);
            return 0;
        }
    }
    return -1;
}

bool isHandleOpened(const SerialHandle * target){
    return target->handle != NULL;
}

bool closeHandle(SerialHandle * target){
    bool result = CloseHandle(target->handle);
    if(result){
        target->handle = NULL;
    }
    return result;
}

#endif