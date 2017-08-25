#ifndef UTIL_CONV
#define UTIL_CONV

//returns number of unmached chars
uint32 convUTF8toAscii(const byte * source, const uint32 bytesize, char ** target, uint32 * targetSize){
    //inefficient alloc now, fuck that tho
    *target = &PUSHA(char, bytesize);
    *targetSize = 0;
    
    uint32 errors = 0;
    char glyf;
    for(uint32 i = 0; i < bytesize; i++){
        if(!(source[i] & 128)){
            glyf = source[i];
        }else if(!(source[i] & 32)){
            glyf = '_';
            i++;
            errors++;
        }else if(!(source[i] & 16)){
            glyf = '_';
            i+=2;
            errors++;
        }else{
            glyf = '_';
            i+=4;
            errors++;
        }
        (*target)[*targetSize] = glyf;
        *targetSize = *targetSize + 1;
    }
    return errors;
}


#include "util_compress.cpp"


enum ByteOrder{
    ByteOrder_Invalid,
    ByteOrder_LittleEndian,
    ByteOrder_BigEndian
};

struct ReadHead{
    char * offset;
};

union converter{
    uint8 bytes[8];
    uint16 words[4];
    uint32 dwords[2];
    uint64 qword;
};

static inline void swapEndians(converter * martyr){
    for(uint8 i = 0; i < 8; i++){
        uint8 tmp = 0;
        for(uint8 b = 0; b < 8; b++){
            tmp |= ((martyr->bytes[i] >> b) << (7-b));
        }
        martyr->bytes[i] = tmp;
    }
}

static inline uint32 swapEndians(const uint32 source){
    uint32 target = 0;
    for(uint8 b = 0; b < 32; b++){
        target |= ((source >> b) << (31-b));
    }
    return target;
}

static inline void swapEndians(char * source, uint32 length, char * target){
    for(uint32 i = 0; i < length; i++){
        target[i] = 0;
        for(uint8 b = 0; b < 8; b++){
            target[i] |= ((source[i] >> b) << (7-b));
        }
        
    }
}

static converter martyr;

static inline uint8 scanByte(ReadHead * head){
    uint8 res = head->offset[0];
    head->offset++;
    return res;
}

static inline uint16 scanWord(ReadHead * head){
    uint16 res;
    martyr.bytes[0] = head->offset[0];
    martyr.bytes[1] = head->offset[1];
    head->offset += 2;
    res = martyr.words[0];
    return res;
}


static inline uint32 scanDword(ReadHead * head){
    uint32 res;
    martyr.bytes[0] = head->offset[0];
    martyr.bytes[1] = head->offset[1];
    martyr.bytes[2] = head->offset[2];
    martyr.bytes[3] = head->offset[3];
    head->offset += 4;
    res = martyr.dwords[0];
    return res;
}


static inline uint64 scanQword(ReadHead * head){
    uint64 res;
    martyr.bytes[0] = head->offset[0];
    martyr.bytes[1] = head->offset[1];
    martyr.bytes[2] = head->offset[2];
    martyr.bytes[3] = head->offset[3];
    martyr.bytes[4] = head->offset[4];
    martyr.bytes[5] = head->offset[5];
    martyr.bytes[6] = head->offset[6];
    martyr.bytes[7] = head->offset[7];
    head->offset += 8;
    res = martyr.qword;
    return res;
}



struct Bitmapinfoheader{
    union{
        struct{
            uint32 size;
            uint32 width;
            uint32 height;
            uint16 colorPlanes;
            uint16 bitsPerPixel;
            uint32 compression;
            uint32 datasize;
            int32 pixelPerMeterHorizontal;
            int32 pixelPerMeterVertical;
            uint32 colorsInPallette;
            uint32 importantColorsAmount;
        };
        char data[40];
    };
};

bool decodeBMP(const FileContents * source, Image * target){
    ASSERT(source->contents[0] == 'B');
    ASSERT(source->contents[1] == 'M');
    uint32 filesize = *((uint32 *)(source->contents + 2));
    uint32 dataOffset = *((uint32 *)(source->contents + 10));
    
    Bitmapinfoheader * infoheader = (Bitmapinfoheader *)(source->contents + 14);
    target->info.width = infoheader->width;
    target->info.height = infoheader->height;
    ASSERT(infoheader->compression == 0);
    ASSERT(infoheader->colorPlanes == 1);
    ASSERT(infoheader->bitsPerPixel == 8);
    target->info.bitsPerSample = infoheader->bitsPerPixel;
    target->info.samplesPerPixel = 1;
    target->info.origin = BitmapOriginType_BottomLeft;
    target->info.interpretation = BitmapInterpretationType_GrayscaleBW01;
    target->data = &PUSHA(byte, infoheader->datasize);
    for(uint32 i = 0; i < infoheader->datasize; i++){
        target->data[i] = (source->contents + dataOffset)[i];
    }
    return true;
}

bool encodeBMP(const Image * source, FileContents * target){
    bool palette = false;
    uint32 bitsPerPixel = source->info.bitsPerSample * source->info.samplesPerPixel;
    uint32 savewidth = source->info.width;
    //each line must be within 32 bits boundary, filled with zeroes
    while(((savewidth * bitsPerPixel) / 32) * 32 != savewidth * bitsPerPixel){
        savewidth++;
    }
    if(bitsPerPixel <= 8){
        palette = true;
    }
    if(!palette){
        target->size = 14 + sizeof(Bitmapinfoheader) + (savewidth * source->info.height * bitsPerPixel/8);
    }else{
        target->size = 14 + sizeof(Bitmapinfoheader) + (savewidth * source->info.height * bitsPerPixel/8) + (4 * (1 << bitsPerPixel));
    }
    target->contents = &PUSHA(char, target->size);
    
    target->contents[0] = 'B';
    target->contents[1] = 'M';
    *((uint32 *)(target->contents + 2)) = target->size;
    *((uint32 *)(target->contents + 6)) = 0;
    uint32 dataOffset = *((uint32 *)(target->contents + 10)) = 14 + sizeof(Bitmapinfoheader) + (palette ? (4 * (1 << bitsPerPixel)) : 0);
    
    Bitmapinfoheader * infoheader = (Bitmapinfoheader *)(target->contents + 14);
    infoheader->size = sizeof(Bitmapinfoheader);
    infoheader->width = source->info.width;
    infoheader->height = source->info.height;
    infoheader->colorPlanes = 1;
    infoheader->bitsPerPixel = bitsPerPixel;
    infoheader->compression = 0;
    infoheader->datasize = savewidth * source->info.height * bitsPerPixel/8;
    infoheader->pixelPerMeterVertical = infoheader->pixelPerMeterHorizontal = 0;
    infoheader->colorsInPallette = 1 << infoheader->bitsPerPixel;
    infoheader->importantColorsAmount = 1 << infoheader->bitsPerPixel;
    if(palette){
        ASSERT((1 << bitsPerPixel) == 256);
        ASSERT(source->info.interpretation == BitmapInterpretationType_GrayscaleBW01);
        for(uint32 i = 0; i < (1 << bitsPerPixel); i++){
            //R
            *(target->contents + 14 + sizeof(Bitmapinfoheader) + i*4) = (uint8)i;
            //G
            *(target->contents + 14 + sizeof(Bitmapinfoheader) + i*4 + 1) = (uint8)i;
            //B
            *(target->contents + 14 + sizeof(Bitmapinfoheader) + i*4 + 2) = (uint8)i;
            //unused
            *(target->contents + 14 + sizeof(Bitmapinfoheader) + i*4 + 3) = (uint8)0;
        }
    }
    ASSERT(bitsPerPixel == 8);
    if(source->info.origin == BitmapOriginType_BottomLeft){
        for(uint32 h = 0; h < infoheader->height; h++){
            for(uint32 w = 0; w < infoheader->width; w++){(target->contents + dataOffset)[h * savewidth + w] = source->data[h * infoheader->width + w]; 
            }
            for(uint32 w = infoheader->width; w < savewidth; w++){
                (target->contents + dataOffset)[h * savewidth + w] = 0;
            }
        }
    }else if(source->info.origin == BitmapOriginType_TopLeft){
        for(uint32 h = 0; h < infoheader->height; h++){
            for(uint32 w = 0; w < infoheader->width; w++){
                (target->contents + dataOffset)[h * savewidth + w] =  source->data[(infoheader->height - 1 - h) * infoheader->width + w];
            }
            for(uint32 w = infoheader->width; w < savewidth; w++){
                (target->contents + dataOffset)[h * savewidth + w + w] = 0;
            }
        }
    }else{
        INV;
    }
    return true;
}


bool decodeTiff(const FileContents * file, Image * target){
    PUSHI;
    //http://www.fileformat.info/format/tiff/corion.htm
    ReadHead head;
    head.offset = file->contents;
    ByteOrder order = ByteOrder_Invalid;
    char end = scanByte(&head);
    if(end == 'I'  && scanByte(&head) == 'I'){
        order = ByteOrder_LittleEndian;
    }else if(end == 'M' && scanByte(&head) == 'M'){
        order = ByteOrder_BigEndian;
        ASSERT(!"implement");
        return false;
    }else{
        INV;
        return false;
    }
    
    
    uint16 tiffMagicFlag = scanWord(&head);
    //tiff const flag
    if(tiffMagicFlag != 42){
        INV;
        return false;
    }
    
    uint32 imageOffset = scanDword(&head);
    head.offset = file->contents + imageOffset;
    
    uint16 entries = scanWord(&head);
    
    uint32 * stripOffsets = NULL;
    uint32 rowsPerStrip = -1; //infinity (1 strip for all data)
    uint32 * stripSizes = NULL;
    uint32 stripAmount = 0;
    
    for(uint16 ei = 0; ei < entries; ei++){
        uint16 tag = scanWord(&head);
        uint16 type = scanWord(&head);
        uint32 length = scanDword(&head);
        uint32 headerOffset = scanDword(&head);
        
        ASSERT(type >= 1 && type <= 5);
        //these appear in ascending order, but we do not care
        switch(tag){
            case 254:{
                //new subfile type
                ASSERT(headerOffset == 0);
                if(headerOffset != 0){
                    POPI;
                    return false;
                }
                //not supporting anything special now, only raw data in data field
            }break;
            case 256:{
                //image width
                target->info.width = headerOffset;
            }break;
            case 257:{
                //image height
                target->info.height = headerOffset;
            }break;
            case 258:{
                //bits per sample //could wary, depends on samples per pixel
                target->info.bitsPerSample = headerOffset;
            }break;
            case 259:{
                //compression
                ASSERT(headerOffset == 5);
                if(headerOffset != 5){
                    POPI;
                    return false;
                }
                //supporting LZW only now
            }break;
            case 262:{
                //photometric interpretation
                ASSERT(headerOffset == 1);
                if(headerOffset != 1){
                    POPI;
                    return false;
                }
                //supporting bw only now
                target->info.interpretation = BitmapInterpretationType_GrayscaleBW01;
            }break;
            
            case 266:{
                //http://www.awaresystems.be/imaging/tiff/tifftags/fillorder.html
                //logical bits in bytes order
                ASSERT(headerOffset == 1);
                if(headerOffset != 1){
                    POPI;
                    return false;
                }
                //support MSB first only for now
            }break;
            case 269:{
                //The name of the document from which this image was scanned.
            }break;
            case 270:{
                //For example,  a user  may wish  to attach a comment such as "1988 company picnic" to an image.
            }break;
            case 273:{
                //strip offsets (essentialy compressed data lines?);
                ASSERT(type == 4);
                if(type != 4){
                    POPI;
                    return false;
                }
                //support dwords only now
                stripOffsets = &PUSHA(uint32, length);
                ASSERT(stripAmount == 0 || length == stripAmount);
                if(stripAmount != 0 && length != stripAmount){
                    POPI;
                    return false;
                }
                stripAmount = length;
                if(length == 1){
                    stripOffsets[0] = headerOffset;
                }else{
                    for(uint32 stripIndex = 0; stripIndex < length; stripIndex++){
                        stripOffsets[stripIndex] = ((uint32 *)(file->contents + headerOffset))[stripIndex];
                    }
                }
            }break;
            case 274:{
                //data orientation
                ASSERT(headerOffset == 1);
                if(headerOffset != 1){
                    POPI;
                    return false;
                }
                //supporting TopLeft (rows->rows, cols->cols) so far
                target->info.origin = BitmapOriginType_TopLeft;
            }break;
            case 277:{
                //samples per pixel
                ASSERT(headerOffset == 1);
                if(headerOffset != 1){
                    POPI;
                    return false;
                }
                //support BW for now only
                target->info.samplesPerPixel = headerOffset;
            }break;
            case 278:{
                rowsPerStrip = headerOffset;
            }break;
            case 279:{
                //strip byte counts (essentially bytes per strip);
                ASSERT(type == 4);
                if(type != 4){
                    POPI;
                    return false;
                }
                //support dwords only now
                stripSizes = &PUSHA(uint32, length);
                ASSERT(stripAmount == 0 || length == stripAmount);
                if(stripAmount != 0 && length != stripAmount){
                    POPI;
                    return false;
                }
                stripAmount = length;
                if(length == 1){
                    stripSizes[0] = headerOffset;
                }else{
                    for(uint32 stripIndex = 0; stripIndex < length; stripIndex++){
                        stripSizes[stripIndex] = ((uint32 *)(file->contents + headerOffset))[stripIndex];
                    }
                }
            }break;
            case 282:{
                //Xresolution - real image data ignore for now
            }break;
            case 283:{
                //Yresolution - real image data ignore for now
            }break;
            case 284:{
                //planar configuration
                ASSERT(headerOffset == 1);
                if(headerOffset != 1){
                    POPI;
                    return false;
                }
                //use continuous data representation for now
                
            }break;
            case 296:{
                //resolution unit
                //cm/inch we dont give fuck for now
            }break;
            case 306:{
                //date time we dont give fuck for now
            }break;
            case 315:{
                //artist we dont give fuck for now
            }break;
            default:{
                INV;
                POPI;
                return false;
            }break;
            
        }
    }
    
    uint32 nextFile = scanDword(&head);
    if(nextFile != 0){
        INV;
        return false;
    }
    
    target->data = &PPUSHA(byte, target->info.width * target->info.height * target->info.samplesPerPixel * (target->info.bitsPerSample / 8));
    
    for(uint32 i = 0; i < target->info.width * target->info.height * target->info.samplesPerPixel * (target->info.bitsPerSample / 8); i++){
        target->data[i] = (char)255;
    }
    
    uint32 total = 0;
    uint32 last = 0;
    
    for(uint32 stripIndex = 0; stripIndex < stripAmount; stripIndex++){
        last = decompressLZW((const byte *)file->contents + stripOffsets[stripIndex], stripSizes[stripIndex], target->data + stripIndex * rowsPerStrip * target->info.width);
        if(last == 0){
            POPI;
            return false;
        }
        total += last;
    }
    
    ASSERT(total == target->info.width * target->info.height * target->info.samplesPerPixel * (target->info.bitsPerSample / 8));
    
    POPI;
    return true;
}



struct WriteHead{
    char * offset;
};

static inline void writeByte(WriteHead * head, uint8 data){
    head->offset[0] = data;
    head->offset++;
}

static inline void writeWord(WriteHead * head, uint16 data){
    martyr.words[0] = data;
    head->offset[0] = martyr.bytes[0];
    head->offset[1] = martyr.bytes[1];
    head->offset += 2;
}


static inline void writeDword(WriteHead * head, uint32 data){
    martyr.dwords[0] = data;
    head->offset[0] = martyr.bytes[0];
    head->offset[1] = martyr.bytes[1];
    head->offset[2] = martyr.bytes[2];
    head->offset[3] = martyr.bytes[3];
    head->offset += 4;
}


static inline void writeQword(WriteHead * head, uint64 data){
    martyr.qword = data;
    head->offset[0] = martyr.bytes[0];
    head->offset[1] = martyr.bytes[1];
    head->offset[2] = martyr.bytes[2];
    head->offset[3] = martyr.bytes[3];
    head->offset[4] = martyr.bytes[4];
    head->offset[5] = martyr.bytes[5];
    head->offset[6] = martyr.bytes[6];
    head->offset[7] = martyr.bytes[7];
    head->offset += 8;
}




bool encodeTiff(const Image * source, FileContents * target){
    PUSHI;
    
    //this should be calculated dynamically later
    uint32 entries  = 9;
    
    uint32 bytesize = source->info.width * source->info.height * (source->info.bitsPerSample/8) * source->info.samplesPerPixel;
    uint32 rowBytesize = bytesize / source->info.height;
    
    uint32 stripRecommendedSize = KILOBYTE(8);
    uint32 rowsPerStrip = stripRecommendedSize / rowBytesize;
    
    if(rowsPerStrip == 0) rowsPerStrip++;
    if(rowsPerStrip > source->info.height) rowsPerStrip = source->info.height;
    
    uint32 stripAmount = source->info.height / rowsPerStrip;
    if(source->info.height % rowsPerStrip != 0){
        stripAmount++;
    }
    uint32 stripSize = rowBytesize * rowsPerStrip;
    ASSERT(stripAmount >= 1);
    ASSERT(rowsPerStrip >= 1);
    
    //compress - wild quess now, later compress first, then work with exact numbers
    uint32 compressedSize = MEGABYTE(50);
    target->size =  (8 + (12 * entries) + 4) + (source->info.samplesPerPixel + 2*stripAmount) * 4 + compressedSize;
    target->contents = &PPUSHA(char, target->size);
    
    
    
    //http://www.fileformat.info/format/tiff/corion.htm
    WriteHead head;
    head.offset = target->contents;
    
    //endianity
    writeByte(&head, (uint8) 'I');
    writeByte(&head, (uint8) 'I');
    //magic constant
    writeWord(&head, 42);
    
    //image data offset
    writeDword(&head, 8); //this dword + previous word + prev 2 bytes
    
    
    writeWord(&head, entries);
    
    
    
    
    
    //every head strip has 12 bytes
    WriteHead dataHead;
    dataHead.offset = head.offset +  (12 * entries) + 4;
    
    //image division
    uint32 * stripOffsets = NULL;
    uint32 * stripSizes = NULL;
    
    
    
    //tags must be in ascending order!!!!!!!!!
    
    //word tag
    //word type 1 - byte, 2 ascii string, 3 word, 4 dword, 5 rational
    //dword length
    //dword headerOffset
    
    
    //new subfile type, default is 0, no need to fill it in
    /*writeWord(&head, 254);
    writeWord(&head, 4);
    writeDword(&head, 1);
    writeDword(&head, 0);//not supporting anything special now, only raw data in data field
    */
    
    //image width
    writeWord(&head, 256);
    writeWord(&head, 4);
    writeDword(&head, 1);
    writeDword(&head, source->info.width);
    
    //image height
    writeWord(&head, 257);
    writeWord(&head, 4);
    writeDword(&head, 1);
    writeDword(&head, source->info.height);
    
    //bits per sample
    writeWord(&head, 258);
    writeWord(&head, 3);
    writeDword(&head, source->info.samplesPerPixel);
    if(source->info.samplesPerPixel == 1){
        writeDword(&head, source->info.bitsPerSample);
    }else{
        writeDword(&head, dataHead.offset - target->contents);
        for(uint16 i = 0; i < source->info.samplesPerPixel; i++){
            writeWord(&dataHead, source->info.bitsPerSample);
        }
    }
    
    //compression - lzw
    writeWord(&head, 259);
    writeWord(&head, 3);
    writeDword(&head, 1);
    writeDword(&head, 5); //lzw
    
    //photometric interpretation
    writeWord(&head, 262);
    writeWord(&head, 3);
    writeDword(&head, 1);
    writeDword(&head, 1); //supporting bw only now
    ASSERT(source->info.interpretation == BitmapInterpretationType_GrayscaleBW01);
    
    
    //http://www.awaresystems.be/imaging/tiff/tifftags/fillorder.html
    //logical bits in bytes order
    /* default suits us 
    writeWord(&head, 266);
    writeWord(&head, 3);
    writeDword(&head, 1);
    writeDword(&head, 1);
    */
    
    //For example,  a user  may wish  to attach a comment such as "1988 company picnic" to an image.
    writeWord(&head, 270);
    writeWord(&head, 2);
    const char * desc = "Generated by GeLab. Visit gelab.fidli.eu for more information";
    writeDword(&head, strlen(desc) + 1);
    writeDword(&head, dataHead.offset - target->contents);
    strcpy(dataHead.offset, desc);
    dataHead.offset += strlen(desc) + 1;
    
    
    //strip offsets ?);
    //these will be filled later when compressing
    writeWord(&head, 273);
    writeWord(&head, 4);
    writeDword(&head, stripAmount);
    writeDword(&head, dataHead.offset - target->contents);
    stripOffsets = (uint32 *) dataHead.offset;
    dataHead.offset += stripAmount * 4; //allocate space for future
    
    
    //data orientation
    /* default suits us 
    writeWord(&head, 274);
    writeWord(&head, 3);
    writeDword(&head, 1);
    writeDword(&head, 1); //supporting TopLeft (rows->rows, cols->cols)
    */
    ASSERT(source->info.origin == BitmapOriginType_TopLeft);
    
    //data orientation
    /* default suits us
    writeWord(&head, 277);
    writeWord(&head, 3);
    writeDword(&head, 1);
    writeDword(&head, 1); ////support BW for now only
    */
    ASSERT(source->info.samplesPerPixel == 1);
    
    //rows per strip
    writeWord(&head, 278);
    writeWord(&head, 4);
    writeDword(&head, 1);
    writeDword(&head, rowsPerStrip);
    
    
    //strip byte counts (essentially bytes per strip);
    //these will be filled later, when compressing
    writeWord(&head, 279);
    writeWord(&head, 4);
    writeDword(&head, stripAmount);
    writeDword(&head, dataHead.offset - target->contents);
    stripSizes = (uint32 *) dataHead.offset;
    dataHead.offset += stripAmount * 4; //allocate space for future
    
    //can be ommited?
    /*
    case 282:{
    //Xresolution - real image data ignore for now
    }break;
    case 283:{
    //Yresolution - real image data ignore for now
    }break;
    */
    
    //planar configuration
    /* default suits us
    writeWord(&head, 284);
    writeWord(&head, 3);
    writeDword(&head, 1);
    writeDword(&head, 1); //continuous pixels in memory 
    */
    
    //can be ommited?
    /*
    case 296:{
    //resolution unit
    //cm/inch we dont give fuck for now
    }break;
    */
    
    
    //1 image in this tiff data
    writeDword(&head, 0); //no next file
    
    
    for(uint32 stripIndex = 0; stripIndex < stripAmount; stripIndex++){
        uint32 sourceSize = stripSize;
        if(stripIndex == stripAmount - 1){
            sourceSize = bytesize - (stripIndex * sourceSize);
        }
        uint32 size = compressLZW((byte *) (source->data + stripIndex * rowsPerStrip * source->info.width), sourceSize, (byte *) dataHead.offset); 
        stripSizes[stripIndex] = size;
        stripOffsets[stripIndex] = dataHead.offset - target->contents;
        dataHead.offset += size;
    }
    
    target->size = dataHead.offset - target->contents;
    
    
    POPI;
    return true;
}


#endif