#ifndef UTIL_COMPRESS
#define UTIL_COMPRESS

struct LZWTable{
    struct {
        uint8 carryingSymbol[1 << 12];
        uint16 previousNode[1 << 12];
    } data;
    uint8 bits;
    uint16 count;
};

struct LZWStack{
    uint8 symbols[1 << 12]; //all 12 bit numbers
    uint16 currentIndex;
};

static inline void LZWpush(LZWStack * stack, const uint8 symbol){
    ASSERT(stack->currentIndex < (1 << 11));
    stack->symbols[stack->currentIndex++] = symbol;
}

static inline uint8 LZWpop(LZWStack * stack){
    ASSERT(stack->currentIndex > 0);
    uint8 ret = stack->symbols[stack->currentIndex-1];
    stack->currentIndex--;
    return ret;
}

static inline uint8 LZWpeek(const LZWStack * stack){
    return stack->symbols[stack->currentIndex-1];
}

static inline bool LZWempty(const LZWStack * stack){
    return stack->currentIndex == 0;
}

struct ReadHeadBit{
    const byte * source;
    uint32 byteOffset;
    uint8 bitOffset;
};

static inline uint16 readBits(ReadHeadBit * head, const uint8 bits){
    ASSERT(bits > 0);
    ASSERT(bits <= 16);
#ifndef RELEASE
    static uint16 oldByteOffset;
    oldByteOffset = head->byteOffset;
#endif
    uint16 result = 0;
    int16 toRead = bits;
    while(toRead > 0){
        uint8 remainingBits = 8 - head->bitOffset;
        ASSERT(remainingBits <= 8);
        if(remainingBits == 0){
            head->bitOffset = 0;
            head->byteOffset++;
            remainingBits = 8;
        }
        uint8 reading = (toRead >= remainingBits) ? remainingBits : toRead;
        result <<= reading;
        result |=(uint8)( (uint8)((uint8)((uint8)head->source[head->byteOffset] << head->bitOffset) >> head->bitOffset) >> (8-head->bitOffset - reading));
        ASSERT(reading <= toRead);
        toRead -= reading;
        head->bitOffset += reading;
        ASSERT(head->bitOffset <= 8);
    }
    
    if(head->bitOffset == 8){
        head->bitOffset = 0;
        head->byteOffset++;
    }
    
    ASSERT(head->byteOffset == oldByteOffset + 1 || head->byteOffset == oldByteOffset + 2);
    return result;
}

bool decompressLZW(const byte * source, const uint32 sourceSize, byte * target){
    //http://www.fileformat.info/format/tiff/corion-lzw.htm
    //every message begins with clear code and ends with end of information code
    
    ReadHeadBit compressedDataHead = {};
    compressedDataHead.source = source;
    uint16 endOfInfo = 257;
    uint16 clearCode = 256;
    PUSHI;
    LZWTable * table = &PUSH(LZWTable);
    LZWStack * stack = &PUSH(LZWStack);
    table->bits = 9;
    stack->currentIndex = 0;
    uint16 currentTableIndex;
    uint16 previousTableIndex;
    uint32 targetIndex = 0;
    uint8 previousFirstCharacter;
    ASSERT(*source == 128 && (~*(source + 1) & 128) == 128);
    
    do{
        ReadHeadBit oldHead = compressedDataHead;
        currentTableIndex = readBits(&compressedDataHead, table->bits);
        if(currentTableIndex == clearCode){
            for(uint16 i = 0; i < 258; i++){
                table->data.carryingSymbol[i] = i;
                table->data.previousNode[i] = endOfInfo;
            }
            //256 = clear code = reinitialize code table
            //257 = end of info code
            table->bits = 9;
            table->count = 258;
            currentTableIndex = readBits(&compressedDataHead, table->bits);
            ASSERT(currentTableIndex <= 257);
            if(currentTableIndex == endOfInfo) break;
            ASSERT(currentTableIndex != 256);
            target[targetIndex] = table->data.carryingSymbol[currentTableIndex];
            targetIndex++;
        }else if(currentTableIndex == endOfInfo){
            break;
        }else{
            ASSERT(currentTableIndex <= table->count);
            
            //get the output string
            ASSERT(LZWempty(stack));
            uint16 crawlIndex = currentTableIndex;
            do{
                LZWpush(stack, table->data.carryingSymbol[crawlIndex]);
                crawlIndex = table->data.previousNode[crawlIndex];
            }while(crawlIndex != endOfInfo);
            
            //write to output
            ASSERT(!LZWempty(stack));
            uint8 firstChar = LZWpeek(stack);
            while(!LZWempty(stack)){
                target[targetIndex] = LZWpop(stack);
                targetIndex++;
            }
            if(currentTableIndex == table->count){
                target[targetIndex++] = firstChar;
            }
            
            //add to table
            table->data.carryingSymbol[table->count] = firstChar;
            table->data.previousNode[table->count] = previousTableIndex;
            
            //table count inc (not accounting for the two special codes)
            if(table->count + 2 >= (1 << 8)) table->bits = 9;
            if(table->count + 2 >= (1 << 9)) table->bits = 10;
            if(table->count + 2 >= (1 << 10)) table->bits = 11;
            if(table->count + 2 >= (1 << 11)) table->bits = 12;
            table->count++;
            
        }
        previousTableIndex = currentTableIndex;
    }
    while(compressedDataHead.byteOffset < sourceSize);
    POPI;
    return true;
}


#endif