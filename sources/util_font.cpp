 #include "common.h"
#include "util_image.cpp"
 
#ifndef UTIL_FONT
#define UTIL_FONT
 
 struct BitmapFont{
     struct {
         Image data;
         uint32 gridSize;
     } original;
     struct {
         Image data;
         uint32 gridSize;
     } current;
 };
 
 bool initBitmapFont(BitmapFont * target, const Image * source, uint32 gridSize){
     memcpy(&target->original.data, source, sizeof(Image));
     memcpy(&target->current.data, source, sizeof(Image));
     
     uint32 bytesize =  target->current.data.info.width * target->current.data.info.height *target->current.data.info.bitsPerSample * target->current.data.info.samplesPerPixel;
     
     target->current.data.data = &PUSHA(byte,bytesize);
     
     memcpy(target->current.data.data, source->data, bytesize);
     
     target->original.gridSize = gridSize;
     target->current.gridSize = gridSize;
     return true;
 }
 
 bool printToBitmap(Image * target, uint32 startX, uint32 startY, const char * asciiText, BitmapFont * font, uint32 fontSize){
     ASSERT(fontSize <= font->original.gridSize);
     if(fontSize > font->original.gridSize){
         return false;
     }
     if(font->current.gridSize != fontSize){
         if(!scaleImage(&font->original.data, &font->current.data, fontSize * 16, fontSize * 16)){
             return false;
         }
         font->current.gridSize = fontSize;
     }
     
     ASSERT(startY <= (int64)target->info.height - fontSize);
     ASSERT(startX <= (int64)target->info.width - fontSize * strlen(asciiText));
     ASSERT(target->info.samplesPerPixel * target->info.bitsPerSample == 8);
     
     if(startY > (int64)target->info.height - fontSize || startX > (int64)target->info.width - fontSize * strlen(asciiText) ||target->info.samplesPerPixel * target->info.bitsPerSample != 8){
         return false;
     }
     
     uint32 letterIndex = 0;
     while(asciiText[letterIndex] != '\0'){
         uint32 sourcePixel = (asciiText[letterIndex] / 16) * fontSize * font->current.data.info.width + (asciiText[letterIndex] % 16) * fontSize;
         uint32 targetPixel = startX + startY * target->info.width + letterIndex * fontSize;
         
         for(uint32 rW = 0; rW < fontSize; rW++){
             for(uint32 rH = 0; rH < fontSize; rH++){
                 if(font->current.data.data[sourcePixel + rW + rH * font->current.data.info.width] > 0){
                     target->data[targetPixel + rW + rH * target->info.width] = font->current.data.data[sourcePixel + rW + rH * font->current.data.info.width];
                 }
             }
         }
         
         letterIndex++;
     }
     return true;
 }
 
 #endif