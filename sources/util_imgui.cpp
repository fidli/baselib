#ifndef UTIL_IMGUI_CPP
#define UTIL_IMGUI_CPP
bool isInit = false;

struct GuiGL{
    struct{
        GLint vertexShader;
        GLint fragmentShader;
        GLint program;
        
        GLint scaleLocation;
        GLint positionLocation;
        GLint overlayColorLocation;
        GLint samplerLocation;
        GLint textureOffsetLocation;
        GLint textureScaleLocation;
        
        GLuint texture;
    } font;
    struct{
        GLint vertexShader;
        GLint fragmentShader;
        GLint program;
        
        GLint scaleLocation;
        GLint positionLocation;
        GLint overlayColorLocation;
    } flat;
    GLuint quad;
};

GuiGL * guiGl;

static inline void initFontShader(){
    guiGl->font.positionLocation = glGetUniformLocation(guiGl->font.program, "position");
    guiGl->font.scaleLocation = glGetUniformLocation(guiGl->font.program, "scale");
    guiGl->font.overlayColorLocation = glGetUniformLocation(guiGl->font.program, "overlayColor");
    guiGl->font.samplerLocation = glGetUniformLocation(guiGl->font.program, "sampler");
    guiGl->font.textureOffsetLocation = glGetUniformLocation(guiGl->font.program, "textureOffset");
    guiGl->font.textureScaleLocation = glGetUniformLocation(guiGl->font.program, "textureScale");
}

static inline void initFlatShader(){
    guiGl->flat.positionLocation = glGetUniformLocation(guiGl->flat.program, "position");
    guiGl->flat.scaleLocation = glGetUniformLocation(guiGl->flat.program, "scale");
    guiGl->flat.overlayColorLocation = glGetUniformLocation(guiGl->flat.program, "overlayColor");
}

#include "util_graphics.cpp"
#include "util_font.cpp"
#include "util_sort.cpp"

struct GuiElementStyle{
    struct {
        int32 t;
        int32 r;
        int32 b;
        int32 l;
    } padding;
    struct {
        int32 t;
        int32 r;
        int32 b;
        int32 l;
    } margin;
    Color fgColor;
    Color bgColor;
    int32 minWidth;
    int32 minHeight;
};

struct GuiStyle{
    struct {
        GuiElementStyle active;
        GuiElementStyle passive;
    } button;
    struct {
        GuiElementStyle active;
        GuiElementStyle passive;
        GuiElementStyle selection;
    } input;
    GuiElementStyle container;
    GuiElementStyle text;
    AtlasFont * font;
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
    int32 bgWidth;
    int32 bgHeight;
    int8 zIndex;
    int32 width;
    int32 height;
    GuiJustify defaultJustify;
    GuiElementStyle elementStyle;
};

struct GuiBool{
    bool set;
    bool value;
};

struct{
    struct{
        int32 x;
        int32 y;
        struct {
            bool leftUp;
            bool leftDown;
            bool leftDoubleClick;
            bool leftTripleClick;
        } buttons;
        bool leftHold;
        float32 lastDoubleClickTime;
    } mouse;
} guiInput;


struct GuiId{
    int32 x;
    int32 y;
    
    int8 z;
};

#define CARET_TICK 0.5f

struct GuiContext{
    AtlasFont font;
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
    int32 caretWidth;
    bool caretPositioning;
    bool caretVisible;
    char * inputText;
    int32 inputMaxlen;
    const char * inputCharlist;
	
	char popups[10][20];
	int32 popupCount;
	bool popupLocked;
    bool dropdownLocked;

    char messagePopups[10][20];
    int32 messagePopupCount;
    char messagePopupTitles[10][50];
    char messagePopupMessages[10][255];
    GuiStyle messagePopupStyles[10];
    
    GuiContainer defaultContainer;
    int32 width;
    int32 height;

    GuiContainer * addedContainers[20];
    int32 addedContainersCount;
    
    struct {
        bool isActive;
        int32 elementIndex;
        int32 inputsRendered;
        bool activate;
    } selection;
    bool mouseInContainer;
    bool escapeClick;

    int32 membraneIndex;
};

GuiContext * guiContext;

static void guiInvalidate(GuiId * a){
    a->x = -1;
    a->y = -1;
}

static bool guiEq(const GuiId A, const GuiId B){
    return A.x == B.x && A.y == B.y && A.z == B.z;
}

bool guiValid(const GuiId a){
    return a.x != -1 && a.y != -1;
}

void guiCancelCaretPositioning(){
    guiContext->caretPositioning = false;
    guiContext->caretWidth = 0;
}

void guiDeselectInput(){
    if(guiValid(guiContext->activeInput) || guiValid(guiContext->activeDropdown)){
        guiContext->escapeClick = true;
    }
    guiContext->selection.elementIndex = -1;
    guiInvalidate(&guiContext->activeInput);
    // NOTE(fidli): dropown gets sorted out at the end of the gui
    guiContext->selection.isActive = false;
    guiContext->selection.activate = false;
    guiCancelCaretPositioning();
}

bool guiInit(const char * fontImagePath, const char * fontDescriptionPath){
    guiInput = {};
    guiContext = &PPUSH(GuiContext);
    memset(CAST(void*, guiContext), 0, sizeof(GuiContext));
    guiGl = &PPUSH(GuiGL);
    memset(CAST(void*, guiGl), 0, sizeof(GuiGL));

    bool r = true;    
    {//font
        LOG(default, startup, "Reading in font");
        r &= initAtlasFont(&guiContext->font, fontImagePath, fontDescriptionPath);
        ASSERT(r);
        LOG(default, startup, "Initing font success: %u", r);
        if(!r){
            return false;
        }
        ASSERT(guiContext->font.data.info.interpretation == BitmapInterpretationType_RGBA);
        glGenTextures(1, &guiGl->font.texture);
        glBindTexture(GL_TEXTURE_2D, guiGl->font.texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, guiContext->font.data.info.width, guiContext->font.data.info.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, guiContext->font.data.data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        
    }
#ifndef RELEASE
    r &= loadAndCompileShaders("..\\sources\\shaders\\font.vert", "..\\sources\\shaders\\font.frag", &guiGl->font.vertexShader, &guiGl->font.fragmentShader, &guiGl->font.program);
    ASSERT(r);
    initFontShader();
    LOG(default, shaders, "Font shaders loaded");
    r &= loadAndCompileShaders("..\\sources\\shaders\\flat.vert", "..\\sources\\shaders\\flat.frag", &guiGl->flat.vertexShader, &guiGl->flat.fragmentShader, &guiGl->flat.program);
    ASSERT(r);
    initFlatShader();
    LOG(default, shaders, "Flat shaders loaded");
#else
    r &= compileShaders(___sources_shaders_font_vert, ___sources_shaders_font_vert_len, ___sources_shaders_font_frag, ___sources_shaders_font_frag_len, &guiGl->font.vertexShader, &guiGl->font.fragmentShader, &guiGl->font.program);
    ASSERT(r);
    initFontShader();
    LOG(default, shaders, "Font shaders loaded");
    r &= compileShaders(___sources_shaders_flat_vert, ___sources_shaders_flat_vert_len, ___sources_shaders_flat_frag, ___sources_shaders_flat_frag_len, &guiGl->flat.vertexShader, &guiGl->flat.fragmentShader, &guiGl->flat.program);
    ASSERT(r);
    initFlatShader();
    LOG(default, shaders, "Flat shaders loaded");
#endif
    guiInvalidate(&guiContext->lastActive);
    guiInvalidate(&guiContext->lastHover);
    guiInvalidate(&guiContext->activeDropdown);
    guiInvalidate(&guiContext->activeInput);
	guiContext->popupCount = 0;
    guiDeselectInput();
    guiContext->selection.activate = false;
    guiContext->mouseInContainer = false;
    guiContext->escapeClick = false;
    guiContext->caretPositioning = false;
    isInit = r;
    
    {
        glGenBuffers(1, &guiGl->quad);
        glBindBuffer(GL_ARRAY_BUFFER, guiGl->quad);
        const float32 box[] = {
            // triangle
            1, 0,// 0.0f, 1.0f,
            0, 0,// 0.0f, 1.0f,
            1, 1,// 0.0f, 1.0f,
            0, 1, //, 0.0f, 1.0f,
            // lines
            0, 0,
            0, 1,
            1, 1,
            1, 0
        };
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_STATIC_DRAW);
    }
    return isInit;
}

bool guiAnyInputSelected(){
    return guiContext->selection.isActive;
}

bool guiAnyInputActive(){
    return guiValid(guiContext->activeInput);
}

void guiActivateSelection(){
    guiContext->selection.activate = true;
}


void guiSelectFirstInput(){
    guiContext->selection.elementIndex = 0;
    guiContext->selection.isActive = true;
    guiContext->selection.activate = false;
    guiInvalidate(&guiContext->activeInput);
    guiCancelCaretPositioning();
}

void guiSelectNextInput(){
    guiContext->selection.elementIndex++;
    guiContext->selection.isActive = true;
    guiContext->selection.activate = false;
    guiInvalidate(&guiContext->activeInput);
    guiCancelCaretPositioning();
}

void guiSelectPreviousInput(){
    guiContext->selection.elementIndex--;
    guiContext->selection.isActive = true;
    guiContext->selection.activate = false;
    guiInvalidate(&guiContext->activeInput);
    guiCancelCaretPositioning();
}


void guiBegin(int32 width, int32 height){
    PROFILE_SCOPE(gui_begin);
    float64 currentTimestamp = getProcessCurrentTime();
    guiContext->activeInputTimeAccumulator += currentTimestamp - guiContext->activeInputLastTimestamp;
    guiContext->activeInputLastTimestamp = currentTimestamp;
    guiInvalidate(&guiContext->currentHover);
    guiContext->minZ = guiContext->maxZ = 0;
	guiContext->popupLocked = true;
    guiContext->dropdownLocked = false;
    memset(CAST(void*, &guiContext->defaultContainer), 0, sizeof(guiContext->defaultContainer));
    guiContext->defaultContainer.width = guiContext->width = width;
    guiContext->defaultContainer.height = guiContext->height = height;
    guiContext->defaultContainer.defaultJustify = GuiJustify_Left;
    guiContext->addedContainersCount = 0;
    guiContext->selection.inputsRendered = 0;
    glBindTexture(GL_TEXTURE_2D, guiGl->font.texture);
    glBindBuffer(GL_ARRAY_BUFFER, guiGl->quad);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
}

GuiContainer * guiAddContainer(GuiContainer * parent, const GuiStyle * style, int32 posX, int32 posY, int32 width = 0, int32 height = 0, const GuiElementStyle * overrideStyle = NULL, GuiJustify defaultJustify = GuiJustify_Default, int32 zIndex = -1234){
    GuiContainer * result = &PUSH(GuiContainer);
    memset(CAST(void*, result), 0, sizeof(GuiContainer));
    if(!overrideStyle){
        result->elementStyle = style->container;
    }else{
        result->elementStyle = *overrideStyle;
    }
    if(!parent){
        result->parent = parent = &guiContext->defaultContainer;
    }
    if(width == 0){
        result->width = parent->width - parent->widthUsed - result->elementStyle.margin.l - result->elementStyle.padding.l - result->elementStyle.margin.r - result->elementStyle.padding.r;
    }else{
        result->width = width;
        result->bgWidth = width;
    }
    if(height == 0){
        result->height = parent->height - parent->heightUsed - result->elementStyle.margin.t - result->elementStyle.padding.t - result->elementStyle.margin.b - result->elementStyle.padding.b;
    }else{
        result->height = height;
        result->bgHeight = height;
    }
    // parent margin padding right, element padding and margin left
    result->startX = posX;
    result->startY = posY;

    result->cursor[GuiJustify_Left].x = result->startX + result->elementStyle.padding.l;
    result->cursor[GuiJustify_Left].y = result->startY + result->elementStyle.padding.t;
            
    result->cursor[GuiJustify_Middle].x = result->startX + result->width/2;
    result->cursor[GuiJustify_Middle].y = result->startY + result->elementStyle.padding.t;

    result->cursor[GuiJustify_Right].x = result->startX + result->width + result->elementStyle.padding.l;
    result->cursor[GuiJustify_Right].y = result->startY + result->elementStyle.padding.t;
    
    for(int32 i = 0; i < ARRAYSIZE(result->cursor); i++){
        result->defaultCursor[i].x = result->cursor[i].x;
        result->defaultCursor[i].y = result->cursor[i].y;
    }
    if(zIndex != -1234){
        result->zIndex = zIndex;
    }else{
        result->zIndex = parent->zIndex+1;
    }
    if(defaultJustify == GuiJustify_Default){
        result->defaultJustify = GuiJustify_Left;
    }else{
        result->defaultJustify = defaultJustify;
    }
    guiContext->addedContainers[guiContext->addedContainersCount++] = result;
    ASSERT(guiContext->addedContainersCount <= ARRAYSIZE(guiContext->addedContainers));
    result->parent = parent;
    return result;
}


bool guiIsActiveInput(){
    return guiValid(guiContext->activeInput);
}


void guiOpenPopup(const char * key){
	ASSERT(guiContext->popupCount < ARRAYSIZE(guiContext->popups));
	strncpy(guiContext->popups[guiContext->popupCount], key, 20);
	guiContext->popupCount++;
    guiDeselectInput();
    guiContext->selection.inputsRendered = 0;
}


bool guiIsPopupOpened(const char * key){
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

bool guiPopupRendering(){
	return guiAnyPopup() && !guiContext->popupLocked;
}

bool guiClick(){
    return guiPopupRendering() || (guiContext->mouseInContainer && (guiInput.mouse.buttons.leftUp || guiInput.mouse.buttons.leftDown)) || guiContext->escapeClick;
}

GuiContainer * guiBeginPopup(const char * key, const GuiStyle * style, int32 width = 0, int32 height = 0){
    int32 i = 0;
	for(; i < guiContext->popupCount; i++){
		if(!strncmp(key, guiContext->popups[i], 20)){
            break;
		}
	}
    ASSERT(i < guiContext->popupCount);
    GuiContainer * result = guiAddContainer(NULL, style, guiContext->width/2 - width/2, guiContext->height/2 - height/2, width, height);
    result->zIndex = 100 + (i+1)*5;
    guiContext->membraneIndex = result->zIndex - 1;
	guiContext->popupLocked = i != guiContext->popupCount - 1;
    return result;
}

void guiEndPopup(){
	guiContext->popupLocked = true;
}

bool guiClosePopup(const char * key = NULL){
	if(guiContext->popupCount > 0){
        if(key != NULL){
            if(!strncmp(key, guiContext->popups[guiContext->popupCount-1], 20)){
                guiContext->popupCount--;
                guiDeselectInput();
                guiContext->selection.inputsRendered = 0;
                if(guiContext->messagePopupCount > 0){
                    ASSERT(!strncmp(key, guiContext->messagePopupTitles[guiContext->messagePopupCount-1], 20));
                    guiContext->messagePopupCount--;
                }
                return true;
            }
        }else{
                guiDeselectInput();
                guiContext->selection.inputsRendered = 0;
                guiContext->popupCount--;
                if(guiContext->messagePopupCount > 0){
                    guiContext->messagePopupCount--;
                }
                return true;
        }
	}
	return false;
}

void guiEndline(GuiContainer * container, GuiStyle * style){
    ASSERT(container);
    ASSERT(style);
    // NOTE(fidli): not essentially correct, but surely enouht
    int32 smallestAddition = 100000000;
    for(int32 i = 0; i < ARRAYSIZE(container->cursor); i++){
        container->cursor[i].x = container->defaultCursor[i].x;
        int32 newValue = container->defaultCursor[i].y + container->heightUsed + style->text.padding.b + style->text.margin.b + style->text.margin.t;
        int32 addition = container->cursor[i].y - newValue;
        smallestAddition = MIN(smallestAddition, addition);
        container->cursor[i].y = newValue;
    }
    if(smallestAddition == 0){
        container->heightUsed += ptToPx(CAST(float32, style->pt)) + style->text.margin.b + style->text.margin.t + style->text.padding.t + style->text.padding.b;
        guiEndline(container, style);
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
        for(int32 i = 0; i < ARRAYSIZE(parent->cursor); i++){
            parent->cursor[i].x = MAX(container->cursor[i].x, parent->cursor[i].x);
        }
        recalculateParentUsage(container->parent);
    }
}

static void calculateAndAdvanceCursor(GuiContainer ** container, const GuiStyle * style, const GuiElementStyle * elementStyle, const char * text, GuiJustify justify, RenderElementInfo * info){
    PROFILE_SCOPE(gui_calcnadvance);
    if(!*container){
        *container = &guiContext->defaultContainer;
    }
    if(justify == GuiJustify_Default){
        justify = (*container)->defaultJustify;
    }
    int32 textWidth = MAX(calculateAtlasTextWidth(style->font, text, style->pt), elementStyle->minWidth);
    int32 textHeight = MAX(CAST(int32, style->font->lineHeight*ptToPx(CAST(float32, style->pt))/style->font->pixelSize), elementStyle->minHeight);
    int32 startX = (*container)->cursor[justify].x;
    int32 startY = (*container)->cursor[justify].y;
    int32 advanceX = textWidth + elementStyle->margin.r + elementStyle->margin.l + elementStyle->padding.l + elementStyle->padding.r;
    switch(justify){
        case GuiJustify_Right:{
            startX -= textWidth - elementStyle->margin.r;
            advanceX = -advanceX;
        }break;
        case GuiJustify_Middle:{
            startX -= textWidth/2;
            advanceX = advanceX/2;
        }break;
        case GuiJustify_Left:{
        }break;
    }
    // advanceCursor
    (*container)->cursor[justify].x += advanceX;
    int32 heightUsed = 0;
    int32 widthUsed = 0;
    for(int32 i = 0; i < ARRAYSIZE((*container)->cursor); i++){
        heightUsed = MAX(heightUsed, (*container)->cursor[i].y - (*container)->defaultCursor[i].y + elementStyle->padding.t + elementStyle->padding.b + elementStyle->margin.t + elementStyle->margin.b + textHeight);
    }
    if((*container)->cursor[GuiJustify_Right].x != (*container)->defaultCursor[GuiJustify_Right].x){
        widthUsed = (*container)->width;
    }else if((*container)->cursor[GuiJustify_Middle].x != (*container)->defaultCursor[GuiJustify_Middle].x){
        widthUsed = (*container)->cursor[GuiJustify_Middle].x - (*container)->startX;
    }else{
        widthUsed = (*container)->cursor[GuiJustify_Left].x - (*container)->startX;
    }
    (*container)->widthUsed = MAX(widthUsed, (*container)->widthUsed);
    (*container)->heightUsed = MAX(heightUsed, (*container)->heightUsed);
    // adjust parent
    recalculateParentUsage(*container);
    // set render element info
    info->startX = startX + elementStyle->margin.l;
    info->startY = startY + elementStyle->margin.r;
    info->renderWidth = textWidth + elementStyle->padding.l + elementStyle->padding.r;
    info->renderHeight = textHeight + elementStyle->padding.t + elementStyle->padding.b;
}

static bool registerInputAndIsSelected(){
    if(guiValid(guiContext->activeDropdown)){
        if(guiContext->dropdownLocked){
            bool res = guiContext->selection.isActive && guiContext->selection.inputsRendered == guiContext->selection.elementIndex;
            guiContext->selection.inputsRendered++;
            return res;
        }
        return false;
    }else if(guiAnyPopup()){
        if(!guiContext->popupLocked){
            bool res = guiContext->selection.isActive && guiContext->selection.inputsRendered == guiContext->selection.elementIndex;
            guiContext->selection.inputsRendered++;
            return res;
        }
        return false;
    }else{
        bool res = guiContext->selection.isActive && guiContext->selection.inputsRendered == guiContext->selection.elementIndex;
        guiContext->selection.inputsRendered++;
        return res;
    }
}

bool renderText(const AtlasFont * font, const char * text, int startX, int startY, int pt, const Color * color, int32 zIndex = 0){
    PROFILE_SCOPE(gui_render_text);
    glUseProgram(guiGl->font.program);
    glBindTexture(GL_TEXTURE_2D, guiGl->font.texture);

    int32 advance = 0;
    float32 resScaleY = 1.0f / (guiContext->height);
    float32 resScaleX = 1.0f / (guiContext->width);
    int32 targetSize = ptToPx(CAST(float32, pt));
    float32 fontScale = (float32)targetSize / font->pixelSize;
    char prevGlyph = 0;
    int32 len = strlen(text);
    for(int i = 0; i < len; i++){
        const GlyphData * glyph = &font->glyphs[CAST(uint8, text[i])];
        if(!glyph->valid){
            continue;
        }
        int32 positionX = startX + CAST(int32, fontScale*(advance + glyph->marginX));
        int32 positionY = startY + CAST(int32, fontScale*glyph->marginY);
        //kerning
        if(prevGlyph){
            positionX += (int32)((float32)glyph->kerning[prevGlyph]*fontScale);
            advance += (int32)((float32)glyph->kerning[prevGlyph]*fontScale);
        }
        
        float32 zOffset = CAST(float32, clamp(CAST(int32, zIndex), -INT8_MAX, INT8_MAX)) / INT8_MAX;
        //position
        glUniform3f(guiGl->font.positionLocation, resScaleX * 2 * positionX - 1, resScaleY * 2 * positionY - 1, zOffset);
        //scale
        glUniform2f(guiGl->font.scaleLocation, fontScale * (glyph->AABB.width) * resScaleX * 2,
                    fontScale * resScaleY * (glyph->AABB.height) * 2);
        
        //texture offset
        glUniform2f(guiGl->font.textureOffsetLocation, (float32)glyph->AABB.x / font->data.info.width,
                    (float32)glyph->AABB.y / font->data.info.height);
        
        //texture scale
        glUniform2f(guiGl->font.textureScaleLocation, ((float32)glyph->AABB.width) / font->data.info.width,
                    ((float32)(glyph->AABB.height)) / font->data.info.height);
        
        //color
        glUniform4f(guiGl->font.overlayColorLocation, color->x/255.0f, color->y/255.0f, color->z/255.0f, color->w/255.0f);
        
        //draw it
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        advance += glyph->width;
        prevGlyph = glyph->glyph;
    }
    return true;
}

bool renderTextXYCentered(const AtlasFont * font, const char * text, int centerX, int centerY, int pt, const Color * color, int32 zOffset = 0){
    return renderText(font, text, centerX - calculateAtlasTextWidth(font, text, pt)/2, centerY - CAST(int32, ((CAST(float32, ptToPx(CAST(float32, pt)))/font->pixelSize) * font->lineHeight)/2), pt, color, zOffset);
}

static bool renderRect(const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * color, float32 zIndex = 0){
    PROFILE_SCOPE(gui_render_rect);
    glUseProgram(guiGl->flat.program);
    
    float32 resScaleY = 1.0f / (guiContext->height);
    float32 resScaleX = 1.0f / (guiContext->width);
    
    //position
    float32 zOffset = clamp(zIndex, CAST(float32, -INT8_MAX), CAST(float32, INT8_MAX)) / CAST(float32, INT8_MAX);
    glUniform3f(guiGl->flat.positionLocation, resScaleX * 2 * positionX - 1, resScaleY * 2 * positionY - 1, zOffset);
    //scale
    glUniform2f(guiGl->flat.scaleLocation, (width) * resScaleX * 2, resScaleY * (height) * 2);
    
    //color
    glUniform4f(guiGl->flat.overlayColorLocation, color->x/255.0f, color->y/255.0f, color->z/255.0f, color->w/255.0f);
    
    //draw it
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    return true;
}

static bool renderWireRect(const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * color, float zIndex = 0){
    glUseProgram(guiGl->flat.program);
    
    float32 resScaleY = 1.0f / (guiContext->height);
    float32 resScaleX = 1.0f / (guiContext->width);
    
    //position
    float32 zOffset = CAST(float32, clamp(CAST(int32, zIndex), -INT8_MAX, INT8_MAX)) / INT8_MAX;
    glUniform3f(guiGl->flat.positionLocation, resScaleX * 2 * positionX - 1, resScaleY * 2 * positionY - 1, zOffset);
    //scale
    glUniform2f(guiGl->flat.scaleLocation, (width-1) * resScaleX * 2, resScaleY * (height-1) * 2);
    
    //color
    glUniform4f(guiGl->flat.overlayColorLocation, color->x/255.0f, color->y/255.0f, color->z/255.0f, color->w/255.0f);
    
    //draw it
    glDrawArrays(GL_LINE_LOOP, 4, 4);
    
    return true;
}

void guiResetCaretVisually(){
    guiContext->activeInputLastTimestamp = getProcessCurrentTime();
    guiContext->activeInputTimeAccumulator = 0;
    guiContext->caretVisible = true;
}

void guiInputCharacters(const char * input, int32 len){
    char digitRangeLow = 0;
    char digitRangeHigh = 0;
    char smallLetterRangeLow = 0;
    char smallLetterRangeHigh = 0;
    char capitalLetterRangeLow = 0;
    char capitalLetterRangeHigh = 0;
    const char* charlist[4] = {};
    uint8 charlistLengths[4] = {};
    uint8 charlistCount = 0;
    bool inverted = false;
    if(guiContext->inputCharlist != NULL){
        //start parse format
        const char * format = guiContext->inputCharlist;
        int32 formatIndex = 0;
        if(format[formatIndex] == '^'){
            inverted = true;
            formatIndex++;
        }
        
        uint8 charlistLen = 0;
        for(;format[formatIndex] != 0; formatIndex++){
            if(format[formatIndex+1] == '-'){
                if(charlistLen > 0){
                    charlistLengths[charlistCount] = charlistLen;
                    charlistLen = 0;
                }
                
                if(format[formatIndex] >= 'A' && format[formatIndex] <= 'Z'){
                    capitalLetterRangeLow = format[formatIndex];                         
                }else if(format[formatIndex] >= 'a' && format[formatIndex] <= 'z'){
                    smallLetterRangeLow = format[formatIndex];                            
                }else if(format[formatIndex] >= '0' && format[formatIndex] <= '9'){
                    digitRangeLow = format[formatIndex];
                }else{
                    INV; //implement me maybe? makes sense? i havent been in these depths for long
                }
                
                formatIndex += 2;
                
                if(format[formatIndex] >= 'A' && format[formatIndex] <= 'Z'){
                    capitalLetterRangeHigh = format[formatIndex];                    
                    ASSERT(capitalLetterRangeHigh > capitalLetterRangeLow);
                }else if(format[formatIndex] >= 'a' && format[formatIndex] <= 'z'){
                    smallLetterRangeHigh = format[formatIndex];
                    ASSERT(capitalLetterRangeHigh > capitalLetterRangeLow);
                }else if(format[formatIndex] >= '0' && format[formatIndex] <= '9'){
                    digitRangeHigh = format[formatIndex];
                    ASSERT(digitRangeHigh > digitRangeLow);
                }else{
                    INV; //implement me maybe? makes sense?
                }
                
            }else{
                
                if(charlistLen == 0){
                    charlistCount++;
                    charlist[charlistCount-1] = &format[formatIndex];
                }
                charlistLengths[charlistCount-1]++;
                charlistLen++;
                
            }
        }
        //end parse format
    }
    ASSERT(guiContext->inputText);
    int32 textlen = strnlen(guiContext->inputText, guiContext->inputMaxlen);
    int32 ci = 0;
    bool isValid = true;
    for(; ci < len && isValid; ci++){
        char c = input[ci];
        if(guiContext->inputCharlist != NULL){
            isValid = false;
            //start validate char
            {
                if(digitRangeLow != '\0'){
                    if(c >= digitRangeLow && c <= digitRangeHigh && !inverted){
                        isValid = true;
                    }else if((c < digitRangeLow && c > digitRangeHigh) && inverted){
                        isValid = true;
                    }
                }
                if(capitalLetterRangeLow != '\0'){
                    if(c >= capitalLetterRangeLow && c <= capitalLetterRangeHigh && !inverted){
                        isValid = true;
                    }else if((c < capitalLetterRangeLow && c > capitalLetterRangeHigh) && inverted){
                        isValid = true;
                    }
                }
                if(smallLetterRangeLow != '\0'){
                    if(c >= smallLetterRangeLow && c <= smallLetterRangeHigh && !inverted){
                        isValid = true;
                    }else if((c < smallLetterRangeLow || c > smallLetterRangeHigh) && inverted){
                        isValid = true;
                    }                        
                }
                
                bool found = false;
                for(int charlistIndex = 0; charlistIndex < charlistCount && !found; charlistIndex++){
                    for(int charIndex = 0; charIndex < charlistLengths[charlistIndex]; charIndex++){
                        if(c == charlist[charlistIndex][charIndex]){
                            isValid = true;
                        }
                    }
                }
                
                if(!inverted && found){
                    isValid = true;
                }else if(inverted && !found){
                    isValid = true;
                }
            }
            //end validate char
        }
        if(isValid){
            if(textlen + 2 < guiContext->inputMaxlen){
                for(int32 i = textlen; i > guiContext->caretPos + ci; i--){
                    guiContext->inputText[i] = guiContext->inputText[i-1];
                } 
                guiContext->inputText[guiContext->caretPos + ci] = c;
                textlen++;
            }
        }
    }
    guiContext->inputText[textlen+1] = '\0';
    guiContext->caretPos += ci - !isValid;
    guiResetCaretVisually();
}

void guiDeleteInputCharacters(int32 from, int32 to){
    int32 start = from;
    int32 end = to;
    if(end < start){
        SWAP(end, start);
    }
    int32 textlen = strnlen(guiContext->inputText, guiContext->inputMaxlen);
    start = clamp(start, 0, textlen);
    end = clamp(end, 0, textlen);
    if(start != end){
        memcpy(guiContext->inputText + start, guiContext->inputText + end, textlen-end);
        memset(guiContext->inputText + textlen - (end-start), 0, end-start); 
    }
    guiContext->caretPos = start;
    guiCancelCaretPositioning();
}

static int32 guiFindNextWordCaret(int32 start){
    int32 textlen = strnlen(guiContext->inputText, guiContext->inputMaxlen);
    int32 caretPos = start;
    bool jumped = false;
    // inner word
    while(caretPos < textlen){
        char c = guiContext->inputText[caretPos];
        if(c != ' ' && c != '\t' && c != '\n' && c != '\r'){
            caretPos++;
            jumped = true;
        }else{
            break;
        }
    }
    // whitespaces
    if(!jumped){
        while(caretPos < textlen){
            char c =  guiContext->inputText[caretPos];
            if(c == ' ' || c == '\t' || c == '\n' || c == '\r'){
                caretPos++;
                jumped = true;
            }else{
                break;
            }
        }
    }
    ASSERT(caretPos <= textlen);
    return caretPos;
}

static int32 guiFindPrevWordCaret(int32 start){
    int32 caretPos = start - 1;
    if(caretPos <= 1){
        caretPos = 0;
    }else{
        bool jumped = false;
        // inner word
        while(caretPos > 0){
            char c = guiContext->inputText[caretPos];
            if(c != ' ' && c != '\t' && c != '\n' && c != '\r'){
                caretPos--;
                jumped = true;
            }else{
                caretPos += jumped;
                break;
            }
        }
        // whitespaces
        if(!jumped){
            while(caretPos > 0){
                char c =  guiContext->inputText[caretPos];
                if(c == ' ' || c == '\t' || c == '\n' || c == '\r'){
                    caretPos--;
                    jumped = true;
                }else{
                    caretPos += jumped;
                    break;
                }
            }
        }
        // boundary
        if(!jumped){
            while(caretPos > 0){
                char c =  guiContext->inputText[caretPos-1];
                if(c == ' ' || c == '\t' || c == '\n' || c == '\r'){
                    caretPos--;
                    jumped = true;
                }else{
                    caretPos += jumped;
                    break;
                }
            }
        }
    }
    ASSERT(caretPos >= 0);
    return caretPos;
}

void guiJumpToNextWord(){
    int32 caretPos = guiFindNextWordCaret(MAX(guiContext->caretPos, guiContext->caretPos + guiContext->caretWidth));
    guiContext->caretPos = caretPos;
    guiContext->caretWidth = 0;
}

void guiJumpToPrevWord(){
    int32 caretPos = guiFindPrevWordCaret(MIN(guiContext->caretPos, guiContext->caretPos + guiContext->caretWidth));
    guiContext->caretPos = caretPos;
    guiContext->caretWidth = 0;
}

void guiAppendNextWordToSelection(){
    int32 oldWidth = guiContext->caretWidth;
    int32 caretPos = guiFindNextWordCaret(guiContext->caretPos + guiContext->caretWidth);
    guiContext->caretWidth = caretPos - guiContext->caretPos;
    if(oldWidth * guiContext->caretWidth < 0){
        guiContext->caretWidth = 0;
    }
}

void guiAppendPrevWordToSelection(){
    int32 oldWidth = guiContext->caretWidth;
    int32 caretPos = guiFindPrevWordCaret(guiContext->caretPos + guiContext->caretWidth);
    guiContext->caretWidth = caretPos - guiContext->caretPos;
    if(oldWidth * guiContext->caretWidth < 0){
        guiContext->caretWidth = 0;
    }
}

void guiSelectWholeInput(){
    guiCancelCaretPositioning();
    nint textlen = strnlen(guiContext->inputText, guiContext->inputMaxlen);
    guiContext->caretPos = 0;
    guiContext->caretWidth = textlen; 
}

static bool renderSlider(float32 * progress, const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * boxBgColor, const Color * fieldColor, int8 zIndex = 0){
    GuiId id = {positionX, positionY, zIndex};
    bool result = false;
    bool isLastActive = guiEq(id, guiContext->lastActive);
    int32 diameter = height/2;
    int32 marginX = diameter/2 + height/10;
    int32 marginY = MIN(10, height/10);
    bool isHoverNow = guiInput.mouse.x >= positionX + marginX && guiInput.mouse.x <= positionX + width - marginX && guiInput.mouse.y >= positionY + marginY && guiInput.mouse.y <= positionY + height - marginY;
    bool isSelected = registerInputAndIsSelected();
    bool wasSubmitted = isSelected && guiContext->selection.activate;
    if(isHoverNow){
        if(!guiValid(guiContext->currentHover) || guiContext->currentHover.z < zIndex){
            guiContext->currentHover = id;
        }
    }
    bool isHoverBeforeAndNow = isHoverNow && guiEq(id, guiContext->lastHover);
    if(guiInput.mouse.leftHold && isLastActive){
        // slider position
        *progress = clamp(CAST(float32, CAST(float32, guiInput.mouse.x - (positionX + marginX)) / CAST(float32, width-2*marginX)), 0.0f, 1.0f);
        result = true;
    }
    if(isLastActive){
        if(guiInput.mouse.buttons.leftUp){
            if(isHoverBeforeAndNow){
                result = true;
                guiContext->activeInput = id;
                // NOTE(fidli): slider position is adjusted on the way
            }
            guiInvalidate(&guiContext->lastActive);
        }
    }else if(isSelected){
        result = true;
        if(!guiEq(guiContext->activeInput, id)){
            guiContext->activeInput = id;
            // NOTE(fidli): slider position is adjusted on the way
        }
    }
    
    if(isHoverBeforeAndNow && !isLastActive){
        if(guiInput.mouse.buttons.leftDown){
            guiContext->lastActive = id;
            guiContext->selection.elementIndex = guiContext->selection.inputsRendered - 1; 
        }
    }
    
    renderRect(positionX, positionY, width, height, boxBgColor, zIndex);
    if(isSelected){
        renderWireRect(positionX, positionY, width, height, fieldColor, zIndex + 0.1f);
    }
    renderRect(positionX+marginX, positionY + height/2, width-2*marginX, 2, fieldColor, zIndex);
    renderRect(CAST(int32, positionX+marginX - diameter/2 + ((width-2*marginX)*(*progress))), CAST(int32, CAST(float32, positionY) + height/2 + 1 - diameter/2), diameter, diameter, fieldColor, zIndex);
    return result;
}

static bool renderInput(const AtlasFont * font, int32 pt, char * text, int32 textMaxlen, const char * charlist, const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * boxBgColor, const Color * fieldColor, const Color * inputTextColor, const Color * selectBgColor, const Color * selectTextColor, int8 zIndex = 0){
    GuiId id = {positionX, positionY, zIndex};
    bool result = false;
    bool isLastActive = guiEq(id, guiContext->lastActive);
    int32 margin = MIN(height/10, 10);
    bool isHoverNow = guiInput.mouse.x >= positionX + margin && guiInput.mouse.x <= positionX + width - margin && guiInput.mouse.y >= positionY + margin && guiInput.mouse.y <= positionY + height - margin;
    bool isSelected = registerInputAndIsSelected();
    bool wasSubmitted = isSelected && guiContext->selection.activate;
    if(isHoverNow){
        if(!guiValid(guiContext->currentHover) || guiContext->currentHover.z < zIndex){
            guiContext->currentHover = id;
        }
    }

    bool isHoverBeforeAndNow = isHoverNow && guiEq(id, guiContext->lastHover);
    int32 textPxWidthHalf = calculateAtlasTextWidth(font, text, pt)/2;
    int32 textPxStartX = positionX + width/2 - textPxWidthHalf;
    if(guiInput.mouse.buttons.leftTripleClick && isHoverNow){
        guiSelectWholeInput();
    }else if(guiInput.mouse.buttons.leftDoubleClick && isHoverNow){
        guiJumpToPrevWord();
        guiAppendNextWordToSelection();
    }else if(guiInput.mouse.buttons.leftDown && isHoverNow){
        guiContext->caretPos = calculateAtlasTextCaretPosition(font, text, pt, guiInput.mouse.x - textPxStartX);
        guiContext->caretPositioning = true;
    }
    if(isLastActive){
        if(guiContext->caretPositioning == true){
            guiContext->caretWidth = calculateAtlasTextCaretPosition(font, text, pt, guiInput.mouse.x - textPxStartX) - guiContext->caretPos;
        }
        if(guiInput.mouse.buttons.leftUp){
            if(isHoverBeforeAndNow){
                result = true;
                guiContext->activeInput = id;
                guiContext->inputText = text;
                guiContext->inputMaxlen = textMaxlen;
                guiContext->inputCharlist = charlist;
                guiContext->caretPositioning = false;
                guiResetCaretVisually();
            }
            guiInvalidate(&guiContext->lastActive);
        }
    }else if(isSelected){
        result = true;
        if(!guiEq(guiContext->activeInput, id)){
            guiContext->activeInput = id;
            guiContext->inputText = text;
            guiContext->inputMaxlen = textMaxlen;
            guiContext->inputCharlist = charlist;
            guiResetCaretVisually();
            guiSelectWholeInput();
        }
    }else{
        if(guiInput.mouse.buttons.leftDown && guiEq(id, guiContext->activeInput)){
            guiInvalidate(&guiContext->activeInput);
        }
    }
    
    if(isHoverBeforeAndNow && !isLastActive){
        if(guiInput.mouse.buttons.leftDown){
            guiContext->lastActive = id;
            guiContext->selection.elementIndex = guiContext->selection.inputsRendered - 1; 
        }
    }
    
    renderRect(positionX, positionY, width, height, boxBgColor, zIndex);
    if(isSelected){
        renderWireRect(positionX, positionY, width, height, inputTextColor, zIndex + 0.1f);
    }
    renderRect(positionX+margin, positionY+margin, width-2*margin, height-2*margin, fieldColor, zIndex);
    const Color * textColor = inputTextColor;
	if(guiEq(guiContext->activeInput, id) || (isLastActive && guiContext->caretPositioning)){
        if(guiContext->caretWidth != 0){
            int32 caretXStart = calculateAtlasTextWidth(font, text, pt, guiContext->caretPos);
            int32 caretXEnd = calculateAtlasTextWidth(font, text, pt, guiContext->caretPos + guiContext->caretWidth);
            if(caretXEnd < caretXStart){
                SWAP(caretXStart, caretXEnd);
            }
            renderRect(textPxStartX + caretXStart, positionY, caretXEnd - caretXStart, height, selectBgColor, zIndex);
            textColor = selectTextColor;
        }
    }
    renderText(font, text, textPxStartX, positionY + height/2 - CAST(int32, ((CAST(float32, ptToPx(CAST(float32, pt)))/font->pixelSize) * font->lineHeight)/2), pt, textColor, zIndex);
    
    //do something at all
	if(guiEq(guiContext->activeInput, id)){
        //is time to flip
        if(guiContext->activeInputTimeAccumulator > CARET_TICK){
            guiContext->activeInputTimeAccumulator -= CARET_TICK;
            guiContext->caretVisible = !guiContext->caretVisible;
        }
        //draw for now?
        if(guiContext->caretWidth == 0 && guiContext->caretVisible){
            int32 carretXOffset = calculateAtlasTextWidth(font, text, pt, guiContext->caretPos);
            int32 textXOffset = (width-2*margin)/2-textPxWidthHalf;
            renderRect(positionX+margin+carretXOffset+textXOffset, positionY + 2*margin, 4, height - 4*margin, inputTextColor, zIndex);
        }        
    }
    
    return result;
}

static bool renderButton(const AtlasFont * font, int32 pt, const char * text, const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * inactiveBgColor, const Color * inactiveTextColor, const Color * activeBgColor, const Color * activeTextColor, int8 zIndex = 0){
    GuiId id = {positionX, positionY, zIndex};
    bool result = false;
    bool isLastActive = guiEq(id, guiContext->lastActive);
    bool isHoverNow = guiInput.mouse.x >= positionX && guiInput.mouse.x <= positionX + width && guiInput.mouse.y >= positionY && guiInput.mouse.y <= positionY + height;
    bool isSelected = registerInputAndIsSelected();
    bool wasSubmitted = isSelected && guiContext->selection.activate;

    if(isHoverNow){
        if(!guiValid(guiContext->currentHover) || guiContext->currentHover.z < zIndex){
            guiContext->currentHover = id;
        }
    }
    
    bool isHoverBeforeAndNow = isHoverNow && guiEq(id, guiContext->lastHover);
    bool isBlockedByPopup = guiAnyPopup() & guiContext->popupLocked;

    if(isLastActive){
        if(guiInput.mouse.buttons.leftUp){
			if(!isBlockedByPopup && isHoverBeforeAndNow){
                result = true;
            }
            guiInvalidate(&guiContext->lastActive);
        }
    }else if(isHoverBeforeAndNow){
        if(guiInput.mouse.buttons.leftDown){
            guiContext->lastActive = id;
            guiContext->selection.elementIndex = guiContext->selection.inputsRendered - 1; 
        }
    }

    if(wasSubmitted){
        result = true;
    }
    
    const Color * textColor;
    const Color * bgColor;
    if(isLastActive && isHoverNow && !isBlockedByPopup){
        textColor = activeTextColor;
        bgColor = activeBgColor;
    }else{
        textColor = inactiveTextColor;
        bgColor = inactiveBgColor;
    }
    
    renderRect(positionX, positionY, width, height, bgColor, zIndex);
    if(isSelected){
        renderWireRect(positionX, positionY, width, height, textColor, zIndex + 0.1f);
    }
    renderTextXYCentered(font, text, positionX + width/2, positionY + height/2, pt, textColor, zIndex);
    
    return result;
}


static bool renderDropdown(const AtlasFont * font, int32 pt, char * searchtext, int32 searchMaxlen, const char * buttonText, const char ** list, int32 listSize, int32 * resultIndex, int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * inactiveBgColor, const Color * inactiveTextColor, const Color * activeBgColor, const Color * activeTextColor, const Color * selectBgColor, const Color * selectTextColor, int8 zIndex = 0){
    bool isHeadLastActive;
    bool isDropdownActive;
    GuiId headId = {positionX, positionY, zIndex};
    bool isBlockedByPopup = guiAnyPopup() & guiContext->popupLocked;
    //start dropdown head
    {
        
        isHeadLastActive = guiEq(headId, guiContext->lastActive);
        isDropdownActive = guiEq(headId, guiContext->activeDropdown);
        bool isHeadHoverNow = guiInput.mouse.x >= positionX && guiInput.mouse.x <= positionX + width && guiInput.mouse.y >= positionY && guiInput.mouse.y <= positionY + height;
        bool isHeadSelected = registerInputAndIsSelected();
        bool wasHeadSubmitted = isHeadSelected && guiContext->selection.activate;

        if(isHeadHoverNow){
            if(!guiValid(guiContext->currentHover) || guiContext->currentHover.z < zIndex){
                guiContext->currentHover = headId;
            }
        }
        bool isHeadHoverBeforeAndNow = isHeadHoverNow && guiEq(headId, guiContext->lastHover);
        
        
        if(isHeadLastActive){
            if(guiInput.mouse.buttons.leftUp){
				if(!isBlockedByPopup && isHeadHoverBeforeAndNow){
                    guiContext->activeDropdown = headId;
                    guiSelectFirstInput();
                    guiContext->caretWidth = 0;
                    guiContext->caretPos = 0;
                    memset(searchtext, 0, searchMaxlen);
                }
                guiInvalidate(&guiContext->lastActive);
            }
        }else{
            if(guiContext->escapeClick && guiEq(headId, guiContext->activeDropdown)){
                guiContext->escapeClick = false;
                // if no click into searchbar
                if(!(guiInput.mouse.x > positionX && guiInput.mouse.x <= positionX + width && guiInput.mouse.y > positionY + height && guiInput.mouse.y <= positionY + height*2)){
                    guiInvalidate(&guiContext->activeDropdown);
                }
            }
        }
        if(isHeadHoverBeforeAndNow && !isHeadLastActive){
            if(guiInput.mouse.buttons.leftDown){
                guiContext->lastActive = headId;
                guiContext->selection.elementIndex = guiContext->selection.inputsRendered - 1; 
            }
        }
        if(wasHeadSubmitted){
            guiContext->activeDropdown = headId;
            guiSelectFirstInput();
            guiContext->caretWidth = 0;
            guiContext->caretPos = 0;
            memset(searchtext, 0, searchMaxlen);
        }
        
        const Color * textColor;
        const Color * bgColor;
        if(isHeadLastActive && isHeadHoverNow && !isBlockedByPopup){
            textColor = activeTextColor;
            bgColor = activeBgColor;
        }else{
            textColor = inactiveTextColor;
            bgColor = inactiveBgColor;
        }
        renderRect(positionX, positionY, width, height, bgColor, zIndex);
        if(isHeadSelected){
            renderWireRect(positionX, positionY, width, height, textColor, zIndex + 0.1f);
        }
        if(buttonText != NULL){
            renderTextXYCentered(font, buttonText, positionX + width/2, positionY + height/2, pt, textColor, zIndex);
        }else if(*resultIndex <= listSize && *resultIndex >= 0){
            renderTextXYCentered(font, list[*resultIndex], positionX + width/2, positionY + height/2, pt, textColor, zIndex);
        }
    }
    //end dropdown head
    bool selected = false;
    //start list members
    if(isDropdownActive){
        guiContext->dropdownLocked = true;
        renderInput(font, pt, searchtext, searchMaxlen, NULL, positionX, positionY + height, width, height, inactiveBgColor, activeBgColor, activeTextColor, selectBgColor, selectTextColor, zIndex+1); 
        int32 rendered = 0;
        for(int32 i = 0; i < listSize; i++){
            if(strstr(list[i], searchtext))
            //start individual buttons
            {
                int32 buttonPositionX = positionX;
                int32 buttonPositionY = positionY + (rendered+2)*height;
                GuiId id = {buttonPositionX, buttonPositionY, zIndex+1};
                bool isLastActive = guiEq(id, guiContext->lastActive);
                bool result = false;
                bool isHoverNow = guiInput.mouse.x >= buttonPositionX && guiInput.mouse.x <= buttonPositionX + width && guiInput.mouse.y >= buttonPositionY && guiInput.mouse.y <= buttonPositionY + height;
                bool isSelected = registerInputAndIsSelected();
                bool wasSubmitted = isSelected && guiContext->selection.activate;
                
                if(isHoverNow){
                    if(!guiValid(guiContext->currentHover)){
                        guiContext->currentHover = id;
                    }
                }
                bool isHoverBeforeAndNow = isHoverNow && guiEq(id, guiContext->lastHover);

                if(isLastActive){
                    if(guiInput.mouse.buttons.leftUp){
						if(!isBlockedByPopup && isHoverBeforeAndNow){
                            result = true;
                        }
                        guiInvalidate(&guiContext->lastActive);
                        guiContext->activeDropdown = headId;
                    }
                }else if(isHoverBeforeAndNow){
                    if(guiInput.mouse.buttons.leftDown){
                        guiContext->lastActive = id;
                        guiContext->activeDropdown = headId;
                        guiContext->selection.elementIndex = guiContext->selection.inputsRendered - 1; 
                    }
                }
                if(wasSubmitted){
                    result = true;
                }
                
                const Color * textColor;
                const Color * bgColor;
                if(isHoverNow && !isBlockedByPopup){
                    textColor = activeTextColor;
                    bgColor = activeBgColor;
                }else{
                    textColor = inactiveTextColor;
                    bgColor = inactiveBgColor;
                }
                renderRect(buttonPositionX, buttonPositionY, width, height, bgColor, CAST(float, zIndex + 1));
                if(isSelected){
                    renderWireRect(buttonPositionX, buttonPositionY, width, height, textColor, zIndex + 1.1f);
                }
                renderTextXYCentered(font, list[i], buttonPositionX + width/2, buttonPositionY + height/2, pt, textColor, zIndex + 1);
                
                if(result){
                    *resultIndex = i;
                    selected = true;
                }
                rendered++;
            }
            //end individual buttons
        }
        if(guiInput.mouse.buttons.leftUp){
            guiContext->activeDropdown = headId;
        }
        guiContext->dropdownLocked = false;
    }
    //end list members
    if(selected){
        guiInvalidate(&guiContext->activeDropdown);
    }
    return selected || isDropdownActive || isHeadLastActive;
}

void guiRenderText(GuiContainer * container, const GuiStyle * style, const char * text, const GuiElementStyle * overrideStyle = NULL, const GuiJustify justify = GuiJustify_Default){
    PROFILE_SCOPE(gui_render_text);
    RenderElementInfo rei;
    const GuiElementStyle * elementStyle = &style->text;
    if(overrideStyle != NULL){
        elementStyle = overrideStyle;
    }
    calculateAndAdvanceCursor(&container, style, elementStyle, text, justify, &rei);
    renderText(style->font, text, rei.startX+elementStyle->padding.l, rei.startY+elementStyle->padding.t, style->pt, &elementStyle->fgColor, container->zIndex);
}

void guiRenderBoxText(GuiContainer * container, const GuiStyle * style, const char * text, const GuiElementStyle * overrideStyle = NULL, const GuiJustify justify = GuiJustify_Default){
    PROFILE_SCOPE(gui_render_box_text);
    RenderElementInfo rei;
    const GuiElementStyle * elementStyle = &style->text;
    if(overrideStyle != NULL){
        elementStyle = overrideStyle;
    }
    calculateAndAdvanceCursor(&container, style, elementStyle, text, justify, &rei);
    renderRect(rei.startX, rei.startY, rei.renderWidth, rei.renderHeight, &elementStyle->bgColor, container->zIndex);
    renderText(style->font, text, rei.startX+elementStyle->padding.l, rei.startY+elementStyle->padding.t, style->pt, &elementStyle->fgColor, container->zIndex);
}

bool guiRenderButton(GuiContainer * container, const GuiStyle * style, const char * text, const GuiElementStyle * overrideStylePassive = NULL, const GuiElementStyle * overrideStyleActive = NULL, const GuiJustify justify = GuiJustify_Default){
    PROFILE_SCOPE(gui_render_button);
    RenderElementInfo rei;
    const GuiElementStyle * elementStyleActive = &style->button.active;
    const GuiElementStyle * elementStylePassive = &style->button.passive;
    if(overrideStyleActive != NULL){
        elementStyleActive = overrideStyleActive;
    }
    if(overrideStylePassive != NULL){
        elementStylePassive = overrideStylePassive;
    }
    calculateAndAdvanceCursor(&container, style, elementStyleActive, text, justify, &rei);
    return renderButton(style->font, style->pt, text, rei.startX, rei.startY, rei.renderWidth, rei.renderHeight, &elementStylePassive->bgColor, &elementStylePassive->fgColor, &elementStyleActive->bgColor, &elementStyleActive->fgColor, container->zIndex);
}

void guiSetCheckboxValue(GuiBool * target, bool newValue){
    target->set = true;
    target->value = newValue;
}

bool guiRenderCheckbox(GuiContainer * container, const GuiStyle * style, GuiBool * checked, const GuiElementStyle * overrideStylePassive = NULL, const GuiElementStyle * overrideStyleActive = NULL, const GuiJustify justify = GuiJustify_Default){
    PROFILE_SCOPE(gui_render_checkbox);
    RenderElementInfo rei;
    const GuiElementStyle * elementStyleActive = &style->button.active;
    const GuiElementStyle * elementStylePassive = &style->button.passive;
    if(overrideStyleActive != NULL){
        elementStyleActive = overrideStyleActive;
    }
    if(overrideStylePassive != NULL){
        elementStylePassive = overrideStylePassive;
    }
    char text[2] = {};
    if(checked->set){
        if(checked->value){
            text[0] = 'Y';
        }else{
            text[0] = 'n';
        }
    }else{
        text[0] = ' ';
    }
    calculateAndAdvanceCursor(&container, style, elementStyleActive, text, justify, &rei);
    bool r = renderButton(style->font, style->pt, text, rei.startX, rei.startY, rei.renderWidth, rei.renderHeight, &elementStylePassive->bgColor, &elementStylePassive->fgColor, &elementStyleActive->bgColor, &elementStyleActive->fgColor, container->zIndex);
    if(r){
        checked->set = true;
        checked->value = !checked->value;
    }
    return r;
}

bool guiRenderInput(GuiContainer * container, const GuiStyle * style, char * text, int32 textMaxlen, const char * dictionary, const GuiElementStyle * overrideStylePassive = NULL, const GuiElementStyle * overrideStyleActive = NULL, const GuiElementStyle * overrideStyleSelection = NULL, const GuiJustify justify = GuiJustify_Default){
    PROFILE_SCOPE(gui_render_input);
    RenderElementInfo rei;
    const GuiElementStyle * elementStyleActive = &style->input.active;
    const GuiElementStyle * elementStylePassive = &style->input.passive;
    const GuiElementStyle * elementStyleSelection = &style->input.selection;
    if(overrideStyleActive != NULL){
        elementStyleActive = overrideStyleActive;
    }
    if(overrideStylePassive != NULL){
        elementStylePassive = overrideStylePassive;
    }
    if(overrideStyleSelection != NULL){
        elementStyleSelection = overrideStyleSelection;
    }
    calculateAndAdvanceCursor(&container, style, elementStyleActive, text, justify, &rei);
    return renderInput(style->font, style->pt, text, textMaxlen, dictionary, rei.startX, rei.startY, rei.renderWidth, rei.renderHeight, &elementStylePassive->bgColor, &elementStyleActive->bgColor, &elementStyleActive->fgColor, &elementStyleSelection->bgColor, &elementStyleSelection->fgColor, container->zIndex);
}

bool guiRenderSlider(GuiContainer * container, const GuiStyle * style, float leftValue, float rightValue, float * currentValue, const GuiElementStyle * overrideStyle = NULL, const GuiJustify justify = GuiJustify_Default){
    PROFILE_SCOPE(gui_render_slider);
    RenderElementInfo rei;
    const GuiElementStyle * elementStyle = &style->input.passive;
    if(overrideStyle != NULL){
        elementStyle = overrideStyle;
    }
    // TODO(fidli): use real width
    calculateAndAdvanceCursor(&container, style, elementStyle, "SLIDER_TEXT", justify, &rei);
    float32 relativeProgress = clamp((*currentValue - leftValue)/(rightValue-leftValue), 0.0f, 1.0f);
    bool result = renderSlider(&relativeProgress, rei.startX, rei.startY, rei.renderWidth, rei.renderHeight, &elementStyle->bgColor, &elementStyle->fgColor, container->zIndex);
    *currentValue = (relativeProgress * (rightValue-leftValue)) + leftValue;
    return result;
}

bool guiRenderDropdown(GuiContainer * container, const GuiStyle * style, char * searchtext, int32 searchMaxlen, const char * buttonText, const char ** fullList, int32 listSize, int32 * resultIndex, const GuiElementStyle * overrideStylePassive = NULL, const GuiElementStyle * overrideStyleActive = NULL, const GuiElementStyle * overrideStyleSelection = NULL, const GuiJustify justify = GuiJustify_Default){
    PROFILE_SCOPE(gui_render_dropdown);
    RenderElementInfo rei;
    const GuiElementStyle * elementStyleActive = &style->input.active;
    const GuiElementStyle * elementStylePassive = &style->input.passive;
    const GuiElementStyle * elementStyleSelection = &style->input.selection;
    if(overrideStyleActive != NULL){
        elementStyleActive = overrideStyleActive;
    }
    if(overrideStylePassive != NULL){
        elementStylePassive = overrideStylePassive;
    }
    if(overrideStyleSelection != NULL){
        elementStyleSelection = overrideStyleSelection;
    }
    calculateAndAdvanceCursor(&container, style, elementStyleActive, searchtext, justify, &rei);
    return renderDropdown(style->font, style->pt, searchtext, searchMaxlen, buttonText, fullList, listSize, resultIndex, rei.startX, rei.startY, rei.renderWidth, rei.renderHeight, &elementStylePassive->bgColor, &elementStylePassive->fgColor, &elementStyleActive->bgColor, &elementStyleActive->fgColor, &elementStyleSelection->bgColor, &elementStyleSelection->fgColor, container->zIndex);
}

void guiOpenPopupMessage(GuiStyle * style, const char * title, const char * message){
    guiOpenPopup(title);
    
	ASSERT(guiContext->messagePopupCount < ARRAYSIZE(guiContext->messagePopups));
    ASSERT(style);
	strncpy(guiContext->messagePopupTitles[guiContext->messagePopupCount], title, 50);
	strncpy(guiContext->messagePopupMessages[guiContext->messagePopupCount], message, 255);
	memcpy(CAST(void *, &guiContext->messagePopupStyles[guiContext->messagePopupCount]), CAST(void *, style), sizeof(GuiStyle));
    guiContext->messagePopupCount++;
}

void guiEnd(){
    PROFILE_SCOPE(gui_end);
    if(guiContext->escapeClick){
        guiInvalidate(&guiContext->activeDropdown);
    }
    // messages
    bool close = false;
    for(int32 i = 0; i < guiContext->messagePopupCount; i++){
        if(guiIsPopupOpened(guiContext->messagePopupTitles[i])){
            GuiStyle * style = &guiContext->messagePopupStyles[i];
            GuiContainer * popupContainer = guiBeginPopup(guiContext->messagePopupTitles[i], style, 500, 200);
                
            guiRenderBoxText(popupContainer, style, guiContext->messagePopupTitles[i], NULL, GuiJustify_Middle);
            guiEndline(popupContainer, style);
            guiRenderBoxText(popupContainer, style, guiContext->messagePopupMessages[i], NULL, GuiJustify_Middle);
            guiEndline(popupContainer, style);
            if(guiRenderButton(popupContainer, style, "Ok :(", NULL, NULL, GuiJustify_Middle)){
                guiClosePopup();
            }
            guiEndPopup();
        }
    }
    guiInput.mouse.buttons = {};
    guiContext->lastHover = guiContext->currentHover;
    guiContext->mouseInContainer = false;
    guiContext->escapeClick = false;
    if(guiContext->popupCount){
        Color black;
        black.full = 0xA0000000;
        GuiElementStyle style;
        style.bgColor = black;
        GuiContainer * membrane = guiAddContainer(NULL, NULL, 0, 0, guiContext->width, guiContext->height, &style);
        membrane->zIndex = guiContext->membraneIndex;
    }
    insertSort(&guiContext->addedContainers[0], guiContext->addedContainersCount, [](GuiContainer * a, GuiContainer * b) -> int32 { return a->zIndex - b->zIndex; });
    for(int32 i = 0; i < guiContext->addedContainersCount; i++){
        GuiContainer * container = guiContext->addedContainers[i];
        int32 w = container->widthUsed;
        int32 h = container->heightUsed;
        if(container->bgWidth){
            w = container->bgWidth;
        }
        if(container->bgHeight){
            h = container->bgHeight;
        }
        guiContext->mouseInContainer |= guiInput.mouse.x >= container->startX && guiInput.mouse.x < container->startX + w && guiInput.mouse.y >= container->startY && guiInput.mouse.y < container->startY + h;
        renderRect(container->startX, container->startY, w, h, &container->elementStyle.bgColor, container->zIndex-0.5f);
    } 
    if(guiContext->selection.isActive){
        if(guiContext->selection.elementIndex >= guiContext->selection.inputsRendered){
            guiContext->selection.elementIndex = 0;
        }else if(guiContext->selection.elementIndex < 0){
            guiContext->selection.elementIndex = guiContext->selection.inputsRendered - 1;
        }
        guiContext->selection.activate = false;
    }
    guiContext->membraneIndex = 0;
}

void guiFinalize(){
    glDeleteShader(guiGl->font.fragmentShader);
    glDeleteShader(guiGl->font.vertexShader);
    glDeleteProgram(guiGl->font.program);
    
    glDeleteShader(guiGl->flat.fragmentShader);
    glDeleteShader(guiGl->flat.vertexShader);
    glDeleteProgram(guiGl->flat.program);
}
#endif

