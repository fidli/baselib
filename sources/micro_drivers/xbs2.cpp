#ifndef XBS2
#define XBS2

#include "util_time.h"

#include "util_serial.h"

struct XBS2InitSettings{
    bool prepareForBroadcast;
};

struct XBS2Handle : SerialHandle{
    char sidLower[9];
    f32 guardTime;
    u32 baudrate;
    u16 frequency;
    char channel[3];
    char pan[5];
};


void wait(f32 seconds){
    f32 start = getProcessCurrentTime();
    while(aseqr(getProcessCurrentTime() - start, seconds, 0.00005f)){}
}

static i32 xbs2_sendByte(XBS2Handle * module, const char byteContents){
    return writeSerial(module, &byteContents, 1);
}

static void xbs2_sendByteQuick(XBS2Handle * module, const char byteContents){
    writeSerialQuick(module, &byteContents, 1);
}


static i32 xbs2_sendMessage(XBS2Handle * module, const char * buffer){
    return writeSerial(module, buffer, strlen(buffer));
}

static void xbs2_sendMessageQuick(XBS2Handle * module, const char * buffer){
    writeSerialQuick(module, buffer, strlen(buffer));
}

static void waitForMessage(XBS2Handle * module, char * responseBuffer, const char * message, f32 timeout = -1){
    i32 offset = 0;
    u32 msglen = strlen(message);
    while(offset < msglen || strncmp(responseBuffer + offset - 3, message, msglen)){
        offset += readSerial(module, responseBuffer + offset, 70 - offset, timeout);
    }
    
}

i32 xbs2_waitForAnyMessage(XBS2Handle * module, char * responseBuffer, u32 bufferLength, f32 timeout = -1){
    i32 offset = 0;
    if(timeout == -1){
        while((offset == 0 || responseBuffer[offset-1] != '\r') && offset < bufferLength){
            offset += readSerial(module, responseBuffer + offset, 1, timeout);
        }
    }else{
        while((offset == 0 || responseBuffer[offset-1] != '\r') && offset < bufferLength){
            i32 res = readSerial(module, responseBuffer + offset, 1, timeout);
            if(res <= 0) return offset;
            offset += res;
        }
        
    }
    return offset;
}

i32 xbs2_waitForAnyByte(XBS2Handle * module, char * response, f32 timeout = -1){
    return readSerial(module, response, 1, timeout);
}


static bool xbs2_enterCommandMode(XBS2Handle * module){
    //enter command mode
    //a 1 second pause [GT (Guard Times) parameter]
    //b "+++"
    //c second pause
    
    char result[7] = {};
    wait(module->guardTime);
    if(xbs2_sendMessage(module, "+++") != 3) return false;
    wait(module->guardTime);
    xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result), 1.1f);
    if(!strncmp("OK\r", result, 3)){
        return true;
    }
    return false;
}

static void xbs2_enterCommandModeQuick(XBS2Handle * module){
    wait(module->guardTime);
    xbs2_sendMessageQuick(module, "+++");
    wait(module->guardTime);
}


static bool xbs2_exitCommandMode(XBS2Handle * module){
    char result[7] = {};
    if(xbs2_sendMessage(module, "ATCN\r") != 5) return false;
    xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result), 1.1f);
    if(!strncmp("OK\r", result, 3)){
        return true;
    }
    return false;
}

static void xbs2_exitCommandModeQuick(XBS2Handle * module){
    xbs2_sendMessageQuick(module, "ATCN\r");
}

bool xbs2_detectAndSetStandardBaudRate(XBS2Handle * module){
    module->guardTime = 1.1f;
    //sorted by default/popularity?
    u32 rates[] = {9600, 115200,  19200, 38400, 57600, 4800, 2400, 1200};
    for(u8 rateIndex = 0; rateIndex < ARRAYSIZE(rates); rateIndex++){
        if(setBaudRate(module, rates[rateIndex])){
            if(clearSerialPort(module)){
                if(xbs2_enterCommandMode(module)){
                    module->baudrate = rates[rateIndex];
                    if(!xbs2_exitCommandMode(module)){
                        return false;
                    }
                    return true;
                }
                //command mode timeout
                wait(10);
                clearSerialPort(module);
            }
        }
    }
    return false;
}

bool xbs2_transmitByte(XBS2Handle * source, const char byteContents){
    return xbs2_sendByte(source, byteContents);
}

void xbs2_transmitByteQuick(XBS2Handle * source, const char byteContents){
    xbs2_sendByteQuick(source, byteContents);
}

bool xbs2_changeAddress(XBS2Handle * source, const char * lowerAddress){
    if(xbs2_enterCommandMode(source)){
        char buff[14];
        sprintf(buff, "ATDL%8s\r", lowerAddress);
        char result[20];
        if((xbs2_sendMessage(source, buff)  && xbs2_waitForAnyMessage(source, result, ARRAYSIZE(result)) > 0 && !strncmp("OK\r", result, 3))){
            return xbs2_exitCommandMode(source);
        }
    }
    xbs2_exitCommandModeQuick(source);
    return false;
    
}

void xbs2_changeAddressQuick(XBS2Handle * source, const char * lowerAddress){
    xbs2_enterCommandModeQuick(source);
    char buff[14];
    sprintf(buff, "ATDL%8s\r", lowerAddress);
    xbs2_sendMessageQuick(source, buff);
    xbs2_exitCommandModeQuick(source);
}


bool xbs2_transmitMessage(XBS2Handle * source,  const char * message){
    return xbs2_sendMessage(source, message);
}

bool xbs2_initNetwork(XBS2Handle * module, const char * channelMask = "1FFE"){
    if(xbs2_enterCommandMode(module)){
        //reset nework defaults
        char result[70] = {};
        bool success = true;
        
        char command[10];
        success = success && sprintf(command, "ATSC%4s\r", channelMask) != -1 && xbs2_sendMessage(module, command)  && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && !strncmp("OK\r", result, 3);
        
        success = success && xbs2_sendMessage(module, "ATNR0\r")  && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && !strncmp("OK\r", result, 3);
        
        //wait for network restart
        wait(2);
        if(success && xbs2_enterCommandMode(module)){
            success = success && xbs2_sendMessage(module, "ATAI\r")  && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && !strncmp("0\r", result, 2); 
            
            // operating channel
            success = success && xbs2_sendMessage(module, "ATCH\r") && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && sscanf(result, "%5[^\r]", module->channel) == 1;
            if(success){
                /**
            bit flag (channel)
            0 (0x0B) 4 (0x0F) 8 (0x13) 12 (0x17)
                1 (0x0C) 5 (0x10) 9 (0x14) 13 (0x18)
                2 (0x0D) 6 (0x11) 10 (0x15) 14 (0x19)
                3 (0x0E) 7 (0x12) 11 (0x16) 15 (0x1A)
                
                */
                if(!strncmp(module->channel, "B", 2)){
                    module->frequency = 2405;
                }else if(!strncmp(module->channel, "C", 2)){
                    module->frequency = 2410;
                }else if(!strncmp(module->channel, "D", 2)){
                    module->frequency = 2415;
                }else if(!strncmp(module->channel, "E", 2)){
                    module->frequency = 2420;
                }else if(!strncmp(module->channel, "F", 2)){
                    module->frequency = 2425;
                }else if(!strncmp(module->channel, "10", 2)){
                    module->frequency = 2430;
                }else if(!strncmp(module->channel, "11", 2)){
                    module->frequency = 2435;
                }else if(!strncmp(module->channel, "12", 2)){
                    module->frequency = 2440;
                }else if(!strncmp(module->channel, "13", 2)){
                    module->frequency = 2445;
                }else if(!strncmp(module->channel, "14", 2)){
                    module->frequency = 2450;
                }else if(!strncmp(module->channel, "15", 2)){
                    module->frequency = 2455;
                }else if(!strncmp(module->channel, "16", 2)){
                    module->frequency = 2460;
                }else if(!strncmp(module->channel, "17", 2)){
                    module->frequency = 2465;
                }else if(!strncmp(module->channel, "18", 2)){
                    module->frequency = 2470;
                }else if(!strncmp(module->channel, "19", 2)){
                    module->frequency = 2475;
                }else if(!strncmp(module->channel, "1A", 2)){
                    module->frequency = 2480;
                }
            }
            //-----GET
            // pan id
            success = success && xbs2_sendMessage(module, "ATID\r") && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && sscanf(result, "%7[^\r]", module->pan) == 1;
            xbs2_exitCommandMode(module);
        }else{
            
            if(!success) xbs2_exitCommandMode(module);
            return false;
        }
        
        return success;
    }else{
        wait(10);
        return false;
    }
}

bool xbs2_getChannelMask(const char * channel, char * mask){
    /**
            bit flag (channel)
            0 (0x0B) 4 (0x0F) 8 (0x13) 12 (0x17)
                1 (0x0C) 5 (0x10) 9 (0x14) 13 (0x18)
                2 (0x0D) 6 (0x11) 10 (0x15) 14 (0x19)
                3 (0x0E) 7 (0x12) 11 (0x16) 15 (0x1A)
                
                */
    if(!strncmp(channel, "B", 2)){
        strcpy(mask, "0001");
    }else if(!strncmp(channel, "C", 2)){
        strcpy(mask, "0002");
    }else if(!strncmp(channel, "D", 2)){
        strcpy(mask, "0004");
    }else if(!strncmp(channel, "E", 2)){
        strcpy(mask, "0008");
    }else if(!strncmp(channel, "F", 2)){
        strcpy(mask, "0010");
    }else if(!strncmp(channel, "10", 2)){
        strcpy(mask, "0020");
    }else if(!strncmp(channel, "11", 2)){
        strcpy(mask, "0040");
    }else if(!strncmp(channel, "12", 2)){
        strcpy(mask, "0080");
    }else if(!strncmp(channel, "13", 2)){
        strcpy(mask, "0100");
    }else if(!strncmp(channel, "14", 2)){
        strcpy(mask, "0200");
    }else if(!strncmp(channel, "15", 2)){
        strcpy(mask, "0400");
    }else if(!strncmp(channel, "16", 2)){
        strcpy(mask, "0800");
    }else if(!strncmp(channel, "17", 2)){
        strcpy(mask, "1000");
    }else if(!strncmp(channel, "18", 2)){
        strcpy(mask, "2000");
    }else if(!strncmp(channel, "19", 2)){
        strcpy(mask, "4000");
    }else if(!strncmp(channel, "1A", 2)){
        strcpy(mask, "8000");
    }else{
        return false;
    }
    return true;
}

bool xbs2_readValues(XBS2Handle * module){
    //reading values
    
    char result[70] = {};
    
    bool success = true;
    
    if(xbs2_enterCommandMode(module)){
        
        //low factory address "ATSL\r"
        success = success && xbs2_sendMessage(module, "ATSL\r") && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && sscanf(result, "%9[^\r]", module->sidLower) == 1;
        
        xbs2_exitCommandMode(module);
        return success;
        
    }else{
        return false;
    }
}

bool xbs2_initModule(XBS2Handle * module, XBS2InitSettings * settings){
    
    module->guardTime = 1.1f;
    char result[70] = {};
    
    bool success = true;
    
    if(xbs2_enterCommandMode(module)){
        
        //reset factory defaults
        success = success && xbs2_sendMessage(module, "ATRE\r") && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && !strncmp("OK\r", result, 3);
        
        
        //reset power
        success = success && xbs2_sendMessage(module, "ATFR\r") && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && !strncmp("OK\r", result, 3);
        
        module->baudrate = 9600;
        success = success && setBaudRate(module, module->baudrate);
        
        if(success) wait(4); //after 2 seconds is reset, 2 seconds reserve
        
        if(success && xbs2_enterCommandMode(module)){
            //other settings are default and it seems fine
            
            //send bytes as they arrive
            success = success && xbs2_sendMessage(module, "ATRO0\r") && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && !strncmp("OK\r", result, 3);
            
            //disable rssi
            success = success && xbs2_sendMessage(module, "ATP00\r") && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && !strncmp("OK\r", result, 3);
            
            //disable network indicator diod
            success = success && xbs2_sendMessage(module, "ATD50\r") && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && !strncmp("OK\r", result, 3);
            
            //disable flow control flags
            success = success && xbs2_sendMessage(module, "ATD70\r") && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && !strncmp("OK\r", result, 3);
            
            //broadcast
            //broadcast hops = max 1
            success = success && xbs2_sendMessage(module, "ATBH1\r") && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && !strncmp("OK\r", result, 3);
            
            //broadcast timeout = 0
            success = success && xbs2_sendMessage(module, "ATAR0\r") && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && !strncmp("OK\r", result, 3);
            
            
            
            if(settings->prepareForBroadcast){
                //broadcast destination address, SH is 0, which is default
                success = success && xbs2_sendMessage(module, "ATDLFFFF\r") && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && !strncmp("OK\r", result, 3);
            }else{
                //carousel/unicast
                //destination high is constant, at least in this case
                success = success && xbs2_sendMessage(module, "ATDH13A200\r") && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && !strncmp("OK\r", result, 3);
                
            }
            
            /*
                        //api addresing - cluster
                        success = success && xbs2_sendMessage(module, "ATZA1\r") && xbs2_waitForAnyMessage(module, result) > 0 && !strncmp("OK\r", result, 3);
                        */
            
            
            /*
            //3ms guard time lesser is hardly achievable, sometimes it does not work
            success = success && xbs2_sendMessage(module, "ATGT003\r") && xbs2_waitForAnyMessage(module, result) > 0 && !strncmp("OK\r", result, 3);
            if(success) module->guardTime = 0.0031f;
            */
            
            
            //100ms 64
            success = success && xbs2_sendMessage(module, "ATGT64\r") && xbs2_waitForAnyMessage(module, result, ARRAYSIZE(result)) > 0 && !strncmp("OK\r", result, 3);
            if(success) module->guardTime = 0.110f;
            
            /*
            //baud rate 115200
            success = success && xbs2_sendMessage(module, "ATBD7\r") && xbs2_waitForAnyMessage(module, result) > 0 && !strncmp("OK\r", result, 3);
            if(success)
            {
                module->baudrate = 115200;
                xbs2_exitCommandMode(module);
                success = success && setBaudRate(module, module->baudrate);
                return success;
            }
            */
            
            
        }
        xbs2_exitCommandMode(module);
        return success;
    }else{
        return false;
    }
}

#endif