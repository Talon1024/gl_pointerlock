#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "glad.h"
#include <GLFW/glfw3.h>
#include "pngload.h"
#include "cube.h"
#include "shader.h"
#include <cglm/affine.h>
#include <cglm/cam.h>

typedef struct glmaterial {
    shader_t* shaderProgram;
    int modelMatrixUniform;
} glmaterial_t;

typedef struct globject {
    vec3 offset;
    float rot_x;
    float rot_y;
    glmaterial_t material;
    unsigned int VAO; // Vertex array
    unsigned int VBO; // Vertex buffer
    unsigned int NBO; // Normal buffer
    unsigned int EBO; // Element buffer
    unsigned int drawCount; // Number of elements to draw using glDrawArrays or glDrawElements
} globject_t;

#define POINTER_LOCKED (1 << 0)
#define POINTER_HOVERING (1 << 1)
#define POINTER_RMBDOWN (1 << 2)
#define POINTER_LMBDOWN (1 << 3)
typedef struct eventdata {
    globject_t* cubey;
    unsigned char pointerLockStatus;
    double subx;
    double suby;
} eventdata_t;

GLFWwindow* createWindow();

unsigned int createGLTexture(image_t source);
void setupPointerLockIcon(globject_t* object, int posAttribute, int uvAttribute);
void setupObject(indexedgeometry3d_t* source, globject_t* object);
void drawObject(const globject_t* object);

void handleKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mod);
void handleCursorPosition(GLFWwindow* window, double xpos, double ypos);
void handleCursorHover(GLFWwindow* window, int entered);
void handleMouseButtonPress(GLFWwindow* window, int button, int action, int mod);

int main(int argc, char** argv)
{
    if(!glfwInit())
    {
        fputs("Failed to initialize GLFW!\n", stderr);
        glfwTerminate();
        return 1;
    }
    // Create window
    GLFWwindow* window = createWindow();
    if(window == NULL)
    {
        fputs("Failed to create a GLFW window!\n", stderr);
        glfwTerminate();
        return 1;
    }
    eventdata_t eventData;
    eventData.subx = 0;
    eventData.suby = 0;
    eventData.cubey = malloc(sizeof(globject_t));
    memset(eventData.cubey, 0, sizeof(globject_t));
    glfwSetWindowUserPointer(window, &eventData);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    // Now that the window is open, load the data
    indexedgeometry3d_t* cube = makeIndexedCube();
    image_t pointerLockImage = load_image("ptrlockd.png");
    // Set up OpenGL resources
    // First, the texture for the icon indicating pointer lock
    glActiveTexture(GL_TEXTURE0);
    unsigned int pointerLockTexture = createGLTexture(pointerLockImage);
    // Set up shader program
    shader_t cubeyShader;
    int shaderProgram = shader_setup(&cubeyShader, "cube.vert", "cube.frag");
    if(shaderProgram == 0)
    {
        fputs("Failed to set up shaders!\n", stderr);
        shader_destroy(&cubeyShader);
        glfwTerminate();
        return 1;
    }
    eventData.cubey->material.shaderProgram = &cubeyShader;
    eventData.cubey->material.modelMatrixUniform = shader_get_uniform(&cubeyShader, "model");
    int posAttribute = shader_get_attribute(&cubeyShader, "in_position");
    int colourAttribute = shader_get_attribute(&cubeyShader, "in_colour");
    int viewUniform = shader_get_uniform(&cubeyShader, "view");
    int projectionUniform = shader_get_uniform(&cubeyShader, "projection");
    // Set up vertex array object and buffers for cube
    setupObject(cube, eventData.cubey);
    glBindVertexArray(eventData.cubey->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, eventData.cubey->VBO);
    glVertexAttribPointer(posAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(posAttribute);
    glBindBuffer(GL_ARRAY_BUFFER, eventData.cubey->NBO);
    glVertexAttribPointer(colourAttribute, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), NULL);
    glEnableVertexAttribArray(colourAttribute);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    eventData.cubey->drawCount = cube->triIndexCount * 3;
    // Set up pointer lock icon
    globject_t lockIcon;
    shader_t lockShader;
    shaderProgram = shader_setup(&lockShader, "lockIcon.vert", "lockIcon.frag");
    if(shaderProgram == 0)
    {
        fputs("Failed to set up shaders!\n", stderr);
        glfwTerminate();
        return 1;
    }
    lockIcon.material.shaderProgram = &lockShader;
    int windowSizeUniform = shader_get_uniform(&lockShader, "windowSize");
    int iconTextureUniform = shader_get_uniform(&lockShader, "iconTex");
    int posAttribute2 = shader_get_attribute(&lockShader, "in_position");
    int uvAttribute = shader_get_attribute(&lockShader, "in_uv");
    setupPointerLockIcon(&lockIcon, posAttribute2, uvAttribute);
    // Process input and render
    glfwSetKeyCallback(window, handleKeyEvent);
    glfwSetCursorPosCallback(window, handleCursorPosition);
    glfwSetCursorEnterCallback(window, handleCursorHover);
    glfwSetMouseButtonCallback(window, handleMouseButtonPress);
    glfwSwapInterval(1);
    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.2, 0., 0., 0.);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnablei(GL_BLEND, 0);
        glBlendFuncSeparate(GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA, GL_ZERO);
        // Draw pointer lock icon
        if((eventData.pointerLockStatus & POINTER_LOCKED) == POINTER_LOCKED)
        {
            glDisable(GL_CULL_FACE);
            glFrontFace(GL_CCW);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glUseProgram(lockIcon.material.shaderProgram->programId);
            glUniform2f(windowSizeUniform, 400., 300.);
            glUniform1i(iconTextureUniform, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, pointerLockTexture);
            glBindVertexArray(lockIcon.VAO);
            glBindBuffer(GL_ARRAY_BUFFER, lockIcon.VBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lockIcon.EBO);
            glDrawElements(GL_TRIANGLES, lockIcon.drawCount, GL_UNSIGNED_INT, 0);
        }
        // Draw teh cube
        mat4 view, projection;
        glm_mat4_identity(view);
        glm_translate_z(view, -100);
        glm_mat4_identity(projection);
        glm_perspective(75.0, 4./3., 0.1, 1000., projection);
        glUniformMatrix4fv(viewUniform, 1, GL_FALSE, view[0]);
        glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projection[0]);
        drawObject(eventData.cubey);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    shader_destroy(&cubeyShader);
    destroyIndexedObject(cube);
    // Free OpenGL resources
    glDeleteTextures(1, &pointerLockTexture);
    // Free GLFW resources
    glfwDestroyWindow(window);
    glfwTerminate();
    // Free app resources
    free(eventData.cubey);
    // Exit
    return 0;
}

void handleKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    eventdata_t* eventData = (eventdata_t*) glfwGetWindowUserPointer(window);
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if((eventData->pointerLockStatus & POINTER_LOCKED) == POINTER_LOCKED)
        {
            eventData->pointerLockStatus &= ~POINTER_LOCKED;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

void handleCursorHover(GLFWwindow* window, int entered)
{
    eventdata_t* eventData = (eventdata_t*) glfwGetWindowUserPointer(window);
    if(entered)
    {
        eventData->pointerLockStatus |= POINTER_HOVERING;
    }
    else
    {
        eventData->pointerLockStatus &= ~POINTER_HOVERING;
    }
}

void handleMouseButtonPress(GLFWwindow* window, int button, int action, int mod)
{
    eventdata_t* eventData = (eventdata_t*) glfwGetWindowUserPointer(window);
    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        eventData->pointerLockStatus |= POINTER_RMBDOWN;
    }
    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        eventData->pointerLockStatus &= ~POINTER_RMBDOWN;
    }
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        eventData->pointerLockStatus |= POINTER_LMBDOWN;
    }
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        eventData->pointerLockStatus &= ~POINTER_LMBDOWN;
    }
    if((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_PRESS)
    {
        if((eventData->pointerLockStatus & POINTER_HOVERING) == POINTER_HOVERING)
        {
            eventData->pointerLockStatus |= POINTER_LOCKED;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            if(glfwRawMouseMotionSupported())
            {
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }
        }
    }
}

void handleCursorPosition(GLFWwindow* window, double xpos, double ypos)
{
    eventdata_t* eventData = (eventdata_t*) glfwGetWindowUserPointer(window);
    if((eventData->pointerLockStatus & POINTER_LOCKED) == POINTER_LOCKED)
    {
        printf("Mouse movement: %.3f %.3f\n", xpos - eventData->subx, ypos - eventData->suby);
        if((eventData->pointerLockStatus & POINTER_LMBDOWN) == POINTER_LMBDOWN)
        {
            eventData->cubey->offset[0] -= (xpos - eventData->subx) * .125;
            eventData->cubey->offset[1] += (ypos - eventData->suby) * .125;
        }
        else if((eventData->pointerLockStatus & POINTER_RMBDOWN) == POINTER_RMBDOWN)
        {
            eventData->cubey->rot_x -= glm_rad(ypos - eventData->suby);
            eventData->cubey->rot_y += glm_rad(xpos - eventData->subx);
        }
    }
    eventData->subx = xpos;
    eventData->suby = ypos;
}


GLFWwindow* createWindow()
{
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(400, 300, "Pointer lock", NULL, NULL);
    return window;
}

// Call glActiveTexture before calling this function!
unsigned int createGLTexture(image_t source)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        source.width, source.height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, source.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

void setupPointerLockIcon(globject_t* object, int posAttribute, int uvAttribute)
{
    float lockIconVertices[] = {
        0.0, 0.0, 0.0, 0.0, // X Y S T
        0.0, 1.0, 0.0, 1.0,
        1.0, 0.0, 1.0, 0.0,
        1.0, 1.0, 1.0, 1.0,
    };
    unsigned int lockIconElements[] = {
        0, 1, 2,
        3, 2, 1
    };
    glGenVertexArrays(1, &object->VAO);
    glGenBuffers(1, &object->VBO);
    // glGenBuffers(1, &object->NBO);
    glGenBuffers(1, &object->EBO);
    glBindBuffer(GL_ARRAY_BUFFER, object->VBO);
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), lockIconVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), lockIconElements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(object->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, object->VBO);
    glVertexAttribPointer(posAttribute, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(posAttribute);
    glVertexAttribPointer(uvAttribute, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(uvAttribute);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    object->drawCount = 6;
}

void setupObject(indexedgeometry3d_t* source, globject_t* object)
{
    glGenVertexArrays(1, &object->VAO);
    glGenBuffers(1, &object->VBO);
    glGenBuffers(1, &object->NBO);
    glGenBuffers(1, &object->EBO);
    glBindBuffer(GL_ARRAY_BUFFER, object->VBO);
    glBufferData(GL_ARRAY_BUFFER,
        source->geometry.vertexCount * 3 * sizeof(float),
        source->geometry.vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, object->NBO);
    // Generate random vertex colours. It's simpler than calculating normals,
    // and the cube isn't really set up for per-triangle vertex normals
    float* cubeColours = (float*) malloc(source->geometry.vertexCount * sizeof(float));
    srand(time(NULL));
    for(int i = 0; i < source->geometry.vertexCount; i++)
    {
        unsigned int random = (unsigned int) rand();
        float component = random / (float)RAND_MAX;
        if(component < 0)
        {
            component = 0;
        }
        else if(component > 1)
        {
            component = 1;
        }
        cubeColours[i] = component;
    }
    glBufferData(GL_ARRAY_BUFFER,
        source->geometry.vertexCount * sizeof(float),
        cubeColours, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    free(cubeColours);
    // Upload EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        source->triIndexCount * 3 * sizeof(unsigned int),
        source->indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void drawObject(const globject_t* object)
{
    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, object->offset);
    glm_rotate_x(model, object->rot_x, model);
    glm_rotate_y(model, object->rot_y, model);
    glUseProgram(object->material.shaderProgram->programId);
    glUniformMatrix4fv(object->material.modelMatrixUniform, 1, GL_FALSE, model[0]);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(object->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object->EBO);
    glDrawElements(GL_TRIANGLES, object->drawCount, GL_UNSIGNED_INT, 0);
}