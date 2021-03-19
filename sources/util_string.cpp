#ifndef UTIL_STRING
#define UTIL_STRING

#include <stdarg.h>
#include "common.h"
#include "util_math.cpp"

#ifndef CRT_PRESENT

int32 strcmp(const char * a, const char * b){
    i32 result = 0;
    for(u32 index = 0; ;index++){
        
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
    i32 result = 0;
    for(u32 index = 0; index < maxlen;index++){
        
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
    u32 i = 0;
    do{
        target[i] = source[i];
        i++;
    }while(source[i] != '\0' && i < limit);
    if(i < limit){
        target[i] = 0;
    }
    return target;
    
}

char * strcpy(char * target, const char * source){
    u32 i = 0;
    do{
        target[i] = source[i];
        i++;
    }while(source[i-1] != '\0');
    return target;
}

nint strnlen(const char * source, nint limit){
    nint length = 1;
    if(source){
        while(source[length-1] != '\0' && length-1 < limit){
            length++;
        }
    }
    return length - 1;
}

nint strlen(const char * source){
    nint length = 1;
    if(source){
        while(source[length-1] != '\0'){
            length++;
        }
    }
    return length-1;
}

const char * strstr(const char * data, const char * searchFor){
    i32 searchLen = strlen(searchFor);
    i32 dataLen = strlen(data);
    if(searchLen > dataLen){
        return NULL;
    }
    // TODO(fidli): more efficient way?
    for(i32 i = 0; i <= dataLen - searchLen; i++){
        if(!strncmp(data + i, searchFor, searchLen)){
            return data + i;
        }
    }
    return NULL;
}

char * strcat(char * first, const char * second){
    u32 index = 0;
    while(first[index] != '\0'){
        first[index] = first[index];
        index++;
    }
    u32 index2 = 0;
    while(second[index2] != '\0'){
        first[index + index2] = second[index2];
        index2++;
    }
    first[index + index2] = '\0';
    return first;
}

#endif

//todo: overflow control + return 0 on overflow
static u8 scanNumber16(const char * source, i16 * target, u8 maxDigits = 6){
    if(maxDigits > 6) maxDigits = 6; //cant fit more with sign
    u8 i = 0;
    bool first = true;
    bool negative = false;
    if(source[i] == '-'){
        negative = true;
        i++;
    }
    for(;source[i] != '\0' && i < maxDigits; i++){
        i8 digit = (int8) source[i];
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

static u8 scanNumber8(const char * source, i8 * target, u8 maxDigits = 3){
    if(maxDigits > 6) maxDigits = 6; //cant fit more with sign
    u8 i = 0;
    bool first = true;
    bool negative = false;
    if(source[i] == '-'){
        negative = true;
        i++;
    }
    for(;source[i] != '\0' && i < maxDigits; i++){
        i8 digit = (int8) source[i];
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

static u8 scanUnumber8(const char * source, u8 * target, u8 maxDigits = 3){
    if(maxDigits > 3) maxDigits = 3; //cant fit more
    u8 i = 0;
    bool first = true;
    for(;source[i] != '\0' && i < maxDigits; i++){
        i8 digit = (int8) source[i];
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

static u8 scanUnumber16(const char * source, u16 * target, u8 maxDigits = 5){
    if(maxDigits > 5) maxDigits = 5; //cant fit more
    u8 i = 0;
    bool first = true;
    for(;source[i] != '\0' && i < maxDigits; i++){
        i8 digit = (int8) source[i];
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
static u8 scanNumber(const char * source, i32 * target, u8 maxDigits = 11){
    if(maxDigits > 11) maxDigits = 11; //cant fit more into i32 with sign
    u8 i = 0;
    bool first = true;
    bool negative = false;
    if(source[i] == '-'){
        negative = true;
        i++;
    }
    for(;source[i] != '\0' && i < maxDigits; i++){
        i8 digit = (int8) source[i];
        
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

//todo: overflow control + return 0 on overflow
static u8 scanNumber64(const char * source, i64 * target, u8 maxDigits = 20){
    if(maxDigits > 20) maxDigits = 20; //cant fit more into i64 with sign
    u8 i = 0;
    bool first = true;
    bool negative = false;
    if(source[i] == '-'){
        negative = true;
        i++;
    }
    for(;source[i] != '\0' && i < maxDigits; i++){
        i8 digit = (int8) source[i];
        
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

static u8 scanUnumber(const char * source, u32 * target, u8 maxDigits = 10){
    if(maxDigits > 10) maxDigits = 10; //cant fit more
    u8 i = 0;
    bool first = true;
    for(;source[i] != '\0' && i < maxDigits; i++){
        i8 digit = (int8) source[i];
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


static u8 scanUnumber64(const char * source, u64 * target, u8 maxDigits = 20){
    if(maxDigits > 20) maxDigits = 20; //cant fit more
    u8 i = 0;
    bool first = true;
    for(;source[i] != '\0' && i < maxDigits; i++){
        i8 digit = (int8) source[i];
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


static u8 printDigits(char * target, i64 number){
    char digitsStack[20];
    i8 stackSize = 0;
    i64 temp = ABS(number);
    do{
        digitsStack[stackSize] = char((temp % 10) + 48);
        temp /= 10;
        stackSize++;
    } while(temp != 0);
    stackSize--;
    
    u8 i = 0;
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
    FormatTypeSize_hh,
    FormatTypeSize_l,
    FormatTypeSize_ll
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
    struct {
        bool dryRun;
    } scan;
    struct {
        bool varLen;
    } print;
    u32 width;
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
            u8 charlistLengths[4];
            u8 charlistCount;
        } charlist;
        struct{
            char * start;
            u32 length;
        } immediate;
        struct {
            u8 precision;
            bool defaultPrecision;
        } real;
    };
    u32 length;
    bool padding0;
    bool forceSign;
    bool leftJustify;
};




static inline bool isDigit19(const char digit){
    return digit >= '1' && digit <= '9';
}


//unsafe, format must be 0 terminated
static FormatInfo parseFormat(const char * format){
    u32 formatIndex = 0;
    FormatInfo info = {};
    u32 localMaxread = -1;
    //%... or immediate
    if(format[formatIndex] == '%'){
        formatIndex++;
        if(format[formatIndex] == '%'){
            //immediate
            info.immediate.start = (char *)&format[formatIndex];
            info.immediate.length = 1;
            info.type = FormatType_immediate;
            formatIndex++;
            info.length = formatIndex;
            return info;
        }
        info.scan.dryRun = false;
        info.print.varLen = false;
        info.width = -1;
        info.typeLength = FormatTypeSize_Default;
        info.padding0 = false;
        info.leftJustify = false;
        while(format[formatIndex] == '*' || format[formatIndex] == '+' || format[formatIndex] == '0' || format[formatIndex] == '-'){
            
            if(format[formatIndex] == '*'){
                info.scan.dryRun = true;
                info.print.varLen = true;
            }
            if(format[formatIndex] == '+'){
                info.forceSign = true;
            }
            if(format[formatIndex] == '0'){
                info.padding0 = true;
            }
            if(format[formatIndex] == '-'){
                info.leftJustify = true;
            }
            formatIndex++;
        }
        if(isDigit19(format[formatIndex])){
            formatIndex += scanUnumber(&format[formatIndex], &info.width);
        }
        
        if(format[formatIndex] == 'h'){
            info.typeLength = FormatTypeSize_h;
            formatIndex++;
        }else if(format[formatIndex] == 'l'){
            info.typeLength = FormatTypeSize_l;
            formatIndex++;
            if(format[formatIndex] == 'l'){
                info.typeLength = FormatTypeSize_ll;
                formatIndex++;
            }
        }
        
        if(info.typeLength == FormatTypeSize_h && format[formatIndex] == 'h'){
            info.typeLength = FormatTypeSize_hh;
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
            info.width = 1;
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
            u8 charlistLen = 0;
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
                    }else if(format[formatIndex] >= '0' && format[formatIndex] <= '9'){
                        info.charlist.digitRangeLow = format[formatIndex];
                    }else{
                        //INV; //implement me maybe? makes sense? i havent been in these depths for long
                    }
                    
                    formatIndex += 2;
                    
                    if(format[formatIndex] >= 'A' && format[formatIndex] <= 'Z'){
                        info.charlist.capitalLetterRangeHigh = format[formatIndex];                            
                        ASSERT(info.charlist.capitalLetterRangeHigh > info.charlist.capitalLetterRangeLow);
                    }else if(format[formatIndex] >= 'a' && format[formatIndex] <= 'z'){
                        info.charlist.smallLetterRangeHigh = format[formatIndex];
                        ASSERT(info.charlist.capitalLetterRangeHigh > info.charlist.capitalLetterRangeLow);
                    }else if(format[formatIndex] >= '0' && format[formatIndex] <= '9'){
                        info.charlist.digitRangeHigh = format[formatIndex];
                        ASSERT(info.charlist.digitRangeHigh > info.charlist.digitRangeLow);
                    }else{
                        //INV; //implement me maybe? makes sense?
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
            bool setPrecision = false;
            if(format[formatIndex] == '.'){
                formatIndex++;
                formatIndex += scanUnumber8(format + formatIndex, &info.real.precision);
                setPrecision = true;
            }
            if(format[formatIndex] == 'l'){
                info.typeLength = FormatTypeSize_l;
                formatIndex++;
            }
            if(format[formatIndex] == 'f'){
                info.type = FormatType_f;
            }
            formatIndex++;
            if(!setPrecision){
                if(info.typeLength == FormatTypeSize_l){
                    info.real.precision = 15;
                }else{
                    info.real.precision = 7;
                    ASSERT(info.typeLength == FormatTypeSize_Default);
                }
            }
            info.real.defaultPrecision = !setPrecision;
        }else{
            //INV; //implement me or genuine errer
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

//returns printed characters
int32 printPrepend(char * target, char value, i32 len){
    if(len > 0){
        memset(target, value, len);
        return len;
    }
    return 0;
}
uint32 printFormatted(u32 maxprint, char * target, const char * format, va_list ap){
    
    
    u32 targetIndex = 0;
    u32 formatOffset = 0;
    FormatInfo info;
	u32 successfullyPrinted = 0; //printed entities
    while((info = parseFormat(format + formatOffset)).type != FormatType_Invalid){
        formatOffset += info.length;
        if(info.print.varLen){
            i32 len = va_arg(ap, int32);
            info.width = len;
        }
        u32 previousTargetIndex = targetIndex;
        switch(info.type){
            case FormatType_c:
            case FormatType_s:{
                char * source;
                char sourceSlot;
                i32 toPrint = 0x7FFFFFFF;
                if(info.type == FormatType_c){
                    toPrint = 1;
                    sourceSlot = CAST(char, va_arg(ap, int));
                    source = &sourceSlot;
                }else{
                    source = va_arg(ap, char*);
                    if(!info.leftJustify){
                        toPrint = strlen(source);
                    }
                }
                if(!info.leftJustify){
                    targetIndex += printPrepend(target + targetIndex, ' ', info.width - toPrint);
                }
                //set proper max length to avoid buffer overflow
                u32 i = 0;
                for(; targetIndex + i < maxprint && i < toPrint && source[i] != '\0'; i++){
                    target[targetIndex + i] = source[i];
                }
                targetIndex += i;
                successfullyPrinted++;
            }break;
            case FormatType_d:
            case FormatType_u:{
                u64 absValue;
                bool negative = false;
                if(info.type == FormatType_u){
                    if(info.typeLength == FormatTypeSize_Default){
                        u32 source = va_arg(ap, uint32);
                        absValue = source;
                    }else if (info.typeLength == FormatTypeSize_h){
                        u16 source = CAST(uint16, va_arg(ap, int));
                        absValue = source;
                    }else if (info.typeLength == FormatTypeSize_hh){
                        u8 source = CAST(uint8, va_arg(ap, int));
                        absValue = source;
                    }else if(info.typeLength == FormatTypeSize_ll){
                        u64 source = va_arg(ap, uint64);
                        absValue = source;
                    }else{
                        return -1;
                    }
                }else if(info.type == FormatType_d){
                    if(info.typeLength == FormatTypeSize_Default){
                        i32 source = va_arg(ap, int32);
                        if(source < 0){
                            source *= -1;
                            negative = true;
                        }
                        absValue = source;
                    }else if (info.typeLength == FormatTypeSize_h){
                        i16 source = CAST(int16, va_arg(ap, int));
                        if(source < 0){
                            source *= -1;
                            negative = true;
                        }
                        absValue = source;
                    }else if (info.typeLength == FormatTypeSize_hh){
                        i8 source = CAST(int8, va_arg(ap, int));
                        if(source < 0){
                            source *= -1;
                            negative = true;
                        }
                        absValue = source;
                    }else if(info.typeLength == FormatTypeSize_ll){
                        i64 source = va_arg(ap, int64);
                        if(source < 0){
                            source *= -1;
                            negative = true;
                        }
                        absValue = source;
                    }else{
                        INV; //implement me or genuine error
                        return -1;
                    }
                }
                
                if(!info.leftJustify || info.padding0){
                    i8 prependLen = info.width - numlen(absValue);
                    if(info.forceSign || negative){
                        prependLen--;
                    }
                    if(info.padding0){
                        targetIndex += printPrepend(target + targetIndex, '0', prependLen);
                    }else{
                        targetIndex += printPrepend(target + targetIndex, ' ', prependLen);
                    }
                }
                if(info.forceSign && !negative){
                    target[targetIndex++] = '+';
                }else if(negative){
                    target[targetIndex++] = '-';
                }
                targetIndex += printDigits(target + targetIndex, absValue);
                successfullyPrinted++;
            }break;
            case FormatType_charlist:{
                ASSERT(!"doesnt make sense, maybe is immediate?");
                return -1;
            }break;
            case FormatType_immediate:{
                u32 i = 0;
                for(;i < info.immediate.length; i++){
                    target[targetIndex + i] = info.immediate.start[i];
                }
                targetIndex += i;
                //successfullyPrinted++;
            }break;
            case FormatType_f:{
                char delim = '.';
                //float is promoted to double,...
                f64 source = (float64)va_arg(ap, float64);
                i64 wholePart = (int64) source;
                
                u8 numlength = numlen(ABS(wholePart));
                bool negative = source < 0;
                
                if(numlength + info.real.precision + 1 <= info.width){
                    if(!info.leftJustify || info.padding0){
                        i8 prependLen = info.width - numlength;
                        if(info.forceSign || negative){
                            prependLen--;
                        }
                        if(info.padding0){
                            targetIndex += printPrepend(target + targetIndex, '0', prependLen);
                        }else{
                            targetIndex += printPrepend(target + targetIndex, ' ', prependLen);
                        }
                    }
                    if(info.forceSign && !negative){
                        target[targetIndex] = '+';
                        targetIndex++;
                    }else if(negative){
                        target[targetIndex] = '-';
                        targetIndex++;
                    }
                    targetIndex += printDigits(target + targetIndex, ABS(wholePart));
                    
                    target[targetIndex] = delim;
                    
                    targetIndex++;
                    u8 precision = info.real.precision;
                    
                    i64 decimalPart = ABS((int64)((source - wholePart) * powd64(10, precision)));
                    i32 prependLen = precision - numlen(decimalPart);
                    for(int i = 0; i < prependLen; i++){
                        target[targetIndex] = '0';
                        targetIndex++;
                    }
                    targetIndex += printDigits(target + targetIndex, decimalPart);
                    successfullyPrinted++;
                }
            }break;
            case FormatType_Invalid:
            default:{
                INV; //implement me or genuine errer
                return -1;
            }break;
        }
        u32 printedCharacters = targetIndex - previousTargetIndex;  
        if((info.leftJustify && info.width != (uint32)-1) && printedCharacters < info.width){
            for(i32 i = 0; i < info.width - printedCharacters; i++){
                target[targetIndex++] = ' ';
            }
        }
    }
    
    target[targetIndex] = '\0';
    
    return targetIndex;
}



uint32 scanFormatted(i32 limit, const char * source, const char * format, va_list ap){
    u32 formatOffset = 0;
    u32 sourceIndex = 0;
    u32 successfullyScanned = 0;
    u32 maxread = (uint32) limit;
    FormatInfo info;
    bool exit = false;
    while(!exit && (info = parseFormat(format + formatOffset)).type != FormatType_Invalid){
        formatOffset += info.length;
        
        u32 scannedChars = 0;
        
        switch(info.type){
            case FormatType_c:
            case FormatType_s:{
                char * targetVar = va_arg(ap, char *);
                //set proper max length to avoid buffer overflow
                ASSERT(info.width != 0); 
                bool first = true;
                u32 i = 0;
                for(; sourceIndex < maxread && i < info.width && source[sourceIndex] != '\0'; sourceIndex++, i++){
                    if(first) first = false;
                    if(!info.scan.dryRun)
                        *(targetVar+i) = source[sourceIndex];
                    
                }
                if(!first)
                    successfullyScanned++;
                if(info.type == FormatType_s)
                    if(!info.scan.dryRun)
                    targetVar[MIN(i, MIN(info.width, maxread) - 1)] = '\0';
            }break;
            case FormatType_d:
            case FormatType_u:{
                ASSERT(!info.scan.dryRun);
                if(info.type == FormatType_u){
                    if(info.typeLength == FormatTypeSize_Default){
                        u32 * targetVar = va_arg(ap, u32 * );
                        u8 maxDigits = 10;
                        if(info.width != 0){
                            maxDigits = info.width;
                        }
                        scannedChars = scanUnumber(source + sourceIndex, (u32 *) targetVar, maxDigits);
                    }else if (info.typeLength == FormatTypeSize_h){
                        u16 * targetVar = va_arg(ap, u16 * );
                        u8 maxDigits = 5;
                        if(info.width != 0){
                            maxDigits = info.width;
                        }
                        scannedChars = scanUnumber16(source + sourceIndex, (u16 *) targetVar, maxDigits);
                    }else if (info.typeLength == FormatTypeSize_ll){
                        u64 * targetVar = va_arg(ap, u64 * );
                        u8 maxDigits = 20;
                        if(info.width != 0){
                            maxDigits = info.width;
                        }
                        scannedChars = scanUnumber64(source + sourceIndex, (u64 *) targetVar, maxDigits);
                    }else if (info.typeLength == FormatTypeSize_hh){
                        u8 * targetVar = va_arg(ap, u8 * );
                        u8 maxDigits = 3;
                        if(info.width != 0){
                            maxDigits = info.width;
                        }
                        scannedChars = scanUnumber8(source + sourceIndex, (u8 *) targetVar, maxDigits);
                    }
                    else{
                        INV; //implement me or genuine error
                    }
                }else if(info.type == FormatType_d){
                    if(info.typeLength == FormatTypeSize_Default){
                        i32 * targetVar = va_arg(ap, i32 * );
                        u8 maxDigits = 11;
                        if(info.width != 0){
                            maxDigits = info.width;
                        }
                        scannedChars = scanNumber(source + sourceIndex, (i32 *) targetVar, maxDigits);
                    }else if (info.typeLength == FormatTypeSize_h){
                        i16 * targetVar = va_arg(ap, i16 * );
                        u8 maxDigits = 6;
                        if(info.width != 0){
                            maxDigits = info.width;
                        }
                        scannedChars = scanNumber16(source + sourceIndex, (i16 *) targetVar, maxDigits);
                    }else if (info.typeLength == FormatTypeSize_ll){
                        i64 * targetVar = va_arg(ap, i64 * );
                        u8 maxDigits = 20;
                        if(info.width != 0){
                            maxDigits = info.width;
                        }
                        scannedChars = scanNumber64(source + sourceIndex, (i64 *) targetVar, maxDigits);
                    }else if (info.typeLength == FormatTypeSize_hh){
                        i8 * targetVar = va_arg(ap, i8 * );
                        u8 maxDigits = 3;
                        if(info.width != 0){
                            maxDigits = info.width;
                        }
                        scannedChars = scanNumber8(source + sourceIndex, (i8 *) targetVar, maxDigits);
                    }
                    else{
                        INV; //implement me or genuine error
                    }
                }
                
                if(scannedChars > 0){
                    successfullyScanned++;
                    sourceIndex += scannedChars;
                }
                
            }break;
            case FormatType_f:{
                bool negative = source[sourceIndex] == '-';
                if(negative) sourceIndex++;
                switch(info.typeLength){
                    
                    case FormatTypeSize_Default:{
                        f32 * targetVar = va_arg(ap, f32 *);
                        
                        u8 maxDigits = 7;
                        if(info.width != 0){
                            maxDigits = info.width;
                        }
                        // TODO(fidli): scanReal()
                        i32 wholePart = 0;
                        scannedChars = scanNumber(source + sourceIndex, &wholePart, maxDigits);
                        *targetVar = (float32) wholePart;
                        sourceIndex += scannedChars;
                        if(source[sourceIndex] == '.'){
                            sourceIndex++;
                            u8 numlength = numlen(wholePart);
                            
                            i32 decimalPart = 0;
                            u8 partScannedChars = scanNumber(source + sourceIndex, &decimalPart, maxDigits);
                            
                            if(decimalPart != 0){
                                u8 prepended = 0;
                                for(;prepended < maxDigits; prepended++){
                                    if(source[sourceIndex + prepended] != '0') break;
                                }
                                u8 len = numlen(decimalPart) + prepended;
                                *targetVar += (float32)(decimalPart) / powd(10, len);
                            }
                            scannedChars += partScannedChars;
                            sourceIndex += partScannedChars;
                            
                        }
                        if(negative) *targetVar *= -1;
                    }break;
                    case FormatTypeSize_l:{
                        f64 * targetVar = va_arg(ap, f64 *);
                        u8 maxDigits = 15;
                        if(info.width != 0){
                            maxDigits = info.width;
                        }
                        i64 wholePart = 0;
                        scannedChars = scanNumber64(source + sourceIndex, &wholePart, maxDigits);
                        *targetVar = (float64) wholePart;
                        sourceIndex += scannedChars;
                        if(source[sourceIndex] == '.'){
                            sourceIndex++;
                            u8 numlength = numlen(wholePart);
                            
                            i64 decimalPart = 0;
                            u8 partScannedChars = scanNumber64(source + sourceIndex, &decimalPart, maxDigits);
                            
                            if(decimalPart != 0){
                                u8 prepended = 0;
                                for(;prepended < maxDigits; prepended++){
                                    if(source[sourceIndex + prepended] != '0') break;
                                }
                                u8 len = numlen(decimalPart) + prepended;
                                *targetVar += (float64)(decimalPart) / powd(10, len);
                            }
                            scannedChars += partScannedChars;
                            sourceIndex += partScannedChars;
                            
                        }
                        if(negative) *targetVar *= -1;
                    }break;
                    case FormatTypeSize_h:
                    case FormatTypeSize_hh:
                    case FormatTypeSize_ll:
                    default:{
                        INV;
                    };
                }
                
                if(scannedChars > 0){
                    successfullyScanned++;
                }
            }break;
            case FormatType_charlist:{
                char * targetVar = va_arg(ap, char *);
                bool scanned = false;
                u32 i = 0;
                for(; sourceIndex < maxread && i < info.width && source[sourceIndex] != '\0'; sourceIndex++, i++){
                    if(info.charlist.digitRangeLow != '\0'){
                        if(source[sourceIndex] >= info.charlist.digitRangeLow && source[sourceIndex] <= info.charlist.digitRangeHigh && !info.charlist.inverted){
                            if(!info.scan.dryRun) targetVar[i] = source[sourceIndex];
                            scanned = true;
                            continue;               
                        }else if((source[sourceIndex] < info.charlist.digitRangeLow && source[sourceIndex] > info.charlist.digitRangeHigh) && info.charlist.inverted){
                            if(!info.scan.dryRun) targetVar[i] = source[sourceIndex];
                            scanned = true;
                            continue;                            
                        }
                    }
                    if(info.charlist.capitalLetterRangeLow != '\0'){
                        if(source[sourceIndex] >= info.charlist.capitalLetterRangeLow && source[sourceIndex] <= info.charlist.capitalLetterRangeHigh && !info.charlist.inverted){
                            if(!info.scan.dryRun) targetVar[i] = source[sourceIndex];
                            scanned = true;
                            continue;               
                        }else if((source[sourceIndex] < info.charlist.capitalLetterRangeLow && source[sourceIndex] > info.charlist.capitalLetterRangeHigh) && info.charlist.inverted){
                            if(!info.scan.dryRun) targetVar[i] = source[sourceIndex];
                            scanned = true;
                            continue;                            
                        }
                    }
                    if(info.charlist.smallLetterRangeLow != '\0'){
                        if(source[sourceIndex] >= info.charlist.smallLetterRangeLow && source[sourceIndex] <= info.charlist.smallLetterRangeHigh && !info.charlist.inverted){
                            if(!info.scan.dryRun) targetVar[i] = source[sourceIndex];
                            scanned = true;
                            continue;               
                        }else if((source[sourceIndex] < info.charlist.smallLetterRangeLow || source[sourceIndex] > info.charlist.smallLetterRangeHigh) && info.charlist.inverted){
                            if(!info.scan.dryRun) targetVar[i] = source[sourceIndex];
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
                        if(!info.scan.dryRun) targetVar[i] = source[sourceIndex];
                        scanned = true;
                    }else if(info.charlist.inverted && !found){
                        if(!info.scan.dryRun) targetVar[i] = source[sourceIndex];
                        scanned = true;
                    }
                    
                    
                    if(!info.charlist.inverted && !found){
                        break;    
                    }else if(info.charlist.inverted && found){
                        break;
                    }
                    
                }
                if(scanned)
                    successfullyScanned++;
                if(!info.scan.dryRun){ 
                    if(i == info.width){
                        targetVar[i-1] = '\0';
                    }else{
                        targetVar[i] = '\0';
                    }
                }
            }break;
            case FormatType_immediate:{
                u32 i;
                for(i = 0; i < info.immediate.length; i++, sourceIndex++){
                    if(source[sourceIndex] != info.immediate.start[i]){
                        break;
                    }
                }
                if(i != info.immediate.length){
                    exit = true;
                }
            }break;
            case FormatType_Invalid:
            default:{
                INV; //implement me or genuine error
            }break;
        }
        
    }
    
    
    
    return successfullyScanned;
}

#ifndef CRT_PRESENT
uint32 vsnprintf(char * target, nint limit, const char * format, va_list ap){
	u32 successfullyPrinted = printFormatted(limit, target, format, ap);
	return successfullyPrinted;
}

uint32 snprintf(char * target, nint limit, const char * format, ...){
    va_list ap;    
    va_start(ap, format);
	u32 successfullyPrinted = vsnprintf(target, limit, format, ap);
    va_end(ap);
    return successfullyPrinted;
}
#endif

uint32 snscanf(const char * target, nint limit, const char * format, ...){
    va_list ap;    
    va_start(ap, format);
    u32 successfullyScanned  = scanFormatted(limit, target, format, ap);
    va_end(ap);
    return successfullyScanned;
}

#ifndef CRT_PRESENT

uint32 sprintf(char * target, const char * format, ...){
    va_list ap;    
    va_start(ap, format);
    u32 successfullyPrinted = printFormatted(-1, target, format, ap);
    va_end(ap);
    return successfullyPrinted;
}

uint32 sscanf(const char * target, const char * format, ...){
    va_list ap;    
    va_start(ap, format);
    u32 successfullyScanned  = scanFormatted(-1, target, format, ap);
    va_end(ap);
    return successfullyScanned;
}

#else
#include <cstdio>
#endif

#endif
