#ifndef UTIL_CONV
#define UTIL_CONV


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




bool convTiffToBitmap(const FileContents * file, Image * target){
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
    
    for(uint16 ei = 0; ei < entries; ei++){
        uint16 tag = scanWord(&head);
        uint16 type = scanWord(&head);
        uint32 length = scanDword(&head);
        uint32 headerOffset = scanDword(&head);
        
        switch(tag){
            case 259:{
                //compression
            }break;
            case 257:{
                //image height
            }break;
            case 283:{
                //Yresolution = pixels per height param
            }break;
            case 256:{
                //image width
            }break;
            case 282:{
                //Xresolution = pixels per width param
            }break;
            case 254:{
                //new subfile type
            }break;
            case 258:{
                //bits per sample
            }break;
            case 262:{
                //photometric interpretation
            }break;
            case 273:{
                //strip offsets (essentialy compressed data lines?);
            }break;
            case 278:{
                //rows per strip
            }break;
            case 279:{
                //strip byte counts (essentially bytes per strip);
            }break;
            case 284:{
                //planar configuration (lzw might use)
            }break;
            case 277:{
                //samples per pixel
            }break;
            case 274:{
                //orientation
            }break;
            case 306:{
                //date time
            }break;
            case 315:{
                //artist
            }break;
            case 296:{
                //resolution unit
            }break;
            case 266:{
                //undef fuckery
            }break;
            default:{
                INV;
            }break;
            
        }
    }
    
    uint32 nextFile = scanDword(&head);
    if(nextFile != 0){
        INV;
        return false;
    }
    
    
    
    
    return true;
}


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

#endif