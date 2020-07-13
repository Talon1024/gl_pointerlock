#ifndef CUBE_H
#define CUBE_H

typedef struct geometry3d {
    float* vertices;
    unsigned int vertexCount;
} geometry3d_t;

typedef struct indexedgeometry3d {
    geometry3d_t* geometry;
    unsigned int* indices;
    unsigned int triIndexCount;
} indexedgeometry3d_t;

// geometry3d_t makeCube();
indexedgeometry3d_t makeIndexedCube();

void delete_indexedgeometry3d_t(indexedgeometry3d_t object);

#endif