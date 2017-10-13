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
     
     target->current.data.data = &PUSHA(byte,bytesize*2);
     
     memcpy(target->current.data.data, source->data, bytesize);
     
     target->original.gridSize = gridSize;
     target->current.gridSize = gridSize;
     return true;
 }
 
 bool printToBitmap(Image * target, uint32 startX, uint32 startY, const char * asciiText, BitmapFont * font, uint32 fontSize, Color color = {0xFF,0xFF,0xFF,0xFF}){
     
     
     if(font->current.gridSize != fontSize){
         //font->current.data.data = &PUSHA(byte, fontSize*16*fontSize*16);
         if(!scaleImage(&font->original.data, &font->current.data, fontSize * 16, fontSize * 16)){
             return false;
         }
         font->current.gridSize = fontSize;
     }
     
     ASSERT(startY <= (int64)target->info.height - fontSize);
     ASSERT(startX <= (int64)target->info.width - fontSize * strlen(asciiText));
     ASSERT(target->info.samplesPerPixel * target->info.bitsPerSample >= 8);
     ASSERT(target->info.samplesPerPixel * target->info.bitsPerSample % 8 == 0);
     
     if(startY > (int64)target->info.height - fontSize || startX > (int64)target->info.width - fontSize * strlen(asciiText) ||target->info.samplesPerPixel * target->info.bitsPerSample < 8 ||
        target->info.samplesPerPixel * target->info.bitsPerSample % 8 != 0){
         return false;
     }
     
     
     uint8 bytes = (target->info.samplesPerPixel * target->info.bitsPerSample) / 8;
     uint32 letterIndex = 0;
     while(asciiText[letterIndex] != '\0'){
         uint32 sourcePixel = (asciiText[letterIndex] / 16) * fontSize * font->current.data.info.width + (asciiText[letterIndex] % 16) * fontSize;
         uint32 targetPixel = startX + startY * target->info.width + letterIndex * fontSize;
         
         
         for(uint32 rH = 0; rH < fontSize; rH++){
             uint32 pitch = (targetPixel + rH * target->info.width);
             uint32 fontpitch = sourcePixel + rH * font->current.data.info.width;
             for(uint32 rW = 0; rW < fontSize; rW++){
                 if(font->current.data.data[rW + fontpitch] > 0){
                     uint32 bpitch = (rW + pitch)*bytes;
                     for(uint8 bi = 0; bi < bytes; bi++){
                         target->data[bpitch + bi] = color.channel[bi];
                     }
                 }
             }
         }
         
         letterIndex++;
     }
     return true;
 }
 
 #endif