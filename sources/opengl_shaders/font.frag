#version 330
out vec4 color;

smooth in vec2 surfaceLocation;
flat in int glyph;

uniform vec4 overlayColor;
uniform sampler2D samplerAtlas;
uniform sampler1D samplerAtlasOffset;

void main() {
    vec4 glyphData = texelFetch(samplerAtlasOffset, glyph, 0);
    vec2 textureOffset = vec2(glyphData.x, glyphData.y);
    vec2 textureScale = vec2(glyphData.z, glyphData.w);
    vec4 texel  = texture2D(samplerAtlas, textureOffset + textureScale * surfaceLocation);
    color = overlayColor * texel.a + texel * (1-texel.a);
}
