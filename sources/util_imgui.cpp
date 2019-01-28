#ifndef UTIL_IMGUI_CPP
#define UTIL_IMGUI_CPP

#include "util_graphics.cpp"
#include "util_font.cpp"

struct GuiId{
    int32 x;
    int32 y;
};

struct {
    GuiId lastHover;
    GuiId lastActive;
    GuiId activeDropdown;
} context;

struct{
    int32 x;
    int32 y;
    struct {
        bool leftUp;
        bool leftDown;
    } buttons;
} mouse;

static void guiInvaliate(GuiId * a){
    a->x = -1;
    a->y = -1;
}

static bool guiEq(const GuiId A, const GuiId B){
    return A.x == B.x && A.y == B.y;
}

bool renderText(const AtlasFont * font, const char * text, int startX, int startY, int pt, const Color * color){
    glUseProgram(gl->font.program);
    glUniform1i(gl->font.samplerLocation, gl->font.atlasTextureUnit);
    glActiveTexture(GL_TEXTURE0 + gl->font.atlasTextureUnit);
    
    
    
    int32 advance = 0;
    float32 resScaleY = 1.0f / (game->resolution.y);
    float32 resScaleX = 1.0f / (game->resolution.x);
    int32 targetSize = pt;
    float32 fontScale = (float32)targetSize / platform->font.size;
    char prevGlyph = 0;
    for(int i = 0; i < strlen(text); i++){
        GlyphData * glyph = &platform->font.glyphs[text[i]];
        ASSERT(glyph->valid);
        
        
        int32 positionX = startX + advance + (int32)((float32)glyph->marginX*fontScale);
        int32 positionY = startY + (int32)((float32)glyph->marginY*fontScale);
        //kerning
        if(prevGlyph){
            positionX += (int32)((float32)glyph->kerning[prevGlyph]*fontScale);
            advance += (int32)((float32)glyph->kerning[prevGlyph]*fontScale);
        }
        
        //position
        glUniform2f(gl->font.positionLocation, resScaleX * 2 * positionX - 1, resScaleY * 2 * positionY - 1);
        //scale
        glUniform2f(gl->font.scaleLocation, fontScale * (glyph->AABB.width) * resScaleX * 2,
                    fontScale * resScaleY * (glyph->AABB.height) * 2);
        
        //texture offset
        glUniform2f(gl->font.textureOffsetLocation, (float32)glyph->AABB.x / platform->font.data.info.width,
                    (float32)glyph->AABB.y / platform->font.data.info.height);
        
        //texture scale
        glUniform2f(gl->font.textureScaleLocation, ((float32)glyph->AABB.width) / platform->font.data.info.width,
                    ((float32)(glyph->AABB.height)) / platform->font.data.info.height);
        
        //color
        glUniform4f(gl->font.overlayColorLocation, color->x/255.0f, color->y/255.0f, color->z/255.0f, color->w/255.0f);
        
        //draw it
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        advance += (int32)((float32)glyph->width * fontScale);
        prevGlyph = glyph->glyph;
    }
    return true;
}

bool renderRect(const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * color){
    glUseProgram(gl->flat.program);
    
    float32 resScaleY = 1.0f / (game->resolution.y);
    float32 resScaleX = 1.0f / (game->resolution.x);
    
    //position
    glUniform2f(gl->flat.positionLocation, resScaleX * 2 * positionX - 1, resScaleY * 2 * positionY - 1);
    //scale
    glUniform2f(gl->flat.scaleLocation, (width) * resScaleX * 2, resScaleY * (height) * 2);
    
    //color
    glUniform4f(gl->font.overlayColorLocation, color->x/255.0f, color->y/255.0f, color->z/255.0f, color->w/255.0f);
    
    //draw it
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    return true;
}

bool renderTextXCentered(const AtlasFont * font, const char * text, int centerX, int startY, int pt, const Color * color){
    return renderText(font, text, centerX - calculateAtlasTextWidth(font, text, pt)/2, startY, pt, color);
}

bool renderTextXYCentered(const AtlasFont * font, const char * text, int centerX, int centerY, int pt, const Color * color){
    return renderText(font, text, centerX - calculateAtlasTextWidth(font, text, pt)/2, centerY - font->height*pt/(2*font->size), pt, color);
}


bool renderButton(const AtlasFont * font, const char * text, const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * inactiveBgColor, const Color * inactiveTextColor, const Color * activeBgColor, const Color * activeTextColor){
    GuiId id = {positionX, positionY};
    bool result = false;
    bool isLastActive = guiEq(id, context.lastActive);
    bool isLastHover = guiEq(id, context.lastHover);
    bool isVisuallyHover;
    if(isLastActive){
        if(mouse.buttons.leftUp){
            if(isLastHover) result = true;
            guiInvaliate(&context.lastActive);
        }
    }else if(isLastHover){
        if(mouse.buttons.leftDown) context.lastActive = id;
    }
    
    if(mouse.x >= positionX && mouse.x <= positionX + width && mouse.y >= positionY && mouse.y <= positionY + height){
        context.lastHover = id;
        isVisuallyHover = true;
    }else{
        isVisuallyHover = false;
    }
    
    const Color * textColor;
    const Color * bgColor;
    if(isLastActive && isVisuallyHover){
        textColor = activeTextColor;
        bgColor = activeBgColor;
    }else{
        textColor = inactiveTextColor;
        bgColor = inactiveBgColor;
    }
    renderRect(positionX, positionY, width, height, bgColor);
    renderTextXYCentered(font, text, positionX + width/2, positionY + height/2, 14, textColor);
    
    return result;
}


bool renderDropdown(const AtlasFont * font, const char * text,const char ** list, int32 listSize, int32 * resultIndex, int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * inactiveBgColor, const Color * inactiveTextColor, const Color * activeBgColor, const Color * activeTextColor){
    bool isHeadLastActive;
    bool isDropdownActive;
    GuiId headId = {positionX, positionY};
    //start dropdown head
    {
        isHeadLastActive = guiEq(headId, context.lastActive);
        isDropdownActive = guiEq(headId, context.activeDropdown);
        bool isLastHover = guiEq(headId, context.lastHover);
        if(isHeadLastActive || isDropdownActive){
            if(mouse.buttons.leftUp){
                if(isLastHover){
                    context.activeDropdown = headId;
                }
                guiInvaliate(&context.lastActive);
            }
            if(mouse.buttons.leftDown){
                //NOTE(AK): this will be set 
                guiInvaliate(&context.activeDropdown);
            }
        }
        if(isLastHover && !isHeadLastActive){
            if(mouse.buttons.leftDown) context.lastActive = headId;
        }
        if(!isHeadLastActive && !isDropdownActive && !isLastOver && guiEq(context.activeDropdown, headId)){
            guiInvaliate(&context.activeDropdown);
        }
        
        if(mouse.x >= positionX && mouse.x <= positionX + width && mouse.y >= positionY && mouse.y <= positionY + height){
            context.over = headId;
        }else if(isOver){
            guiInvaliate(&context.active);
        }
        
        const Color * textColor;
        const Color * bgColor;
        if(isHeadActive || isDropdownActive){
            textColor = activeTextColor;
            bgColor = activeBgColor;
        }else{
            textColor = inactiveTextColor;
            bgColor = inactiveBgColor;
        }
        renderRect(positionX, positionY, width, height, bgColor);
        renderTextXYCentered(font, text, positionX + width/2, positionY + height/2, 14, textColor);
    }
    //end dropdown head
    
    //start list members
    *resultIndex = -1;
    if(isDropdownActive){
        for(int32 i = 0; i < listSize; i++){
            
            //start individual buttons
            {
                int32 buttonPositionX = positionX;
                int32 buttonPositionY = positionY + (i+1)*height;
                GuiId id = {buttonPositionX, buttonPositionY};
                bool isActive = guiEq(id, context.active);
                bool isOver = guiEq(id, context.over);
                bool result = false;
                if(isActive){
                    if(mouse.buttons.leftUp){
                        if(isOver) result = true;
                        guiInvaliate(&context.active);
                    }
                }else if(isOver){
                    if(mouse.buttons.leftDown){
                        context.active = id;
                        context.activeDropdown = headId;
                    }
                }
                
                if(mouse.x >= buttonPositionX && mouse.x <= buttonPositionX + width && mouse.y >= buttonPositionY && mouse.y <= buttonPositionY + height){
                    context.over = id;
                }else if(isOver){
                    guiInvaliate(&context.active);
                }
                
                const Color * textColor;
                const Color * bgColor;
                if(isActive){
                    textColor = activeTextColor;
                    bgColor = activeBgColor;
                }else{
                    textColor = inactiveTextColor;
                    bgColor = inactiveBgColor;
                }
                renderRect(buttonPositionX, buttonPositionY, width, height, bgColor);
                renderTextXYCentered(font, list[i], buttonPositionX + width/2, buttonPositionY + height/2, 14, textColor);
                
                if(result){
                    *resultIndex = i;
                }
            }
            //end individual buttons
        }
        if(mouse.buttons.leftUp){
            context.activeDropdown = headId;
        }
    }
    //end list members
    
    
    return *resultIndex != -1 || isDropdownActive || isHeadActive;
}

#endif


