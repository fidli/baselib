#ifndef UTIL_IMAGE_H
#define UTIL_IMAGE_H

#include "util_math.cpp"

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
    byte * data;
    
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



#endif
