#ifndef UTIL_IMGUI_CPP
#define UTIL_IMGUI_CPP

#include "util_graphics.cpp"
#include "util_font.cpp"

struct GuiElementStyle{
    int32 padding;
    int32 margin;
};

struct GuiStyle{
    struct {
        Color fgColor;
        Color bgColor;
    } active;
    struct {
        Color fgColor;
        Color bgColor;
    } passive;
    GuiElementStyle button;
    GuiElementStyle container;
    AtlasFont font;
    int32 pt;
};

enum GuiJustify{
    GuiJustify_Left,
    GuiJustify_Middle,
    GuiJustify_Right,
   
    GuiJustify_Default,
 
    GuiJustifyCount
};

struct GuiContainer{
    GuiContainer * parent;
    struct {
        int32 x;
        int32 y;
    } cursor[3];
    struct {
        int32 x;
        int32 y;
    } defaultCursor[3];
    int32 startX;
    int32 startY;
    int32 widthUsed;
    int32 heightUsed;
    int32 width;
    int32 height;
    int8 zIndex;
    GuiJustify defaultJustify;
    GuiElementStyle elementStyle;
};

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
    
    int8 z;
};

#define CARET_TICK 0.5f

struct GuiContext{
    int8 minZ;
    int8 maxZ;
    
    GuiId lastActive;
    GuiId lastHover;
    GuiId currentHover;
    
    GuiId activeDropdown;
    
    GuiId activeInput;
    float32 activeInputLastTimestamp;
    float32 activeInputTimeAccumulator;
    int32 caretPos;
    bool caretVisible;
    char * inputData;
    const char * inputCharlist;
	
	char popups[10][20];
	int32 popupCount;
	bool popupLocked;
    
    GuiContainer defaultContainer;
    int32 width;
    int32 height;
};

GuiContext * guiContext;

static void guiInvaliate(GuiId * a){
    a->x = -1;
    a->y = -1;
}

static bool guiEq(const GuiId A, const GuiId B){
    return A.x == B.x && A.y == B.y && A.z == B.z;
}

bool guiValid(const GuiId a){
    return a.x != -1 && a.y != -1;
}

bool guiInit(){
    guiInput = {};
    guiContext = &PPUSH(GuiContext);
    *guiContext = {};
    guiInvaliate(&guiContext->lastActive);
    guiInvaliate(&guiContext->lastHover);
    guiInvaliate(&guiContext->activeDropdown);
    guiInvaliate(&guiContext->activeInput);
	guiContext->popupCount = 0;
    return true;
}

void guiBegin(int32 width, int32 height){
    float64 currentTimestamp = getProcessCurrentTime();
    guiContext->activeInputTimeAccumulator += currentTimestamp - guiContext->activeInputLastTimestamp;
    guiContext->activeInputLastTimestamp = currentTimestamp;
    guiInvaliate(&guiContext->currentHover);
    guiContext->minZ = guiContext->maxZ = 0;
	guiContext->popupLocked = true;
    guiContext->defaultContainer = {};
    guiContext->defaultContainer.width = guiContext->width = width;
    guiContext->defaultContainer.height = guiContext->height = height;
    guiContext->defaultContainer.defaultJustify = GuiJustify_Left;
}

void guiEnd(){
    guiInput.mouse.buttons = {};
    guiContext->lastHover = guiContext->currentHover;
}

GuiContainer * guiAddContainer(const GuiContainer * parent, const GuiStyle * style, GuiJustify defaultJustify = GuiJustify_Left){
    GuiContainer * result = &PUSH(GuiContainer);
    memset(CAST(void*, result), 0, sizeof(GuiContainer));
    result->elementStyle = style->container;
    if(!parent){
        parent = &guiContext->defaultContainer;
    }
    result->width = parent->width - parent->widthUsed - 2*(result->elementStyle.margin + result->elementStyle.padding);
    result->height = parent->height - parent->heightUsed - 2*(result->elementStyle.margin - result->elementStyle.padding);
    // parent margin padding right, element padding and margin left
    result->startX = parent->startX + parent->widthUsed + result->elementStyle.margin + result->elementStyle.padding + parent->elementStyle.margin + parent->elementStyle.padding;
    result->startY = parent->startY + parent->heightUsed + result->elementStyle.margin + result->elementStyle.padding + parent->elementStyle.margin + parent->elementStyle.padding;    

    result->cursor[GuiJustify_Left].x = result->startX;
    result->cursor[GuiJustify_Left].y = result->startY;
            
    result->cursor[GuiJustify_Middle].x = result->startX + result->width/2;
    result->cursor[GuiJustify_Middle].y = result->startY;

    result->cursor[GuiJustify_Right].x = result->startX + result->width;
    result->cursor[GuiJustify_Right].y = result->startY;
    
    for(int32 i = 0; i < ARRAYSIZE(result->cursor); i++){
        result->defaultCursor[i].x = result->cursor[i].x;
        result->defaultCursor[i].y = result->cursor[i].y;
    }
    result->zIndex = parent->zIndex;
    result->defaultJustify = defaultJustify;
    return result;
}

bool guiIsActiveInput(){
    return guiValid(guiContext->activeInput);
}

bool guiClick(){
    return guiValid(guiContext->currentHover);
}

void guiOpenPopup(const char * key){
	ASSERT(guiContext->popupCount < ARRAYSIZE(guiContext->popups));
	strncpy(guiContext->popups[guiContext->popupCount], key, 20);
	guiContext->popupCount++;
}

bool guiPopup(const char * key){
	for(int32 i = 0; i < guiContext->popupCount; i++){
		if(!strncmp(key, guiContext->popups[i], 20)){
			return true;
		}
	}
	return false;
}

bool guiAnyPopup(){
	return guiContext->popupCount > 0;
}

bool guiPopupBlocking(){
	return guiAnyPopup() && guiContext->popupLocked;
}

GuiContainer * guiBeginPopup(const GuiStyle * style){
    GuiContainer * result = guiAddContainer(NULL, style);
    result->zIndex = 10;
	guiContext->popupLocked = false;
    return result;
}

void guiEndPopup(){
	guiContext->popupLocked = true;
}

bool guiClosePopup(const char * key){
	if(guiContext->popupCount > 0){
		if(!strncmp(key, guiContext->popups[guiContext->popupCount-1], 20)){
			guiContext->popupCount--;
			return true;
		}
	}
	return false;
}

void guiEndline(GuiContainer * container, GuiStyle * style){
    ASSERT(container);
    ASSERT(style);
    for(int32 i = 0; i < ARRAYSIZE(container->cursor); i++){
        container->cursor[i].x = container->defaultCursor[i].x;
        container->cursor[i].y += style->pt;
    }
}

struct RenderElementInfo{
    int32 startX;
    int32 startY;
    int32 renderWidth;
    int32 renderHeight;
};

static void recalculateParentUsage(GuiContainer * container){
    // TODO(fidli): check that this considers padding/margin as well
    if(container->parent){
        GuiContainer * parent = container->parent;
        int32 thisMaxX = container->widthUsed + container->startX;
        int32 thisMaxY = container->heightUsed + container->startY;
        if(parent->startX + parent->widthUsed < thisMaxX){
            parent->widthUsed = thisMaxX - parent->startX;
        }
        if(parent->startY + parent->heightUsed < thisMaxY){
            parent->heightUsed = thisMaxY - parent->startY;
        }
        recalculateParentUsage(container->parent);
    }
}

static void calculateAndAdvanceCursor(GuiContainer ** container, const GuiStyle * style, const char * text, GuiJustify justify, RenderElementInfo * info){
    if(!*container){
        *container = &guiContext->defaultContainer;
    }
    if(justify == GuiJustify_Default){
        justify = (*container)->defaultJustify;
    }
    int32 textWidth = calculateAtlasTextWidth(&style->font, text, style->pt);
    int32 textHeight = style->font.height*style->pt/style->font.size;
    int32 startX = (*container)->cursor[justify].x;
    int32 startY = (*container)->cursor[justify].y;
    int32 advanceX = 0;
    switch(justify){
        case GuiJustify_Right:{
            startX -= textWidth;
            advanceX = -textWidth;
        }break;
        case GuiJustify_Middle:{
            startX -= textWidth/2;
            advanceX = textWidth/2;
        }break;
        case GuiJustify_Left:{
            advanceX = textWidth;
        }break;
    }
    // advanceCursor
    (*container)->cursor[justify].x += advanceX;
    int32 heightUsed = 0;
    int32 widthUsed = 0;
    for(int32 i = 0; i < ARRAYSIZE((*container)->cursor); i++){
        heightUsed = MAX(heightUsed, (*container)->cursor[i].y - (*container)->defaultCursor[i].y + style->pt);
    }
    if((*container)->cursor[GuiJustify_Right].x != (*container)->defaultCursor[GuiJustify_Right].x){
        widthUsed = (*container)->width;
    }else if((*container)->cursor[GuiJustify_Middle].x != (*container)->defaultCursor[GuiJustify_Middle].x){
        widthUsed = (*container)->cursor[GuiJustify_Middle].x - (*container)->startX;
    }else{
        widthUsed = (*container)->cursor[GuiJustify_Left].x - (*container)->startX;
    }
    (*container)->widthUsed = widthUsed;
    (*container)->heightUsed = heightUsed;
    // adjust parent
    recalculateParentUsage(*container);
    // set render element info
    info->startX = startX;
    info->startY = startY;
    info->renderWidth = textWidth;
    info->renderHeight = textHeight;
}

static bool renderText(const AtlasFont * font, const char * text, int startX, int startY, int pt, const Color * color, int32 zIndex = 0){
    glUseProgram(gl->font.program);
    glUniform1i(gl->font.samplerLocation, gl->font.atlasTextureUnit);
    glActiveTexture(GL_TEXTURE0 + gl->font.atlasTextureUnit);
    
    
    
    int32 advance = 0;
    float32 resScaleY = 1.0f / (game->resolution.y);
    float32 resScaleX = 1.0f / (game->resolution.x);
    int32 targetSize = pt;
    float32 fontScale = (float32)targetSize / platform->font.size;
    //fontScale = 1;
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
        
        float32 zOffset = -(float32)zIndex / INT8_MAX;
        //position
        glUniform3f(gl->font.positionLocation, resScaleX * 2 * positionX - 1, resScaleY * 2 * positionY - 1, zOffset);
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

bool renderTextXYCentered(const AtlasFont * font, const char * text, int centerX, int centerY, int pt, const Color * color, int32 zOffset = 0){
    return renderText(font, text, centerX - calculateAtlasTextWidth(font, text, pt)/2, centerY - font->height*pt/(2*font->size), pt, color, zOffset);
}

static bool renderRect(const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * color, int32 zIndex = 0){
    glUseProgram(gl->flat.program);
    
    float32 resScaleY = 1.0f / (game->resolution.y);
    float32 resScaleX = 1.0f / (game->resolution.x);
    
    //position
    float32 zOffset = -(float32)zIndex / INT8_MAX;
    glUniform3f(gl->flat.positionLocation, resScaleX * 2 * positionX - 1, resScaleY * 2 * positionY - 1, zOffset);
    //scale
    glUniform2f(gl->flat.scaleLocation, (width) * resScaleX * 2, resScaleY * (height) * 2);
    
    //color
    glUniform4f(gl->flat.overlayColorLocation, color->x/255.0f, color->y/255.0f, color->z/255.0f, color->w/255.0f);
    
    //draw it
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    return true;
}



static void renderBoxText(const AtlasFont * font, const char * text, const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * bgColor, const Color * textColor, int8 zIndex = 0){
    GuiId id = {positionX, positionY, zIndex};
    bool isHoverNow = guiInput.mouse.x >= positionX && guiInput.mouse.x <= positionX + width && guiInput.mouse.y >= positionY && guiInput.mouse.y <= positionY + height;
    if(isHoverNow){
        if(!guiValid(guiContext->currentHover) || guiContext->currentHover.z < zIndex){
            guiContext->currentHover = id;
        }
    }
    
    renderRect(positionX, positionY, width, height, bgColor, zIndex);
    renderTextXYCentered(font, text, positionX + width/2, positionY + height/2, 14, textColor, zIndex);
}

static bool renderInput(const AtlasFont * font, char * text, const char * charlist, const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * boxBgColor, const Color * fieldColor, const Color * inputTextColor, int8 zIndex = 0){
    GuiId id = {positionX, positionY, zIndex};
    bool result = false;
    bool isLastActive = guiEq(id, guiContext->lastActive);
    int32 margin = MIN(height/10, 10);
    bool isHoverNow = guiInput.mouse.x >= positionX + margin && guiInput.mouse.x <= positionX + width - margin && guiInput.mouse.y >= positionY + margin && guiInput.mouse.y <= positionY + height - margin;
    if(isHoverNow){
        if(!guiValid(guiContext->currentHover) || guiContext->currentHover.z < zIndex){
            guiContext->currentHover = id;
        }
    }
    
    bool isHoverBeforeAndNow = isHoverNow && guiEq(id, guiContext->lastHover);
    
    if(isLastActive){
        if(guiInput.mouse.buttons.leftUp){
            if(isHoverBeforeAndNow){
                result = true;
                guiContext->activeInput = id;
                guiContext->activeInputLastTimestamp = getProcessCurrentTime();
                guiContext->activeInputTimeAccumulator = 0;
                guiContext->caretVisible = true;
                guiContext->caretPos = 0;
                guiContext->inputData = text;
                guiContext->inputCharlist = charlist;
            }
            guiInvaliate(&guiContext->lastActive);
        }
    }else{
        if(guiInput.mouse.buttons.leftDown && guiEq(id, guiContext->activeInput)) guiInvaliate(&guiContext->activeInput);
    }
    
    if(isHoverBeforeAndNow && !isLastActive){
        if(guiInput.mouse.buttons.leftDown) guiContext->lastActive = id;
    }
    
    renderRect(positionX, positionY, width, height, boxBgColor, zIndex);
    renderRect(positionX+margin, positionY+margin, width-2*margin, height-2*margin, fieldColor, zIndex);
    
    
    
    renderTextXYCentered(font, text, positionX + width/2, positionY + height/2, 14, inputTextColor, zIndex);
    
    //do something at all
	if(!guiPopupBlocking() && guiEq(guiContext->activeInput, id)){
        //is time to flip
        if(guiContext->activeInputTimeAccumulator > CARET_TICK){
            guiContext->activeInputTimeAccumulator -= CARET_TICK;
            guiContext->caretVisible = !guiContext->caretVisible;
        }
        int32 texlen = strlen(text);
        if(guiContext->caretPos > texlen) guiContext->caretPos = texlen;
        //draw for now?
        if(guiContext->caretVisible){
            char * tmpbuf = &PUSHA(char, texlen+1); 
            strncpy(tmpbuf, text, guiContext->caretPos);
            tmpbuf[guiContext->caretPos] = 0;
            int32 carretXOffset = calculateAtlasTextWidth(font, tmpbuf, 14);
            int32 textXOffset = (width-2*margin)/2-calculateAtlasTextWidth(font, text, 14)/2;
            POP;
            renderRect(positionX+margin+carretXOffset+textXOffset, positionY + 2*margin, 4, height - 4*margin, inputTextColor, zIndex);
        }
        
    }
    
    return result;
}

static bool renderButton(const AtlasFont * font, const char * text, const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * inactiveBgColor, const Color * inactiveTextColor, const Color * activeBgColor, const Color * activeTextColor, int8 zIndex = 0){
    GuiId id = {positionX, positionY, zIndex};
    bool result = false;
    bool isLastActive = guiEq(id, guiContext->lastActive);
    bool isHoverNow = guiInput.mouse.x >= positionX && guiInput.mouse.x <= positionX + width && guiInput.mouse.y >= positionY && guiInput.mouse.y <= positionY + height;
    
    if(isHoverNow){
        if(!guiValid(guiContext->currentHover) || guiContext->currentHover.z < zIndex){
            guiContext->currentHover = id;
        }
    }
    
    bool isHoverBeforeAndNow = isHoverNow && guiEq(id, guiContext->lastHover);
    
    if(isLastActive){
        if(guiInput.mouse.buttons.leftUp){
			if(!guiPopupBlocking() && isHoverBeforeAndNow) result = true;
            guiInvaliate(&guiContext->lastActive);
        }
    }else if(isHoverBeforeAndNow){
        if(guiInput.mouse.buttons.leftDown) guiContext->lastActive = id;
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
    
    renderRect(positionX, positionY, width, height, bgColor, zIndex);
    renderTextXYCentered(font, text, positionX + width/2, positionY + height/2, 14, textColor, zIndex);
    
    return result;
}


static bool renderDropdown(const AtlasFont * font, const char * text,const char ** list, int32 listSize, int32 * resultIndex, int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * inactiveBgColor, const Color * inactiveTextColor, const Color * activeBgColor, const Color * activeTextColor, int8 zIndex = 0){
    bool isHeadLastActive;
    bool isDropdownActive;
    GuiId headId = {positionX, positionY, zIndex};
    //start dropdown head
    {
        
        isHeadLastActive = guiEq(headId, guiContext->lastActive);
        isDropdownActive = guiEq(headId, guiContext->activeDropdown);
        bool isHeadHoverNow = guiInput.mouse.x >= positionX && guiInput.mouse.x <= positionX + width && guiInput.mouse.y >= positionY && guiInput.mouse.y <= positionY + height;
        
        if(isHeadHoverNow){
            if(!guiValid(guiContext->currentHover) || guiContext->currentHover.z < zIndex){
                guiContext->currentHover = headId;
            }
        }
        bool isHeadHoverBeforeAndNow = isHeadHoverNow && guiEq(headId, guiContext->lastHover);
        
        
        if(isHeadLastActive){
            if(guiInput.mouse.buttons.leftUp){
				if(!guiPopupBlocking() && isHeadHoverBeforeAndNow){
                    guiContext->activeDropdown = headId;
                }
                guiInvaliate(&guiContext->lastActive);
            }
        }else{
            if(guiInput.mouse.buttons.leftDown && guiEq(headId, guiContext->activeDropdown)) guiInvaliate(&guiContext->activeDropdown);
        }
        if(isHeadHoverBeforeAndNow && !isHeadLastActive){
            if(guiInput.mouse.buttons.leftDown) guiContext->lastActive = headId;
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
                GuiId id = {buttonPositionX, buttonPositionY, zIndex+1};
                bool isLastActive = guiEq(id, guiContext->lastActive);
                bool result = false;
                bool isHoverNow = guiInput.mouse.x >= buttonPositionX && guiInput.mouse.x <= buttonPositionX + width && guiInput.mouse.y >= buttonPositionY && guiInput.mouse.y <= buttonPositionY + height;
                
                if(isHoverNow){
                    if(!guiValid(guiContext->currentHover) || guiContext->currentHover.z < zIndex){
                        guiContext->currentHover = id;
                    }
                }
                bool isHoverBeforeAndNow = isHoverNow && guiEq(id, guiContext->lastHover);
                
                if(isLastActive){
                    if(guiInput.mouse.buttons.leftUp){
						if(!guiPopupBlocking() && isHoverBeforeAndNow) result = true;
                        guiInvaliate(&guiContext->lastActive);
                        guiContext->activeDropdown = headId;
                    }
                }else if(isHoverBeforeAndNow){
                    if(guiInput.mouse.buttons.leftDown){
                        guiContext->lastActive = id;
                        guiContext->activeDropdown = headId;
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
                renderRect(buttonPositionX, buttonPositionY, width, height, bgColor, zIndex + 1);
                renderTextXYCentered(font, list[i], buttonPositionX + width/2, buttonPositionY + height/2, 14, textColor, zIndex + 1);
                
                if(result){
                    *resultIndex = i;
                    selected = true;
                }
            }
            //end individual buttons
        }
        if(guiInput.mouse.buttons.leftUp){
            guiContext->activeDropdown = headId;
        }
    }
    //end list members
    if(selected){
        guiInvaliate(&guiContext->activeDropdown);
    }
    return selected || isDropdownActive || isHeadLastActive;
}

void guiRenderText(GuiContainer * container, const GuiStyle * style, const char * text, const GuiJustify justify = GuiJustify_Default){
    RenderElementInfo rei;
    calculateAndAdvanceCursor(&container, style, text, justify, &rei);
    renderText(&style->font, text, rei.startX, rei.startY, style->pt, &style->passive.fgColor, container->zIndex);
}

void guiRenderBoxText(GuiContainer * container, const GuiStyle * style, const char * text, const GuiJustify justify = GuiJustify_Default){
    RenderElementInfo rei;
    calculateAndAdvanceCursor(&container, style, text, justify, &rei);
    return renderBoxText(&style->font, text, rei.startX, rei.startY, rei.renderWidth, style->pt, &style->passive.bgColor, &style->passive.fgColor, container->zIndex);
}

bool guiRenderButton(GuiContainer * container, const GuiStyle * style, const char * text, const GuiJustify justify = GuiJustify_Default){
    RenderElementInfo rei;
    calculateAndAdvanceCursor(&container, style, text, justify, &rei);
    return renderButton(&style->font, text, rei.startX, rei.startY, rei.renderWidth, style->pt, &style->passive.bgColor, &style->passive.fgColor, &style->active.bgColor, &style->active.fgColor, container->zIndex);
}

bool guiRenderInput(GuiContainer * container, const GuiStyle * style, char * data, const char * dictionary, const GuiJustify justify = GuiJustify_Default){
    RenderElementInfo rei;
    calculateAndAdvanceCursor(&container, style, data, justify, &rei);
    return renderInput(&style->font, data, dictionary, rei.startX, rei.startY, rei.renderWidth, style->pt, &style->passive.bgColor, &style->active.bgColor, &style->active.fgColor, container->zIndex);
}

#endif


