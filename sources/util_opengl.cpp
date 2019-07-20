#ifndef OPENGL_CPP
#define OPENGL_CPP

#include "glew\include\GL\glew.h"
#include <gl\gl.h>

bool loadAndCompileShaders(const char * vertexShaderPath, const char * fragmentShaderPath, GLint * vertexShader, GLint * fragmentShader, GLint * program){
    
    FileContents vertexContents = {};
    FileContents fragmentContents = {};
    
    //shaders compilation
    bool r = readFile(vertexShaderPath, &vertexContents);
    ASSERT(r);
    
    *vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(*vertexShader, 1, (const char **)&vertexContents.contents, (GLint*) &vertexContents.size);
    glCompileShader(*vertexShader);
    
    GLint vertexStatus;
    glGetShaderiv(*vertexShader, GL_COMPILE_STATUS, &vertexStatus);
    if (vertexStatus == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(*vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
        char error[1024];
        glGetShaderInfoLog(*vertexShader, maxLength, &maxLength, error);
        LOGE(default, "shaders", error);
        return false;
    }
    
    r = readFile(fragmentShaderPath, &fragmentContents);
    ASSERT(r);
    fragmentContents.contents[fragmentContents.size] = 0;
    
    *fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(*fragmentShader, 1, (const char **)&fragmentContents.contents, (GLint *) &fragmentContents.size);
    glCompileShader(*fragmentShader);
    
    GLint fragmentStatus;
    glGetShaderiv(*fragmentShader, GL_COMPILE_STATUS, &fragmentStatus);
    if (fragmentStatus == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(*fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);
        char error[1024];
        glGetShaderInfoLog(*fragmentShader, maxLength, &maxLength, error);
        LOGE(default, "shaders", error);
        return false;
    }
    
    
    *program = glCreateProgram();
    glAttachShader(*program, *vertexShader);
    glAttachShader(*program, *fragmentShader);
    
    GLint programStatus;
    glLinkProgram(*program);
    glGetProgramiv(*program, GL_LINK_STATUS, &programStatus);
    if (programStatus == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &maxLength);
        char error[1024];
        glGetProgramInfoLog(*program, maxLength, &maxLength, error);
        LOGE(default, "shaders", error);
        return false;
    }
    return r;
}

#endif


