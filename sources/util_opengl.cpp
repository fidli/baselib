#ifndef OPENGL_CPP
#define OPENGL_CPP

bool compileShaders(const unsigned char * vertexContents, u32 vertexContentsSize, const unsigned char * fragmentContents, u32 fragmentContentsSize, GLint * vertexShader, GLint * fragmentShader, GLint * program){
    bool r = true;    
    *vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(*vertexShader, 1, (const char **)&vertexContents, (GLint*) &vertexContentsSize);
    glCompileShader(*vertexShader);
    
    GLint vertexStatus;
    glGetShaderiv(*vertexShader, GL_COMPILE_STATUS, &vertexStatus);
    if (vertexStatus == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(*vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
        char error[1024];
        glGetShaderInfoLog(*vertexShader, maxLength, &maxLength, error);
        LOGE(default, shaders, error);
        return false;
    }
    
    
    *fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(*fragmentShader, 1, (const char **)&fragmentContents, (GLint *) &fragmentContentsSize);
    glCompileShader(*fragmentShader);
    
    GLint fragmentStatus;
    glGetShaderiv(*fragmentShader, GL_COMPILE_STATUS, &fragmentStatus);
    if (fragmentStatus == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(*fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);
        char error[1024];
        glGetShaderInfoLog(*fragmentShader, maxLength, &maxLength, error);
        LOGE(default, shaders, error);
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
        LOGE(default, shaders, error);
        return false;
    }
    return r;
}


bool loadAndCompileShaders(const char * vertexShaderPath, const char * fragmentShaderPath, GLint * vertexShader, GLint * fragmentShader, GLint * program){
    
    FileContents vertexContents = {};
    FileContents fragmentContents = {};
    
    //shaders compilation
    if(readFile(vertexShaderPath, &vertexContents) && readFile(fragmentShaderPath, &fragmentContents)){
        return compileShaders(CAST(const unsigned char *, vertexContents.contents), CAST(uint32, vertexContents.size), CAST(const unsigned char *, fragmentContents.contents), CAST(uint32, fragmentContents.size), vertexShader, fragmentShader, program);
    }
    LOGE(default, shaders, "Failed to read shader files");
    return false;
}

#endif


