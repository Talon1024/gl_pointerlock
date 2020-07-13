#include "cube.h"
#include <stdlib.h>

indexedgeometry3d_t new_indexedgeometry3d()
{
    indexedgeometry3d_t object;
    object.geometry = malloc(sizeof(geometry3d_t));
    return object;
}

void delete_indexedgeometry3d_t(indexedgeometry3d_t object)
{
    free(object.geometry);
}

indexedgeometry3d_t makeIndexedCube()
{
    indexedgeometry3d_t cube = new_indexedgeometry3d();
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
    cube.geometry->vertices = vertices;
    cube.geometry->vertexCount = 8;
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
    cube.indices = indices;
    cube.triIndexCount = 12;
    return cube;
}