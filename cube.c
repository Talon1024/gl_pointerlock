#include "cube.h"
#include <stdlib.h>
#include <string.h>

indexedgeometry3d_t* makeIndexedCube()
{
    indexedgeometry3d_t* cube = malloc(sizeof(indexedgeometry3d_t));
    float vertices[] = {
        1.0f, 1.0f, 1.0f,       // 0
        -1.0f, 1.0f, 1.0f,      // 1
        -1.0f, -1.0f, 1.0f,     // 2
        1.0f, -1.0f, 1.0f,      // 3
        1.0f, 1.0f, -1.0f,      // 4
        -1.0f, 1.0f, -1.0f,     // 5
        -1.0f, -1.0f, -1.0f,    // 6
        1.0f, -1.0f, -1.0f,     // 7
    };
    cube->geometry.vertices = malloc(3 * 8 * sizeof(float));
    memcpy(cube->geometry.vertices, vertices, 3 * 8 * sizeof(float));
    cube->geometry.vertexCount = 8;
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0,
        5, 1, 0,
        0, 4, 5,
        7, 4, 0,
        7, 0, 3,
        6, 5, 4,
        7, 6, 4,
        5, 2, 1,
        5, 6, 2,
        2, 6, 3,
        6, 7, 3,
    };
    cube->indices = malloc(3 * 12 * sizeof(unsigned int));
    memcpy(cube->indices, indices, 3 * 12 * sizeof(unsigned int));
    cube->triIndexCount = 12;
    return cube;
}

void destroyIndexedObject(indexedgeometry3d_t* object)
{
    free(object->geometry.vertices);
    free(object->indices);
    free(object);
}