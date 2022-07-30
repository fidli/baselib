#version 330
layout(location = 0) in vec2 model;

uniform vec3 position;
uniform vec2 scale;

void main()
{
    gl_Position = vec4(scale.x*model.x + position.x, -(scale.y*model.y+position.y), position.z, 1);
}
