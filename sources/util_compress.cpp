#ifndef UTIL_COMPRESS
#define UTIL_COMPRESS

#include "util_sort.cpp"

struct LZWCompressNode{
    uint16 index;
    LZWCompressNode * next;
};

struct LZWTable{
    struct {
        uint8 carryingSymbol[1 << 12];
        uint16 previousNode[1 << 12];
        LZWCompressNode * children[1 << 12];
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

static inline uint16 invertBits(const uint16 sourceBits, uint8 bitCount){
    uint16 result = 0;
    
    uint16 workBits = sourceBits;
    int16 currentBit = bitCount-1;
    
    while(currentBit >= 0){
        result |= ((workBits & 1) << currentBit);
        currentBit--;
        workBits >>= 1;
    }
    
    return result;
}

static inline uint16 readBits(ReadHeadBit * head, const uint8 bits){
    ASSERT(bits > 0);
    ASSERT(bits <= 16);
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
    
    return result;
}

uint32 decompressLZW(const byte * source, const uint32 sourceSize, byte * target){
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
    if(*source != 128 || (~*(source + 1) & 128) != 128){
        POPI;
        return 0;
    }
    
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
            uint16 crawlIndex = currentTableIndex;
            if(currentTableIndex == table->count){
                crawlIndex = previousTableIndex;
            }
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
    uint32 byteOffset;
    uint8 bitOffset;
};

static inline void writeBits(WriteHeadBit * head, uint16 data, const uint8 bits){
    ASSERT(bits > 0);
    ASSERT(bits <= 16);
    uint8 toWrite = bits;
    data = data << (16 - bits);
    while(toWrite != 0){
        uint8 currentWriting = MIN(8 - head->bitOffset, toWrite);
        
        //clear with zeros
        head->source[head->byteOffset] &=
            (uint8)((uint8)(0xFF >> (8 - head->bitOffset)) << (8 - head->bitOffset)) |
            (uint8)(0xFF >>  (head->bitOffset + currentWriting));
        
        uint8 write = ((uint8)(((uint8)(((data >> 8) >> head->bitOffset)) >> (8 - head->bitOffset - currentWriting)))) << (8 - head->bitOffset - currentWriting);
        
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


uint32 compressLZW(byte * source, const uint32 sourceSize, byte * target){
    //http://www.fileformat.info/format/tiff/corion-lzw.htm
    //every message begins with clear code and ends with end of information code
    WriteHeadBit compressedDataHead = {};
    compressedDataHead.source = target;
    uint16 endOfInfo = 257;
    uint16 clearCode = 256;
    PUSHI;
    LZWTable * table = &PUSH(LZWTable);
    LZWCompressNode * pool = &PUSHA(LZWCompressNode, 1 << 12);
    uint16 poolCount = 0;
    table->count = 4094;
    table->bits = 9;
    
    uint16 currentCrawl = source[0];
    bool fresh = true;
    for(uint32 i = 1; i < sourceSize; i++){
        if(table->count == 4094){
            ASSERT((i == 1 && table->bits == 9) || (i != 1 && table->bits == 12));
            writeBits(&compressedDataHead, clearCode, table->bits);
            for(uint16 i = 0; i < 258; i++){
                table->data.carryingSymbol[i] = i;
                table->data.children[i] = NULL;
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
            uint16 newIndex = table->count;
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


struct HuffmanNode{
    bool leaf;
    int32 value;
    uint8 bit;
    HuffmanNode * one;
    HuffmanNode * zero;
};

struct CodeWord{
    uint8 dynamicCode;
    uint16 code;
    uint8 bitSize;
};



void addHuffmanNodeRec(HuffmanNode * tree, HuffmanNode * node, uint16 code, uint8 codeLenght){
    
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

//deflate/inflate
//http://www.gzip.org/algorithm.txt - idea

//more specific - https://en.wikipedia.org/wiki/DEFLATE or RFC
bool decompressDeflate(const char * compressedData, const uint32 compressedSize, char * target){
    //dict 32kbytes
    //maxLen = 256
    
    //start of block - huffman trees for indices and lenghts
    ReadHeadBit head = {};
    head.source = (const unsigned char*)compressedData;
    
    //phase1 repeat counts, is this hardcoded defined? or where do these repeat times come from?
    uint8 dynamicCodes[] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
    uint8 ascendingIndices[] = {
        3, 17, 15, 13, 11, 9, 7, 5, 4, 6, 8, 10, 12, 14, 16, 18, 0, 1, 2 
    };
    
    int32 extraCounts[] = {11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163,195, 227};
    int32 extraBackOffsets[] = {4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512, 768, 1024, 1536, 2048, 3072, 4096, 6144, 8192, 12288, 16384, 24576};
    
    uint32 targetOffset = 0;
    
    bool processBlock = true;
    while(processBlock){
        processBlock = readBits(&head, 1) & 1;
        uint16 header = invertBits(readBits(&head, 2), 2);
        
        
        if((header & 3) == 0){
            //a stored/raw/literal section, between 0 and 65,535 bytes in length.
            ASSERT(!"implement me");
        }else if((header & 3) == 1){
            
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
            
            uint16 code = 48;
            
            for(int32 value = 0; value <= 143; value++){
                HuffmanNode * node = &PUSH(HuffmanNode);
                
                node->value = value;
                node->one = false;
                node->zero = false;
                node->leaf = true;
                
                addHuffmanNodeRec(&tree, node, invertBits(code, 8), 8);
                code++;
            }
            code = 400;
            for(int32 value = 144; value <= 255; value++){
                HuffmanNode * node = &PUSH(HuffmanNode);
                
                node->value = value;
                node->one = false;
                node->zero = false;
                node->leaf = true;
                
                addHuffmanNodeRec(&tree, node, invertBits(code, 9), 9);
                code++;
            }
            code = 0;
            for(int32 value = 256; value <= 279; value++){
                HuffmanNode * node = &PUSH(HuffmanNode);
                
                node->value = value;
                node->one = false;
                node->zero = false;
                node->leaf = true;
                
                addHuffmanNodeRec(&tree, node, invertBits(code, 7), 7);
                code++;
            }
            
            code = 192;
            for(int32 value = 280; value <= 287; value++){
                HuffmanNode * node = &PUSH(HuffmanNode);
                
                node->value = value;
                node->one = false;
                node->zero = false;
                node->leaf = true;
                
                addHuffmanNodeRec(&tree, node, invertBits(code, 8), 8);
                code++;
            }
            
            
            //inflating huffman codes
            HuffmanNode * currentNode = &tree;
            while(head.byteOffset < compressedSize){
                if(readBits(&head, 1) & 1){
                    currentNode = currentNode->one;
                }else{
                    currentNode = currentNode->zero;
                }
                if(currentNode->leaf){
                    if(currentNode->value < 256){
                        target[targetOffset++] = (char)currentNode->value;
                    }else if(currentNode->value == 256){
                        break;
                    }else{
                        //repetition, defined in specification
                        int32 count;
                        if(currentNode->value < 265){
                            count = 254 - currentNode->value;
                        }else if(currentNode->value < 285){
                            uint8 bits = (currentNode->value - 261) / 4;
                            count = invertBits(readBits(&head, bits), bits) + extraCounts[currentNode->value - 265];
                        }else{
                            count = 258;
                        }
                        
                        int32 backOffset = readBits(&head, 5);
                        if(backOffset > 3){
                            uint8 bits = (backOffset-2)/2;
                            backOffset =  invertBits(readBits(&head, bits), bits) + extraBackOffsets[backOffset-4];
                        }
                        
                        char * start = target + targetOffset - 1 - backOffset;
                        
                        for(int32 i = 0; i < count; i++){
                            target[targetOffset++] = start[i];
                        }
                        
                    }
                    currentNode = &tree;
                }
            }
            ASSERT(currentNode->value == 256); //ended with close node
            
            
        }else if((header & 3) == 2){
            
            
            // - http://commandlinefanatic.com/cgi-bin/showarticle.cgi?article=art001
            //see Listing 14: Read the GZIP pre-header
            
            uint16 litCode = invertBits(readBits(&head, 5), 5);
            uint16 distCode = invertBits(readBits(&head, 5), 5);
            
            
            //huffmann tree for following lenghts
            uint16 howMany3bitCodes = invertBits(readBits(&head, 4), 4);
            howMany3bitCodes += 4;
            
            uint16 codeLengthsForRepeat[19];
            for(uint8 i = 0; i < ARRAYSIZE(codeLengthsForRepeat); i++){
                codeLengthsForRepeat[i] = 0;
            }
            
            
            CodeWord codeWords[19];
            for(uint8 ci = 0; ci < ARRAYSIZE(codeWords); ci++){
                codeWords[ci].bitSize = 0;
                codeWords[ci].dynamicCode = dynamicCodes[ci]; 
            }
            
            
            //kinda RlE 
            for(uint8 bci = 0; bci < howMany3bitCodes; bci++){
                uint16 codeLength = invertBits(readBits(&head, 3), 3);
                codeLengthsForRepeat[dynamicCodes[bci]] = codeLength;
            }
            
            //code words:
            //This is one more than the last code of the previous bit length, left-shifted once.
            //8 for 3 bits
            uint16 currentCode = 0;
            
#if 0
            uint16 mockedLengths[] = {6, 7, 7, 3, 3, 2, 3, 3, 4, 4, 5, 4};
            
            howMany3bitCodes = 12;
            for(uint8 bci = 0; bci < howMany3bitCodes; bci++){
                codeLengthsForRepeat[dynamicCodes[bci]] = mockedLengths[bci];
            }
            
#endif
            for(uint8 codeLength = 1; codeLength < 8; codeLength++){
                //proccess each code length
                
                uint8 currentLengthIndices[19];
                uint8 currentLengthCount = 0;
                for(uint8 bci = 0; bci < howMany3bitCodes; bci++){
                    if(codeLengthsForRepeat[dynamicCodes[bci]] == codeLength){
                        currentLengthIndices[currentLengthCount++] = bci;
                    }
                }
                
                if(currentLengthCount > 0){
                    //we now have indices with currennt length
                    //we need to sort them 
                    
                    
                    struct WeightAndIndex{
                        uint8 weight;
                        uint8 index;
                    };
                    
                    WeightAndIndex currentWeights[19];
                    
                    
                    for(uint8 li = 0; li < currentLengthCount; li++){
                        
                        for(uint8 weight = 0; weight < ARRAYSIZE(ascendingIndices); weight++){
                            if(ascendingIndices[weight] == currentLengthIndices[li]){
                                currentWeights[li].weight = weight;
                                currentWeights[li].index = currentLengthIndices[li];
                                break;
                            }
                        }
                        
                    }
                    
                    //sort by weight
                    insertSort((byte*)currentWeights, sizeof(WeightAndIndex), currentLengthCount, [](void * a, void * b) -> int8 {
                               WeightAndIndex * A = (WeightAndIndex *) a;
                               WeightAndIndex * B = (WeightAndIndex *) b;
                               if(A->weight < B->weight){
                               return -1;
                               }
                               return 1;
                               });
                    
                    //we now have weights, e.g order of code 
                    
                    for(uint8 li = 0; li < currentLengthCount; li++){
                        codeWords[currentWeights[li].index].code = currentCode;
                        codeWords[currentWeights[li].index].bitSize = codeLength;
                        currentCode++;
                    }
                    
                    
                }
                currentCode = currentCode << 1;
            }
            
            
            
            
            //now we have code book for following lz77 lengths
            
            HuffmanNode tree;
            tree.leaf = false;
            tree.value = -1;
            tree.one = NULL;
            tree.zero = NULL;
            
            for(uint8 ci = 0; ci < ARRAYSIZE(codeWords); ci++){
                if(codeWords[ci].bitSize != 0){
                    HuffmanNode * node = &PUSH(HuffmanNode);
                    
                    node->leaf = true;
                    node->value = codeWords[ci].dynamicCode;
                    node->one = NULL;
                    node->zero = NULL;
                    
                    addHuffmanNodeRec(&tree, node, invertBits(codeWords[ci].code, codeWords[ci].bitSize), codeWords[ci].bitSize);
                }
            }
            
            //now decoding literals
            uint16 literalIndex = 0;
            uint16 literalCodeLengths[258];
            
            HuffmanNode * currentNode = &tree;
            while(head.byteOffset < compressedSize && literalIndex < 258){
                
                if(readBits(&head, 1) & 1){
                    currentNode = currentNode->one;
                }else{
                    currentNode = currentNode->zero;
                }
                ASSERT(currentNode != NULL);
                if(currentNode->leaf){
                    
                    //17 repeat 0 n times n = 3 bits
                    if(currentNode->value == 17){
                        uint16 times0 = invertBits(readBits(&head, 3), 3);
                        times0 += 3;
                        
                        for(uint16 ri = 0; ri < times0; ri++){
                            literalCodeLengths[literalIndex++] = 0;
                        }
                        
                        
                    }else if(currentNode->value == 18){
                        //18 repeat 0 n times n = 7 bits
                        uint16 times0 = invertBits(readBits(&head, 7), 7);
                        times0 += 11;
                        for(uint16 ri = 0; ri  < times0; ri++){
                            literalCodeLengths[literalIndex++] = 0;
                        }
                        
                    }else if(currentNode->value == 16){
                        ////16 repeat previous n times
                        uint16 timesRepeat = invertBits(readBits(&head, 2), 2);
                        timesRepeat += 3;
                        for(uint16 ri = 0; ri < timesRepeat; ri++){
                            literalCodeLengths[literalIndex] = literalCodeLengths[literalIndex-1];
                            literalIndex++;
                        }
                    }else{
                        literalCodeLengths[literalIndex] = currentNode->value;
                        literalIndex++;
                    }
                    
                    
                    currentNode = &tree;
                }
                
                
                
                bool tru = true;
            }
        }else{
            ASSERT(false);
            return false;
        }
        
    }
    
    return true;
}


#endif