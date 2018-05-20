#ifndef UTIL_SERIAL
#define UTIL_SERIAL

struct SerialHandle;

bool openHandle(const char * addr, SerialHandle * result);

bool setRate(SerialHandle * target, int32 rate);

bool clearSerialPort(SerialHandle * target);



#endif