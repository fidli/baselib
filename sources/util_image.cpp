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

struct NearestNeighbourColor{
    uint32 color;
    uint8 times;
};


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
    for(uint32 h = 0; h < temp.info.height; h++){
        float32 rY = ((float32)(h) / (float32)temp.info.width) - centerY;
        for(uint32 w = 0; w < temp.info.width; w++){
            float32 rX = ((float32)w / (float32)temp.info.width) - centerX;
            float32 nX = cosA * rX - sinA*rY + centerX;
            float32 nY = sinA * rX + cosA*rY + centerY;
            if(nX >= 0 && nX <= 1 && nY >= 0 && nY <= 1){
                temp.data[temp.info.width * h + w] = image->data[temp.info.width * (uint32)(nY * temp.info.height) + (uint32)(nX * temp.info.width)];
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


/* chechk and redo properly 
void scaleImage(const Image * source, Image * target, uint32 targetWidth, uint32 targetHeight){
float32 scaleX = (float32)source->width/(float32)targetWidth;
float32 scaleY = (float32)source->height/(float32)targetHeight;

NearestNeighbourColor * nn = &PUSH(NearestNeighbourColor);

target->width = targetWidth;
target->height = targetHeight;

ASSERT(source->width > targetWidth && source->height > targetHeight); //support scaling up later

//each target pixel
for(int tw = 0; tw < target->width; tw++){
for(int th = 0; th < target->height; th++){
int i = target->width*th  + tw;//final index

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

uint32 srcColor = source->pixeldata[srcH*source->width + srcW];                
//look for same color;
bool found = false;
for(int s = 0; s < nncount; s++){
if(nn[s].color == srcColor){
nn[s].times++;
found = true;
break;
}
}
if(!found){
nn[nncount++].color = srcColor;
}
//do not overstep image boundaries, last neighbours could be a little bit off
if(srcH > source->width){
break;
}
}
//do not overstep image boundaries, last neighbours could be a little bit off
if(srcW > source->height){
break;
}
}

uint32 resultColor = 0xFFFFFFFF;
int8  highest = -1;
for(int s = 0; s < nncount; s++){
if(nn[s].times > highest){
resultColor = nn[s].color;
}
}
target->pixeldata[i] = resultColor;

}
}

}


*/

#endif
