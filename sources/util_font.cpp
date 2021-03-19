#ifndef UTIL_FONT
#define UTIL_FONT

#include "common.h"
#include "util_image.cpp"
#include "util_conv.cpp"

struct BitmapFont{
    struct {
        Image data;
        u32 gridSize;
    } original;
    struct {
        Image data;
        u32 gridSize;
    } current;
};

struct GlyphData{
    bool valid;
    char glyph;
    i32 width;
    
    i32 marginX;
    i32 marginY;
    
    i32 kerning[256];
    
    struct{
        i32 x,y;
        i32 width, height;
    } AABB;
};

struct AtlasFont{
    Image data;
    i32 pixelSize;
    i32 lineHeight;
    i32 base;
    GlyphData glyphs[256];
};

int32 platformGetDpi();
int32 ptToPx(f32 pt){
    return CAST(i32, (pt * CAST(f32, platformGetDpi())) / 72.0f);
}

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
    
    if(strncmp("font", font->name, 4)){
        INV;
        success = false;
    }else{
    }
    
    for(i32 i = 0; i < font->childrenCount && success; i++){
        XMLNode * ch = font->children[i];
        if(!strncmp(ch->name, "info", 4)){
            for(i32 a = 0; a < ch->attributesCount && success; a++){
                if(!strncmp("size", ch->attributeNames[a], 4)){
                    if(!sscanf(ch->attributeValues[a], "%d", &target->pixelSize)){
                        INV;
                        success = false;
                        break;
                    }
                }
            }
        }else if(!strncmp(ch->name, "common", 6)){
            for(i32 a = 0; a < ch->attributesCount && success; a++){
                if(!strncmp("lineHeight", ch->attributeNames[a], 6)){
                    if(!sscanf(ch->attributeValues[a], "%d", &target->lineHeight)){
                        INV;
                        success = false;
                        break;
                    }
                }
                if(!strncmp("base", ch->attributeNames[a], 6)){
                    if(!sscanf(ch->attributeValues[a], "%d", &target->base)){
                        INV;
                        success = false;
                        break;
                    }
                }
            }
        }else if(!strncmp(ch->name, "char", 4)){
            for(i32 j = 0; j < ch->childrenCount && success; j++){
                XMLNode * chr = ch->children[j];
                char glyph;
                GlyphData tmp = {};
                for(i32 a = 0; a < chr->attributesCount; a++){
                    if(!strncmp("width", chr->attributeNames[a], 5)){
                        if(!sscanf(chr->attributeValues[a], "%d", &tmp.AABB.width)){
                            INV;
                            success = false;
                            break;
                        }
                    }else if(!strncmp("xoffset", chr->attributeNames[a], 7)){
                        if(sscanf(chr->attributeValues[a], "%d", &tmp.marginX) != 1){
                            INV;
                            success = false;
                            break;
                        }
                    }else if(!strncmp("yoffset", chr->attributeNames[a], 7)){
                        if(sscanf(chr->attributeValues[a], "%d", &tmp.marginY) != 1){
                            INV;
                            success = false;
                            break;
                        }
                    }else if(!strncmp("xadvance", chr->attributeNames[a], 8)){
                        if(sscanf(chr->attributeValues[a], "%d", &tmp.width) != 1){
                            INV;
                            success = false;
                            break;
                        }
                    }else if(!strncmp("x", chr->attributeNames[a], 1)){
                        if(sscanf(chr->attributeValues[a], "%d", &tmp.AABB.x) != 1){
                            INV;
                            success = false;
                            break;
                        }
                    }else if(!strncmp("y", chr->attributeNames[a], 1)){
                        if(sscanf(chr->attributeValues[a], "%d", &tmp.AABB.y) != 1){
                            INV;
                            success = false;
                            break;
                        }
                    }else if(!strncmp("height", chr->attributeNames[a], 6)){
                        if(sscanf(chr->attributeValues[a], "%d", &tmp.AABB.height) != 1){
                            INV;
                            success = false;
                            break;
                        }
                    }else if(!strncmp("id", chr->attributeNames[a], 2)){
                        if(sscanf(chr->attributeValues[a], "%hhu", &glyph)){
                            tmp.glyph = glyph;
                        }else{
                            success = false;
                            break;
                            INV;
                        }
                    }
                }
                tmp.valid = true;
                ASSERT(tmp.AABB.width != 0);
                ASSERT(tmp.AABB.height != 0);
                target->glyphs[glyph] = tmp;
            /*
            for(i32 k = 0; k < ch->childrenCount; k++){
                XMLNode * kerningChild = ch->children[k];
                char against = 0;
                i32 advance = 0;
                for(i32 a = 0; a < kerningChild->attributesCount; a++){
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
            }*/
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

bool initBitmapFont(BitmapFont * target, const Image * source, u32 gridSize){
    memcpy(&target->original.data, source, sizeof(Image));
    memcpy(&target->current.data, source, sizeof(Image));
    
    u32 bytesize =  target->current.data.info.width * target->current.data.info.height *target->current.data.info.bitsPerSample * target->current.data.info.samplesPerPixel;
    
    target->current.data.data = &PUSHA(byte,bytesize*2);
    
    memcpy(target->current.data.data, source->data, bytesize);
    
    target->original.gridSize = gridSize;
    target->current.gridSize = gridSize;
    return true;
}


bool printToBitmap(Image * target, u32 startX, u32 startY, const char * asciiText, BitmapFont * font, u32 fontSize, Color color = {0xFF,0xFF,0xFF,0xFF}){
    
    if(startY > (i64)target->info.height - fontSize) return false;
    if(startX > (i64)target->info.width - fontSize * strlen(asciiText)) return false;
    
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
    
    if(startY > (i64)target->info.height - fontSize || startX > (i64)target->info.width - fontSize * strlen(asciiText) ||target->info.samplesPerPixel * target->info.bitsPerSample < 8 ||
       target->info.samplesPerPixel * target->info.bitsPerSample % 8 != 0){
        return false;
    }
    
    
    u8 bytes = (target->info.samplesPerPixel * target->info.bitsPerSample) / 8;
    u32 letterIndex = 0;
    while(asciiText[letterIndex] != '\0'){
        ASSERT((u8)asciiText[letterIndex] <= 127);
        u32 sourcePixel = (asciiText[letterIndex] / 16) * fontSize * font->current.data.info.width + (asciiText[letterIndex] % 16) * fontSize;
        u32 offsetRow = startY * target->info.width;
        u32 offsetCol = startX + letterIndex * fontSize;
        u32 targetPixel = offsetRow + offsetCol;
        
        
        for(u32 rH = 0; rH < fontSize; rH++){
            u32 pitch = (targetPixel + rH * target->info.width);
            u32 fontpitch = sourcePixel + rH * font->current.data.info.width;
            for(u32 rW = 0; rW < fontSize; rW++){
                if(font->current.data.data[rW + fontpitch] > 0){
                    u32 bpitch = (rW + pitch)*bytes;
                    for(u8 bi = 0; bi < bytes; bi++){
                        target->data[bpitch + bi] = color.channel[bi];
                    }
                }
            }
        }
        
        letterIndex++;
    }
    return true;
}

int32 calculateAtlasTextCaretPosition(const AtlasFont * font, const char * text, i32 pt, i32 pxPosition){
    if(pxPosition < 0){
        return 0;
    }
    i32 width = 0;
    i32 targetSize = ptToPx(CAST(f32, pt));
    f32 fontScale = (f32)targetSize / font->pixelSize;
    char prevGlyph = 0;
    nint len = strlen(text);
    i32 unscaledTarget = CAST(i32, pxPosition/fontScale);
    i32 caretPosition = 0;
    for(; caretPosition < len; caretPosition++){
        i32 contribution = 0;
        const GlyphData * glyph = &font->glyphs[CAST(u8, text[caretPosition])];
        ASSERT(glyph->valid);
        //kerning?
        if(prevGlyph){
            contribution -= glyph->kerning[prevGlyph];
        }
        contribution += glyph->width;
        i32 nextWidth = width + contribution;
        if(unscaledTarget <= nextWidth && unscaledTarget >= width){
            if(unscaledTarget - width < nextWidth - unscaledTarget){
                return caretPosition;
            }else{
                return caretPosition + 1;
            }
        }
        width = nextWidth;
        
    }
    return caretPosition;
}


int32 calculateAtlasTextWidth(const AtlasFont * font, const char * text, i32 pt, i32 textLen = -1){
    i32 width = 0;
    i32 targetSize = ptToPx(CAST(f32, pt));
    f32 fontScale = (f32)targetSize / font->pixelSize;
    char prevGlyph = 0;
    nint len;
    if(textLen != -1){
        len = textLen;
    }else{
        len = text ? strlen(text) : 0;
    }
    for(int i = 0; i < len; i++){
        u8 c = text[i];
        const GlyphData * glyph = &font->glyphs[c];
        if(!glyph->valid && (c == '\r' || c == '\n')){
            continue;
        }
        ASSERT(glyph->valid);
        //kerning?
        if(prevGlyph){
            width -= glyph->kerning[prevGlyph];
        }
        width += glyph->width;
    }
    return CAST(i32, width*fontScale);
}


#endif
