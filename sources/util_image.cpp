#ifndef UTIL_IMAGE_H
#define UTIL_IMAGE_H

struct Image{
    uint32 width;
    uint32 height;
    uint32 * pixeldata;
    
};

struct NearestNeighbourColor{
    uint32 color;
    uint8 times;
};




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




#endif
