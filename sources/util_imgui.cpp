#ifndef UTIL_IMGUI_CPP
#define UTIL_IMGUI_CPP

#include "util_graphics.cpp"
#include "util_font.cpp"

struct{
    struct{
        int32 x;
        int32 y;
        struct {
            bool leftUp;
            bool leftDown;
        } buttons;
    } mouse;
} guiInput;


struct GuiId{
    int32 x;
    int32 y;
};

#define CARET_TICK 0.5f

struct {
    GuiId lastActive;
    GuiId currentHover;
    
    GuiId activeDropdown;
    
    GuiId activeInput;
    float32 activeInputLastTimestamp;
    float32 activeInputTimeAccumulator;
    int32 caretPos;
    bool caretVisible;
    char * inputData;
    const char * inputCharlist;
    
} guiContext;

static void guiInvaliate(GuiId * a){
    a->x = -1;
    a->y = -1;
}

static bool guiEq(const GuiId A, const GuiId B){
    return A.x == B.x && A.y == B.y;
}

bool guiValid(const GuiId a){
    return a.x != -1 && a.y != -1;
}

bool guiInit(){
    guiInput = {};
    return true;
}

void guiBegin(){
    float64 currentTimestamp = getProcessCurrentTime();
    guiContext.activeInputTimeAccumulator += currentTimestamp - guiContext.activeInputLastTimestamp;
    guiContext.activeInputLastTimestamp = currentTimestamp;
    guiInvaliate(&guiContext.currentHover);
}

void guiEnd(){
    guiInput.mouse.buttons = {};
}

bool guiIsActiveInput(){
    return guiValid(guiContext.activeInput);
}

bool guiClick(){
    return guiValid(guiContext.currentHover);
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


void renderBoxText(const AtlasFont * font, const char * text, const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * bgColor, const Color * textColor){
    GuiId id = {positionX, positionY};
    bool isHoverNow = guiInput.mouse.x >= positionX && guiInput.mouse.x <= positionX + width && guiInput.mouse.y >= positionY && guiInput.mouse.y <= positionY + height;
    if(isHoverNow){
        guiContext.currentHover = id;
    }
    
    renderRect(positionX, positionY, width, height, bgColor);
    renderTextXYCentered(font, text, positionX + width/2, positionY + height/2, 14, textColor);
}

bool renderInput(const AtlasFont * font, char * text, const char * charlist, const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * boxBgColor, const Color * fieldColor, const Color * inputTextColor){
    GuiId id = {positionX, positionY};
    bool result = false;
    bool isLastActive = guiEq(id, guiContext.lastActive);
    int32 margin = MIN(height/10, 10);
    bool isHoverNow = guiInput.mouse.x >= positionX + margin && guiInput.mouse.x <= positionX + width - margin && guiInput.mouse.y >= positionY + margin && guiInput.mouse.y <= positionY + height - margin;
    if(isHoverNow){
        guiContext.currentHover = id;
    }
    
    
    if(isLastActive){
        if(guiInput.mouse.buttons.leftUp){
            if(isHoverNow){
                result = true;
                guiContext.activeInput = id;
                guiContext.activeInputLastTimestamp = getProcessCurrentTime();
                guiContext.activeInputTimeAccumulator = 0;
                guiContext.caretVisible = true;
                guiContext.caretPos = 0;
                guiContext.inputData = text;
                guiContext.inputCharlist = charlist;
            }
            guiInvaliate(&guiContext.lastActive);
        }
    }else{
        if(guiInput.mouse.buttons.leftDown && guiEq(id, guiContext.activeInput)) guiInvaliate(&guiContext.activeInput);
    }
    
    if(isHoverNow && !isLastActive){
        if(guiInput.mouse.buttons.leftDown) guiContext.lastActive = id;
    }
    
    renderRect(positionX, positionY, width, height, boxBgColor);
    renderRect(positionX+margin, positionY+margin, width-2*margin, height-2*margin, fieldColor);
    
    
    
    renderTextXYCentered(font, text, positionX + width/2, positionY + height/2, 14, inputTextColor);
    
    //do something at all
    if(guiEq(guiContext.activeInput, id)){
        //is time to flip
        if(guiContext.activeInputTimeAccumulator > CARET_TICK){
            guiContext.activeInputTimeAccumulator -= CARET_TICK;
            guiContext.caretVisible = !guiContext.caretVisible;
        }
        int32 texlen = strlen(text);
        if(guiContext.caretPos > texlen) guiContext.caretPos = texlen;
        //draw for now?
        if(guiContext.caretVisible){
            char * tmpbuf = &PUSHA(char, texlen+1); 
            strncpy(tmpbuf, text, guiContext.caretPos);
            tmpbuf[guiContext.caretPos] = 0;
            int32 carretXOffset = calculateAtlasTextWidth(font, tmpbuf, 14);
            int32 textXOffset = (width-2*margin)/2-calculateAtlasTextWidth(font, text, 14)/2;
            POP;
            renderRect(positionX+margin+carretXOffset+textXOffset, positionY + 2*margin, 4, height - 4*margin, inputTextColor);
        }
        
    }
    
    return result;
}

bool renderButton(const AtlasFont * font, const char * text, const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * inactiveBgColor, const Color * inactiveTextColor, const Color * activeBgColor, const Color * activeTextColor){
    GuiId id = {positionX, positionY};
    bool result = false;
    bool isLastActive = guiEq(id, guiContext.lastActive);
    bool isHoverNow = guiInput.mouse.x >= positionX && guiInput.mouse.x <= positionX + width && guiInput.mouse.y >= positionY && guiInput.mouse.y <= positionY + height;
    
    if(isHoverNow){
        guiContext.currentHover = id;
    }
    
    if(isLastActive){
        if(guiInput.mouse.buttons.leftUp){
            if(isHoverNow) result = true;
            guiInvaliate(&guiContext.lastActive);
        }
    }else if(isHoverNow){
        if(guiInput.mouse.buttons.leftDown) guiContext.lastActive = id;
    }
    
    const Color * textColor;
    const Color * bgColor;
    if(isLastActive && isHoverNow){
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
        
        isHeadLastActive = guiEq(headId, guiContext.lastActive);
        isDropdownActive = guiEq(headId, guiContext.activeDropdown);
        bool isHeadHoverNow = guiInput.mouse.x >= positionX && guiInput.mouse.x <= positionX + width && guiInput.mouse.y >= positionY && guiInput.mouse.y <= positionY + height;
        
        if(isHeadHoverNow){
            guiContext.currentHover = headId;
        }
        
        if(isHeadLastActive){
            if(guiInput.mouse.buttons.leftUp){
                if(isHeadHoverNow){
                    guiContext.activeDropdown = headId;
                }
                guiInvaliate(&guiContext.lastActive);
            }
        }else{
            if(guiInput.mouse.buttons.leftDown && guiEq(headId, guiContext.activeDropdown)) guiInvaliate(&guiContext.activeDropdown);
        }
        if(isHeadHoverNow && !isHeadLastActive){
            if(guiInput.mouse.buttons.leftDown) guiContext.lastActive = headId;
        }
        
        
        const Color * textColor;
        const Color * bgColor;
        if(isHeadLastActive){
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
    bool selected = false;
    //start list members
    if(isDropdownActive){
        for(int32 i = 0; i < listSize; i++){
            
            //start individual buttons
            {
                int32 buttonPositionX = positionX;
                int32 buttonPositionY = positionY + (i+1)*height;
                GuiId id = {buttonPositionX, buttonPositionY};
                bool isLastActive = guiEq(id, guiContext.lastActive);
                bool result = false;
                bool isHoverNow = guiInput.mouse.x >= buttonPositionX && guiInput.mouse.x <= buttonPositionX + width && guiInput.mouse.y >= buttonPositionY && guiInput.mouse.y <= buttonPositionY + height;
                
                if(isHoverNow){
                    guiContext.currentHover = id;
                }
                
                if(isLastActive){
                    if(guiInput.mouse.buttons.leftUp){
                        if(isHoverNow) result = true;
                        guiInvaliate(&guiContext.lastActive);
                        guiContext.activeDropdown = headId;
                    }
                }else if(isHoverNow){
                    if(guiInput.mouse.buttons.leftDown){
                        guiContext.lastActive = id;
                        guiContext.activeDropdown = headId;
                    }
                }
                
                const Color * textColor;
                const Color * bgColor;
                if(isHoverNow){
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
                    selected = true;
                }
            }
            //end individual buttons
        }
        if(guiInput.mouse.buttons.leftUp){
            guiContext.activeDropdown = headId;
        }
    }
    //end list members
    if(selected){
        guiInvaliate(&guiContext.activeDropdown);
    }
    return selected || isDropdownActive || isHeadLastActive;
}

#endif


