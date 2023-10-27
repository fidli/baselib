#ifndef UTIL_COMPRESS
#define UTIL_COMPRESS

#include "util_sort.cpp"
#include <emmintrin.h>

enum ByteOrder{
    ByteOrder_Invalid,
    ByteOrder_LittleEndian,
    ByteOrder_BigEndian
};
// TODO do this really

#define NATIVE_BYTE_ORDER ByteOrder_LittleEndian

struct ReadHead{
    u8 * offset;
};

union converter{
    u8 bytes[8];
    u16 words[4];
    u32 dwords[2];
    u64 qword;
};


static converter martyr;

static inline u8 scanByte(ReadHead * head){
    u8 res = head->offset[0];
    head->offset++;
    return res;
}

static inline u16 scanWord(ReadHead * head, ByteOrder order){
    u16 res;
    if (order == NATIVE_BYTE_ORDER)
    {
        martyr.bytes[0] = head->offset[0];
        martyr.bytes[1] = head->offset[1];
    }else{
        martyr.bytes[1] = head->offset[0];
        martyr.bytes[0] = head->offset[1];
    }
    head->offset += 2;
    res = martyr.words[0];
    return res;
}


static inline u32 scanDword(ReadHead * head, ByteOrder order){
    u32 res;
    if (order == NATIVE_BYTE_ORDER)
    {
        martyr.bytes[0] = head->offset[0];
        martyr.bytes[1] = head->offset[1];
        martyr.bytes[2] = head->offset[2];
        martyr.bytes[3] = head->offset[3];
    }
    else{
        martyr.bytes[3] = head->offset[0];
        martyr.bytes[2] = head->offset[1];
        martyr.bytes[1] = head->offset[2];
        martyr.bytes[0] = head->offset[3];
    }
    head->offset += 4;
    res = martyr.dwords[0];
    return res;
}


static inline u64 scanQword(ReadHead * head, ByteOrder order){
    u64 res;
    if (order == NATIVE_BYTE_ORDER)
    {
        martyr.bytes[0] = head->offset[0];
        martyr.bytes[1] = head->offset[1];
        martyr.bytes[2] = head->offset[2];
        martyr.bytes[3] = head->offset[3];
        martyr.bytes[4] = head->offset[4];
        martyr.bytes[5] = head->offset[5];
        martyr.bytes[6] = head->offset[6];
        martyr.bytes[7] = head->offset[7];
    }
    else{
        martyr.bytes[7] = head->offset[0];
        martyr.bytes[6] = head->offset[1];
        martyr.bytes[5] = head->offset[2];
        martyr.bytes[4] = head->offset[3];
        martyr.bytes[3] = head->offset[4];
        martyr.bytes[2] = head->offset[5];
        martyr.bytes[1] = head->offset[6];
        martyr.bytes[0] = head->offset[7];
    }
    head->offset += 8;
    res = martyr.qword;
    return res;
}

struct LZWCompressNode{
    u16 index;
    LZWCompressNode * next;
};

struct LZWTable{
    struct {
        u8 carryingSymbol[1 << 12];
        u16 previousNode[1 << 12];
        LZWCompressNode * children[1 << 12];
    } data;
    u8 bits;
    u16 count;
};

struct LZWStack{
    u8 symbols[1 << 12]; //all 12 bit numbers
    u16 currentIndex;
};

static inline void LZWpush(LZWStack * stack, const u8 symbol){
    ASSERT(stack->currentIndex < (1 << 11));
    stack->symbols[stack->currentIndex++] = symbol;
}

static inline u8 LZWpop(LZWStack * stack){
    ASSERT(stack->currentIndex > 0);
    u8 ret = stack->symbols[stack->currentIndex-1];
    stack->currentIndex--;
    return ret;
}

static inline u8 LZWpeek(const LZWStack * stack){
    return stack->symbols[stack->currentIndex-1];
}

static inline bool LZWempty(const LZWStack * stack){
    return stack->currentIndex == 0;
}

struct ReadHeadBit{
    bool inverted;
    u8 * source;
    u8 bitOffset;
    byte currentByte;
    u32 byteOffset;

    u64 currentBits;
    u8 bitsLeft;

    u8 bitsRead;
};

struct ReadHeadBit2{
    u8 * source;
    u8 bitsRead;
};

static inline u16 invertBits(const u16 sourceBits, u8 bitCount){
    u16 result = 0;
    
    u16 workBits = sourceBits;
    i16 currentBit = bitCount-1;
    
    while(currentBit >= 0){
        result |= ((workBits & 1) << currentBit);
        currentBit--;
        workBits >>= 1;
    }
    
    return result;
}

static inline u8 invertBits(const u8 sourceBits, u8 bitCount){
    u8 result = 0;
    
    u8 workBits = sourceBits;
    i8 currentBit = bitCount-1;
    
    while(currentBit >= 0){
        result |= ((workBits & 1) << currentBit);
        currentBit--;
        workBits >>= 1;
    }
    
    return result;
}

static inline u16 readBits2(ReadHeadBit2 * head, const u8 bits){
    ASSERT(head->bitsRead < 32);
    u64 readMask = (1 << bits) - 1;
    u16 result = CAST(u16, ((*CAST(u64*, head->source)) >> head->bitsRead) & readMask);
    head->bitsRead += bits;
    if (head->bitsRead >= 32)
    {
        head->bitsRead -= 32;
        head->source += 4;
    }
    return result;
}

static inline u16 readBits(ReadHeadBit * head, const u8 bits){
    PROFILE_FUNC();
    ASSERT(bits > 0);
    ASSERT(bits <= 16);
    u16 result = 0;
    
    
    
    i16 toRead = bits;
    while(toRead > 0){
        u8 remainingBits = 8 - head->bitOffset;
        ASSERT(remainingBits <= 8);
        if(remainingBits == 0){
            head->bitOffset = 0;
            head->byteOffset++;
            remainingBits = 8;
        }
        if(head->bitOffset == 0){
            head->currentByte = *(head->source + head->byteOffset);
            if(head->inverted){
                head->currentByte = invertBits(head->currentByte, 8);
            }
        }
        
        u8 reading = CAST(u8, (toRead >= remainingBits) ? remainingBits : toRead);
        result <<= reading;
        result |=(u8)( (u8)((u8)((u8)head->currentByte << head->bitOffset) >> head->bitOffset) >> (8-head->bitOffset - reading));
        ASSERT(reading <= toRead);
        toRead -= reading;
        head->bitOffset += reading;
        ASSERT(head->bitOffset <= 8);
    }
    
    if(head->bitOffset == 8){
        head->bitOffset = 0;
        head->byteOffset++;
    }
    
    if (head->inverted){
        result = invertBits(result, bits);
    }
    return result;
}

u32 decompressLZW(u8 * source, const u32 sourceSize, byte * target){
    //http://www.fileformat.info/format/tiff/corion-lzw.htm
    //every message begins with clear code and ends with end of information code
    
    ReadHeadBit compressedDataHead = {};
    compressedDataHead.source = source;
    compressedDataHead.inverted = false;
    u16 endOfInfo = 257;
    u16 clearCode = 256;
    PUSHI;
    LZWTable * table = &PUSH(LZWTable);
    LZWStack * stack = &PUSH(LZWStack);
    table->bits = 9;
    stack->currentIndex = 0;
    u16 currentTableIndex;
    u16 previousTableIndex = 0;
    u32 targetIndex = 0;
    ASSERT(*source == 128 && (~*(source + 1) & 128) == 128);
    if(*source != 128 || (~*(source + 1) & 128) != 128){
        POPI;
        return 0;
    }
    
    do{
        ReadHeadBit oldHead = compressedDataHead;
        currentTableIndex = readBits(&compressedDataHead, table->bits);
        if(currentTableIndex == clearCode){
            for(u16 i = 0; i < 258; i++){
                INV; // check correctness of i > 255
                table->data.carryingSymbol[i] = CAST(u8, i);
                table->data.previousNode[i] = endOfInfo;
            }
            //256 = clear code = reinitialize code table
            //257 = end of info code
            table->bits = 9;
            table->count = 258;
            currentTableIndex = readBits(&compressedDataHead, table->bits);
            ASSERT(currentTableIndex <= 257);
            ASSERT(currentTableIndex != 256);
            if(currentTableIndex > 257 || currentTableIndex == 256){
                POPI;
                return 0;
            }
            if(currentTableIndex == endOfInfo) break;
            
            target[targetIndex] = table->data.carryingSymbol[currentTableIndex];
            targetIndex++;
        }else if(currentTableIndex == endOfInfo){
            break;
        }else{
            ASSERT(currentTableIndex <= table->count);
            if(currentTableIndex > table->count){
                POPI;
                return 0;
            }
            //get the output string
            ASSERT(LZWempty(stack));
            u16 crawlIndex = currentTableIndex;
            if(currentTableIndex == table->count){
                crawlIndex = previousTableIndex;
            }
            do{
                LZWpush(stack, table->data.carryingSymbol[crawlIndex]);
                crawlIndex = table->data.previousNode[crawlIndex];
            }while(crawlIndex != endOfInfo);
            
            //write to output
            ASSERT(!LZWempty(stack));  
            u8 firstChar = LZWpeek(stack);
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
            
            table->count++;
            //table count inc (as soon as last possible entry is added, use more bits)
            if(table->count + 1 >= (1 << 8)) table->bits = 9;
            if(table->count + 1 >= (1 << 9)) table->bits = 10;
            if(table->count + 1 >= (1 << 10)) table->bits = 11;
            if(table->count + 1 >= (1 << 11)) table->bits = 12;
            
            
        }
        previousTableIndex = currentTableIndex;
    }
    while(compressedDataHead.byteOffset < sourceSize);
    
    ASSERT(currentTableIndex == endOfInfo);
    ASSERT(sourceSize == compressedDataHead.byteOffset + 1 || sourceSize == compressedDataHead.byteOffset);
    if(currentTableIndex != endOfInfo){
        POPI;
        return 0;
    }
    POPI;
    
    return targetIndex;
}


struct WriteHeadBit{
    byte * source;
    u32 byteOffset;
    u8 bitOffset;
};

static inline void writeBits(WriteHeadBit * head, u16 data, const u8 bits){
    ASSERT(bits > 0);
    ASSERT(bits <= 16);
    u8 toWrite = bits;
    data = data << (16 - bits);
    while(toWrite != 0){
        u8 currentWriting = MIN(8 - head->bitOffset, toWrite);
        
        //clear with zeros
        head->source[head->byteOffset] &=
            (u8)((u8)(0xFF >> (8 - head->bitOffset)) << (8 - head->bitOffset)) |
            (u8)(0xFF >>  (head->bitOffset + currentWriting));
        
        u8 write = ((u8)(((u8)(((data >> 8) >> head->bitOffset)) >> (8 - head->bitOffset - currentWriting)))) << (8 - head->bitOffset - currentWriting);
        
        //write
        head->source[head->byteOffset] |= write;
        head->bitOffset += currentWriting;
        if(head->bitOffset == 8){
            head->bitOffset = 0;
            head->byteOffset++;
        }
        toWrite -= currentWriting;
        //modify data
        data <<= currentWriting;
    }
}


u32 compressLZW(byte * source, const u32 sourceSize, byte * target){
    //http://www.fileformat.info/format/tiff/corion-lzw.htm
    //every message begins with clear code and ends with end of information code
    WriteHeadBit compressedDataHead = {};
    compressedDataHead.source = target;
    u16 endOfInfo = 257;
    u16 clearCode = 256;
    PUSHI;
    LZWTable * table = &PUSH(LZWTable);
    LZWCompressNode * pool = &PUSHA(LZWCompressNode, 1 << 12);
    u16 poolCount = 0;
    table->count = 4094;
    table->bits = 9;
    
    u16 currentCrawl = source[0];
    for(u32 i = 1; i < sourceSize; i++){
        if(table->count == 4094){
            ASSERT((i == 1 && table->bits == 9) || (i != 1 && table->bits == 12));
            writeBits(&compressedDataHead, clearCode, table->bits);
            for(u16 j = 0; j < 258; j++){
                INV;    // check u16 -> u8 correctness
                table->data.carryingSymbol[j] = CAST(u8, j);
                table->data.children[j] = NULL;
            }
            //256 = clear code = reinitialize code table
            //257 = end of info code
            table->bits = 9;
            table->count = 258;
            poolCount = 0;
        }
        
        
        bool found = false;
        LZWCompressNode * current = table->data.children[currentCrawl];
        LZWCompressNode * previous = NULL;
        while(current != NULL){
            if(table->data.carryingSymbol[current->index] == source[i]){
                currentCrawl = current->index;
                found = true;
                break;
            }
            previous = current;
            current = current->next;
        } 
        if(!found){
            writeBits(&compressedDataHead, currentCrawl, table->bits);
            u16 newIndex = table->count;
            table->data.carryingSymbol[newIndex] = source[i];
            table->data.children[newIndex] = NULL;
            if(previous == NULL){
                table->data.children[currentCrawl] = &pool[poolCount++];
                table->data.children[currentCrawl]->next = NULL;
                table->data.children[currentCrawl]->index = newIndex;
                ASSERT(poolCount <= 1 << 12);
            }else{
                previous->next = &pool[poolCount++];
                previous->next->next = NULL;
                previous->next->index = newIndex;
                ASSERT(poolCount <= 1 << 12);
                
            }
            currentCrawl = source[i];
            table->count++;
            if(table->count >= (1 << 8)) table->bits = 9;
            if(table->count >= (1 << 9)) table->bits = 10;
            if(table->count >= (1 << 10)) table->bits = 11;
            if(table->count >= (1 << 11)) table->bits = 12;
            
            
        }
    }
    writeBits(&compressedDataHead, currentCrawl, table->bits);
    table->count++;
    if(table->count >= (1 << 8)) table->bits = 9;
    if(table->count >= (1 << 9)) table->bits = 10;
    if(table->count >= (1 << 10)) table->bits = 11;
    if(table->count >= (1 << 11)) table->bits = 12;
    writeBits(&compressedDataHead, endOfInfo, table->bits);
    POPI;
    return compressedDataHead.byteOffset + (compressedDataHead.bitOffset != 0 ? 1 : 0);
}

struct CodeTable{
#if 0
    struct Element {
        u16 code;
        u16 value;
        u8 bitSize;
    } elements[257+32];
#endif
    
    struct Pack {
        __m128i code;
        __m128i mask;
        __m128i value;
        __m128i bitSize;
    } packs[37];

    u8 packCount;
    u16 count;
};


u32 matchCode(const CodeTable * table, ReadHeadBit2 * sourceHead)
{
    u64 currentBits = ((*CAST(u64*, sourceHead->source)) >> sourceHead->bitsRead);
    __m128i currentBitsPack = _mm_set1_epi16(CAST(u16, currentBits));
    __m128i zeroesPack = _mm_setzero_si128();
    __m128i indices = _mm_set_epi16(8, 7, 6, 5, 4, 3, 2, 1);
    for(u32 i = 0; i < table->packCount; i++)
    {

        __m128i test = _mm_cmpeq_epi16(_mm_and_si128(currentBitsPack, table->packs[i].mask), table->packs[i].code);
        __m128i match = _mm_and_si128(test, indices);
        __m128i sad = _mm_sad_epu8(match, zeroesPack);
        int index = sad.m128i_u16[0] + sad.m128i_u16[4];
        if (index)
        {
            int currentBitSize = table->packs[i].bitSize.m128i_u16[index-1];
            sourceHead->bitsRead += currentBitSize;
            if (sourceHead->bitsRead >= 32)
            {
                sourceHead->bitsRead -= 32;
                sourceHead->source += 4;
            }
            return table->packs[i].value.m128i_u16[index-1];
        }
    }
    INV;
    return CAST(u32, -1);
}

struct HuffmanNode{
    bool leaf;
    i32 value;
    u8 bit;
    HuffmanNode * one;
    HuffmanNode * zero;
};

struct CodeWord{
    u32 weight;
    u32 code;
    u8 bitSize;
};

inline static u32 decompressHuffman2(ReadHeadBit2 * head, const CodeTable * literalTable, const CodeTable * distanceTable, byte * target){
    PROFILE_FUNC();
#if PROFILE
    u8* byteStart = head->source;
#endif
    
    ASSERT(distanceTable->count > 0); // implement with outer if. inside while loop its slow
            /*
            if(distanceTable->count > 0){
                u32 i = matchCode(distanceTable, head);
                backOffset = i;
                
            }else{
                //invert?
                INV; // dont know
                backOffset = readBits2(head, 5);
            }
            */
    
    u32 localOffset = 0;
    u32 i = 0;
    while(i != 256){
        i = matchCode(literalTable, head);
        if(i < 256){
            target[localOffset++] = (char)i;
        }
        else if(i > 256){
            //repetition, defined in specification
            i32 count;

            if (i == 285)
            {
                count = 258;
            }else if(i >= 265){
                u8 bits = CAST(u8, (i - 261) / 4);
                i32 extraCountI = i - 265;
                //static const i32 extraCounts[] = {11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163,195, 227};
                i32 ec = 11 + 2*extraCountI + (extraCountI>4)*2*(extraCountI-4) + (extraCountI>8)*4*(extraCountI-8) + (extraCountI>12)*8*(extraCountI-12) + (extraCountI>16)*16*(extraCountI-16);
                count = readBits2(head, bits) + ec;
            }else {
                count = i - 254;
            }
            
            i32 backOffset = matchCode(distanceTable, head);
            
            if(backOffset > 3){
                u8 bits = CAST(u8, (backOffset-2)/2);
                //static i32 extraBackOffsets[] = {4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512, 768, 1024, 1536, 2048, 3072, 4096, 6144, 8192, 12288, 16384, 24576};
                i32 extraBackOffsetI = backOffset-4;
                i32 ebo = ((extraBackOffsetI&1)^1)*2*(1<<((extraBackOffsetI/2) + 1)) + (extraBackOffsetI&1)*6*(1<<((extraBackOffsetI/2)));
                backOffset =  readBits2(head, bits) + ebo;
            }
            
            unsigned char * start = target + localOffset - 1 - backOffset;

            memcpy(target + localOffset, start, count);
            localOffset += count;
            
        }
    }
    PROFILE_BYTES(head->source - byteStart + localOffset);
    return localOffset;
}

inline static u32 decompressHuffman(ReadHeadBit * head, const HuffmanNode * literalsTree, const HuffmanNode * distancesTree, byte * target){
    PROFILE_FUNC();
#if PROFILE
    u32 byteStart = head->byteOffset;
#endif
    
    
    const i32 extraCounts[] = {11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163,195, 227};
    const i32 extraBackOffsets[] = {4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512, 768, 1024, 1536, 2048, 3072, 4096, 6144, 8192, 12288, 16384, 24576};
    
    
    //inflating huffman codes
    u32 localOffset = 0;
    const HuffmanNode * currentNode = literalsTree;
    while(true){
        if(readBits(head, 1) & 1){
            currentNode = currentNode->one;
        }else{
            currentNode = currentNode->zero;
        }
        if(currentNode->leaf){
            if(currentNode->value < 256){
                target[localOffset++] = (char)currentNode->value;
            }
            if(currentNode->value == 256){
                
                break;
            }
            if(currentNode->value > 256){
                //repetition, defined in specification
                i32 count;
                if(currentNode->value < 265){
                    count = currentNode->value - 254;
                }else if(currentNode->value < 285){
                    ASSERT(((currentNode->value - 261)/4) <= 255 && ((currentNode->value - 261)/4) >= 0);
                    u8 bits = CAST(u8, (currentNode->value - 261) / 4);
                    count = readBits(head, bits) + extraCounts[currentNode->value - 265];
                }else{
                    count = 258;
                }
                
                i32 backOffset;
                
                if(distancesTree){
                    const HuffmanNode * distNode = distancesTree;
                    while(!distNode->leaf){
                        if(readBits(head, 1) & 1){
                            distNode = distNode->one;
                        }else{
                            distNode = distNode->zero;
                        }
                    }
                    backOffset = distNode->value;
                    
                    
                }else{
                    //invert?
                    backOffset = readBits(head, 5);
                }
                
                
                
                if(backOffset > 3){
                    ASSERT((backOffset-2)/2 <= 255 && (backOffset-2)/2 >= 0);
                    u8 bits = CAST(u8, (backOffset-2)/2);
                    backOffset =  readBits(head, bits) + extraBackOffsets[backOffset-4];
                }
                
                unsigned char * start = target + localOffset - 1 - backOffset;
                
                for(i32 i = 0; i < count; i++){
                    target[localOffset++] = start[i];
                }
                
            }
            currentNode = literalsTree;
        }
    }
    ASSERT(currentNode->value == 256); //ended with close node
    PROFILE_BYTES(head->byteOffset - byteStart);
    return localOffset;
}


static void addHuffmanNodeRec(HuffmanNode * tree, HuffmanNode * node, u16 code, u8 codeLenght){
    
    //if this fires, not a prefix code
    ASSERT(tree->value == -1);
    
    if(codeLenght == 1){
        if(code & 1){
            ASSERT(tree->one == NULL);
            tree->one = node;
        }else{
            ASSERT(tree->zero == NULL);
            tree->zero = node;
        }
        tree->bit = code & 1;
        return;
    }
    
    HuffmanNode * subtree;
    
    if(code & 1){
        if(tree->one == NULL){
            tree->one = &PUSH(HuffmanNode);
            tree->one->bit = 1;
            tree->one->leaf = false;
            tree->one->value = -1;
            tree->one->one = NULL;
            tree->one->zero = NULL;
        }
        subtree = tree->one;
    }else{
        if(tree->zero == NULL){
            tree->zero = &PUSH(HuffmanNode);
            tree->zero->bit = 0;
            tree->zero->leaf = false;
            tree->zero->value = -1;
            tree->zero->one = NULL;
            tree->zero->zero = NULL;
        }
        subtree = tree->zero;
    }
    
    addHuffmanNodeRec(subtree, node, code >> 1, codeLenght-1);
    
    
}

static inline void assignCodesAndBuildTree2(u8 * bitSizes, u16 count, CodeTable * target){
    u32 amounts[19] = {};

    struct Temp{
        u8 bitSize;
        u16 value;
        u16 code;
        u16 mask;
    } temp[257+32];

    for(u16 i = 0; i < count; i++){
        temp[i].bitSize = bitSizes[i];
        temp[i].value = i;
        amounts[bitSizes[i]]++;
    }
    amounts[0] = 0;

    u32 codeDispenser[19];
    u32 startingCode = 0;
    for(i8 i = 1; i <= 18; i++){
        startingCode = (startingCode + amounts[i-1]) << 1;
        codeDispenser[i] = startingCode;
    }
    
    // sorting to remove 0 bit sizes
    mergeSort(temp, count, [] (Temp & A, Temp & B) -> i32 {
           if(A.bitSize > B.bitSize){
               return 1;
           }else if(A.bitSize == B.bitSize){
               return 0;
           }
               return -1;
           });

    u16 zeroes = 0;
    for (; zeroes < count; zeroes++){
        if (temp[zeroes].bitSize != 0){
            break;
        }
    }
    if (zeroes > 0){
        memcpy(CAST(void*, temp), CAST(void*, &temp[zeroes]), sizeof(Temp) * (count-zeroes));
    }
    count -= zeroes;
    ASSERT(temp[count-1].bitSize <= 16);
    
    for(u32 i = 0; i < count; i++){
        u32 code = invertBits(CAST(u16, codeDispenser[temp[i].bitSize]), temp[i].bitSize);
        ASSERT(((code << (32 - temp[i].bitSize)) >> (32 - temp[i].bitSize)) == code);
        ASSERT(code <= 65535);
        codeDispenser[temp[i].bitSize]++;
        temp[i].code = code;
    }

    target->count = count;
    ASSERT(target->count > 0);
    target->packCount = ((target->count - 1)/8) + 1;
    for(u8 pi = 0; pi < target->packCount; pi++)
    {
        u16 bi = pi * 8;
        u16 codes[8] = {0};
        u16 values[8] = {0};
        u16 bitSizes[8] = {0};
        u16 masks[8] = {0};
        for(u8 i = 0; i < 8; i++)
        {
            if (i + bi < target->count)
            {
                codes[i] = temp[i+bi].code;
                values[i] = temp[i+bi].value;
                bitSizes[i] = temp[i+bi].bitSize;
                masks[i] = (1 << bitSizes[i]) - 1;
            }
            else
            {
                // later when comparing we need something non 0
                codes[i] = 1;
            }
        }
        target->packs[pi].code = _mm_load_si128(CAST(__m128i*, codes));
        target->packs[pi].mask = _mm_load_si128(CAST(__m128i*, masks));
        target->packs[pi].value = _mm_load_si128(CAST(__m128i*, values));
        target->packs[pi].bitSize = _mm_load_si128(CAST(__m128i*, bitSizes));
    }
}

static inline void readCodes2(ReadHeadBit2 * head, const CodeTable * bootstrapTable, u8 * target, u16 targetCount){
    
    u32 decodeWordI = 0;
    while(decodeWordI < targetCount){
    
        u32 i = matchCode(bootstrapTable, head);
            
        //17 repeat 0 n times n = 3 bits
        if(i == 17){

            u16 times0 = 3;
            times0 += readBits2(head, 3);
            
            for(u16 ri = 0; ri < times0; ri++){
                target[decodeWordI] = 0;
                decodeWordI++;
            }
            
            
        }else if(i == 18){
            //18 repeat 0 n times n = 7 bits
            u16 times0 = 11;
            times0 += readBits2(head, 7);
            
            for(u16 ri = 0; ri < times0; ri++){
                target[decodeWordI] = 0;
                decodeWordI++;
            }
        }else if(i == 16){
            ////16 repeat previous n times
            u16 timesRepeat = 3;
            timesRepeat += readBits2(head, 2);

            ASSERT(decodeWordI-1 >= 0);

            u8 bitSize = target[decodeWordI-1];
            for(u16 ri = 0; ri < timesRepeat; ri++){
                target[decodeWordI] = bitSize;
                decodeWordI++;
            }
        }else{
            ASSERT(i <= 18 && i >= 0);
            target[decodeWordI]= i;
            decodeWordI++;
        }
    }
    ASSERT(decodeWordI == targetCount);
}

static inline HuffmanNode assignCodesAndBuildTree(CodeWord * codeWords, u32 itemCount){
    
    
    
    //sort by weigth, then by size //then assign codes
    insertSort(codeWords, itemCount, [](CodeWord & A, CodeWord & B) -> i32{
               if(A.weight > B.weight){
               return 1;
               }else if(A.weight == B.weight){
               return 0;
               }
               return -1;
               });
    
    insertSort(codeWords, itemCount, [](CodeWord & A, CodeWord & B) -> i32{
               if(A.bitSize > B.bitSize){
               return 1;
               }else if(A.bitSize == B.bitSize){
               return 0;
               }
               return -1;
               });
    // remove 0 bit sizes
    i32 zeroes = 0;
    for (; zeroes < CAST(i32, itemCount); zeroes++){
        if (codeWords[zeroes].bitSize != 0){
            break;
        }
    }
    if (zeroes > 0){
        memcpy(CAST(void*, codeWords), CAST(void*, &codeWords[zeroes]), sizeof(CodeWord) * (itemCount-zeroes));
    }
    itemCount -= zeroes;
    
    u8 maxCodeLength = codeWords[itemCount-1].bitSize;
    
    u32 * amounts = &PUSHA(u32, maxCodeLength+1);
    for(u8 i = 0; i <= maxCodeLength; i++){
        amounts[i] = 0;
    }

    for(u32 i = 0; i < itemCount; i++){
        amounts[codeWords[i].bitSize]++;
    }
    ASSERT(amounts[0] == 0);

    u32 * codeDispenser = &PUSHA(u32, maxCodeLength+1);
    for(u8 i = 0; i <= maxCodeLength; i++){
        codeDispenser[i] = 0;
    }
    
    u32 code = 0;
    for(i8 i = 1; i <= CAST(i8, maxCodeLength); i++){
        code = (code + amounts[i-1]) << 1;
        codeDispenser[i] = code;
    }
    
    
    for(u32 i = 0; i < itemCount; i++){
        //proccess each code length
        ASSERT(codeWords[i].bitSize != 0);
        
        codeWords[i].code = codeDispenser[codeWords[i].bitSize];
        ASSERT(((codeWords[i].code << (32 - codeWords[i].bitSize)) >> (32 - codeWords[i].bitSize)) == codeWords[i].code);
        codeDispenser[codeWords[i].bitSize]++;
        
    }
    
    HuffmanNode tree;
    tree.leaf = false;
    tree.value = -1;
    tree.one = NULL;
    tree.zero = NULL;
    
    for(u32 i = 0; i < itemCount; i++){
        if(codeWords[i].bitSize == 0) continue;
        HuffmanNode * node = &PUSH(HuffmanNode);
        
        node->leaf = true;
        node->value = codeWords[i].weight;
        node->one = NULL;
        node->zero = NULL;
        
        ASSERT(codeWords[i].code <= 65535);
        addHuffmanNodeRec(&tree, node, invertBits(CAST(u16, codeWords[i].code), codeWords[i].bitSize), codeWords[i].bitSize);
        
    }
    
    return tree;
    
}



static inline void readCodes(ReadHeadBit * head, const HuffmanNode * tree, CodeWord * target, u32 limit){
    
    const HuffmanNode * currentNode = tree;
    u32 i = 0;
    while(i < limit){
        
        if(readBits(head, 1) & 1){
            currentNode = currentNode->one;
        }else{
            currentNode = currentNode->zero;
        }
        ASSERT(currentNode != NULL);
        if(currentNode->leaf){
            
            //17 repeat 0 n times n = 3 bits
            if(currentNode->value == 17){
                u16 times0 = readBits(head, 3);
                times0 += 3;
                
                for(u16 ri = 0; ri < times0; ri++){
                    target[i].bitSize = 0;
                    target[i].weight = i;
                    i++;
                }
                
                
            }else if(currentNode->value == 18){
                //18 repeat 0 n times n = 7 bits
                u16 times0 = readBits(head, 7);
                times0 += 11;
                for(u16 ri = 0; ri  < times0; ri++){
                    target[i].bitSize = 0;
                    target[i].weight = i;
                    i++;
                }
                
            }else if(currentNode->value == 16){
                ////16 repeat previous n times
                u16 timesRepeat = readBits(head, 2);
                timesRepeat += 3;
                ASSERT(i-1 >= 0);
                for(u16 ri = 0; ri < timesRepeat; ri++){
                    target[i].bitSize = target[i-1].bitSize;
                    target[i].weight = i;
                    i++;
                }
            }else{
                ASSERT(currentNode->value <= 255 && currentNode->value >= 0);
                target[i].bitSize = CAST(u8, currentNode->value);
                target[i].weight = i;
                i++;
            }
            
            
            currentNode = tree;
        }
    }
    ASSERT(i == limit);
}


//deflate/inflate
//http://www.gzip.org/algorithm.txt - idea

//more specific - https://en.wikipedia.org/wiki/DEFLATE or RFC
u32 decompressDeflate(u8 * compressedData, u8 * target){
    PROFILE_FUNC();
    
    //dict 32kbytes
    //maxLen = 256
    
    //start of block - huffman trees for indices and lenghts
    ReadHeadBit head = {};
    head.inverted = true;
    head.source = compressedData;
    
    //phase1 repeat counts, is this hardcoded defined? or where do these repeat times come from?
    u8 dynamicCodes[] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
    
    u32 targetOffset = 0;
    
    bool processBlock = true;
    while(processBlock){
        PUSHI;
        processBlock = readBits(&head, 1) == 0;
        u16 header = readBits(&head, 2);
        
        
        if((header & 3) == 0){
            PROFILE_SCOPE("Huffman literal");
            //a stored/raw/literal section, between 0 and 65,535 bytes in length.
            //http://www.bolet.org/~pornin/deflate-flush.html
            while(head.bitOffset != 0){
                readBits(&head, 1);
                //ASSERT(bits == 0);
            } //get rid of 0 bytes
            
            u16 dataSize = readBits(&head, 16);
            //this is redundancy
            readBits(&head, 16); //complement
            //now the data should be present
            ASSERT(head.bitOffset == 0);
            for(u32 i = 0; i < dataSize; i++){
                target[targetOffset++] = CAST(u8, readBits(&head, 8));
            }
            PROFILE_BYTES(dataSize);
        }else{
            // - http://commandlinefanatic.com/cgi-bin/showarticle.cgi?article=art001
            //see Listing 14: Read the GZIP pre-header
            
            HuffmanNode literalTree;
            HuffmanNode distanceTree;
            
            if((header & 3) == 1){
                PROFILE_SCOPE("Huffman static");
                //a static Huffman compressed block, using a pre-agreed Huffman tree.
                /** 
                * Build a Huffman tree for the following values:
                *   0 - 143: 00110000  - 10111111     (8)
                * 144 - 255: 110010000 - 111111111    (9)
                * 256 - 279: 0000000   - 0010111      (7)
                * 280 - 287: 11000000  - 11000111     (8)
                * According to the RFC 1951 rules in section 3.2.2
                * This is used to (de)compress small inputs.
                */
                
                HuffmanNode tree;
                tree.leaf = false;
                tree.value = -1;
                tree.one = NULL;
                tree.zero = NULL;
                
                u16 code = 48;
                
                for(i32 value = 0; value <= 143; value++){
                    HuffmanNode * node = &PUSH(HuffmanNode);
                    
                    node->value = value;
                    node->one = NULL;
                    node->zero = NULL;
                    node->leaf = true;
                    
                    addHuffmanNodeRec(&tree, node, invertBits(code, 8), 8);
                    code++;
                }
                code = 400;
                for(i32 value = 144; value <= 255; value++){
                    HuffmanNode * node = &PUSH(HuffmanNode);
                    
                    node->value = value;
                    node->one = NULL;
                    node->zero = NULL;
                    node->leaf = true;
                    
                    addHuffmanNodeRec(&tree, node, invertBits(code, 9), 9);
                    code++;
                }
                code = 0;
                for(i32 value = 256; value <= 279; value++){
                    HuffmanNode * node = &PUSH(HuffmanNode);
                    
                    node->value = value;
                    node->one = NULL;
                    node->zero = NULL;
                    node->leaf = true;
                    
                    addHuffmanNodeRec(&tree, node, invertBits(code, 7), 7);
                    code++;
                }
                
                code = 192;
                for(i32 value = 280; value <= 287; value++){
                    HuffmanNode * node = &PUSH(HuffmanNode);
                    
                    node->value = value;
                    node->one = NULL;
                    node->zero = NULL;
                    node->leaf = true;
                    
                    addHuffmanNodeRec(&tree, node, invertBits(code, 8), 8);
                    code++;
                }
                distanceTree = literalTree = tree;
            }
            else if((header & 3) == 2){
                PROFILE_SCOPE("Huffman dynamic");
#if PROFILE
                u32 byteStart = head.byteOffset;
#endif
                u16 litCode = readBits(&head, 5);
                u16 distCode = readBits(&head, 5);
                
                //huffmann tree for following lenghts
                u16 howMany3bitCodes = readBits(&head, 4);
                howMany3bitCodes += 4;
                
                CodeWord codeWords[19];
                for(u8 ci = 0; ci < ARRAYSIZE(codeWords); ci++){
                    codeWords[ci].weight = dynamicCodes[ci];
                    codeWords[ci].bitSize = 0;
                }
                
                
                for(u8 bci = 0; bci < howMany3bitCodes; bci++){
                    u8 codeLength = CAST(u8, readBits(&head, 3));
                    codeWords[bci].bitSize = codeLength;
                }
                
                
                //now we have code book for following lz77 lengths
                HuffmanNode quickTree = assignCodesAndBuildTree(codeWords, howMany3bitCodes);
                
                
                //now decoding literals
                CodeWord * literalCodes = &PUSHA(CodeWord, 257 + litCode); 
                readCodes(&head, &quickTree, literalCodes, 257 + litCode);
                
                
                CodeWord * distanceCodes = &PUSHA(CodeWord, distCode + 1);
                readCodes(&head, &quickTree, distanceCodes, distCode + 1);
                
                literalTree = assignCodesAndBuildTree(literalCodes, 257 + litCode);
                distanceTree = assignCodesAndBuildTree(distanceCodes, distCode + 1);
                PROFILE_BYTES(head.byteOffset - byteStart);
            }

            targetOffset += decompressHuffman(&head, &literalTree, &distanceTree, (unsigned char *)target + targetOffset);
            POPI;
        }
    }
    
    PROFILE_BYTES(head.byteOffset);
    return targetOffset;
}


//deflate/inflate
//http://www.gzip.org/algorithm.txt - idea

//more specific - https://en.wikipedia.org/wiki/DEFLATE or RFC
u32 decompressDeflate2(u8 * compressedData, u8 * target){
    PROFILE_FUNC();
    
    //dict 32kbytes
    //maxLen = 256
    
    //start of block - huffman trees for indices and lenghts
    ReadHeadBit2 head = {};
    head.source = compressedData;
    
    //phase1 repeat counts, is this hardcoded defined? or where do these repeat times come from?
    u8 dynamicCodes[] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
    
    u32 targetOffset = 0;
    
    bool processBlock = true;
    while(processBlock){
        PUSHI;
        processBlock = readBits2(&head, 1) == 0;
        u16 header = readBits2(&head, 2);
        
        
        if((header & 3) == 0){
            PROFILE_SCOPE("Huffman literal");
            INV;
            /*
            //a stored/raw/literal section, between 0 and 65,535 bytes in length.
            //http://www.bolet.org/~pornin/deflate-flush.html
            while(head.bitOffset != 0){
                readBits2(&head, 1);
                //ASSERT(bits == 0);
            } //get rid of 0 bytes
            
            u16 dataSize = readBits2(&head, 16);
            //this is redundancy
            readBits2(&head, 16); //complement
            //now the data should be present
            ASSERT(head.bitOffset == 0);
            for(u32 i = 0; i < dataSize; i++){
                target[targetOffset++] = CAST(u8, readBits2(&head, 8));
            }
            PROFILE_BYTES(dataSize);
            */
        }else{
            // - http://commandlinefanatic.com/cgi-bin/showarticle.cgi?article=art001
            //see Listing 14: Read the GZIP pre-header
            HuffmanNode literalTree;
            HuffmanNode distanceTree;
            
            if((header & 3) == 1){
                PROFILE_SCOPE("Huffman static");
                //a static Huffman compressed block, using a pre-agreed Huffman tree.
                /** 
                * Build a Huffman tree for the following values:
                *   0 - 143: 00110000  - 10111111     (8)
                * 144 - 255: 110010000 - 111111111    (9)
                * 256 - 279: 0000000   - 0010111      (7)
                * 280 - 287: 11000000  - 11000111     (8)
                * According to the RFC 1951 rules in section 3.2.2
                * This is used to (de)compress small inputs.
                */
                
                HuffmanNode tree;
                tree.leaf = false;
                tree.value = -1;
                tree.one = NULL;
                tree.zero = NULL;
                
                u16 code = 48;
                
                for(i32 value = 0; value <= 143; value++){
                    HuffmanNode * node = &PUSH(HuffmanNode);
                    
                    node->value = value;
                    node->one = NULL;
                    node->zero = NULL;
                    node->leaf = true;
                    
                    addHuffmanNodeRec(&tree, node, invertBits(code, 8), 8);
                    code++;
                }
                code = 400;
                for(i32 value = 144; value <= 255; value++){
                    HuffmanNode * node = &PUSH(HuffmanNode);
                    
                    node->value = value;
                    node->one = NULL;
                    node->zero = NULL;
                    node->leaf = true;
                    
                    addHuffmanNodeRec(&tree, node, invertBits(code, 9), 9);
                    code++;
                }
                code = 0;
                for(i32 value = 256; value <= 279; value++){
                    HuffmanNode * node = &PUSH(HuffmanNode);
                    
                    node->value = value;
                    node->one = NULL;
                    node->zero = NULL;
                    node->leaf = true;
                    
                    addHuffmanNodeRec(&tree, node, invertBits(code, 7), 7);
                    code++;
                }
                
                code = 192;
                for(i32 value = 280; value <= 287; value++){
                    HuffmanNode * node = &PUSH(HuffmanNode);
                    
                    node->value = value;
                    node->one = NULL;
                    node->zero = NULL;
                    node->leaf = true;
                    
                    addHuffmanNodeRec(&tree, node, invertBits(code, 8), 8);
                    code++;
                }
                distanceTree = literalTree = tree;
                //targetOffset += decompressHuffman(&head, &literalTree, &distanceTree, (unsigned char *)target + targetOffset);
                INV; // re-do with new API from Huffman dynamic
            }
            else if((header & 3) == 2){
                PROFILE_SCOPE("Huffman dynamic");
#if PROFILE
                u8* byteStart = head.source;
                u32 targetOffsetStart = targetOffset;
#endif
                u16 litCode = readBits2(&head, 5);
                u16 distCode = readBits2(&head, 5);
                
                //huffmann tree for following lenghts
                u16 howMany3bitCodes = readBits2(&head, 4);
                howMany3bitCodes += 4;
                
                CodeTable bootstrapTable;
                u8 bitSizes[ARRAYSIZE(dynamicCodes)] = {};
                
                for(u8 bci = 0; bci < howMany3bitCodes; bci++){
                    u8 codeLength = CAST(u8, readBits2(&head, 3));
                    bitSizes[dynamicCodes[bci]] = codeLength;
                }
                
                //now we have code book for following lz77 lengths
                assignCodesAndBuildTree2(bitSizes, ARRAYSIZE(bitSizes), &bootstrapTable);

                //now decoding literals
                u8 bitSizesDyn[257+32];

                u16 literalTableCount = 257 + litCode;
                readCodes2(&head, &bootstrapTable, bitSizesDyn, literalTableCount);
                CodeTable literalTable;
                assignCodesAndBuildTree2(bitSizesDyn, literalTableCount, &literalTable);
                
                u16 distanceTableCount = distCode + 1;
                readCodes2(&head, &bootstrapTable, bitSizesDyn, distanceTableCount);
                CodeTable distanceTable;
                assignCodesAndBuildTree2(bitSizesDyn, distanceTableCount, &distanceTable);
                targetOffset += decompressHuffman2(&head, &literalTable, &distanceTable, (unsigned char *)target + targetOffset);
                PROFILE_BYTES(head.source - byteStart + targetOffset - targetOffsetStart);
            }

            POPI;
        }
    }
    
    PROFILE_BYTES(head.source - compressedData + targetOffset);
    return targetOffset;
}
#endif
