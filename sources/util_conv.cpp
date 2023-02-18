#ifndef UTIL_CONV
#define UTIL_CONV

#include "util_image.cpp"
#include "util_audio.h"

//returns number of unmached chars
u32 convUTF8toAscii(const byte * source, const u32 bytesize, char ** target, u32 * targetSize){
    
    *targetSize = 0;
    
    u32 errors = 0;
    char glyf;
    for(u32 i = 0; i < bytesize; i++){
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
    u8 bytes[8];
    u16 words[4];
    u32 dwords[2];
    u64 qword;
};

static inline void swapEndians(converter * martyr){
    for(u8 i = 0; i < 8; i++){
        u8 tmp = 0;
        for(u8 b = 0; b < 8; b++){
            tmp |= ((martyr->bytes[i] >> b) << (7-b));
        }
        martyr->bytes[i] = tmp;
    }
}

static inline u32 swapEndians(const u32 source){
    u32 target = 0;
    for(u8 b = 0; b < 32; b++){
        target |= ((source >> b) << (31-b));
    }
    return target;
}

static inline void swapEndians(char * source, u32 length, char * target){
    for(u32 i = 0; i < length; i++){
        target[i] = 0;
        for(u8 b = 0; b < 8; b++){
            target[i] |= ((source[i] >> b) << (7-b));
        }
        
    }
}

static converter martyr;

static inline u8 scanByte(ReadHead * head){
    u8 res = head->offset[0];
    head->offset++;
    return res;
}

static inline u16 scanWord(ReadHead * head){
    u16 res;
    martyr.bytes[0] = head->offset[0];
    martyr.bytes[1] = head->offset[1];
    head->offset += 2;
    res = martyr.words[0];
    return res;
}


static inline u32 scanDword(ReadHead * head){
    u32 res;
    martyr.bytes[0] = head->offset[0];
    martyr.bytes[1] = head->offset[1];
    martyr.bytes[2] = head->offset[2];
    martyr.bytes[3] = head->offset[3];
    head->offset += 4;
    res = martyr.dwords[0];
    return res;
}


static inline u64 scanQword(ReadHead * head){
    u64 res;
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
            u32 size;
            u32 width;
            u32 height;
            u16 colorPlanes;
            u16 bitsPerPixel;
            u32 compression;
            u32 datasize;
            i32 pixelPerMeterHorizontal;
            i32 pixelPerMeterVertical;
            u32 colorsInPallette;
            u32 importantColorsAmount;
        };
        char data[40];
    };
};

bool decodeBMP(const FileContents * source, Image * target){
    ASSERT(source->contents[0] == 'B');
    ASSERT(source->contents[1] == 'M');
    if(source->contents[0] != 'B' || source->contents[1] != 'M'){
        return false;
    }
    //u32 filesize = *((u32 *)(source->contents + 2));
    i32 dataOffset = *((i32 *)(source->contents + 10));
    
    Bitmapinfoheader * infoheader = (Bitmapinfoheader *)(source->contents + 14);
    target->info.width = infoheader->width;
    target->info.height = infoheader->height;
    ASSERT(infoheader->compression == 0 || infoheader->compression == 3);
    ASSERT(infoheader->colorPlanes == 1);
    ASSERT(infoheader->bitsPerPixel % 8 == 0);
    //check and correct padding when removing following assert
    if((infoheader->compression != 0 && infoheader->compression != 3) || infoheader->colorPlanes != 1 || infoheader->bitsPerPixel % 8 != 0){
        return false;
    }
    ASSERT(infoheader->bitsPerPixel <= 255);
    target->info.bitsPerSample = CAST(u8, infoheader->bitsPerPixel);
    target->info.samplesPerPixel = 1;
    u64 bits = (target->info.samplesPerPixel * target->info.bitsPerSample * target->info.width * target->info.height);
    target->info.totalSize = bits/8 + (bits % 8 ? 1 : 0);
    target->info.origin = BitmapOriginType_BottomLeft;
    //RGB
    if(infoheader->compression == 0){
        if(infoheader->bitsPerPixel == 8){
            target->info.interpretation = BitmapInterpretationType_GrayscaleBW01;
        }else if(infoheader->bitsPerPixel == 24){
            target->info.interpretation = BitmapInterpretationType_RGB;
        }else{
            //@Incomplete
            INV;//IMPLEMENT OTHER BITS PER PIXEl
            return false;
        }
        
        target->data = &PUSHA(byte, target->info.totalSize);
        u32 rowpitch = (target->info.width % 4 != 0 ? (target->info.width/4 + 1)*4 : target->info.width);
        
        u8 bytesPerPixel = target->info.bitsPerSample/8;
        for(u32 h = 0; h < target->info.height; h++){
            //data are padded with 0 for 32 bit row padding
            u32 sourcepitch = h * rowpitch * bytesPerPixel;
            u32 targetpitch = h * target->info.width * bytesPerPixel;
            for(u32 w = 0; w < target->info.width; w++){
                for(u8 byteIndex = 0; byteIndex < bytesPerPixel; byteIndex++){
                    //the r g b is flipped to b g r due to whatever reason
                    target->data[targetpitch + w*bytesPerPixel + byteIndex] = (source->contents + dataOffset)[sourcepitch + w*bytesPerPixel + (bytesPerPixel-1-byteIndex)];
                }
                
            }
        }
        
    }else if(infoheader->compression == 3){
        if(infoheader->bitsPerPixel == 32){
            ASSERT(infoheader->compression == 3);
            u32 redMask = *(CAST(u32*, infoheader+1)+0);
            u32 redMaskFallShift = redMask > 0xFF ? (redMask > 0xFF00 ? (redMask > 0xFF0000 ? 24 : 16) : 8) : 0;
            
            u32 greenMask = *(CAST(u32*,infoheader+1)+1);
            u32 greenMaskFallShift = greenMask > 0xFF ? (greenMask > 0xFF00 ? (greenMask > 0xFF0000 ? 24 : 16) : 8) : 0;
            
            u32 blueMask = *(CAST(u32*,infoheader+1)+2);
            u32 blueMaskFallShift = blueMask > 0xFF ? (blueMask > 0xFF00 ? (blueMask > 0xFF0000 ? 24 : 16) : 8) : 0;
            
            u32 alfaMask = *(CAST(u32*,infoheader+1)+3);
            u32 alfaMaskFallShift = alfaMask > 0xFF ? (alfaMask > 0xFF00 ? (alfaMask > 0xFF0000 ? 24 : 16) : 8) : 0;
            
            target->info.interpretation = BitmapInterpretationType_RGBA;
            
            target->data = &PUSHA(byte, infoheader->datasize);
            u32 * pixelData = CAST(u32 *, target->data);
            
            //NOTE(AK): 32 bit pixels will always be 32 bit aligned
            for(u32 h = 0; h < target->info.height; h++){
                u32 pitch = h * target->info.width;
                for(u32 w = 0; w < target->info.width; w++){
                    u32 sourceData = CAST(u32*,source->contents + dataOffset)[pitch + w];
                    pixelData[pitch + w] = 0 | (((redMask & sourceData) >> redMaskFallShift)) | (((greenMask & sourceData) >> greenMaskFallShift) << 8)  | (((blueMask & sourceData) >> blueMaskFallShift) << 16) | (((alfaMask & sourceData) >> alfaMaskFallShift) << 24);
                }
            }
            
        }else{
            //@Incomplete
            INV;
            return false;
        }
    }else{
        //@Incomplete
        INV;
        return false;
    }
    
    return true;
}

bool encodeBMP(const Image * source, FileContents * target){
    bool palette = false;
    u16 bitsPerPixel = source->info.bitsPerSample * source->info.samplesPerPixel;
    u32 savewidth = source->info.width;
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
    *((u32 *)(target->contents + 2)) = target->size;
    *((u32 *)(target->contents + 6)) = 0;
    u32 dataOffset = *((u32 *)(target->contents + 10)) = 14 + sizeof(Bitmapinfoheader) + (palette ? (4 * (1 << bitsPerPixel)) : 0);
    
    Bitmapinfoheader * infoheader = (Bitmapinfoheader *)(target->contents + 14);
    infoheader->size = sizeof(Bitmapinfoheader);
    infoheader->width = source->info.width;
    infoheader->height = source->info.height;
    infoheader->colorPlanes = 1;
    infoheader->bitsPerPixel = bitsPerPixel;
    infoheader->compression = 0; //RGB
    infoheader->datasize = savewidth * source->info.height * bitsPerPixel/8;
    infoheader->pixelPerMeterVertical = infoheader->pixelPerMeterHorizontal = 0;
    infoheader->colorsInPallette = 1 << infoheader->bitsPerPixel;
    infoheader->importantColorsAmount = 1 << infoheader->bitsPerPixel;
    if(palette){
        ASSERT((1 << bitsPerPixel) == 256);
        ASSERT(source->info.interpretation == BitmapInterpretationType_GrayscaleBW01);
        for(u16 i = 0; i < (1 << bitsPerPixel); i++){
            //R
            *(target->contents + 14 + sizeof(Bitmapinfoheader) + i*4) = (u8)i;
            //G
            *(target->contents + 14 + sizeof(Bitmapinfoheader) + i*4 + 1) = (u8)i;
            //B
            *(target->contents + 14 + sizeof(Bitmapinfoheader) + i*4 + 2) = (u8)i;
            //unused
            *(target->contents + 14 + sizeof(Bitmapinfoheader) + i*4 + 3) = (u8)0;
        }
    }
    //@Incomplete
    //Implement other possibilities
    ASSERT(bitsPerPixel % 8 == 0);
    ASSERT(source->info.interpretation == BitmapInterpretationType_GrayscaleBW01 || source->info.interpretation == BitmapInterpretationType_RGB);
    //the r g b  flipped to b g r
    u16 bytesPerPixel = bitsPerPixel/8;
    if(source->info.origin == BitmapOriginType_BottomLeft){
        for(u32 h = 0; h < infoheader->height; h++){
            for(u32 w = 0; w < infoheader->width; w++){
                for(u8 byteIndex = 0; byteIndex < bytesPerPixel; byteIndex++){
                    (target->contents + dataOffset)[h * savewidth * bytesPerPixel + w * bytesPerPixel + (bytesPerPixel-1-byteIndex)] = source->data[h * infoheader->width * bytesPerPixel + w * bytesPerPixel + byteIndex]; 
                }
            }
            for(u32 w = infoheader->width; w < savewidth; w++){
                for(u8 byteIndex = 0; byteIndex < bytesPerPixel; byteIndex++){
                    (target->contents + dataOffset)[h * savewidth * bytesPerPixel + w * bytesPerPixel + byteIndex] = 0;
                }
            }
        }
    }else if(source->info.origin == BitmapOriginType_TopLeft){
        for(u32 h = 0; h < infoheader->height; h++){
            for(u32 w = 0; w < infoheader->width; w++){
                for(u8 byteIndex = 0; byteIndex < bytesPerPixel; byteIndex++){
                    (target->contents + dataOffset)[h * savewidth * bytesPerPixel + w * bytesPerPixel + (bytesPerPixel-1-byteIndex)] =  source->data[(infoheader->height - 1 - h) * infoheader->width * bytesPerPixel + w * bytesPerPixel + byteIndex];
                }
            }
            for(u32 w = infoheader->width; w < savewidth; w++){
                for(u8 byteIndex = 0; byteIndex < bytesPerPixel; byteIndex++){
                    (target->contents + dataOffset)[h * savewidth * bytesPerPixel + w * bytesPerPixel + byteIndex] = 0;
                }
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
    
    
    u16 tiffMagicFlag = scanWord(&head);
    //tiff const flag
    if(tiffMagicFlag != 42){
        INV;
        return false;
    }
    
    u32 imageOffset = scanDword(&head);
    head.offset = file->contents + imageOffset;
    
    u16 entries = scanWord(&head);
    
    u32 * stripOffsets = NULL;
    u32 rowsPerStrip = CAST(u32, -1); //infinity (1 strip for all data)
    u32 * stripSizes = NULL;
    u32 stripAmount = 0;
    
    //default values
    target->info.origin = BitmapOriginType_TopLeft;
    target->info.samplesPerPixel = 1;
    
    for(u16 ei = 0; ei < entries; ei++){
        u16 tag = scanWord(&head);
        u16 type = scanWord(&head);
        u32 length = scanDword(&head);
        u32 headerOffset = scanDword(&head);
        
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
                ASSERT(headerOffset <= 255);
                target->info.bitsPerSample = CAST(u8, headerOffset);
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
                stripOffsets = &PUSHA(u32, length);
                ASSERT(stripAmount == 0 || length == stripAmount);
                if(stripAmount != 0 && length != stripAmount){
                    POPI;
                    return false;
                }
                stripAmount = length;
                if(stripAmount == 1){
                    stripOffsets[0] = headerOffset;
                }else{
                    for(u32 stripIndex = 0; stripIndex < length; stripIndex++){
                        stripOffsets[stripIndex] = ((u32 *)(file->contents + headerOffset))[stripIndex];
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
                ASSERT(headerOffset <= 255);
                target->info.samplesPerPixel = CAST(u8, headerOffset);
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
                stripSizes = &PUSHA(u32, length);
                ASSERT(stripAmount == 0 || length == stripAmount);
                if(stripAmount != 0 && length != stripAmount){
                    POPI;
                    return false;
                }
                stripAmount = length;
                if(stripAmount == 1){
                    stripSizes[0] = headerOffset;
                }else{
                    for(u32 stripIndex = 0; stripIndex < length; stripIndex++){
                        stripSizes[stripIndex] = ((u32 *)(file->contents + headerOffset))[stripIndex];
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
                //cm/inch we dont care for now
            }break;
            case 306:{
                //date time we dont care for now
            }break;
            case 315:{
                //artist we dont care for now
            }break;
            default:{
                INV;
                POPI;
                return false;
            }break;
            
        }
    }
    
    u32 nextFile = scanDword(&head);
    if(nextFile != 0){
        INV;
        return false;
    }
    
    target->data = &PPUSHA(byte, target->info.width * target->info.height * target->info.samplesPerPixel * (target->info.bitsPerSample / 8));
    
    for(u32 i = 0; i < target->info.width * target->info.height * target->info.samplesPerPixel * (target->info.bitsPerSample / 8); i++){
        target->data[i] = 255;
    }
    
    u32 total = 0;
    u32 last = 0;
    
    for(u32 stripIndex = 0; stripIndex < stripAmount; stripIndex++){
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

static inline void writeByte(WriteHead * head, u8 data){
    head->offset[0] = data;
    head->offset++;
}

static inline void writeWord(WriteHead * head, u16 data){
    martyr.words[0] = data;
    head->offset[0] = martyr.bytes[0];
    head->offset[1] = martyr.bytes[1];
    head->offset += 2;
}


static inline void writeDword(WriteHead * head, u32 data){
    martyr.dwords[0] = data;
    head->offset[0] = martyr.bytes[0];
    head->offset[1] = martyr.bytes[1];
    head->offset[2] = martyr.bytes[2];
    head->offset[3] = martyr.bytes[3];
    head->offset += 4;
}


static inline void writeQword(WriteHead * head, u64 data){
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
    u16 entries  = 10;
    
    u32 bytesize = source->info.width * source->info.height * (source->info.bitsPerSample/8) * source->info.samplesPerPixel;
    u32 rowBytesize = bytesize / source->info.height;
    
    u32 stripRecommendedSize = KILOBYTE(8);
    u32 rowsPerStrip = stripRecommendedSize / rowBytesize;
    
    if(rowsPerStrip == 0) rowsPerStrip++;
    if(rowsPerStrip > source->info.height) rowsPerStrip = source->info.height;
    
    u32 stripAmount = source->info.height / rowsPerStrip;
    if(source->info.height % rowsPerStrip != 0){
        stripAmount++;
    }
    u32 stripSize = rowBytesize * rowsPerStrip;
    ASSERT(stripAmount >= 1);
    ASSERT(rowsPerStrip >= 1);
    if(stripAmount < 1 || 
       rowsPerStrip < 1){
        return false;
    }
    
    //compress - wild quess now, later compress first, then work with exact numbers
    //assuming 4096 2 byte entries
    u32 compressedSize = (4096*2) * stripAmount;
    target->size =  (8 + (12 * entries) + 4) + (source->info.samplesPerPixel + 2*stripAmount) * 4 + compressedSize;
    target->contents = &PPUSHA(char, target->size);
    
    
    
    //http://www.fileformat.info/format/tiff/corion.htm
    WriteHead head;
    head.offset = target->contents;
    
    //endianity
    writeByte(&head, (u8) 'I');
    writeByte(&head, (u8) 'I');
    //magic constant
    writeWord(&head, 42);
    
    //image data offset
    writeDword(&head, 8); //this dword + previous word + prev 2 bytes
    
    
    writeWord(&head, entries);
    
    
    
    
    
    //every head strip has 12 bytes
    WriteHead dataHead;
    dataHead.offset = head.offset +  (12 * entries) + 4;
    
    //image division
    u32 * stripOffsets = NULL;
    u32 * stripSizes = NULL;
    
    
    
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
        ASSERT(!"check me");
        ASSERT(dataHead.offset >= target->contents);
        writeDword(&head, CAST(u32, dataHead.offset - target->contents));
        for(u16 i = 0; i < source->info.samplesPerPixel; i++){
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
    if(source->info.interpretation != BitmapInterpretationType_GrayscaleBW01){
        return false;
    }
    
    
    
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
    writeDword(&head, CAST(u32, strlen(desc) + 1));
    ASSERT(dataHead.offset >= target->contents);
    writeDword(&head, CAST(u32, dataHead.offset - target->contents));
    strcpy(dataHead.offset, desc);
    dataHead.offset += strlen(desc) + 1;
    
    
    //strip offsets ?);
    //these will be filled later when compressing
    writeWord(&head, 273);
    writeWord(&head, 4);
    writeDword(&head, stripAmount);
    if(stripAmount == 1){
        stripOffsets = (u32 *) head.offset;
        head.offset += 4;
    }else{
        ASSERT(dataHead.offset >= target->contents);
        writeDword(&head, CAST(u32, dataHead.offset - target->contents));
        stripOffsets = (u32 *) dataHead.offset;
        dataHead.offset += stripAmount * 4; //allocate space for future
    }
    
    //data orientation
    /* default suits us 
    writeWord(&head, 274);
    writeWord(&head, 3);
    writeDword(&head, 1);
    writeDword(&head, 1); //supporting TopLeft (rows->rows, cols->cols)
    */
    ASSERT(source->info.origin == BitmapOriginType_TopLeft);
    if(source->info.origin != BitmapOriginType_TopLeft){
        return false;
    }
    
    //data orientation
    writeWord(&head, 277);
    writeWord(&head, 3);
    writeDword(&head, 1);
    writeDword(&head, 1); ////support BW for now only
    
    ASSERT(source->info.samplesPerPixel == 1);
    if(source->info.samplesPerPixel != 1){
        return false;
    }
    
    
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
    if(stripAmount == 1){
        stripSizes = (u32 *) head.offset;
        head.offset += 4;
    }else{
        ASSERT(dataHead.offset >= target->contents);
        writeDword(&head, CAST(u32, dataHead.offset - target->contents));
        stripSizes = (u32 *) dataHead.offset;
        dataHead.offset += stripAmount * 4; //allocate space for future
    }
    
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
    //cm/inch we dont give care for now
    }break;
    */
    
    
    //1 image in this tiff data
    writeDword(&head, 0); //no next file
    
    
    for(u32 stripIndex = 0; stripIndex < stripAmount; stripIndex++){
        u32 sourceSize = stripSize;
        if(stripIndex == stripAmount - 1){
            sourceSize = bytesize - (stripIndex * sourceSize);
        }
        u32 size = compressLZW((byte *) (source->data + stripIndex * rowsPerStrip * source->info.width), sourceSize, (byte *) dataHead.offset); 
        stripSizes[stripIndex] = size;
        ASSERT(dataHead.offset >= target->contents);
        stripOffsets[stripIndex] = CAST(u32, dataHead.offset - target->contents);
        dataHead.offset += size;
    }
    
    ASSERT(target->size >= dataHead.offset - target->contents);
    ASSERT(dataHead.offset >= target->contents);
    target->size = CAST(u32, dataHead.offset - target->contents);
    
    
    POPI;
    return true;
}

bool decodeWAV(const FileContents * source, AudioTrack * target){
    if (source->size > 44){
        ReadHead head;
        head.offset = source->contents + source->head;
        ASSERT(strncmp(head.offset, "RIFF", 4) == 0);
        head.offset += 4;
        u32 fileSize = scanDword(&head);
        ASSERT(fileSize <= source->size);
        ASSERT(strncmp(head.offset, "WAVE", 4) == 0);
        head.offset += 4;
        ASSERT(strncmp(head.offset, "fmt ", 4) == 0);
        head.offset += 4;
        u32 lengthOfFormatData = scanDword(&head);
        ASSERT(lengthOfFormatData == 16);
        u16 format = scanWord(&head);
        ASSERT(format == 1); // PCM
        u16 bytesPerSample = scanWord(&head);
        ASSERT(bytesPerSample == 2);
        u32 samplesPerSecond = scanDword(&head);
        ASSERT(samplesPerSecond == 44100);
        u32 bytesPerSecond = scanDword(&head);
        ASSERT(bytesPerSecond == 176400);
        scanWord(&head); // no idea
        u16 bitsPerSample = scanWord(&head);
        ASSERT(bitsPerSample == 16);
        ASSERT(strncmp(head.offset, "data", 4) == 0);
        head.offset += 4;
        u32 dataSize = scanDword(&head);
        ASSERT(44 + dataSize <= source->size);
        ASSERT(head.offset - source->contents - source->head == 44);
        target->byteSize = dataSize;
        target->data = head.offset;
        return true;
    }
    return false;
}


#endif
