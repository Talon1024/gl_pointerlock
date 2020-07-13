#include "pngload.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_LINEAR
#define STBI_ONLY_PNG
#include "stb_image.h"

image_t load_image(const char* fname)
{
    int output_channels = 4;
    int width, height, channels;
    unsigned char* data = stbi_load(fname, &width, &height, &channels, output_channels);
    image_t outImage = {width, height, data};
    return outImage;
}
