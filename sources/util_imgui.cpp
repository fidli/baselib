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
    bool caretVisible;
    char * inputData;
    const char * inputCharlist;
	
	char popups[10][20];
	int32 popupCount;
	bool popupLocked;

    char messagePopups[10][20];
    int32 messagePopupCount;
    char messagePopupTitles[10][50];
    char messagePopupMessages[10][255];
    GuiStyle * messagePopupStyles[10];
    
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

void guiDeselectInput(){
    if(guiValid(guiContext->activeInput) || guiValid(guiContext->activeDropdown)){
        guiContext->escapeClick = true;
    }
    guiContext->selection.elementIndex = -1;
    guiInvalidate(&guiContext->activeInput);
    guiInvalidate(&guiContext->activeDropdown);
    guiContext->selection.isActive = false;
    guiContext->selection.activate = false;
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
    r &= loadAndCompileShaders(___sources_shaders_font_vert, ___sources_shaders_font_vert_len, ___sources_shaders_font_frag, ___sources_shaders_font_frag_len, &guiGl->font.vertexShader, &guiGl->font.fragmentShader, &guiGl->font.program);
    ASSERT(r);
    initFontShader();
    LOG(default, shaders, "Font shaders loaded");
    r &= loadAndCompileShaders(___sources_shaders_flat_vert, ___sources_shaders_flat_vert_len, ___sources_shaders_flat_frag, ___sources_shaders_flat_frag_len, &guiGl->flat.vertexShader, &guiGl->flat.fragmentShader, &guiGl->flat.program);
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
    isInit = r;
    
    {
        glGenBuffers(1, &guiGl->quad);
        glBindBuffer(GL_ARRAY_BUFFER, guiGl->quad);
        const float32 box[] = {
            // triangle
            1, 0,// 0.0f, 1.0f,
            0, 0,// 0.0f, 1.0f,
            1, 1,// 0.0f, 1.0f,
            0, 1 //, 0.0f, 1.0f,
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

void guiSelectNextInput(){
    if(guiContext->selection.isActive){
        guiContext->selection.elementIndex++;
    }else{
        guiContext->selection.elementIndex = 0;
    }
    guiContext->selection.isActive = true;
    guiContext->selection.activate = false;
    guiInvalidate(&guiContext->activeInput);
    guiInvalidate(&guiContext->activeDropdown);
}

void guiSelectPreviousInput(){
    if(guiContext->selection.isActive){
        guiContext->selection.elementIndex--;
    }else{
        guiContext->selection.elementIndex = -1;
    }
    guiContext->selection.isActive = true;
    guiContext->selection.activate = false;
    guiInvalidate(&guiContext->activeInput);
    guiInvalidate(&guiContext->activeDropdown);
}


void guiBegin(int32 width, int32 height){
    float64 currentTimestamp = getProcessCurrentTime();
    guiContext->activeInputTimeAccumulator += currentTimestamp - guiContext->activeInputLastTimestamp;
    guiContext->activeInputLastTimestamp = currentTimestamp;
    guiInvalidate(&guiContext->currentHover);
    guiContext->minZ = guiContext->maxZ = 0;
	guiContext->popupLocked = true;
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

GuiContainer * guiAddContainer(GuiContainer * parent, const GuiStyle * style, int32 posX, int32 posY, int32 width = 0, int32 height = 0, const GuiElementStyle * overrideStyle = NULL, GuiJustify defaultJustify = GuiJustify_Default){
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
    result->zIndex = parent->zIndex+1;
    if(defaultJustify == GuiJustify_Default){
        result->defaultJustify = GuiJustify_Left;
    }else{
        result->defaultJustify = defaultJustify;
    }
    guiContext->addedContainers[guiContext->addedContainersCount++] = result;
    ASSERT(guiContext->addedContainersCount <= ARRAYSIZE(guiContext->addedContainers));
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
    return guiValid(guiContext->currentHover) || guiPopupRendering() || (guiContext->mouseInContainer && (guiInput.mouse.buttons.leftUp || guiInput.mouse.buttons.leftDown)) || guiContext->escapeClick;
}

GuiContainer * guiBeginPopup(const char * key, const GuiStyle * style, int32 width, int32 height){
    int32 i = 0;
	for(; i < guiContext->popupCount; i++){
		if(!strncmp(key, guiContext->popups[i], 20)){
            break;
		}
	}
    ASSERT(i < guiContext->popupCount);
    GuiContainer * result = guiAddContainer(NULL, style, guiContext->width/2 - width/2, guiContext->height/2 - height/2, width, height);
    result->zIndex = 100 + i + 1;
	guiContext->popupLocked = false;
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
    for(int32 i = 0; i < ARRAYSIZE(container->cursor); i++){
        container->cursor[i].x = container->defaultCursor[i].x;
        container->cursor[i].y = container->defaultCursor[i].y + container->heightUsed + style->text.padding.b + style->text.margin.b + style->text.margin.t;
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

static void calculateAndAdvanceCursor(GuiContainer ** container, const GuiStyle * style, const GuiElementStyle * elementStyle, const char * text, GuiJustify justify, RenderElementInfo * info){
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
    if(guiAnyPopup()){
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

static bool renderText(const AtlasFont * font, const char * text, int startX, int startY, int pt, const Color * color, int32 zIndex = 0){
    glUseProgram(guiGl->font.program);

    int32 advance = 0;
    float32 resScaleY = 1.0f / (guiContext->height);
    float32 resScaleX = 1.0f / (guiContext->width);
    int32 targetSize = ptToPx(CAST(float32, pt));
    float32 fontScale = (float32)targetSize / font->pixelSize;
    char prevGlyph = 0;
    for(int i = 0; i < strlen(text); i++){
        const GlyphData * glyph = &font->glyphs[CAST(uint8, text[i])];
        ASSERT(glyph->valid);
        if(!glyph->valid){
            continue;
        }
        
        int32 positionX = startX + advance + (int32)((float32)glyph->marginX*fontScale);
        int32 positionY = startY + (int32)((float32)glyph->marginY*fontScale);
        //kerning
        if(prevGlyph){
            positionX += (int32)((float32)glyph->kerning[prevGlyph]*fontScale);
            advance += (int32)((float32)glyph->kerning[prevGlyph]*fontScale);
        }
        
        float32 zOffset = (float32)zIndex / INT8_MAX;
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
        
        advance += (int32)((float32)glyph->width * fontScale);
        prevGlyph = glyph->glyph;
    }
    return true;
}

bool renderTextXYCentered(const AtlasFont * font, const char * text, int centerX, int centerY, int pt, const Color * color, int32 zOffset = 0){
    return renderText(font, text, centerX - calculateAtlasTextWidth(font, text, pt)/2, centerY - CAST(int32, ((CAST(float32, ptToPx(CAST(float32, pt)))/font->pixelSize) * font->lineHeight)/2), pt, color, zOffset);
}

static bool renderRect(const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * color, float zIndex = 0){
    glUseProgram(guiGl->flat.program);
    
    float32 resScaleY = 1.0f / (guiContext->height);
    float32 resScaleX = 1.0f / (guiContext->width);
    
    //position
    float32 zOffset = (float32)zIndex / INT8_MAX;
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
    float32 zOffset = (float32)zIndex / INT8_MAX;
    glUniform3f(guiGl->flat.positionLocation, resScaleX * 2 * positionX - 1, resScaleY * 2 * positionY - 1, zOffset);
    //scale
    glUniform2f(guiGl->flat.scaleLocation, (width) * resScaleX * 2, resScaleY * (height) * 2);
    
    //color
    glUniform4f(guiGl->flat.overlayColorLocation, color->x/255.0f, color->y/255.0f, color->z/255.0f, color->w/255.0f);
    
    //draw it
    // TODO(fidli): create vertex in GPU memory, this uses domain code vertices
    glDrawArrays(GL_LINE_LOOP, 4, 4);
    
    return true;
}

static bool renderInput(const AtlasFont * font, char * text, const char * charlist, const int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * boxBgColor, const Color * fieldColor, const Color * inputTextColor, int8 zIndex = 0){
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
            guiInvalidate(&guiContext->lastActive);
        }
    }else if(isSelected){
        result = true;
        if(!guiEq(guiContext->activeInput, id)){
            guiContext->activeInput = id;
            guiContext->activeInputLastTimestamp = getProcessCurrentTime();
            guiContext->activeInputTimeAccumulator = 0;
            guiContext->caretVisible = true;
            guiContext->caretPos = 0;
            guiContext->inputData = text;
            guiContext->inputCharlist = charlist;
        }
    }else{
        if(guiInput.mouse.buttons.leftDown && guiEq(id, guiContext->activeInput)) guiInvalidate(&guiContext->activeInput);
    }
    
    if(isHoverBeforeAndNow && !isLastActive){
        if(guiInput.mouse.buttons.leftDown) guiContext->lastActive = id;
    }
    
    renderRect(positionX, positionY, width, height, boxBgColor, zIndex);
    if(isSelected){
        renderWireRect(positionX, positionY, width, height, inputTextColor, zIndex);
    }
    renderRect(positionX+margin, positionY+margin, width-2*margin, height-2*margin, fieldColor, zIndex);
    
    
    
    renderTextXYCentered(font, text, positionX + width/2, positionY + height/2, 14, inputTextColor, zIndex);
    
    //do something at all
	if(guiEq(guiContext->activeInput, id)){
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
        renderWireRect(positionX, positionY, width, height, textColor, zIndex);
    }
    renderTextXYCentered(font, text, positionX + width/2, positionY + height/2, pt, textColor, zIndex);
    
    return result;
}


static bool renderDropdown(const AtlasFont * font, char * text, const char ** list, int32 listSize, int32 * resultIndex, int32 positionX, const int32 positionY, const int32 width, const int32 height, const Color * inactiveBgColor, const Color * inactiveTextColor, const Color * activeBgColor, const Color * activeTextColor, int8 zIndex = 0){
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
                }
                guiInvalidate(&guiContext->lastActive);
            }
        }else{
            if(guiInput.mouse.buttons.leftDown && guiEq(headId, guiContext->activeDropdown)){
               guiInvalidate(&guiContext->activeDropdown);
            }
        }
        if(isHeadHoverBeforeAndNow && !isHeadLastActive){
            if(guiInput.mouse.buttons.leftDown){
                guiContext->lastActive = headId;
            }
        }
        if(wasHeadSubmitted){
            guiContext->activeDropdown = headId;
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
            renderWireRect(positionX, positionY, width, height, textColor, zIndex);
        }
        renderTextXYCentered(font, text, positionX + width/2, positionY + height/2, 14, textColor, zIndex);
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
                    }
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
                    renderWireRect(buttonPositionX, buttonPositionY, width, height, textColor, zIndex + 1.0f);
                }
                renderTextXYCentered(font, list[i], buttonPositionX + width/2, buttonPositionY + height/2, 14, textColor, zIndex + 1);
                
                if(result){
                    *resultIndex = i;
                    strcpy(text, list[i]); 
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
        guiInvalidate(&guiContext->activeDropdown);
    }
    return selected || isDropdownActive || isHeadLastActive;
}

void guiRenderText(GuiContainer * container, const GuiStyle * style, const char * text, const GuiElementStyle * overrideStyle = NULL, const GuiJustify justify = GuiJustify_Default){
    RenderElementInfo rei;
    const GuiElementStyle * elementStyle = &style->text;
    if(overrideStyle != NULL){
        elementStyle = overrideStyle;
    }
    calculateAndAdvanceCursor(&container, style, elementStyle, text, justify, &rei);
    renderText(style->font, text, rei.startX+elementStyle->padding.l, rei.startY+elementStyle->padding.t, style->pt, &elementStyle->fgColor, container->zIndex);
}

void guiRenderBoxText(GuiContainer * container, const GuiStyle * style, const char * text, const GuiElementStyle * overrideStyle = NULL, const GuiJustify justify = GuiJustify_Default){
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

bool guiRenderInput(GuiContainer * container, const GuiStyle * style, char * data, const char * dictionary, const GuiElementStyle * overrideStylePassive = NULL, const GuiElementStyle * overrideStyleActive = NULL, const GuiJustify justify = GuiJustify_Default){
    RenderElementInfo rei;
    const GuiElementStyle * elementStyleActive = &style->input.active;
    const GuiElementStyle * elementStylePassive = &style->input.passive;
    if(overrideStyleActive != NULL){
        elementStyleActive = overrideStyleActive;
    }
    if(overrideStylePassive != NULL){
        elementStylePassive = overrideStylePassive;
    }
    calculateAndAdvanceCursor(&container, style, elementStyleActive, data, justify, &rei);
    return renderInput(style->font, data, dictionary, rei.startX, rei.startY, rei.renderWidth, rei.renderHeight, &elementStylePassive->bgColor, &elementStyleActive->bgColor, &elementStyleActive->fgColor, container->zIndex);
}

bool guiRenderDropdown(GuiContainer * container, const GuiStyle * style, char * searchtext, const char ** fullList, int32 listSize, int32 * resultIndex, const GuiElementStyle * overrideStylePassive = NULL, const GuiElementStyle * overrideStyleActive = NULL, const GuiJustify justify = GuiJustify_Default){
    RenderElementInfo rei;
    const GuiElementStyle * elementStyleActive = &style->input.active;
    const GuiElementStyle * elementStylePassive = &style->input.passive;
    if(overrideStyleActive != NULL){
        elementStyleActive = overrideStyleActive;
    }
    if(overrideStylePassive != NULL){
        elementStylePassive = overrideStylePassive;
    }
    calculateAndAdvanceCursor(&container, style, elementStyleActive, searchtext, justify, &rei);
    return renderDropdown(style->font, searchtext, fullList, listSize, resultIndex, rei.startX, rei.startY, rei.renderWidth, rei.renderHeight, &elementStylePassive->bgColor, &elementStylePassive->fgColor, &elementStyleActive->bgColor, &elementStyleActive->fgColor, container->zIndex);
}

void guiOpenPopupMessage(GuiStyle * style, const char * title, const char * message){
    guiOpenPopup(title);
    
	ASSERT(guiContext->messagePopupCount < ARRAYSIZE(guiContext->messagePopups));
	strncpy(guiContext->messagePopupTitles[guiContext->messagePopupCount], title, 50);
	strncpy(guiContext->messagePopupMessages[guiContext->messagePopupCount], message, 255);
	guiContext->messagePopupStyles[guiContext->messagePopupCount] = style;
    guiContext->messagePopupCount++;
}

void guiEnd(){
    // messages
    bool close = false;
    for(int32 i = 0; i < guiContext->messagePopupCount; i++){
        if(guiIsPopupOpened(guiContext->messagePopupTitles[i])){
            GuiStyle * style = guiContext->messagePopupStyles[i];
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
        membrane->zIndex = 100;
    }
    insertSort(CAST(byte*, guiContext->addedContainers), sizeof(GuiContainer), guiContext->addedContainersCount, [](void * a, void * b) -> int32 { return CAST(GuiContainer *, a)->zIndex <= CAST(GuiContainer *, b)->zIndex; });
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

