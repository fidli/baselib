#ifndef UTIL_IMAGE_H
#define UTIL_IMAGE_H

#include "util_math.cpp"
#include "util_compress.cpp"

enum BitmapInterpretationType{
    BitmapInterpretationType_Invalid,
    BitmapInterpretationType_GrayscaleBW01,
    BitmapInterpretationType_ARGB,
    BitmapInterpretationType_RGBA,
    BitmapInterpretationType_RGB,
    BitmapInterpretationType_ABGR,
    //https://linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/pixfmt-yuyv.html
    BitmapInterpretationType_YUY2
};

enum BitmapOriginType{
    BitmapOriginType_Invalid,
    BitmapOriginType_TopLeft,
    BitmapOriginType_BottomLeft
};

struct Image{
    struct{
        u32 width;
        u32 height;
        u8 bitsPerSample;
        u8 samplesPerPixel;
        u64 totalSize;
        BitmapInterpretationType interpretation;
        BitmapOriginType origin;
    } info;
    u8 * data;
    
};

union Color{
    //NOTE(AK): different representations mean different members,
    //keep just x,y,z,w or use functions eg red(Color * c);
    //but there would be uselles switch
    //@Cleanup
    union{
        struct{
            u8 r;
            u8 g;
            u8 b;
            u8 a;
        };
        struct{
            u8 x;
            u8 y;
            u8 z;
            u8 w;
        };
    };
    u8 channel[4];
    u32 full;
    u8 intensity;
};

struct NearestNeighbourColor{
    Color color;
    u8 times;
};


bool cropImageX(Image * image, u32 leftCrop, u32 rightCrop){
    Image temp;
    temp.info = image->info;
    temp.info.width = rightCrop-leftCrop;
    ASSERT(image->info.origin == BitmapOriginType_TopLeft);
    if(image->info.origin != BitmapOriginType_TopLeft){
        return false;
    }
    if(leftCrop > rightCrop){
        u32 tmp = leftCrop;
        leftCrop = rightCrop;
        rightCrop = tmp;
    }
    temp.data = &PUSHA(byte, temp.info.width * temp.info.height * temp.info.samplesPerPixel * temp.info.bitsPerSample/8);
    
    for(u32 h = 0; h < temp.info.height; h++){
        u32 i = h*temp.info.width;
        for(u32 w = leftCrop; w < rightCrop; w++, i++){
            temp.data[i] = image->data[h * image->info.width + w];
        }
    }
    
    image->info.width = temp.info.width;
    for(u32 i = 0; i < image->info.width * image->info.height * image->info.samplesPerPixel * (image->info.bitsPerSample/8); i++){
        image->data[i] = temp.data[i];
    }
    
    
    
    POP;
    return true;
}

bool flipY(Image * target){
    ASSERT(target->info.bitsPerSample % 8 == 0);
    ASSERT(target->info.samplesPerPixel == 1);
    u32 bytesize = target->info.width * target->info.height * (target->info.bitsPerSample/8) * target->info.samplesPerPixel;
    u8 * tmp = &PUSHA(u8, bytesize);
    if(target->info.bitsPerSample % 8 != 0 || target->info.samplesPerPixel != 1){
        return false;
    }
    u8 bytesPerPixel = (target->info.bitsPerSample * target->info.samplesPerPixel)/8;
    for(u32 h = 0; h < target->info.height; h++){
        for(u32 w = 0; w < target->info.width; w++){
            for(u8 byteIndex = 0; byteIndex < bytesPerPixel; byteIndex++){
                tmp[w*bytesPerPixel + byteIndex + (target->info.height - h - 1) * target->info.width * bytesPerPixel] = target->data[w*bytesPerPixel + byteIndex + target->info.width*h*bytesPerPixel];
            }
        }
    }
    
    memcpy(target->data, tmp, bytesize);
    
    switch(target->info.origin){
        case BitmapOriginType_TopLeft:{
            target->info.origin = BitmapOriginType_BottomLeft;
        }break;
        case BitmapOriginType_BottomLeft:{
            target->info.origin = BitmapOriginType_TopLeft;
        }break;
        case BitmapOriginType_Invalid:
        default:{
            INV;
        }break;
    }
    
    POP;
    return true;
}


bool cropImageY(Image * image, u32 bottomCrop, u32 topCrop){
    Image temp;
    temp.info = image->info;
    ASSERT(image->info.origin == BitmapOriginType_TopLeft);
    if(image->info.origin != BitmapOriginType_TopLeft){
        return false;
    }
    if(bottomCrop < topCrop){
        bottomCrop = topCrop;
        topCrop = topCrop;
    }
    temp.info.height = bottomCrop-topCrop;
    temp.data = &PUSHA(byte, temp.info.width * temp.info.height * temp.info.samplesPerPixel * temp.info.bitsPerSample/8);
    
    u32 temp_i = 0;
    for(u32 h = topCrop; h < bottomCrop; h++){
        for(u32 w = 0; w < temp.info.width; w++, temp_i++){
            temp.data[temp_i] = image->data[h * image->info.width + w];
        }
    }
    
    image->info.height = temp.info.height;
    for(u32 i = 0; i < image->info.width * image->info.height * image->info.samplesPerPixel * (image->info.bitsPerSample/8); i++){
        image->data[i] = temp.data[i];
    }
    
    
    
    POP;
    return true;
}


bool rotateImage(Image * image, f32 angleDeg, f32 centerX = 0.5f, f32 centerY = 0.5f){
    ASSERT(image->info.bitsPerSample == 8 && image->info.samplesPerPixel == 1);
    if(image->info.bitsPerSample != 8 || image->info.samplesPerPixel != 1){
        return false;
    }
    Image temp;
    temp.info = image->info;
    temp.data = &PUSHA(byte, temp.info.width * temp.info.height * temp.info.samplesPerPixel * temp.info.bitsPerSample/8);
    f32 angleRad = -angleDeg;
    while(angleRad >= 360) angleRad -= 360;
    while(angleRad < 0) angleRad += 360;
    angleRad = (angleRad / 180) * PI;
    f32 sinA = sin(angleRad);
    f32 cosA = cos(angleRad);
    i32 cX = (u32)(centerX * image->info.width);
    i32 cY = (u32)(centerY * image->info.height);
    for(u32 h = 0; h < temp.info.height; h++){
        i32 rY = h - cY;
        for(u32 w = 0; w < temp.info.width; w++){
            i32 rX = w  - cX;
            i32 nX = (i32)(cosA * rX) - (i32)(sinA*rY) + cX;
            i32 nY = (i32)(sinA * rX) + (i32)(cosA*rY) + cY;
            if(nX >= 0 && nX < CAST(i32, image->info.width) && nY >= 0 && nY < CAST(i32, image->info.height)){
                temp.data[temp.info.width * h + w] = image->data[temp.info.width * nY  + nX];
            }else{
                temp.data[temp.info.width * h + w] = 0;
            }
        }
    }
    
    for(u32 i = 0; i < image->info.width * image->info.height * image->info.samplesPerPixel * (image->info.bitsPerSample/8); i++){
        image->data[i] = temp.data[i];
    }
    
    POP;
    return true;
}

static inline u8 resultingContrast(const u8 originalColor, const f32 contrast){
    f32 factor = (259 * (contrast + 255)) / (255 * (259 - contrast));
    f32 result = factor * (originalColor - 128) + 128;
    if(result < 0) result = 0;
    if(result > 255) result = 255;
    return (u8) result;
}


void applyContrast(Image * target, const f32 contrast){
    ASSERT(target->info.interpretation == BitmapInterpretationType_GrayscaleBW01);
    for(u32 x = 0; x < target->info.width; x++){
        for(u32 y = 0; y < target->info.height; y++){
            u8 originalColor = target->data[y * target->info.width + x];
            
            target->data[y * target->info.width + x] = resultingContrast(originalColor, contrast);
        }
    }
}


bool scaleImage(const Image * source, Image * target, u32 targetWidth, u32 targetHeight){
    f32 scaleX = (f32)source->info.width/(f32)targetWidth;
    f32 scaleY = (f32)source->info.height/(f32)targetHeight;
    
    u32 neighbourCount = (u32)(scaleX*scaleY);
    //does not hurt
    neighbourCount++;
    
    
    
    
    //support lesser bits per pixel later
    ASSERT(source->info.bitsPerSample * source->info.samplesPerPixel >= 8 && source->info.bitsPerSample * source->info.samplesPerPixel % 8 == 0);
    if(source->info.bitsPerSample * source->info.samplesPerPixel < 8 && source->info.bitsPerSample * source->info.samplesPerPixel % 8 != 0){
        return false;
    }
    
    NearestNeighbourColor * nn = &PUSHA(NearestNeighbourColor, neighbourCount);
    u8 channelCount = (source->info.bitsPerSample * source->info.samplesPerPixel) / 8;
    
    //each target pixel
    for(u32 tw = 0; tw < targetWidth; tw++){
        for(u32 th = 0; th < targetHeight; th++){
            u32 i = targetWidth*th  + tw;//final index
            
            //clear NN
            for(u32 clr = 0; clr < neighbourCount; clr++){
                nn[clr] = {};
            }
            u8 nncount = 0;
            
            //for all neighbours to the original pixel
            for(int nw = 0; nw < scaleX; nw++){
                u32 srcW = (int)(tw*scaleX) + nw;
                for(int nh = 0; nh < scaleY; nh++){
                    u32 srcH = (int)(th*scaleY) + nh;
                    
                    Color srcColor = {};
                    for(u8 ci = 0; ci < channelCount; ci++){
                        srcColor.channel[ci] = source->data[(srcH*source->info.width + srcW)*channelCount + ci];
                    }
                    
                    //look for same color;
                    bool found = false;
                    for(int s = 0; s < nncount; s++){
                        if(nn[s].color.full  == srcColor.full){
                            nn[s].times++;
                            found = true;
                            break;
                        }
                    }
                    if(!found){
                        nn[nncount++].color.full = srcColor.full;
                    }
                    //do not overstep image boundaries, last neighbours could be a little bit off
                    if(srcH > source->info.width){
                        break;
                    }
                }
                //do not overstep image boundaries, last neighbours could be a little bit off
                if(srcW > source->info.height){
                    break;
                }
            }
            
            //do some blending
            u32 sum[4] = {};
            //int8  highest = -1;
            for(int s = 0; s < nncount; s++){
                for(u8 ci = 0; ci < channelCount; ci++){
                    sum[ci] += nn[s].color.channel[ci];
                }
                
                /*if(nn[s].times > highest){
                    resultColor = nn[s].color;
                }*/
            }
            for(u8 ci = 0; ci < channelCount; ci++){
                target->data[channelCount * i + ci] = CAST(char, sum[ci]/nncount);
            }
            
            
            
        }
    }
    target->info = source->info;
    target->info.width = targetWidth;
    target->info.height = targetHeight;
    POP;
    return true;
}

bool scaleCanvas(Image * target, u32 newWidth, u32 newHeight, u32 originalOffsetX = 0, u32 originalOffsetY = 0){
    ASSERT(target->info.bitsPerSample * target->info.samplesPerPixel == 8);
    ASSERT(target->info.origin == BitmapOriginType_TopLeft);
    if(target->info.bitsPerSample * target->info.samplesPerPixel != 8 || target->info.origin != BitmapOriginType_TopLeft){
        return false;
    }
    byte * tmp = &PUSHA(byte, newWidth * newHeight);
    for(u32 th = 0; th < newHeight; th++){
        for(u32 tw = 0; tw < newWidth; tw++){
            if(tw >= originalOffsetX &&
               th >= originalOffsetY &&
               th - originalOffsetY < target->info.height &&
               tw - originalOffsetX < target->info.width){
                tmp[tw + th*newWidth] = target->data[(tw - originalOffsetX) + (th - originalOffsetY)*target->info.width];
            }else{
                tmp[tw + th*newWidth] = 0;
            }
        }
    }
    target->data = tmp;
    target->info.width = newWidth;
    target->info.height = newHeight;
    return true;
}

bool decodePNG2(const FileContents * source, Image * target){
    PROFILE_FUNC(source->size);
    ReadHead head;
    head.offset = CAST(u8*, source->contents) + source->head;
    u8 firstByte = scanByte(&head);
    ASSERT(firstByte == 0x89);
    ASSERT(strncmp(CAST(char*, head.offset), "PNG", 3) == 0);
    head.offset += 3;
    u32 headDword = scanDword(&head, ByteOrder_BigEndian);
    ASSERT(headDword == 0x0D0A1A0A);
    
    bool running = true;
    i32 checks = 0;
    u8* compressed = &PUSHA(u8, source->size);
    u32 compressedOffset = 0;
    u8* filterTypes = NULL;
    i32 one = 0;
    i32 two = 0;
    i32 three = 0;
    i32 four = 0;
    {
        PROFILE_SCOPE("PNG - pre parsing & concat", source->size);
        while(head.offset < CAST(u8*, source->contents) + source->head + source->size && running){
            u32 chunkLength = scanDword(&head, ByteOrder_BigEndian);
            char * chunkType = CAST(char*, head.offset);
            head.offset += 4;
            if(strncmp(chunkType, "IDAT", 4) == 0){
                ASSERT(compressedOffset + chunkLength <= source->size);
                memcpy(compressed + compressedOffset, head.offset, chunkLength);
                head.offset += chunkLength;
                compressedOffset += chunkLength;
            }
            else if (strncmp(chunkType, "IHDR", 4) == 0){
                checks++;
                ASSERT(chunkLength == 13);
                target->info.width = scanDword(&head, ByteOrder_BigEndian);
                target->info.height = scanDword(&head, ByteOrder_BigEndian);
                u8 bitsPerChannel = scanByte(&head);
                u8 colorType = scanByte(&head);
                u8 compressionMethod = scanByte(&head);
                ASSERT(compressionMethod == 0);
                if (colorType == 6){
                    target->info.interpretation = BitmapInterpretationType_RGBA;
                    target->info.bitsPerSample = bitsPerChannel*4;
                }
                u8 filterMethod = scanByte(&head);
                ASSERT(filterMethod == 0);
                u8 interlaceMethod = scanByte(&head);
                ASSERT(interlaceMethod == 0);

                target->info.samplesPerPixel = 1;
                target->info.origin = BitmapOriginType_TopLeft;
                u64 bits = (target->info.samplesPerPixel * target->info.bitsPerSample * target->info.width * target->info.height);
                target->info.totalSize = bits/8 + (bits % 8 ? 1 : 0);
                target->data = &PPUSHA(byte, target->info.totalSize);
                filterTypes = &PUSHA(u8, target->info.height);
            }
            else if(strncmp(chunkType, "IEND", 4) != 0){
                head.offset += chunkLength;
            }
            else {
                ASSERT(strncmp(chunkType, "IEND", 4) == 0);
                running = false;
                checks++;
            }
            scanDword(&head, ByteOrder_BigEndian);
            // TODO crc
        }
    }
    u32 bytesPerPixel = 4;
    ReadHead compressedHead = {compressed};
    u32 decodedBytes = 0;
    {
        PROFILE_SCOPE("PNG - decompress", source->size);
        // Zlib encap then deflate
        // need to concat first
        u8 methodAndTag = scanByte(&compressedHead);
        ASSERT((methodAndTag & 0x0F) == 0x08); // deflate
        //u32 windowSize = (1 << (((methodAndTag & 0xF0)>>4) + 8));
        u8 flags = scanByte(&compressedHead);
        ASSERT((flags & 0x20) == 0); // FDICT
        ASSERT(((CAST(u16, methodAndTag) << 8) | flags) % 31 == 0);
        decodedBytes = decompressDeflatePNG(compressedHead.offset, target->data, filterTypes, target->info.width*bytesPerPixel);
        ASSERT(decodedBytes == target->info.samplesPerPixel * (target->info.bitsPerSample/8) * target->info.width * target->info.height);
        PROFILE_BYTES(decodedBytes);
    }
    PROFILE_BYTES(decodedBytes);
    {
        PROFILE_SCOPE("PNG - filter", decodedBytes);
        ASSERT(target->info.interpretation == BitmapInterpretationType_RGBA);
        u32 stride = bytesPerPixel * target->info.width;
        for(u32 h = 0; h < target->info.height; h++){
            u32 pitch = h * stride;
            u8 filterType = filterTypes[h];
            ASSERT(filterType <= 4);
            switch(filterType)
            {
            
                case 1:{
                    u8* left = target->data + pitch;
                    for(u32 w = bytesPerPixel; w < target->info.width*bytesPerPixel; w++){
                        *(target->data + pitch + w) += *left;
                        left++;
                    }
                }break;
                case 2:{
                    ASSERT(h > 0);
                    u8* up = target->data + ((h-1) * bytesPerPixel * target->info.width);
                    for(u32 w = 0; w < target->info.width*bytesPerPixel; w++){
                        *(target->data + pitch + w) += *up;
                        up++;
                    }
               }break;
                case 3:{
                    ASSERT(h > 0);
                    u8* up = target->data + ((h-1) * bytesPerPixel * target->info.width);
                    u8* left = target->data + pitch;
                    for(u32 w = 0; w < bytesPerPixel; w++){
                        *(target->data + pitch + w) -= *up / 2;
                        up++;
                    }
                    for(u32 w = bytesPerPixel; w < target->info.width*bytesPerPixel; w++){
                        *(target->data + pitch + w) += CAST(u8, ((CAST(u16, *left) + CAST(u16, *up)) / 2));
                        up++;
                        left++;
                    }
                }break;
                case 4:{
                    ASSERT(h > 0);
                    u8* up = target->data + ((h-1) * bytesPerPixel * target->info.width);
                    u8* left = target->data + pitch;
                    u8* leftUp = up;
                    for(u32 w = 0; w < bytesPerPixel; w++){
                        *(target->data + pitch + w) += *up;
                        up++;
                    }
                    for(u32 w = bytesPerPixel; w < target->info.width*bytesPerPixel; w++){
                        i32 p = CAST(i32, *left) + CAST(i32, *up) - CAST(i32, *leftUp);
                        i32 distLeft = ABS(CAST(i32, *left)-p);
                        i32 distUp = ABS(CAST(i32, *up)-p);
                        i32 distLeftUp = ABS(CAST(i32, *leftUp)-p);
                        u8 least = 0;
                        if (distLeft <= distUp && distLeft <= distLeftUp){
                            least = *left; 
                        }
                        else if (distUp <= distLeftUp){
                            least = *up;
                        }
                        else{
                            least = *leftUp;
                        }
                        *(target->data + pitch + w) += least;
                        left++;
                        leftUp++;
                        up++;
                    }
                
            }break;
            }
        }
    }
    compressedHead.offset = compressed + compressedOffset - 4;
    // adler
    scanDword(&compressedHead, ByteOrder_BigEndian);
    POP;
    return checks == 2 && (decodedBytes == target->info.samplesPerPixel * (target->info.bitsPerSample/8) * target->info.width * target->info.height);
}

bool decodePNG(const FileContents * source, Image * target){
    PROFILE_FUNC(source->size);
    ReadHead head;
    head.offset = CAST(u8*, source->contents) + source->head;
    u8 firstByte = scanByte(&head);
    ASSERT(firstByte == 0x89);
    ASSERT(strncmp(CAST(char*, head.offset), "PNG", 3) == 0);
    head.offset += 3;
    u32 headDword = scanDword(&head, ByteOrder_BigEndian);
    ASSERT(headDword == 0x0D0A1A0A);
    
    bool running = true;
    i32 checks = 0;
    u8* compressed = &PUSHA(u8, source->size);
    u32 compressedOffset = 0;
    u8* uncompressed = NULL;
    {
        PROFILE_SCOPE("PNG - pre parsing & concat", source->size);
        while(head.offset < CAST(u8*, source->contents) + source->head + source->size && running){
            u32 chunkLength = scanDword(&head, ByteOrder_BigEndian);
            char * chunkType = CAST(char*, head.offset);
            head.offset += 4;
            if (strncmp(chunkType, "IHDR", 4) == 0){
                checks++;
                ASSERT(chunkLength == 13);
                target->info.width = scanDword(&head, ByteOrder_BigEndian);
                target->info.height = scanDword(&head, ByteOrder_BigEndian);
                u8 bitsPerChannel = scanByte(&head);
                u8 colorType = scanByte(&head);
                u8 compressionMethod = scanByte(&head);
                ASSERT(compressionMethod == 0);
                if (colorType == 6){
                    target->info.interpretation = BitmapInterpretationType_RGBA;
                    target->info.bitsPerSample = bitsPerChannel*4;
                }
                u8 filterMethod = scanByte(&head);
                ASSERT(filterMethod == 0);
                u8 interlaceMethod = scanByte(&head);
                ASSERT(interlaceMethod == 0);

                target->info.samplesPerPixel = 1;
                target->info.origin = BitmapOriginType_TopLeft;
                u64 bits = (target->info.samplesPerPixel * target->info.bitsPerSample * target->info.width * target->info.height);
                target->info.totalSize = bits/8 + (bits % 8 ? 1 : 0);
                target->data = &PPUSHA(byte, target->info.totalSize);
                // first byte of each row is filter type
                uncompressed = &PUSHA(u8, target->info.samplesPerPixel * (target->info.bitsPerSample/8) * (target->info.width+1) * target->info.height);
            }
            else if(strncmp(chunkType, "IDAT", 4) == 0){
                ASSERT(compressedOffset + chunkLength <= source->size);
                memcpy(compressed + compressedOffset, head.offset, chunkLength);
                head.offset += chunkLength;
                compressedOffset += chunkLength;
            }
            else if(strncmp(chunkType, "IEND", 4) == 0){
                running = false;
                checks++;
            }
            else{
                head.offset += chunkLength;
            }
            scanDword(&head, ByteOrder_BigEndian);
            // TODO crc
        }
    }
    ReadHead compressedHead = {compressed};
    u32 decodedBytes = 0;
    {
        PROFILE_SCOPE("PNG - decompress", source->size);
        // Zlib encap then deflate
        // need to concat first
        u8 methodAndTag = scanByte(&compressedHead);
        ASSERT((methodAndTag & 0x0F) == 0x08); // deflate
        //u32 windowSize = (1 << (((methodAndTag & 0xF0)>>4) + 8));
        u8 flags = scanByte(&compressedHead);
        ASSERT((flags & 0x20) == 0); // FDICT
        ASSERT(((CAST(u16, methodAndTag) << 8) | flags) % 31 == 0);
        decodedBytes = decompressDeflate(compressedHead.offset, uncompressed);
        ASSERT(decodedBytes == target->info.samplesPerPixel * (target->info.bitsPerSample/8) * target->info.width * target->info.height + target->info.height);
        PROFILE_BYTES(decodedBytes);
    }
    PROFILE_BYTES(decodedBytes);
    {
        PROFILE_SCOPE("PNG - filter", decodedBytes*2);
        ASSERT(target->info.interpretation == BitmapInterpretationType_RGBA);
        u32 bytesPerPixel = 4;
        u32 stride = bytesPerPixel * target->info.width;
        for(u32 h = 0; h < target->info.height; h++){
            u32 pitch = h * stride;
            u8 filterType = *(uncompressed + pitch + h);
            if (filterType == 0)
            {
                memcpy(target->data + pitch, uncompressed + pitch + (h+1), target->info.width * bytesPerPixel);
            }
            else if (filterType == 1){
                for(u32 w = 0; w < target->info.width*bytesPerPixel; w++){
                    u8 left = w >= bytesPerPixel ? *(target->data + pitch + w - bytesPerPixel) : 0;
                    *(target->data + pitch + w) = *(uncompressed + pitch + (h+1) + w) + left;
                }
            }
            else if (filterType == 2){
                for(u32 w = 0; w < target->info.width*bytesPerPixel; w++){
                    u8 up = h > 0 ? *(target->data + ((h-1) * bytesPerPixel * target->info.width) + w) : 0;
                    *(target->data + pitch + w) = *(uncompressed + pitch + (h+1) + w) + up;
                }
            } 
            else if (filterType == 3){
                for(u32 w = 0; w < target->info.width*bytesPerPixel; w++){
                    u8 left = w >= bytesPerPixel ? *(target->data + pitch + w - bytesPerPixel) : 0;
                    u8 up = h > 0 ? *(target->data + ((h-1) * bytesPerPixel * target->info.width) + w) : 0;
                    *(target->data + pitch + w) = *(uncompressed + pitch + (h+1) + w) + CAST(u8, ((CAST(u16, left) + CAST(u16, up)) / 2));
                }
            } 
            else if (filterType == 4){
                for(u32 w = 0; w < target->info.width*bytesPerPixel; w++){
                    u8 left = w >= bytesPerPixel ? *(target->data + pitch + w - bytesPerPixel) : 0;
                    u8 up = h > 0 ? *(target->data + ((h-1) * bytesPerPixel * target->info.width) + w) : 0;
                    u8 leftUp = (h > 0 && w >= bytesPerPixel) ? *(target->data + ((h-1) * bytesPerPixel * target->info.width) + w  - bytesPerPixel) : 0;
                    i32 p = CAST(i32, left) + CAST(i32, up) - CAST(i32, leftUp);
                    i32 distLeft = ABS(CAST(i32, left)-p);
                    i32 distUp = ABS(CAST(i32, up)-p);
                    i32 distLeftUp = ABS(CAST(i32, leftUp)-p);
                    u8 least = 0;
                    if (distLeft <= distUp && distLeft <= distLeftUp){
                        least = left; 
                    }
                    else if (distUp <= distLeftUp){
                        least = up;
                    }
                    else{
                        least = leftUp;
                    }
                    *(target->data + pitch + w) = *(uncompressed + pitch + (h+1) + w) + least;
                }
                
            }
            else{
                INV;
            }
            compressedHead.offset = compressed + compressedOffset - 4;
            // adler
            scanDword(&compressedHead, ByteOrder_BigEndian);
        }
    }
    POP;
    return checks == 2 && (decodedBytes == target->info.samplesPerPixel * (target->info.bitsPerSample/8) * target->info.width * target->info.height + target->info.height);
}

struct Bitmapinfoheader{
    union{
        struct{
            u32 size;
            i32 width;
            i32 height;
            u16 colorPlanes;
            u16 bitsPerPixel;
            u32 compression;
            u32 datasize;
            i32 pixelPerMeterHorizontal;
            i32 pixelPerMeterVertical;
            u32 colorsInPallette;
            u32 importantColorsAmount;
            u32 redMask;
            u32 greenMask;
            u32 blueMask;
            u32 alphaMask;
            char colorSpace[4];
            char whatever[36];
            u32 gammaRed;
            u32 gammaGreen;
            u32 gammaBlue;
            u32 imageIntent;
            u32 profileData;
            u32 profileSize;
            u32 reserved;
        };
        char data[124];
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
            u32 redMask = infoheader->redMask;
            u32 redMaskFallShift = redMask > 0xFF ? (redMask > 0xFF00 ? (redMask > 0xFF0000 ? 24 : 16) : 8) : 0;
            
            u32 greenMask = infoheader->greenMask;
            u32 greenMaskFallShift = greenMask > 0xFF ? (greenMask > 0xFF00 ? (greenMask > 0xFF0000 ? 24 : 16) : 8) : 0;
            
            u32 blueMask = infoheader->blueMask;
            u32 blueMaskFallShift = blueMask > 0xFF ? (blueMask > 0xFF00 ? (blueMask > 0xFF0000 ? 24 : 16) : 8) : 0;
            
            u32 alfaMask = infoheader->alphaMask;
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
    
    u32 headerSize = sizeof(Bitmapinfoheader);
    if(!palette){
        target->size = 14 + headerSize + (savewidth * source->info.height * bitsPerPixel/8);
    }else{
        target->size = 14 + headerSize + (savewidth * source->info.height * bitsPerPixel/8) + (4 * (1 << bitsPerPixel));
    }
    target->contents = &PUSHA(char, target->size);
    
    target->contents[0] = 'B';
    target->contents[1] = 'M';
    *((u32 *)(target->contents + 2)) = target->size;
    *((u32 *)(target->contents + 6)) = 0;
    u32 dataOffset = *((u32 *)(target->contents + 10)) = 14 + headerSize + (palette ? (4 * (1 << bitsPerPixel)) : 0);
    
    Bitmapinfoheader * infoheader = (Bitmapinfoheader *)(target->contents + 14);
    memset(infoheader, 0, sizeof(Bitmapinfoheader));
    infoheader->size = sizeof(Bitmapinfoheader);
    infoheader->width = source->info.width;
    infoheader->height = source->info.height;
    infoheader->colorPlanes = 1;
    infoheader->bitsPerPixel = bitsPerPixel;
    u8 map[4] = {};
    if (source->info.interpretation == BitmapInterpretationType_GrayscaleBW01 || source->info.interpretation == BitmapInterpretationType_RGB){
        infoheader->compression = 0; //RGB
        map[0] = 3;
        map[1] = 2;
        map[2] = 1;
        map[3] = 0;
    }
    else if (source->info.interpretation == BitmapInterpretationType_RGBA){
       infoheader->compression = 3; //RGBA
       memcpy(infoheader->colorSpace, "BGRs", 4);
       ASSERT(infoheader->bitsPerPixel == 32);
       infoheader->redMask  = 0x00FF0000;
       infoheader->greenMask  = 0x0000FF00;
       infoheader->blueMask  = 0x000000FF;
       infoheader->alphaMask  = 0xFF000000;
        map[0] = 2;
        map[1] = 1;
        map[2] = 0;
        map[3] = 3;
    }
    else{
        INV;
    }
    infoheader->datasize = savewidth * source->info.height * bitsPerPixel/8;
    if(palette){
        infoheader->colorsInPallette = 1 << infoheader->bitsPerPixel;
        infoheader->importantColorsAmount = 1 << infoheader->bitsPerPixel;
        ASSERT((1 << bitsPerPixel) == 256);
        ASSERT(source->info.interpretation == BitmapInterpretationType_GrayscaleBW01);
        for(u16 i = 0; i < (1 << bitsPerPixel); i++){
            //R
            *(target->contents + 14 + headerSize + i*4) = (u8)i;
            //G
            *(target->contents + 14 + headerSize + i*4 + 1) = (u8)i;
            //B
            *(target->contents + 14 + headerSize + i*4 + 2) = (u8)i;
            //unused
            *(target->contents + 14 + headerSize + i*4 + 3) = (u8)0;
        }
    }
    //@Incomplete
    //Implement other possibilities
    ASSERT(bitsPerPixel % 8 == 0);
    ASSERT(source->info.interpretation == BitmapInterpretationType_GrayscaleBW01 || source->info.interpretation == BitmapInterpretationType_RGB || source->info.interpretation == BitmapInterpretationType_RGBA);
    //the r g b  flipped to b g r
    u16 bytesPerPixel = bitsPerPixel/8;
    if(source->info.origin == BitmapOriginType_BottomLeft){
        for(i32 h = 0; h < infoheader->height; h++){
            for(i32 w = 0; w < infoheader->width; w++){
                for(u8 byteIndex = 0; byteIndex < bytesPerPixel; byteIndex++){
                    (CAST(u8*, target->contents) + dataOffset)[h * savewidth * bytesPerPixel + w * bytesPerPixel + map[byteIndex]] = source->data[h * infoheader->width * bytesPerPixel + w * bytesPerPixel + byteIndex]; 
                }
            }
            for(u32 w = infoheader->width; w < savewidth; w++){
                for(u8 byteIndex = 0; byteIndex < bytesPerPixel; byteIndex++){
                    (CAST(u8*, target->contents) + dataOffset)[h * savewidth * bytesPerPixel + w * bytesPerPixel + byteIndex] = 0;
                }
            }
        }
    }else if(source->info.origin == BitmapOriginType_TopLeft){
        for(i32 h = 0; h < infoheader->height; h++){
            for(i32 w = 0; w < infoheader->width; w++){
                for(u8 byteIndex = 0; byteIndex < bytesPerPixel; byteIndex++){
                    (CAST(u8*, target->contents) + dataOffset)[h * savewidth * bytesPerPixel + w * bytesPerPixel + map[byteIndex]] =  source->data[(infoheader->height - 1 - h) * infoheader->width * bytesPerPixel + w * bytesPerPixel + byteIndex];
                }
            }
            for(u32 w = infoheader->width; w < savewidth; w++){
                for(u8 byteIndex = 0; byteIndex < bytesPerPixel; byteIndex++){
                    (CAST(u8*, target->contents) + dataOffset)[h * savewidth * bytesPerPixel + w * bytesPerPixel + byteIndex] = 0;
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
    head.offset = CAST(u8*, file->contents);
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
    
    
    u16 tiffMagicFlag = scanWord(&head, ByteOrder_LittleEndian);
    //tiff const flag
    if(tiffMagicFlag != 42){
        INV;
        return false;
    }
    
    u32 imageOffset = scanDword(&head, ByteOrder_LittleEndian);
    head.offset = CAST(u8*, file->contents) + imageOffset;
    
    u16 entries = scanWord(&head, ByteOrder_LittleEndian);
    
    u32 * stripOffsets = NULL;
    u32 rowsPerStrip = CAST(u32, -1); //infinity (1 strip for all data)
    u32 * stripSizes = NULL;
    u32 stripAmount = 0;
    
    //default values
    target->info.origin = BitmapOriginType_TopLeft;
    target->info.samplesPerPixel = 1;
    
    for(u16 ei = 0; ei < entries; ei++){
        u16 tag = scanWord(&head, ByteOrder_LittleEndian);
        u16 type = scanWord(&head, ByteOrder_LittleEndian);
        u32 length = scanDword(&head, ByteOrder_LittleEndian);
        u32 headerOffset = scanDword(&head, ByteOrder_LittleEndian);
        
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
    
    u32 nextFile = scanDword(&head, ByteOrder_LittleEndian);
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
        last = decompressLZW(CAST(u8*, file->contents) + stripOffsets[stripIndex], stripSizes[stripIndex], target->data + stripIndex * rowsPerStrip * target->info.width);
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

#endif
