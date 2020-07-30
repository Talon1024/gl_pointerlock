#include "shader.h"
#include "glad.h"
#include <stdio.h>
#include <stdlib.h>

int shader_setup(shader_t* shader, const char* vertexShaderSourceFilename, const char* fragmentShaderSourceFilename)
{
    unsigned int program = glCreateProgram();
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
    // Read and store vertex shader
    FILE* vShaderFile = fopen(vertexShaderSourceFilename, "rb");
    if(vShaderFile == NULL)
    {
        fprintf(stderr, "Vertex shader %s not found!\n", vertexShaderSourceFilename);
        glDeleteProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(pixelShader);
        return 0;
    }
    fseek(vShaderFile, 0, SEEK_END);
    int vShaderLength = (int) ftell(vShaderFile);
    fseek(vShaderFile, 0, SEEK_SET);
    char* vShaderSource = (char*) malloc(vShaderLength);
    fread(vShaderSource, 1, vShaderLength, vShaderFile);
    fclose(vShaderFile);
    // Read and store fragment shader
    FILE* fShaderFile = fopen(fragmentShaderSourceFilename, "rb");
    if(fShaderFile == NULL)
    {
        fprintf(stderr, "Fragment shader %s not found!\n", fragmentShaderSourceFilename);
        glDeleteProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(pixelShader);
        return 0;
    }
    fseek(fShaderFile, 0, SEEK_END);
    int fShaderLength = (int) ftell(fShaderFile);
    fseek(fShaderFile, 0, SEEK_SET);
    char* fShaderSource = (char*) malloc(fShaderLength);
    fread(fShaderSource, 1, fShaderLength, fShaderFile);
    fclose(fShaderFile);
    // Add sources
    glShaderSource(vertexShader, 1, (const char**) &vShaderSource, &vShaderLength);
    glShaderSource(pixelShader, 1, (const char**) &fShaderSource, &fShaderLength);
    // Compile shaders, and write any errors to stderr
    glCompileShader(vertexShader);
    int shaderInfoLength;
    int compileStatus;
    char* shaderInfo;
    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &shaderInfoLength);
    if(shaderInfoLength > 0)
    {
        shaderInfo = malloc(shaderInfoLength);
        glGetShaderInfoLog(vertexShader, shaderInfoLength, &shaderInfoLength, shaderInfo);
        fprintf(stderr, "Vertex shader %s:\n%s\n", vertexShaderSourceFilename, shaderInfo);
        free(shaderInfo);
    }
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileStatus);
    if(compileStatus == GL_FALSE)
    {
        glDeleteProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(pixelShader);
        return 0;
    }
    glCompileShader(pixelShader);
    glGetShaderiv(pixelShader, GL_INFO_LOG_LENGTH, &shaderInfoLength);
    if(shaderInfoLength > 0)
    {
        shaderInfo = malloc(shaderInfoLength);
        glGetShaderInfoLog(pixelShader, shaderInfoLength, &shaderInfoLength, shaderInfo);
        fprintf(stderr, "Fragment shader %s:\n%s\n", fragmentShaderSourceFilename, shaderInfo);
        free(shaderInfo);
    }
    glGetShaderiv(pixelShader, GL_COMPILE_STATUS, &compileStatus);
    if(compileStatus == GL_FALSE)
    {
        glDeleteProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(pixelShader);
        return 0;
    }
    glAttachShader(program, vertexShader);
    glAttachShader(program, pixelShader);
    glLinkProgram(program);
    shader->programId = program;
    shader->vertexId = vertexShader;
    shader->fragmentId = pixelShader;
    return 1;
}

unsigned int shader_get_uniform(shader_t* shader, const char* name)
{
    return glGetUniformLocation(shader->programId, name);
}
unsigned int shader_get_attribute(shader_t* shader, const char* name)
{
    return glGetAttribLocation(shader->programId, name);
}

void shader_destroy(shader_t* shader)
{
    glDeleteProgram(shader->programId);
    glDeleteShader(shader->vertexId);
    glDeleteShader(shader->fragmentId);
}