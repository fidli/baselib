#ifndef UTIL_STRING
#define UTIL_STRING

#include <stdarg.h>
#include "common.h"
#include "util_math.cpp"

int32 strcmp(const char * a, const char * b){
    int32 result = 0;
    for(uint32 index = 0; ;index++){
        
        result = a[index] - b[index];
        if(result){
            return result;
        }
        
        if(a[index] == '\0')
            break;
    } 
    
    return result;
}

int32 strncmp(const char * a, const char * b, nint maxlen){
    int32 result = 0;
    for(uint32 index = 0; index < maxlen;index++){
        
        result = a[index] - b[index];
        if(result){
            return result;
        }
        
        if(a[index] == '\0')
            break;
    } 
    
    return result;
}

char * strncpy(char * target, const char * source, nint limit){
    uint32 i = 0;
    do{
        target[i] = source[i];
        i++;
    }while(i < limit);
    return target;
    
}

char * strcpy(char * target, const char * source){
    uint32 i = 0;
    do{
        target[i] = source[i];
        i++;
    }while(source[i-1] != '\0');
    return target;
}

nint strlen(const char * source){
    nint length = 0;
    while(source[length] != '\0'){
        length++;
    }
    return length;
}

char * strcat(char * first, const char * second){
    uint32 index = 0;
    while(first[index] != '\0'){
        first[index] = first[index];
        index++;
    }
    uint32 index2 = 0;
    while(second[index2] != '\0'){
        first[index + index2] = second[index2];
        index2++;
    }
    first[index + index2] = '\0';
    return first;
}



//todo: overflow control + return 0 on overflow
static uint8 scanNumber16(const char * source, int16 * target, uint8 maxDigits = 5){
    ASSERT(maxDigits <= 5);
    uint8 i = 0;
    bool first = true;
    bool negative = false;
    if(source[i] == '-'){
        negative = true;
        i++;
    }
    for(;source[i] != '\0' && i < maxDigits; i++){
        int8 digit = (int8) source[i];
        if(digit < 48 || digit > 57) break;
        digit -= 48;
        if(first){
            first = false;
            *target = 0;
        }
        *target = 10 * *target + digit; 
    }
    if(negative && i == 1){//only -
        return 0; 
    }
    if(negative){
        *target = -1 * *target;
    }
    return i;
}

static uint8 scanUnumber8(const char * source, uint8 * target, uint8 maxDigits = 3){
    ASSERT(maxDigits <= 5);
    uint8 i = 0;
    bool first = true;
    for(;source[i] != '\0' && i < maxDigits; i++){
        int8 digit = (int8) source[i];
        if(digit < 48 || digit > 57) break;
        digit -= 48;
        if(first){
            first = false;
            *target = 0;
        }
        *target = 10 * *target + digit; 
    }
    return i;
}

static uint8 scanUnumber16(const char * source, uint16 * target, uint8 maxDigits = 5){
    ASSERT(maxDigits <= 5);
    uint8 i = 0;
    bool first = true;
    for(;source[i] != '\0' && i < maxDigits; i++){
        int8 digit = (int8) source[i];
        if(digit < 48 || digit > 57) break;
        digit -= 48;
        if(first){
            first = false;
            *target = 0;
        }
        *target = 10 * *target + digit; 
    }
    return i;
}


//todo: overflow control + return 0 on overflow
static uint8 scanNumber(const char * source, int32 * target, uint8 maxDigits = 10){
    ASSERT(maxDigits <= 10);
    uint8 i = 0;
    bool first = true;
    bool negative = false;
    if(source[i] == '-'){
        negative = true;
        i++;
    }
    for(;source[i] != '\0' && i < maxDigits; i++){
        int8 digit = (int8) source[i];
        
        if(digit < 48 || digit > 57) break;
        digit -= 48;
        if(first){
            first = false;
            *target = 0;
        }
        *target = 10 * *target + digit; 
    }
    if(negative && i == 1){//only -
        return 0; 
    }
    if(negative){
        *target = -1 * *target;
    }
    return i;
}

static uint8 scanUnumber(const char * source, uint32 * target, uint8 maxDigits = 10){
    ASSERT(maxDigits <= 10);
    uint8 i = 0;
    bool first = true;
    for(;source[i] != '\0' && i < maxDigits; i++){
        int8 digit = (int8) source[i];
        if(digit < 48 || digit > 57) break;
        digit -= 48;
        if(first){
            first = false;
            *target = 0;
        }
        *target = 10 * *target + digit; 
    }
    return i;
}


static uint8 printDigits(char * target, int64 number){
    char digitsStack[20];
    int8 stackSize = 0;
    int64 temp = ABS(number);
    do{
        digitsStack[stackSize] = char((temp % 10) + 48);
        temp /= 10;
        stackSize++;
    } while(temp != 0);
    stackSize--;
    
    uint8 i = 0;
    if(number < 0){
        target[i++] = '-';
    }
    for(;stackSize >= 0; stackSize--){
        target[i++] = digitsStack[stackSize];
    }
    return i;
}

enum FormatTypeSize{
    FormatTypeSize_Default,
    FormatTypeSize_h,
    FormatTypeSize_l
};

enum FormatType{
    FormatType_Invalid,
    FormatType_d,
    FormatType_u,
    FormatType_c,
    FormatType_charlist,
    FormatType_s,
    FormatType_f,
    FormatType_immediate
};

struct FormatInfo{
    bool dryRun;
    uint32 maxlen;
    FormatType type;
    FormatTypeSize typeLength;
    union{
        struct{
            bool inverted;
            char digitRangeLow;
            char digitRangeHigh;
            char smallLetterRangeLow;
            char smallLetterRangeHigh;
            char capitalLetterRangeLow;
            char capitalLetterRangeHigh;
            const char * charlist[4];
            uint8 charlistLengths[4];
            uint8 charlistCount;
        } charlist;
        struct{
            char * start;
            uint32 length;
        } immediate;
        struct {
            uint8 precision;
        } real;
    };
    uint32 length;
};



static inline bool isDigit19(const char digit){
    return digit >= '1' && digit <= '9';
}


//unsafe, format must be 0 terminated
static FormatInfo parseFormat(const char * format){
    uint32 formatIndex = 0;
    FormatInfo info = {};
    uint32 localMaxread = -1;
    //%... or immediate
    if(format[formatIndex] == '%'){
        formatIndex++;
        
        
        info.dryRun = false;
        info.maxlen = 0;
        info.typeLength = FormatTypeSize_Default;
        
        if(format[formatIndex] == '*'){
            info.dryRun = true;
            formatIndex++;
        }
        if(isDigit19(format[formatIndex])){
            formatIndex += scanUnumber(&format[formatIndex], &info.maxlen);
        }
        
        if(format[formatIndex] == 'h'){
            info.typeLength = FormatTypeSize_h;
            formatIndex++;
        }
        
        //type
        if(format[formatIndex] == 'd'){
            info.type = FormatType_d;
            formatIndex++;
        }else if(format[formatIndex] == 'u'){
            info.type = FormatType_u;
            formatIndex++;
        }else if(format[formatIndex] == 'c'){
            info.type = FormatType_c;
            info.maxlen = 1;
            formatIndex++;
        }else if(format[formatIndex] == 's'){
            info.type = FormatType_s;
            formatIndex++;
        }else if(format[formatIndex] == '['){
            info.type = FormatType_charlist;
            formatIndex++;
            info.charlist.inverted = false;
            if(format[formatIndex] == '^'){
                info.charlist.inverted = true;
                formatIndex++;
            }
            uint8 charlistLen = 0;
            for(;format[formatIndex] != ']'; formatIndex++){
                ASSERT(format[formatIndex] != '\0');
                if(format[formatIndex+1] == '-'){
                    
                    if(charlistLen > 0){
                        info.charlist.charlistLengths[info.charlist.charlistCount] = charlistLen;
                        charlistLen = 0;
                    }
                    
                    if(format[formatIndex] >= 'A' && format[formatIndex] <= 'Z'){
                        info.charlist.capitalLetterRangeLow = format[formatIndex];                            
                    }else if(format[formatIndex] >= 'a' && format[formatIndex] <= 'z'){
                        info.charlist.smallLetterRangeLow = format[formatIndex];                            
                    }else if(format[formatIndex] >= 'a' && format[formatIndex] <= 'z'){
                        info.charlist.digitRangeLow = format[formatIndex];
                    }else{
                        ASSERT(!"fuck");
                    }
                    
                    formatIndex += 2;
                    
                    if(format[formatIndex] >= 'A' && format[formatIndex] <= 'Z'){
                        info.charlist.capitalLetterRangeHigh = format[formatIndex];                            
                        ASSERT(info.charlist.capitalLetterRangeHigh > info.charlist.capitalLetterRangeLow);
                    }else if(format[formatIndex] >= 'a' && format[formatIndex] <= 'z'){
                        info.charlist.smallLetterRangeHigh = format[formatIndex];
                        ASSERT(info.charlist.capitalLetterRangeHigh > info.charlist.capitalLetterRangeLow);
                    }else if(format[formatIndex] >= 'a' && format[formatIndex] <= 'z'){
                        info.charlist.digitRangeHigh = format[formatIndex];
                        ASSERT(info.charlist.capitalLetterRangeHigh > info.charlist.capitalLetterRangeLow);
                    }else{
                        ASSERT(!"fuck");
                    }
                    
                }else{
                    
                    if(charlistLen == 0){
                        info.charlist.charlistCount++;
                        info.charlist.charlist[info.charlist.charlistCount-1] = &format[formatIndex];
                    }
                    info.charlist.charlistLengths[info.charlist.charlistCount-1]++;
                    charlistLen++;
                    
                }
            }
            formatIndex++;//jumping over ']'
        }else if(format[formatIndex] == 'f' || format[formatIndex] == '.'){
            
            info.real.precision = 6;
            
            if(format[formatIndex] == '.'){
                formatIndex++;
                formatIndex += scanUnumber8(format + formatIndex, &info.real.precision);
            }
            if(format[formatIndex] == 'f'){
                info.type = FormatType_f;
            }
            formatIndex++;
        }else{
            ASSERT(!"fuck");
        } 
        
    }else{
        //immediate
        info.immediate.start = (char *)&format[formatIndex];
        while(format[formatIndex] != '\0' && format[formatIndex] != '%'){
            formatIndex++;
        }
        info.immediate.length = formatIndex;
        if(formatIndex > 0){
            info.type = FormatType_immediate;
        }
    }
    info.length = formatIndex;
    
    return info;
}
//returns printed entities, not printed characters, when needed, change
uint32 printFormatted(char * target, const char * format, va_list ap){
    
    uint32 maxprint = -1;
    uint32 targetIndex = 0;
    uint32 formatOffset = 0;
    FormatInfo info;
    uint32 successfullyPrinted = 0;
    while((info = parseFormat(format + formatOffset)).type != FormatType_Invalid){
        formatOffset += info.length;
        
        switch(info.type){
            case FormatType_c:
            case FormatType_s:{
                char * source;
                if(info.type == FormatType_c){
                    source = &va_arg(ap, char);
                }else{
                    source = va_arg(ap, char*);
                }
                //set proper max length to avoid buffer overflow
                ASSERT(info.maxlen != 0); 
                uint32 i = 0;
                for(; targetIndex + i < maxprint && i < info.maxlen && source[i] != '\0'; i++){
                    if(!info.dryRun){
                        target[targetIndex + i] = source[i];
                    }
                }
                if(!info.dryRun){
                    targetIndex += i;
                }
                successfullyPrinted++;
            }break;
            case FormatType_d:
            case FormatType_u:{
                ASSERT(!info.dryRun);
                if(info.type == FormatType_u){
                    if(info.typeLength == FormatTypeSize_Default){
                        uint32 source = va_arg(ap, uint32);
                        uint8 maxDigits = 10;
                        if(info.maxlen != 0){
                            maxDigits = info.maxlen;
                        }
                        targetIndex += printDigits(target + targetIndex, source);
                        successfullyPrinted++;
                    }else if (info.typeLength == FormatTypeSize_h){
                        uint16 source = va_arg(ap, uint16);
                        uint8 maxDigits = 5;
                        if(info.maxlen != 0){
                            maxDigits = info.maxlen;
                        }
                        targetIndex += printDigits(target + targetIndex, source);
                        successfullyPrinted++;
                    }else{
                        ASSERT(!"fuk");
                        return -1;
                    }
                }else if(info.type == FormatType_d){
                    if(info.typeLength == FormatTypeSize_Default){
                        int32 source = va_arg(ap, int32);
                        uint8 maxDigits = 10;
                        if(info.maxlen != 0){
                            maxDigits = info.maxlen;
                        }
                        targetIndex += printDigits(target + targetIndex, source);
                        successfullyPrinted++;
                    }else if (info.typeLength == FormatTypeSize_h){
                        int16 source = va_arg(ap, int16);
                        uint8 maxDigits = 5;
                        if(info.maxlen != 0){
                            maxDigits = info.maxlen;
                        }
                        targetIndex += printDigits(target + targetIndex, source);
                        successfullyPrinted++;
                    }else{
                        ASSERT(!"fuk");
                        return -1;
                    }
                }
            }break;
            case FormatType_charlist:{
                ASSERT(!"doesnt make sense, maybe is immediate?");
                return -1;
            }break;
            case FormatType_immediate:{
                if(!info.dryRun){
                    uint32 i = 0;
                    for(;i < info.immediate.length; i++){
                        target[targetIndex + i] = info.immediate.start[i];
                    }
                    targetIndex += i;
                }
                
                //successfullyPrinted++;
            }break;
            case FormatType_f:{
                char delim = '.';
                if(info.typeLength == FormatTypeSize_Default){
                    //float is promoted to double,...
                    float32 source = (float32)va_arg(ap, float64);
                    int32 wholePart = (int32) source;
                    if(wholePart == 0 && source < 0){
                        target[targetIndex] = '-';
                        targetIndex++;
                    }
                    uint8 numlength = numlen(wholePart);
                    if(info.maxlen == 0 || numlength + info.real.precision + 1 <= info.maxlen){
                        targetIndex += printDigits(target + targetIndex, wholePart);
                        target[targetIndex] = delim;
                        
                        targetIndex++;
                        uint8 precision = info.real.precision;
                        
                        uint32 decimalPart = ABS((int32)((source - wholePart) * powd(10, precision)));
                        uint8 prependLen = precision - numlen(decimalPart);
                        for(int i = 0; i < prependLen; i++){
                            target[targetIndex] = '0';
                            targetIndex++;
                        }
                        targetIndex += printDigits(target + targetIndex, decimalPart);
                        successfullyPrinted++;
                    }
                }else if(info.typeLength == FormatTypeSize_l){
                    //implement me
                    ASSERT(false);
                    return -1;
                }
            }break;
            default:{
                ASSERT(!"fuck");
                return -1;
            }break;
        }
    }
    if(successfullyPrinted != 0){
        target[targetIndex] = '\0';
    }
    return successfullyPrinted;
}



uint32 scanFormatted(const char * source, const char * format, va_list ap){
    uint32 formatOffset = 0;
    uint32 sourceIndex = 0;
    uint32 successfullyScanned = 0;
    uint32 maxread = (uint32) -1;
    FormatInfo info;
    bool exit = false;
    while(!exit && (info = parseFormat(format + formatOffset)).type != FormatType_Invalid){
        formatOffset += info.length;
        
        uint32 scannedChars = 0;
        
        switch(info.type){
            case FormatType_c:
            case FormatType_s:{
                char * targetVar = va_arg(ap, char *);
                //set proper max length to avoid buffer overflow
                ASSERT(info.maxlen != 0); 
                bool first = true;
                uint32 i = 0;
                for(; sourceIndex < maxread && i < info.maxlen; sourceIndex++, i++){
                    if(first) first = false;
                    if(!info.dryRun)
                        *(targetVar+i) = source[sourceIndex];
                    
                }
                if(!first)
                    successfullyScanned++;
                if(info.type == FormatType_s)
                    if(!info.dryRun)
                    targetVar[i] = '\0';
            }break;
            case FormatType_d:
            case FormatType_u:{
                ASSERT(!info.dryRun);
                if(info.type == FormatType_u){
                    if(info.typeLength == FormatTypeSize_Default){
                        uint32 * targetVar = va_arg(ap, uint32 * );
                        uint8 maxDigits = 10;
                        if(info.maxlen != 0){
                            maxDigits = info.maxlen;
                        }
                        scannedChars = scanUnumber(source + sourceIndex, (uint32 *) targetVar, maxDigits);
                    }else if (info.typeLength == FormatTypeSize_h){
                        uint16 * targetVar = va_arg(ap, uint16 * );
                        uint8 maxDigits = 5;
                        if(info.maxlen != 0){
                            maxDigits = info.maxlen;
                        }
                        scannedChars = scanUnumber16(source + sourceIndex, (uint16 *) targetVar, maxDigits);
                    }else{
                        ASSERT(!"fuk");
                    }
                }else if(info.type == FormatType_d){
                    if(info.typeLength == FormatTypeSize_Default){
                        int32 * targetVar = va_arg(ap, int32 * );
                        uint8 maxDigits = 10;
                        if(info.maxlen != 0){
                            maxDigits = info.maxlen;
                        }
                        scannedChars = scanNumber(source + sourceIndex, (int32 *) targetVar, maxDigits);
                    }else if (info.typeLength == FormatTypeSize_h){
                        int16 * targetVar = va_arg(ap, int16 * );
                        uint8 maxDigits = 5;
                        if(info.maxlen != 0){
                            maxDigits = info.maxlen;
                        }
                        scannedChars = scanNumber16(source + sourceIndex, (int16 *) targetVar, maxDigits);
                    }else{
                        ASSERT(!"fuk");
                    }
                }
                
                if(scannedChars > 0){
                    successfullyScanned++;
                    sourceIndex += scannedChars;
                }
                
            }break;
            case FormatType_charlist:{
                char * targetVar = va_arg(ap, char *);
                bool scanned = false;
                uint32 i = 0;
                for(; sourceIndex < maxread && i < info.maxlen; sourceIndex++, i++){
                    if(info.charlist.digitRangeLow != '\0'){
                        if(source[sourceIndex] >= info.charlist.digitRangeLow && source[sourceIndex] <= info.charlist.digitRangeHigh && !info.charlist.inverted){
                            if(!info.dryRun) targetVar[i] = source[sourceIndex];
                            scanned = true;
                            continue;               
                        }else if((source[sourceIndex] < info.charlist.digitRangeLow && source[sourceIndex] > info.charlist.digitRangeHigh) && info.charlist.inverted){
                            if(!info.dryRun) targetVar[i] = source[sourceIndex];
                            scanned = true;
                            continue;                            
                        }
                    }
                    if(info.charlist.capitalLetterRangeLow != '\0'){
                        if(source[sourceIndex] >= info.charlist.capitalLetterRangeLow && source[sourceIndex] <= info.charlist.capitalLetterRangeHigh && !info.charlist.inverted){
                            if(!info.dryRun) targetVar[i] = source[sourceIndex];
                            scanned = true;
                            continue;               
                        }else if((source[sourceIndex] < info.charlist.capitalLetterRangeLow && source[sourceIndex] > info.charlist.capitalLetterRangeHigh) && info.charlist.inverted){
                            if(!info.dryRun) targetVar[i] = source[sourceIndex];
                            scanned = true;
                            continue;                            
                        }
                    }
                    if(info.charlist.smallLetterRangeLow != '\0'){
                        if(source[sourceIndex] >= info.charlist.smallLetterRangeLow && source[sourceIndex] <= info.charlist.smallLetterRangeHigh && !info.charlist.inverted){
                            if(!info.dryRun) targetVar[i] = source[sourceIndex];
                            scanned = true;
                            continue;               
                        }else if((source[sourceIndex] < info.charlist.smallLetterRangeLow || source[sourceIndex] > info.charlist.smallLetterRangeHigh) && info.charlist.inverted){
                            if(!info.dryRun) targetVar[i] = source[sourceIndex];
                            scanned = true;
                            continue;                            
                        }                        
                    }
                    
                    bool found = false;
                    for(int charlistIndex = 0; charlistIndex < info.charlist.charlistCount; charlistIndex++){
                        for(int charIndex = 0; charIndex < info.charlist.charlistLengths[charlistIndex]; charIndex++){
                            if(source[sourceIndex] == info.charlist.charlist[charlistIndex][charIndex]){
                                found = true;
                                break;
                            }
                            
                        }
                        if(found){
                            break;
                        }
                    }
                    
                    
                    if(!info.charlist.inverted && found){
                        if(!info.dryRun) targetVar[i] = source[sourceIndex];
                        scanned = true;
                    }else if(info.charlist.inverted && !found){
                        if(!info.dryRun) targetVar[i] = source[sourceIndex];
                        scanned = true;
                    }
                    
                    
                    if(!info.charlist.inverted && !found){
                        i++;
                        break;    
                    }else if(info.charlist.inverted && found){
                        i++;
                        break;
                    }
                    
                }
                if(scanned)
                    successfullyScanned++;
                if(!info.dryRun){ 
                    if(i == info.maxlen){
                        targetVar[i-1] = '\0';
                    }else{
                        targetVar[i] = '\0';
                    }
                }
            }break;
            case FormatType_immediate:{
                uint32 i;
                for(i = 0; i < info.immediate.length; i++, sourceIndex++){
                    if(source[sourceIndex] != info.immediate.start[i]){
                        break;
                    }
                }
                if(i != info.immediate.length){
                    exit = true;
                }
            }break;
            default:{
                ASSERT(!"fuck");
            }break;
        }
        
    }
    
    
    
    return successfullyScanned;
}


uint32 sprintf(char * target, const char * format, ...){
    va_list ap;    
    va_start(ap, format);
    uint32 successfullyPrinted = printFormatted(target, format, ap);
    va_end(ap);
    return successfullyPrinted;
}

uint32 sscanf(const char * target, const char * format, ...){
    va_list ap;    
    va_start(ap, format);
    uint32 successfullyScanned  = scanFormatted(target, format, ap);
    va_end(ap);
    return successfullyScanned;
}



#endif