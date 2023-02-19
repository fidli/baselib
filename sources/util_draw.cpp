#pragma once
#include "util_image.cpp"
#include "util_math.cpp"

void drawLine(Image * target, const dv2 * from, const dv2 * to, const Color color, u8 thickness = 1){
    ASSERT(target->info.origin == BitmapOriginType_TopLeft && target->info.interpretation == BitmapInterpretationType_ARGB);
    
    i32 minY = MIN(from->y, to->y);
    i32 maxY = from->y == minY ? to->y : from->y;
    i32 minX = MIN(from->x, to->x);
    i32 maxX = from->x == minX ? to->x : from->x;
    if(minY < 0) minY = 0;
    if(maxY < 0) maxY = 0;
    if(minX < 0) minX = 0;
    if(maxX < 0) maxX = 0;
    if(CAST(u32, minY) >= target->info.height) minY = target->info.height - 1;
    if(CAST(u32, maxY) >= target->info.height) maxY = target->info.height - 1;
    if(CAST(u32, minX) >= target->info.width) minX = target->info.width - 1;
    if(CAST(u32, maxX) >= target->info.width) maxX = target->info.width - 1;
    
    bool vertical = minX == maxX;
    
    f32 k;
    f32 q;
    
    if(!vertical){
        //width > heigth
        if(maxX - minX > maxY-minY){
            //y = kx + q
            k = (f32)((i32)from->y - (i32)to->y)/((i32)from->x - (i32)to->x);
            q = (i32)to->y - k*(i32)to->x;
            
            for(i32 w = minX; w <= maxX; w++){
                i32 linepoint = (i32)(k*w + q);
                linepoint -= thickness/2;
                for(u8 t = 0; t < thickness; t++){
                    if(linepoint >= minY && linepoint <= maxY){
                        u32 pitch = linepoint*target->info.width;
                        ((u32 *)target->data)[pitch + w] = color.full;
                    }
                    linepoint++;
                }
                
            }
        }else{
            //x = ky + q
            k = (f32)((i32)from->x - (i32)to->x)/((i32)from->y - (i32)to->y);
            q = (i32)to->x - k*(i32)to->y;
            
            for(i32 h = minY; h < maxY; h++){
                i32 linepoint = (i32)(k*h + q);
                linepoint -= thickness/2;
                for(u8 t = 0; t < thickness; t++){
                    if(linepoint >= minX && linepoint <= maxX){
                        u32 pitch = h*target->info.width;
                        ((u32 *)target->data)[pitch + linepoint] = color.full;
                    }
                    linepoint++;
                }
            }
        }
        
        
    }else{
        for(i32 h = minY; h < CAST(i32, target->info.height) && h <= maxY; h++){
            u32 pitch = h*target->info.width;
            ((u32 *)target->data)[pitch + minX] = color.full;
        }
        
    }
    
    
}

//this is not rotated
void drawRectangle(Image * target, const dv2 * topLeft, const dv2 * botRight, const Color borderColor, u8 borderThickness = 1, bool filled = false){
    ASSERT(target->info.origin == BitmapOriginType_TopLeft && target->info.interpretation == BitmapInterpretationType_ARGB);
    if(!filled){
        dv2 topRight = {botRight->x, topLeft->y};
        dv2 botLeft = {topLeft->x, botRight->y};
        drawLine(target, topLeft, &topRight, borderColor, borderThickness); 
        drawLine(target, &topRight, botRight, borderColor, borderThickness); 
        drawLine(target, botRight, &botLeft, borderColor, borderThickness); 
        drawLine(target, &botLeft, topLeft, borderColor, borderThickness);
    }else{
        for(i32 y = topLeft->y; y < CAST(i32, target->info.height) && y <= botRight->y; y++){
            i32 pitch = y * target->info.width;
            for(i32 x = topLeft->x; x < CAST(i32, target->info.width) && x <= botRight->x; x++){
                ((u32 *)target->data)[pitch + x] = borderColor.full;
            }
        }
    }
}

void drawQuad(Image * target, const dv2 * topLeft, const dv2 * topRight, const dv2 * botLeft, const dv2 * botRight, const Color borderColor, u8 borderThickness = 1){
    ASSERT(target->info.origin == BitmapOriginType_TopLeft && target->info.interpretation == BitmapInterpretationType_ARGB);
    drawLine(target, topLeft, topRight, borderColor, borderThickness); 
    drawLine(target, topRight, botRight, borderColor, borderThickness); 
    drawLine(target, botRight, botLeft, borderColor, borderThickness); 
    drawLine(target, botLeft, topLeft, borderColor, borderThickness); 
}


void drawCircle(Image * target, const dv2 * center, u32 radius, const Color borderColor, u8 borderThickness = 1, bool filled = false){
    ASSERT(target->info.origin == BitmapOriginType_TopLeft && target->info.interpretation == BitmapInterpretationType_ARGB);
    u8 thicknessReal = MAX(borderThickness/2, 1);
    i32 startY = MAX(0, (i32)(center->y - radius - thicknessReal));
    i32 startX = MAX(0, (i32)(center->x - radius - thicknessReal));
    //+1 because this is count, not index
    i32 endY = MIN(target->info.height, (i32)(center->y + radius + thicknessReal + 1));
    i32 endX = MIN(target->info.width, (i32)(center->x + radius + thicknessReal + 1));
    
    for(i32 y = startY; y < endY; y++){
        i32 pitch = y*target->info.width;
        for(i32 x = startX; x < endX; x++){
            dv2 pos = {x - center->x, y - center->y};
            f32 len = length(pos);
            if(len < radius + thicknessReal){
                if(filled || len > radius - thicknessReal){
                    ((u32 *)target->data)[pitch + x] = borderColor.full;
                }
            }
        }
    }
    
}

void drawTriangle(Image * target, const dv2 * A, const dv2 * B, const dv2 * C, const Color borderColor, u8 borderThickness = 1, bool filled = false){
    ASSERT(target->info.origin == BitmapOriginType_TopLeft && target->info.interpretation == BitmapInterpretationType_ARGB);
    if(!filled){
        drawLine(target, A, B, borderColor, borderThickness);
        drawLine(target, B, C, borderColor, borderThickness);
        drawLine(target, C, A, borderColor, borderThickness);
    }else{
        u8 thicknessReal = MAX(borderThickness/2, 1);
        dv2 leftTop;
        dv2 rightBot;
        leftTop.x = MAX(0, MIN(MIN(A->x, B->x), C->x));
        leftTop.y = MAX(0, MIN(MIN(A->y, B->y), C->y));
        rightBot.x = MIN(CAST(i32, target->info.width - thicknessReal), MAX(MAX(A->x, B->x), C->x));
        rightBot.y = MIN(CAST(i32, target->info.height - thicknessReal), MAX(MAX(A->y, B->y), C->y));
        
        
        /*convex hull style (is convex hull still triangle?)
        //http://mathworld.wolfram.com/TriangleInterior.html // does not seem to work properly
        
        v2 _v0 = {(f32)A->x, (f32)A->y};
        dv2 v1d = *B - *A;
        dv2 v2d = *C - *A;
        v2 _v1 = {(f32) v1d.x, (f32) v1d.y};
        v2 _v2 = {(f32) v2d.x, (f32) v2d.y};
        
        f32 detV1V2 = det(_v1, _v2);
        f32 detV0V2 = det(_v0, _v2);
        f32 detV0V1 = det(_v0, _v1);
        
        for(i32 y = leftTop.y; y < rightBot.y; y++){
            i32 pitch = y*target->info.width;
            for(i32 x = leftTop.x; x < rightBot.x; x++){
            
                v2 _v = {(f32) (x), (f32) (y)};
                f32 a = (det(_v, _v2) - detV0V2) / detV1V2;
                f32 b = (det(_v, _v1) - detV0V1) / detV1V2;
                if(((a > 0 && b < 0) || (a < 0 && b > 0)) && (a + b < 1)){
                    ((u32 *)target->data)[pitch + x] = borderColor.full;
                }
            }
            */
        //barycentric stuff
        //https://math.stackexchange.com/questions/51326/determining-if-an-arbitrary-point-lies-inside-a-triangle-defined-by-three-points
        
        dv2 b0 = *B - *A;
        dv2 c0 = *C - *A;
        
        f32 d = (f32)(b0.x * c0.y - c0.x * b0.y);
        
        f32 member1 = (f32)(b0.y - c0.y);
        f32 member2 = (f32)(c0.x - b0.x);
        f32 member3 = (f32)(b0.x * c0.y);
        f32 member4 = (f32)(c0.x * b0.y);
        
        
        for(i32 y = leftTop.y; y < rightBot.y; y++){
            i32 pitch = y*target->info.width;
            for(i32 x = leftTop.x; x < rightBot.x; x++){
                
                v2 p0 = {(f32) (x-A->x), (f32) (y-A->y)};
                
                f32 wA = (p0.x * member1 + p0.y * member2 + member3 - member4) / d;
                f32 wB = (p0.x * c0.y - p0.y * c0.x) / d;
                f32 wC = (p0.y * b0.x - p0.x * b0.y) / d;
                
                if(wA > 0 && wB > 0 && wC > 0 && wA < 1 && wB < 1 && wC < 1){
                    ((u32 *)target->data)[pitch + x] = borderColor.full;
                }
            }
        }
        
    }
}
