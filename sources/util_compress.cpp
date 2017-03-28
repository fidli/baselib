#ifndef UTIL_COMPRESS
#define UTIL_COMPRESS

struct LZWTable{
    struct {
        uint8 original[1 << 12];
        uint16 previous[1 << 12];
    } data;
    uint8 bits;
    uint16 count;
};

struct ReadHeadBit{
    const byte * source;
    uint32 byteOffset;
    uint8 bitOffset;
};

static inline uint16 readBits(ReadHeadBit * head, const uint8 bits){
    ASSERT(bits > 0);
    uint16 result = 0;
    int16 toRead = bits;
    while(toRead > 0){
        uint8 remainingBits = 8 - head->bitOffset;
        if(remainingBits == 0){
            head->bitOffset = 0;
            head->byteOffset++;
            remainingBits = 8;
        }
        uint8 reading = (toRead >= remainingBits) ? remainingBits : toRead;
        result <<= reading;
        result |= head->source[head->byteOffset] >> (8 - reading);
        
        toRead -= reading;
        head->bitOffset += reading;
    }
    return result;
}

bool decompressLZW(const byte * source, const uint32 soucreSize, byte * target){
    //http://www.fileformat.info/format/tiff/corion-lzw.htm
    //every message begins with clear code and ends with end of information code
    
    ReadHeadBit compressedDataHead = {};
    compressedDataHead.source = source;
    uint16 endOfInfo = 257;
    uint16 clearCode = 256;
    LZWTable * table = &PUSH(LZWTable);
    table->bits = 9;
    
    uint16 currentTableIndex;
    uint16 previousTableIndex;
    uint32 targetIndex = 0;
    do{
        currentTableIndex = readBits(&compressedDataHead, table->bits);
        if(currentTableIndex == clearCode){
            for(uint16 i = 0; i < 258; i++){
                table->data.original[i] = i;
                table->data.previous[i] = 257;
            }
            //256 = clear code = reinitialize code table
            //257 = end of info code
            table->bits = 9;
            table->count = 258;
            currentTableIndex = readBits(&compressedDataHead, table->bits);
            ASSERT(currentTableIndex <= 257);
            if(currentTableIndex == endOfInfo) break;
            target[targetIndex] = table->data.original[currentTableIndex];
            targetIndex++;
        }else{
            //decompress fuckery
            //redo to top down descend?
            if(currentTableIndex != endOfInfo){
                uint16 crawlIndex = currentTableIndex;
                do{
                    if(crawlIndex >= table->count){
                        ASSERT(crawlIndex == table->count);
                        crawlIndex = previousTableIndex;
                    }
                    target[targetIndex] = table->data.original[crawlIndex];
                    crawlIndex = table->data.previous[crawlIndex];
                    targetIndex++;
                    
                }
                while(table->data.previous[crawlIndex] != endOfInfo);
                
                //add string to table
                table->data.previous[table->count] = previousTableIndex;
                table->data.original[table->count] = table->data.original[crawlIndex];
                table->count++;
                uint8 highestBit;
                for(uint8 bi = 0; bi < 16; bi++){
                    if((table->count >> bi) & 1){
                        highestBit = bi+1;
                    } 
                }
                if(highestBit > table->bits){
                    table->bits = highestBit;
                }
                
            }
            
        }
        previousTableIndex = currentTableIndex;
    }
    while(currentTableIndex != endOfInfo);
    POP;
    return true;
}


#endif