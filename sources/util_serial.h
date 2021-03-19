#ifndef UTIL_SERIAL
#define UTIL_SERIAL

struct SerialHandle;

bool openHandle(const char * addr, SerialHandle * result);
bool closeHandle(SerialHandle * result);
bool isHandleOpened(const SerialHandle * target);

bool setBaudRate(SerialHandle * target, i32 rate);

bool clearSerialPort(SerialHandle * target);


int32 writeSerial(SerialHandle * target, const char * buffer, u32 length);
void writeSerialQuick(SerialHandle * target, const char * buffer, u32 length);
int32 readSerial(SerialHandle * module, char * buffer, u32 maxRead, f32 timeout = -1);

#endif