#include "glad.h"
#include <GLFW/glfw3.h>
#include "pngload.h"
#include "cube.h"

int main(int argc, char** argv)
{
    indexedgeometry3d_t cube = makeIndexedCube();
    image_t pointerLockImage = load_image("ptrlockd.png");
    GLFW_Window* window = glfwCreateWindow();
    return 0;
}