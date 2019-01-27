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

struct GlyphData{
    bool valid;
    char glyph;
    int32 width;
    
    int32 marginX;
    int32 marginY;
    
    int32 kerning[256];
    
    struct{
        int32 x,y;
        int32 width, height;
    } AABB;
};

struct AtlasFont{
    Image data;
    int32 size;
    int32 height;
    GlyphData glyphs[256];
};


bool initAtlasFont(AtlasFont * target, const char * atlasBMPPath, const char * descriptionXMLpath){
    
    PUSHI;
    FileContents fontFile = {};
    bool r = readFile(atlasBMPPath, &fontFile);
    ASSERT(r);
    
    r &= decodeBMP(&fontFile, &target->data);
    ASSERT(r);
    ASSERT(target->data.info.interpretation == BitmapInterpretationType_RGBA);
    if(target->data.info.origin == BitmapOriginType_BottomLeft){
        r &= flipY(&target->data);
        ASSERT(r);
    }
    ASSERT(target->data.info.origin == BitmapOriginType_TopLeft);
    
    //load font atlas map
    FileContents mapFile = {};
    r = readFile(descriptionXMLpath, &mapFile);
    ASSERT(r);
    XMLNode * font = parseXML(&mapFile);
    ASSERT(font);
    
    
    
    bool success = true;
    
    if(strncmp("Font", font->name, 4)){
        INV;
        success = false;
    }else{
        for(int32 a = 0; a < font->attributesCount && success; a++){
            if(!strncmp("size", font->attributeNames[a], 4)){
                if(!sscanf(font->attributeValues[a], "%d", &target->size)){
                    INV;
                    success = false;
                    break;
                }
            }else if(!strncmp("height", font->attributeNames[a], 6)){
                if(!sscanf(font->attributeValues[a], "%d", &target->height)){
                    INV;
                    success = false;
                    break;
                }
            }
        }
    }
    
    for(int32 i = 0; i < font->childrenCount && success; i++){
        XMLNode * ch = font->children[i];
        if(!strncmp(ch->name, "Char", 4)){
            char glyph;
            GlyphData tmp = {};
            for(int32 a = 0; a < ch->attributesCount; a++){
                if(!strncmp("width", ch->attributeNames[a], 5)){
                    if(!sscanf(ch->attributeValues[a], "%d", &tmp.width)){
                        INV;
                        success = false;
                        break;
                    }
                }else if(!strncmp("offset", ch->attributeNames[a], 6)){
                    if(sscanf(ch->attributeValues[a], "%d %d", &tmp.marginX, &tmp.marginY) != 2){
                        INV;
                        success = false;
                        break;
                    }
                }else if(!strncmp("rect", ch->attributeNames[a], 4)){
                    if(sscanf(ch->attributeValues[a], "%d %d %d %d", &tmp.AABB.x, &tmp.AABB.y, &tmp.AABB.width, &tmp.AABB.height) != 4){
                        INV;
                        success = false;
                        break;
                    }
                }else if(!strncmp("code", ch->attributeNames[a], 4)){
                    
                    if(!strncmp("&quot;", ch->attributeValues[a], 6)){
                        glyph = '"';
                    }else if(!strncmp("&amp;", ch->attributeValues[a], 5)){
                        glyph = '&';
                    }else if(!strncmp("&lt;", ch->attributeValues[a], 4)){
                        glyph = '<';
                    }else if(!snscanf(ch->attributeValues[a], 1, "%c", &glyph)){
                        INV;
                        success = false;
                        break;
                    }
                    tmp.glyph = glyph;
                }
            }
            tmp.valid = true;
            target->glyphs[glyph] = tmp;
            for(int32 k = 0; k < ch->childrenCount; k++){
                XMLNode * kerningChild = ch->children[k];
                char against = 0;
                int32 advance = 0;
                for(int32 a = 0; a < kerningChild->attributesCount; a++){
                    if(!strncmp("advance", kerningChild->attributeNames[a], 7)){
                        if(!sscanf(kerningChild->attributeValues[a], "%d", &advance)){
                            INV;
                            success = false;
                            break;
                        }
                    }else if(!strncmp("id", kerningChild->attributeNames[a], 2)){
                        if(!sscanf(kerningChild->attributeValues[a], "%c", &against)){
                            INV;
                            success = false;
                            break;
                        }
                    }
                }
                ASSERT(against != 0);
                ASSERT(advance != 0);
                tmp.kerning[against] = advance;
            }
        }
    }
    //NOTE(AK): copy to persistent memory
    //@Cleanup whet decode BMP allows use of allocator
    Image persOrig = target->data;
    persOrig.data = &PPUSHA(byte, persOrig.info.totalSize);
    
    memcpy(persOrig.data, target->data.data, persOrig.info.totalSize);
    POPI;
    target->data = persOrig;
    return success;
}

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
    
    if(startY > (int64)target->info.height - fontSize) return false;
    if(startX > (int64)target->info.width - fontSize * strlen(asciiText)) return false;
    
    if(font->current.gridSize != fontSize){
        if(!scaleImage(&font->original.data, &font->current.data, fontSize * 16, fontSize * 16)){
            return false;
        }
        font->current.gridSize = fontSize;
    }
    startX = MAX(0, startX);
    startY = MAX(0, startY);
    
    //todo implement support
    ASSERT(target->info.samplesPerPixel * target->info.bitsPerSample >= 8);
    ASSERT(target->info.samplesPerPixel * target->info.bitsPerSample % 8 == 0);
    
    if(startY > (int64)target->info.height - fontSize || startX > (int64)target->info.width - fontSize * strlen(asciiText) ||target->info.samplesPerPixel * target->info.bitsPerSample < 8 ||
       target->info.samplesPerPixel * target->info.bitsPerSample % 8 != 0){
        return false;
    }
    
    
    uint8 bytes = (target->info.samplesPerPixel * target->info.bitsPerSample) / 8;
    uint32 letterIndex = 0;
    while(asciiText[letterIndex] != '\0'){
        ASSERT((uint8)asciiText[letterIndex] <= 127);
        uint32 sourcePixel = (asciiText[letterIndex] / 16) * fontSize * font->current.data.info.width + (asciiText[letterIndex] % 16) * fontSize;
        uint32 offsetRow = startY * target->info.width;
        uint32 offsetCol = startX + letterIndex * fontSize;
        uint32 targetPixel = offsetRow + offsetCol;
        
        
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

int32 calculateAtlasTextWidth(const AtlasFont * font, const char * text, int pt){
    int32 width = 0;
    int32 targetSize = pt;
    float32 fontScale = (float32)targetSize / font->size;
    char prevGlyph = 0;
    for(int i = 0; i < strlen(text); i++){
        const GlyphData * glyph = &font->glyphs[text[i]];
        ASSERT(glyph->valid);
        //kerning?
        if(prevGlyph){
            width -= (int32)((float32)glyph->kerning[prevGlyph]*fontScale);
        }
        width += (int32)((float32)glyph->width * fontScale);
    }
    return width;
}


#endif