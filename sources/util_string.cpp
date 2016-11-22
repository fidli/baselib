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

char * strcpy(char * target, const char * source){
    uint32 i = 0;
    do{
        target[i] = source[i];
        i++;
    }while(source[i-1] != '\0');
    return target;
}

uint64 strlen(const char * source){
    uint64 length = 0;
    while(source[length] != '\0'){
        length++;
    }
    return length;
}

char * strcat(const char * first, const char * second){
    char * result = &PUSHA(char, strlen(first) + strlen(second) + 1);
    uint32 index = 0;
    while(first[index] != '\0'){
        result[index] = first[index];
        index++;
    }
    uint32 index2 = 0;
    while(second[index2] != '\0'){
        result[index + index2] = second[index2];
        index2++;
    }
    result[index + index2] = '\0';
    return result;
}

#define MAX_CHARLIST_CHARS 20

static struct FormatInfo{
    union{
        struct{
            char digitRangeLow;
            char digitRangeHigh;
            char smallLetterRangeLow;
            char smallLetterRangeHigh;
            char capitalLetterRangeLow;
            char capitalLetterRangeHigh;
            char * charlist;
            uint32 charlistCount;
        } charlist;
    };
  };

  static uint8 scanNumber(const char * source, int64 * target){
      uint8 i = 0;
      bool first = true;
      for(;source[i] != '\0'; i++){
              int8 digit = (int8) source[i];
              //todo: negative numbers, i++
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
  
uint32 sscanf(const char * source, const char * format, ...){
    va_list ap;    
    va_start(ap, format);
    
    
    uint32 formatIndex = 0;
    uint32 sourceIndex = 0;
    uint32 successfullyScanned = 0;
    while(format[formatIndex] != '\0' && source[sourceIndex] != '\0'){
        if(format[formatIndex] == '%'){
            FormatInfo target = {};
            formatIndex++;
            
            if(format[formatIndex] == 'd'){
                uint32 digitCount = 0;
                int32 * targetVar = va_arg(ap, int32 * );
                uint8 scannedChars = scanNumber(source + sourceIndex, (int64 *) targetVar);
                                                if(scannedChars > 0){
                                                   successfullyScanned++;
                                                    sourceIndex += scannedChars;
                                                }
                formatIndex++;
           }else if(format[formatIndex] == '['){
                formatIndex++;
                target.charlist.charlist = &PUSHA(char, MAX_CHARLIST_CHARS);
                bool inverted = false;
                if(format[formatIndex] == '^'){
                    inverted = true;
                    formatIndex++;
                }
                
                for(;format[formatIndex] != ']'; formatIndex++){
                    ASSERT(format[formatIndex] != '\0');
                    if(format[formatIndex+1] == '-'){
                        if(format[formatIndex] >= 'A' && format[formatIndex] <= 'Z'){
                            target.charlist.capitalLetterRangeLow = format[formatIndex];                            
                        }else if(format[formatIndex] >= 'a' && format[formatIndex] <= 'z'){
                            target.charlist.smallLetterRangeLow = format[formatIndex];                            
                        }else if(format[formatIndex] >= 'a' && format[formatIndex] <= 'z'){
                            target.charlist.digitRangeLow = format[formatIndex];
                        }else{
                            ASSERT(!"fuck");
                        }
                         
                        formatIndex += 2;
                        
                        if(format[formatIndex] >= 'A' && format[formatIndex] <= 'Z'){
                            target.charlist.capitalLetterRangeHigh = format[formatIndex];                            
                            ASSERT(target.charlist.capitalLetterRangeHigh > target.charlist.capitalLetterRangeLow);
                        }else if(format[formatIndex] >= 'a' && format[formatIndex] <= 'z'){
                            target.charlist.smallLetterRangeHigh = format[formatIndex];
                            ASSERT(target.charlist.capitalLetterRangeHigh > target.charlist.capitalLetterRangeLow);
                        }else if(format[formatIndex] >= 'a' && format[formatIndex] <= 'z'){
                            target.charlist.digitRangeHigh = format[formatIndex];
                            ASSERT(target.charlist.capitalLetterRangeHigh > target.charlist.capitalLetterRangeLow);
                        }else{
                            ASSERT(!"fuck");
                        }
                        
                    }else{
                        ASSERT(target.charlist.charlistCount != MAX_CHARLIST_CHARS);
                        target.charlist.charlist[target.charlist.charlistCount] = format[formatIndex];
                        target.charlist.charlistCount++;
                        
                    }
                }
                formatIndex++;
                
                char * targetVar = va_arg(ap, char *);
                bool first = true;
                uint32 i = 0;
                for(; source[sourceIndex] != '\0'; sourceIndex++, i++){
                    if(first) first = false;
                    if(target.charlist.digitRangeLow != '\0'){
                        if(source[sourceIndex] >= target.charlist.digitRangeLow && source[sourceIndex] <= target.charlist.digitRangeHigh && !inverted){
                                targetVar[i] = source[sourceIndex];
                                continue;               
                        }else if((source[sourceIndex] < target.charlist.digitRangeLow && source[sourceIndex] > target.charlist.digitRangeHigh) && inverted){
                                targetVar[i] = source[sourceIndex];
                                continue;                            
                        }
                    }
                    if(target.charlist.capitalLetterRangeLow != '\0'){
                        if(source[sourceIndex] >= target.charlist.capitalLetterRangeLow && source[sourceIndex] <= target.charlist.capitalLetterRangeHigh && !inverted){
                            targetVar[i] = source[sourceIndex];
                            continue;               
                        }else if((source[sourceIndex] < target.charlist.capitalLetterRangeLow && source[sourceIndex] > target.charlist.capitalLetterRangeHigh) && inverted){
                            targetVar[i] = source[sourceIndex];
                            continue;                            
                        }
                    }
                    if(target.charlist.smallLetterRangeLow != '\0'){
                        if(source[sourceIndex] >= target.charlist.smallLetterRangeLow && source[sourceIndex] <= target.charlist.smallLetterRangeHigh && !inverted){
                            targetVar[i] = source[sourceIndex];
                            continue;               
                        }else if((source[sourceIndex] < target.charlist.smallLetterRangeLow || source[sourceIndex] > target.charlist.smallLetterRangeHigh) && inverted){
                            targetVar[i] = source[sourceIndex];
                            continue;                            
                        }                        
                    }
                    
                    bool found = false;
                    for(int charlistIndex = 0; charlistIndex < target.charlist.charlistCount; charlistIndex++){
                        if(source[sourceIndex] == target.charlist.charlist[charlistIndex] && !inverted){
                           targetVar[i] = source[sourceIndex];
                           found = true;
                           break;
                        }else if(source[sourceIndex] != target.charlist.charlist[charlistIndex] && inverted){
                            targetVar[i] = source[sourceIndex];
                            found = true;
                            break;
                        }
                    }
                    if(!found){
                        i++;
                        break;    
                    }
                }
                if(target.charlist.charlist){
                   POP; //charlist              
                }
                if(!first)
                    successfullyScanned++;
                targetVar[i-1] = '\0';
            }
            else{
                ASSERT(!"fuck");
            }
        }else if(format[formatIndex] == source[sourceIndex]){
            //todo: whitespaces eating
            formatIndex++;
            sourceIndex++;
        }else{
            break;
        }
        
    };
   
    va_end(ap);
    return successfullyScanned;
}

static uint8 printDigits(char * target, int64 number){
    ASSERT(number >= 0); //i+1 (minus sign)
    char digitsStack[20];
    int8 stackSize = 0;
    int64 temp = number;
    do{
        digitsStack[stackSize] = char((temp % 10) + 48);
        temp /= 10;
        stackSize++;
    } while(temp != 0);
    stackSize--;
    uint8 i = 0;
    for(;stackSize >= 0; stackSize--){
        target[i++] = digitsStack[stackSize];
    }
    return i;
}

uint32 sprintf(char * target, const char * format, ...){
    va_list ap;    
    va_start(ap, format);
    
    
    uint32 formatIndex = 0;
    uint32 targetIndex = 0;
    uint32 successfullyPrinted = 0;
    while(format[formatIndex] != '\0'){
        if(format[formatIndex] == '%'){
            FormatInfo targetFormat = {};
            formatIndex++;
            
            if(format[formatIndex] == 'd'){
                formatIndex++;
                int32 source = va_arg(ap, int32);
                targetIndex += printDigits(target + targetIndex, (int64)source);
                successfullyPrinted++;
           }else if(format[formatIndex] == '.'){
               formatIndex++;
               
               int64 precision = 6;
               uint8 scanOffset = scanNumber(format + formatIndex, &precision);
               ASSERT(scanOffset > 0);               
               formatIndex++;
               
               if(format[formatIndex] == 'f'){
                   formatIndex++;
                   float64 source = va_arg(ap, float64);
                   int64 wholePart = (int64) source;
                   targetIndex += printDigits(target + targetIndex, wholePart);
                   target[targetIndex] = '.';
                   targetIndex++;
                   uint64 decimalPart = (source - wholePart) * pow(10, precision);
                   uint8 prependLen = precision - numlen(decimalPart);
                   for(int i = 0; i < prependLen; i++){
                       target[targetIndex] = '0';
                       targetIndex++;
                   }
                   targetIndex += printDigits(target + targetIndex, decimalPart);
               }else{
                   ASSERT(!"fuck");
               }
               
               successfullyPrinted++;
           }
            else{
                ASSERT(!"fuck");
            }
        }else{
            target[targetIndex] = format[formatIndex];
            formatIndex++;
            targetIndex++;
        }
        
    };
    target[targetIndex] = '\0';
    va_end(ap);
    return successfullyPrinted;
}

#endif