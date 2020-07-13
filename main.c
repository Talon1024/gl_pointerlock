#include <stdio.h>
#include <stdlib.h>
#include "glad.h"
#include <GLFW/glfw3.h>
#include "pngload.h"
#include "cube.h"
#include <cglm/affine.h>
#include <cglm/cam.h>

typedef struct glmaterial {
    unsigned int shaderProgram;
    int modelMatrixUniform;
} glmaterial_t;

typedef struct globject {
    vec3 offset;
    glmaterial_t material;
    unsigned int VAO; // Vertex array
    unsigned int VBO; // Vertex buffer
    unsigned int NBO; // Normal buffer
    unsigned int EBO; // Element buffer
    unsigned int drawCount; // Number of elements to draw using glDrawArrays or glDrawElements
} globject_t;

globject_t cubey;

GLFWwindow* createWindow();

unsigned int createGLTexture(image_t source);
void setupPointerLockIcon(globject_t* object);
void setupObject(indexedgeometry3d_t* source, globject_t* object);
unsigned int setupShaderProgram(
    const char* vertexShaderSourceFilename,
    const char* fragmentShaderSourceFilename);
void drawObject(const globject_t* object);

void handleKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mod);
void handleCursorPosition(GLFWwindow* window, double xpos, double ypos);
void handleCursorHover(GLFWwindow* window, int entered);
void handleMouseButtonPress(GLFWwindow* window, int button, int action, int mod);

#define POINTER_LOCKED (1 << 0)
#define POINTER_HOVERING (1 << 1)
unsigned char pointerLocked = 0;

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
    unsigned int shaderProgram = setupShaderProgram("cube.vert", "cube.frag");
    if(shaderProgram == 0)
    {
        fputs("Failed to set up shaders!\n", stderr);
        glfwTerminate();
        return 1;
    }
    cubey.material.shaderProgram = shaderProgram;
    int posAttribute = glGetAttribLocation(shaderProgram, "in_position");
    int colourAttribute = glGetAttribLocation(shaderProgram, "in_colour");
    cubey.material.modelMatrixUniform = glGetUniformLocation(shaderProgram, "model");
    int viewUniform = glGetUniformLocation(shaderProgram, "view");
    int projectionUniform = glGetUniformLocation(shaderProgram, "projection");
    // Set up vertex array object and buffers for cube
    setupObject(cube, &cubey);
    glBindVertexArray(cubey.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubey.VBO);
    glVertexAttribPointer(posAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(posAttribute);
    glBindBuffer(GL_ARRAY_BUFFER, cubey.NBO);
    glVertexAttribPointer(colourAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(colourAttribute);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    cubey.drawCount = cube->triIndexCount * 3;
    // Set up pointer lock icon
    globject_t lockIcon;
    shaderProgram = setupShaderProgram("lockIcon.vert", "lockIcon.frag");
    if(shaderProgram == 0)
    {
        fputs("Failed to set up shaders!\n", stderr);
        glfwTerminate();
        return 1;
    }
    lockIcon.material.shaderProgram = shaderProgram;
    int windowSizeUniform = glGetUniformLocation(shaderProgram, "windowSize");
    int iconTextureUniform = glGetUniformLocation(shaderProgram, "iconTex");
    setupPointerLockIcon(&lockIcon);
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
        // Draw pointer lock icon
        if((pointerLocked & POINTER_LOCKED) == POINTER_LOCKED)
        {
            glUseProgram(lockIcon.material.shaderProgram);
            glUniform2f(windowSizeUniform, 400., 300.);
            glUniform1i(iconTextureUniform, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, pointerLockTexture);
            glDisable(GL_CULL_FACE);
            glFrontFace(GL_CCW);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glBindVertexArray(lockIcon.VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lockIcon.EBO);
            glDrawElements(GL_TRIANGLES, lockIcon.drawCount, GL_UNSIGNED_INT, 0);
        }
        // Draw teh cube
        glUseProgram(cubey.material.shaderProgram);
        mat4 view, projection;
        glm_mat4_identity(view);
        glm_translate_z(view, -7);
        glm_mat4_identity(projection);
        glm_perspective(90.0, 4./3., 0.1, 1000., projection);
        glUniformMatrix4fv(viewUniform, 1, GL_FALSE, view[0]);
        glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projection[0]);
        drawObject(&cubey);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    destroyIndexedObject(cube);
    // Free OpenGL resources
    glDeleteTextures(1, &pointerLockTexture);
    // Free GLFW resources
    glfwDestroyWindow(window);
    glfwTerminate();
    // Exit
    return 0;
}

void handleKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if((pointerLocked & POINTER_LOCKED) == POINTER_LOCKED)
        {
            pointerLocked &= ~POINTER_LOCKED;
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
    if(entered)
    {
        pointerLocked |= POINTER_HOVERING;
    }
    else
    {
        pointerLocked &= ~POINTER_HOVERING;
    }
}

void handleMouseButtonPress(GLFWwindow* window, int button, int action, int mod)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        if((pointerLocked & POINTER_HOVERING) == POINTER_HOVERING)
        {
            pointerLocked |= POINTER_LOCKED;
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
    if((pointerLocked & POINTER_LOCKED) == POINTER_LOCKED)
    {
        printf("Mouse movement: %.3f %.3f\n", xpos, ypos);
    }
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
        source.width, source.height, 0,
        GL_RGBA8, GL_UNSIGNED_BYTE, source.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

void setupPointerLockIcon(globject_t* object)
{
    float lockIconVertices[] = {
        0.0, 0.0, 0.0, 0.0, // X Y S T
        0.0, 1.0, 0.0, 1.0,
        1.0, 0.0, 1.0, 0.0,
        1.0, 1.0, 1.0, 1.0,
    };
    unsigned int lockIconElements = {
        0, 1, 2,
        3, 2, 1
    };
    glGenVertexArrays(1, &object->VAO);
    glGenBuffers(1, &object->VBO);
    glGenBuffers(1, &object->NBO);
    glGenBuffers(1, &object->EBO);
    glBindVertexArray(object->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, object->VBO);
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), lockIconVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object->NBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), lockIconElements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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

unsigned int setupShaderProgram(
    const char* vertexShaderSourceFilename,
    const char* fragmentShaderSourceFilename)
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
    glShaderSource(vertexShader, 1, &vShaderSource, &vShaderLength);
    glShaderSource(pixelShader, 1, &fShaderSource, &fShaderLength);
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
    return program;
}

void drawObject(const globject_t* object)
{
    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, object->offset);
    glUniformMatrix4fv(object->material.modelMatrixUniform, 1, GL_FALSE, model[0]);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(object->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object->EBO);
    glDrawElements(GL_TRIANGLES, object->drawCount, GL_UNSIGNED_INT, 0);
}