#ifndef UTIL_IMAGE_H
#define UTIL_IMAGE_H

enum BitmapInterpretationType{
    BitmapInterpretationType_Invalid,
    BitmapInterpretationType_GrayscaleBW01
};

enum BitmapOriginType{
    BitmapOriginType_Invalid,
    BitmapOriginType_TopLeft,
    BitmapOriginType_BottomLeft
};

struct Image{
    struct{
        uint32 width;
        uint32 height;
        uint8 bitsPerSample;
        uint8 samplesPerPixel;
        BitmapInterpretationType interpretation;
        BitmapOriginType origin;
    } info;
    byte * data;
    
};

union Color{
    struct{
        uint8 r;
        uint8 g;
        uint8 b;
        uint8 a;
    };
    uint8 channel[4];
    uint32 full;
    uint8 intensity;
};

struct NearestNeighbourColor{
    Color color;
    uint8 times;
};


bool cropImageX(Image * image, uint32 leftCrop, uint32 rightCrop){
    Image temp;
    temp.info = image->info;
    temp.info.width = rightCrop-leftCrop;
    ASSERT(image->info.origin == BitmapOriginType_TopLeft);
    if(image->info.origin == BitmapOriginType_TopLeft){
        return false;
    }
    if(leftCrop > rightCrop){
        uint32 tmp = leftCrop;
        leftCrop = rightCrop;
        rightCrop = tmp;
    }
    temp.data = &PUSHA(byte, temp.info.width * temp.info.height * temp.info.samplesPerPixel * temp.info.bitsPerSample/8);
    
    for(uint32 h = 0; h < temp.info.height; h++){
        uint32 i = h*temp.info.width;
        for(uint32 w = leftCrop; w < rightCrop; w++, i++){
            temp.data[i] = image->data[h * image->info.width + w];
        }
    }
    
    image->info.width = temp.info.width;
    for(uint32 i = 0; i < image->info.width * image->info.height * image->info.samplesPerPixel * (image->info.bitsPerSample/8); i++){
        image->data[i] = temp.data[i];
    }
    
    
    
    POP;
    return true;
}

void flipY(Image * target){
    uint32 bytesize = target->info.width * target->info.height * target->info.bitsPerSample * target->info.samplesPerPixel;
    uint8 * tmp = &PUSHA(uint8, bytesize);
    ASSERT(target->info.bitsPerSample * target->info.samplesPerPixel == 8);
    
    for(uint32 w = 0; w < target->info.width; w++){
        for(uint32 h = 0; h < target->info.height; h++){
            tmp[w + (target->info.height - h) * target->info.width] = target->data[w + target->info.height*h];
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
        default:{
            INV;
        }break;
    }
    
    POP;
}


bool cropImageY(Image * image, uint32 bottomCrop, uint32 topCrop){
    Image temp;
    temp.info = image->info;
    ASSERT(image->info.origin == BitmapOriginType_TopLeft);
    if(image->info.origin == BitmapOriginType_TopLeft){
        return false;
    }
    if(bottomCrop < topCrop){
        uint32 tmp = bottomCrop;
        bottomCrop = topCrop;
        topCrop = topCrop;
    }
    temp.info.height = bottomCrop-topCrop;
    temp.data = &PUSHA(byte, temp.info.width * temp.info.height * temp.info.samplesPerPixel * temp.info.bitsPerSample/8);
    
    uint32 i = 0;
    for(uint32 h = topCrop; h < bottomCrop; h++){
        for(uint32 w = 0; w < temp.info.width; w++, i++){
            temp.data[i] = image->data[h * image->info.width + w];
        }
    }
    
    image->info.height = temp.info.height;
    for(uint32 i = 0; i < image->info.width * image->info.height * image->info.samplesPerPixel * (image->info.bitsPerSample/8); i++){
        image->data[i] = temp.data[i];
    }
    
    
    
    POP;
    return true;
}


void rotateImage(Image * image, float32 angleDeg, float32 centerX = 0.5f, float32 centerY = 0.5f){
    
    ASSERT(image->info.bitsPerSample == 8 && image->info.samplesPerPixel == 1);
    Image temp;
    temp.info = image->info;
    temp.data = &PUSHA(byte, temp.info.width * temp.info.height * temp.info.samplesPerPixel * temp.info.bitsPerSample/8);
    float32 angleRad = -angleDeg;
    while(angleRad >= 360) angleRad -= 360;
    while(angleRad < 0) angleRad += 360;
    angleRad = (angleRad / 180) * PI;
    float32 sinA = sin(angleRad);
    float32 cosA = cos(angleRad);
    int32 cX = (uint32)(centerX * image->info.width);
    int32 cY = (uint32)(centerY * image->info.height);
    for(uint32 h = 0; h < temp.info.height; h++){
        int32 rY = h - cY;
        for(uint32 w = 0; w < temp.info.width; w++){
            int32 rX = w  - cX;
            int32 nX = (int32)(cosA * rX) - (int32)(sinA*rY) + cX;
            int32 nY = (int32)(sinA * rX) + (int32)(cosA*rY) + cY;
            if(nX >= 0 && nX < image->info.width && nY >= 0 && nY < image->info.height){
                temp.data[temp.info.width * h + w] = image->data[temp.info.width * nY  + nX];
            }else{
                temp.data[temp.info.width * h + w] = 0;
            }
        }
    }
    
    for(uint32 i = 0; i < image->info.width * image->info.height * image->info.samplesPerPixel * (image->info.bitsPerSample/8); i++){
        image->data[i] = temp.data[i];
    }
    
    POP;
}

static inline uint8 resultingContrast(const uint8 originalColor, const float32 contrast){
    float32 factor = (259 * (contrast + 255)) / (255 * (259 - contrast));
    float32 result = factor * (originalColor - 128) + 128;
    if(result < 0) result = 0;
    if(result > 255) result = 255;
    return (uint8) result;
}


void applyContrast(Image * target, const float32 contrast){
    ASSERT(target->info.interpretation = BitmapInterpretationType_GrayscaleBW01);
    for(uint32 x = 0; x < target->info.width; x++){
        for(uint32 y = 0; y < target->info.height; y++){
            uint8 originalColor = target->data[y * target->info.width + x];
            
            target->data[y * target->info.width + x] = resultingContrast(originalColor, contrast);
        }
    }
}


bool scaleImage(const Image * source, Image * target, uint32 targetWidth, uint32 targetHeight){
    float32 scaleX = (float32)source->info.width/(float32)targetWidth;
    float32 scaleY = (float32)source->info.height/(float32)targetHeight;
    
    NearestNeighbourColor * nn = &PUSH(NearestNeighbourColor);
    
    
    //support lesser bits per pixel later
    ASSERT(source->info.bitsPerSample * source->info.samplesPerPixel >= 8 && source->info.bitsPerSample * source->info.samplesPerPixel % 8 == 0);
    if(source->info.bitsPerSample * source->info.samplesPerPixel < 8 && source->info.bitsPerSample * source->info.samplesPerPixel % 8 != 0){
        return false;
    }
    uint8 channelCount = (source->info.bitsPerSample * source->info.samplesPerPixel) / 8;
    
    //each target pixel
    for(int tw = 0; tw < targetWidth; tw++){
        for(int th = 0; th < targetHeight; th++){
            int i = targetWidth*th  + tw;//final index
            
            //clear NN
            for(int clr = 0; clr < scaleX*scaleY; clr++){
                nn[clr] = {};
            }
            uint8 nncount = 0;
            
            //for all neighbours to the original pixel
            for(int nw = 0; nw < scaleX; nw++){
                int srcW = (int)(tw*scaleX) + nw;
                for(int nh = 0; nh < scaleY; nh++){
                    int srcH = (int)(th*scaleY) + nh;
                    
                    Color srcColor = {};
                    for(uint8 ci = 0; ci < channelCount; ci++){
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
            uint32 sum[4] = {};
            //int8  highest = -1;
            for(int s = 0; s < nncount; s++){
                for(uint8 ci = 0; ci < channelCount; ci++){
                    sum[ci] += nn[s].color.channel[ci];
                }
                
                /*if(nn[s].times > highest){
                    resultColor = nn[s].color;
                }*/
            }
            for(uint8 ci = 0; ci < channelCount; ci++){
                target->data[channelCount * i + ci] = sum[ci]/nncount;
            }
            
            
            
        }
    }
    target->info = source->info;
    target->info.width = targetWidth;
    target->info.height = targetHeight;
    return true;
}

void scaleCanvas(Image * target, uint32 newWidth, uint32 newHeight, uint32 originalOffsetX = 0, uint32 originalOffsetY = 0){
    ASSERT(target->info.bitsPerSample * target->info.samplesPerPixel == 8);
    ASSERT(target->info.origin == BitmapOriginType_TopLeft);
    byte * tmp = &PUSHA(byte, newWidth * newHeight);
    for(uint32 th = 0; th < newHeight; th++){
        for(uint32 tw = 0; tw < newWidth; tw++){
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
}

#endif
