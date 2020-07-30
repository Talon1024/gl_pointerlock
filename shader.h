#ifndef SHADER_H
#define SHADER_H

typedef struct shader {
    unsigned int programId;
    unsigned int vertexId;
    unsigned int fragmentId;
} shader_t;

int shader_setup(shader_t* shader, const char* vertexShaderSourceFilename, const char* fragmentShaderSourceFilename);

unsigned int shader_get_uniform(shader_t* shader, const char* name);
unsigned int shader_get_attribute(shader_t* shader, const char* name);

void shader_destroy(shader_t* shader);

#endif