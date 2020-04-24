#pragma once

#include "windows_dll.cpp"
#include <GL\gl.h>

// https://www.khronos.org/registry/OpenGL/api/GL/glext.h
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_LINK_STATUS 0x8B82
#define GL_TEXTURE0 0x84C0
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_MULTISAMPLE 0x809D
#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_CLAMP_TO_EDGE 0x812F

DEFINEDLLFUNC(GLuint, glCreateShader, GLenum);
DEFINEDLLFUNC(void, glShaderSource, GLuint, GLsizei, const char **, const GLint *);
DEFINEDLLFUNC(void, glCompileShader, GLuint);
DEFINEDLLFUNC(void, glGetShaderiv, GLuint, GLenum, GLint *);
DEFINEDLLFUNC(void, glGetShaderInfoLog, GLuint, GLsizei, GLsizei *, char *);
DEFINEDLLFUNC(GLuint, glCreateProgram, void);
DEFINEDLLFUNC(void, glAttachShader, GLuint, GLuint);
DEFINEDLLFUNC(void, glLinkProgram, GLuint);
DEFINEDLLFUNC(void, glGetProgramiv, GLuint, GLenum, GLint *);
DEFINEDLLFUNC(void, glGetProgramInfoLog, GLuint, GLsizei, GLsizei *, char *);
DEFINEDLLFUNC(void, glUseProgram, GLuint);
DEFINEDLLFUNC(void, glActiveTexture, GLenum);
DEFINEDLLFUNC(void, glGenBuffers, GLsizei, GLuint *);
DEFINEDLLFUNC(void, glBindBuffer, GLenum, GLuint);
DEFINEDLLFUNC(void, glBufferData, GLenum, GLsizei, const void *, GLenum);
DEFINEDLLFUNC(void, glEnableVertexAttribArray, GLuint);
DEFINEDLLFUNC(void, glVertexAttribPointer, GLuint, GLint, GLenum, bool, GLsizei, const void *);
DEFINEDLLFUNC(void, glDeleteShader, GLuint);
DEFINEDLLFUNC(void, glDeleteProgram, GLuint);
DEFINEDLLFUNC(bool, wglSwapIntervalEXT, int);

DEFINEDLLFUNC(GLint, glGetUniformLocation, GLuint, const char *);
DEFINEDLLFUNC(void, glUniform1i, GLint, GLint);
DEFINEDLLFUNC(void, glUniform2f, GLint, GLfloat, GLfloat);
DEFINEDLLFUNC(void, glUniform3f, GLint, GLfloat, GLfloat, GLfloat);
DEFINEDLLFUNC(void, glUniform4f, GLint, GLfloat, GLfloat, GLfloat, GLfloat);

#define OBTAINGLFUNC(HNDL, FNC) \
FNC = (FNC##FuncType) wglGetProcAddress(#FNC);if(FNC == NULL){OBTAINDLLFUNC(HNDL, FNC);}ASSERT(FNC != NULL);

bool initOpenGL(HDC dc){
    HGLRC dummyContext = wglCreateContext(dc);
    ASSERT(dummyContext != NULL);
    auto r = wglMakeCurrent(dc, dummyContext);
    ASSERT(r == TRUE);
    if(r != TRUE){
        return false;
    }
    HMODULE opengl = LoadLibrary("opengl32.dll");
    ASSERT(opengl);
    if(opengl != NULL){
        OBTAINGLFUNC(opengl, glCreateShader);
        OBTAINGLFUNC(opengl, glShaderSource);
        OBTAINGLFUNC(opengl, glCompileShader);
        OBTAINGLFUNC(opengl, glGetShaderiv);
        OBTAINGLFUNC(opengl, glGetShaderInfoLog);
        OBTAINGLFUNC(opengl, glCreateProgram);
        OBTAINGLFUNC(opengl, glAttachShader);
        OBTAINGLFUNC(opengl, glLinkProgram);
        OBTAINGLFUNC(opengl, glGetProgramiv);
        OBTAINGLFUNC(opengl, glGetProgramInfoLog);
        OBTAINGLFUNC(opengl, glUseProgram);
        OBTAINGLFUNC(opengl, glActiveTexture);
        OBTAINGLFUNC(opengl, glGenBuffers);
        OBTAINGLFUNC(opengl, glBindBuffer);
        OBTAINGLFUNC(opengl, glBufferData);
        OBTAINGLFUNC(opengl, glEnableVertexAttribArray);
        OBTAINGLFUNC(opengl, glVertexAttribPointer);
        OBTAINGLFUNC(opengl, glDeleteShader);
        OBTAINGLFUNC(opengl, glDeleteProgram);
        OBTAINGLFUNC(opengl, wglSwapIntervalEXT);

        OBTAINGLFUNC(opengl, glGetUniformLocation);
        OBTAINGLFUNC(opengl, glUniform1i);
        OBTAINGLFUNC(opengl, glUniform2f);
        OBTAINGLFUNC(opengl, glUniform3f);
        OBTAINGLFUNC(opengl, glUniform4f);
        r = true;
    }else{
        r = false;
    }
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(dummyContext);
    return r;
}
