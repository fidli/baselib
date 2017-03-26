#ifndef UTIL_COMPRESS
#define UTIL_COMPRESS

bool decompressLZW(const byte * source, byte * target){
    //http://www.fileformat.info/format/tiff/corion-lzw.htm
    PUSHI;
    
    uint32 destIndex = 0;
    char * table = PUSHA(byte, MEGABYTE(10));
    uint16 bitsize = 8;
    
    
    
    
    POPI;
    return true;
}


#endif