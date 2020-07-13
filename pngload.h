// Old-style header guards may be necessary for C98
#ifndef PNGLOAD_H
#define PNGLOAD_H

typedef struct image {
    unsigned int width;
    unsigned int height;
    unsigned char* data;
} image_t;

image_t load_image(const char* fname);

#endif