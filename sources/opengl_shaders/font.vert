#version 330
layout(location = 0) in vec2 model;
layout(location = 1) in int textIndex;

uniform vec3 position;

smooth out vec2 surfaceLocation;
flat out int glyph;

uniform int text[64];

// NOTE(fidli): for whole text
uniform float pt;

// NOTE(fidli): const-ish
uniform ivec2 screenDims;

// TODO(fidli): informations from textures
uniform float fontPixelSize;

uniform isampler1D samplerGlyphData;
uniform isampler2D samplerKerning;

float ptToPx(float pt){
    // TODO(fidli): real DPI instead of 96
    return pt * (96.0f / 72.0f);
}

int extractGlyph(int i){
    return (text[i/4] >> (i & 3)*8) & 0xFF;
}

int sumAdvance(){
    int sum = 0;
    for(int i = 1; i <= textIndex; i++){
        int thisGlyph = extractGlyph(i);
        int prevGlyph = extractGlyph(i-1);
        ivec2 kerning = texelFetch(samplerKerning, ivec2(thisGlyph, prevGlyph), 0).rg;
        sum += kerning.r + kerning.g;
    }
    return sum;
}

void main(){
    surfaceLocation = model;
    glyph = extractGlyph(textIndex);
    int glyphAdvance = sumAdvance();

    ivec4 glyphData = texelFetch(samplerGlyphData, glyph, 0);
    vec2 glyphPixelDims = vec2(glyphData.r, glyphData.g);
    vec2 glyphMargin = vec2(glyphData.z, glyphData.w);

    float fontScale = ptToPx(pt) / fontPixelSize;
    vec2 resScale = vec2(1.0f / float(screenDims.x), 1.0f / float(screenDims.y));
    vec3 absPosition = vec3(position.x + ((glyphAdvance+glyphMargin.x)*fontScale), position.y + (glyphMargin.y*fontScale), position.z);
    vec3 normalizedPosition = vec3(resScale.x * 2.0f * absPosition.x - 1.0f, resScale.y * 2.0f * absPosition.y - 1.0f, clamp(absPosition.z, -127.0f, 127.0f)/127.0f);
    vec2 scale = vec2(fontScale * resScale.x * 2 * glyphPixelDims.x, fontScale * resScale.y * glyphPixelDims.y * 2);
    gl_Position = vec4(scale.x*model.x + normalizedPosition.x, -(scale.y*model.y+normalizedPosition.y), normalizedPosition.z, 1);
}
