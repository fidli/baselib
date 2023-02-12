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
#define GL_TEXTURE0 0x84C0
#define GL_RGBA32I 0x8D82
#define GL_RGBA_INTEGER 0x8D99
#define GL_RG32I 0x823B
#define GL_RG_INTEGER 0x8228

//https://www.khronos.org/registry/OpenGL/api/GL/wglext.h
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_TYPE_RGBA_ARB 0x202B
#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#define WGL_SAMPLES_ARB 0x2042

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
DEFINEDLLFUNC(void, glVertexAttribIPointer, GLuint, GLint, GLenum, GLsizei, const void *);
DEFINEDLLFUNC(void, glDeleteShader, GLuint);
DEFINEDLLFUNC(void, glDeleteProgram, GLuint);
DEFINEDLLFUNC(bool, wglSwapIntervalEXT, int);
DEFINEDLLFUNC(bool, wglChoosePixelFormatARB, HDC, const int *, const FLOAT *, UINT, int *, UINT *);
DEFINEDLLFUNC(HGLRC, wglCreateContextAttribsARB, HDC, HGLRC, const int *);
DEFINEDLLFUNC(void, glDrawArraysInstanced, GLenum, GLint, GLsizei, GLsizei);

DEFINEDLLFUNC(GLint, glGetUniformLocation, GLuint, const char *);
DEFINEDLLFUNC(void, glUniform1i, GLint, GLint);
DEFINEDLLFUNC(void, glUniform1iv, GLint, GLsizei, const GLint*);
DEFINEDLLFUNC(void, glUniform1f, GLint, GLfloat);
DEFINEDLLFUNC(void, glUniform2f, GLint, GLfloat, GLfloat);
DEFINEDLLFUNC(void, glUniform2i, GLint, GLint, GLint);
DEFINEDLLFUNC(void, glUniform3f, GLint, GLfloat, GLfloat, GLfloat);
DEFINEDLLFUNC(void, glUniform4f, GLint, GLfloat, GLfloat, GLfloat, GLfloat);

#define OBTAINGLFUNC(HNDL, FNC) \
FNC = (FNC##FuncType) wglGetProcAddress(#FNC);if(FNC == NULL){OBTAINDLLFUNC(HNDL, FNC);}ASSERT(FNC != NULL);

bool initOpenGL(){
    WNDCLASSEX style = {};
    style.cbSize = sizeof(WNDCLASSEX);
    style.style = CS_OWNDC | CS_DBLCLKS;
    style.hInstance = hInstance;
    style.lpfnWndProc = DefWindowProc;
    style.lpszClassName = "OpenGLInit";
    bool r = RegisterClassEx(&style) != 0;
    ASSERT(r);
    HWND window_ = CreateWindowEx(NULL,
                                "OpenGLInit", "OpenGL Window", WS_OVERLAPPEDWINDOW | WS_SIZEBOX,
                                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    r &= window_ != NULL;
    if (r != true)
    {
        return r;
    }
    HDC dc = GetDC(window_);
    PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
			PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
			32,                   // Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,                   // Number of bits for the depthbuffer
			0,                    // Number of bits for the stencilbuffer
			0,                    // Number of Aux buffers in the framebuffer.
            0,
			0,
			0, 0, 0
		};
    bool res = SetPixelFormat(dc,ChoosePixelFormat(dc, &pfd), &pfd) == TRUE;
    ASSERT(res);
    HGLRC dummyContext = wglCreateContext(dc);
    ASSERT(dummyContext != NULL);
    r = wglMakeCurrent(dc, dummyContext) == TRUE;
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
        OBTAINGLFUNC(opengl, glVertexAttribIPointer);
        OBTAINGLFUNC(opengl, glDeleteShader);
        OBTAINGLFUNC(opengl, glDeleteProgram);
        OBTAINGLFUNC(opengl, wglSwapIntervalEXT);
        OBTAINGLFUNC(opengl, wglChoosePixelFormatARB);
        OBTAINGLFUNC(opengl, wglCreateContextAttribsARB);
        OBTAINGLFUNC(opengl, glDrawArraysInstanced);

        OBTAINGLFUNC(opengl, glGetUniformLocation);
        OBTAINGLFUNC(opengl, glUniform1f);
        OBTAINGLFUNC(opengl, glUniform1i);
        OBTAINGLFUNC(opengl, glUniform1iv);
        OBTAINGLFUNC(opengl, glUniform2f);
        OBTAINGLFUNC(opengl, glUniform2i);
        OBTAINGLFUNC(opengl, glUniform3f);
        OBTAINGLFUNC(opengl, glUniform4f);
        r = true;
    }else{
        r = false;
    }
    wglMakeCurrent(NULL, NULL);
    DestroyWindow(window_);
    UnregisterClass("OpenGLInit", hInstance);
    wglDeleteContext(dummyContext);
    return r;
}
