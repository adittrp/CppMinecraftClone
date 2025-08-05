#define STB_IMAGE_IMPLEMENTATION
#include "shaders/stb_image.h"
#include "shaders/shader.hpp"

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "headerfiles/Camera.hpp"
#include "headerfiles/Chunk.hpp"
#include "headerfiles/UVHelper.hpp"
#include "headerfiles/Constants.hpp"

bool wireFramed = false;
bool qKeyPressedLastFrame = false;

float opacity = 0.2f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Chunk chunks[WORLD_SIZE_X][WORLD_SIZE_Z];


// Func def
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(Camera& cam, GLFWwindow* window);

unsigned int loadTexture(const char* path);

int main(void)
{
    // Initialize GLFW
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // Create Window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    //glfwSwapInterval(0);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // textures
    //unsigned int grassSide, dirt, grassTop, stone;

    /*grassSide = loadTexture("src/shaders/resources/grass.png");
    dirt = loadTexture("src/shaders/resources/dirt.png");
    grassTop = loadTexture("src/shaders/resources/grass_top.png");
    stone = loadTexture("src/shaders/resources/stone.png");*/

    unsigned int textureAtlas;

    textureAtlas = loadTexture("src/shaders/resources/textureatlasmcclone.png");

    // -----------
    Camera cam(window);

    Shader ourShader("src/shaders/vs/vertexShader.vs", "src/shaders/fs/fragmentShader.fs");

    ourShader.use();
    ourShader.setInt("textureVal", 0);

    glm::vec3 objectColor(1.0f, 1.0f, 1.0f);
    glm::vec3 lightColor(0.8f, 0.8f, 0.7f);
    glm::vec3 lightDir(-0.2f, -1.0f, -0.3f);

    ourShader.setVec3("objectColor", objectColor);
    ourShader.setVec3("lightColor", lightColor);
    ourShader.setVec3("lightDir", lightDir);

    for (int chunkRow = 0; chunkRow < WORLD_SIZE_X; chunkRow++) {
        for (int chunkCell = 0; chunkCell < WORLD_SIZE_Z; chunkCell++) {
            Chunk& chunk = chunks[chunkRow][chunkCell];
            chunk.chunkNumberX = chunkRow;
            chunk.chunkNumberZ = chunkCell;
            
            chunk.generateChunk();
        }
    }
    

    for (int chunkRow = 0; chunkRow < WORLD_SIZE_X; chunkRow++) {
        for (int chunkCell = 0; chunkCell < WORLD_SIZE_Z; chunkCell++) {
            Chunk& chunk = chunks[chunkRow][chunkCell];
            chunk.buildMesh(chunks);
        }
    }

    while (!glfwWindowShouldClose(window))
    {
        float time = glfwGetTime();
        deltaTime = time - lastFrame;
        lastFrame = time;
        //std::cout << "FPS: " << 1.0f / deltaTime << std::endl;

        processInput(cam, window);


        /* Render here */
        glClearColor(0.2f, 0.3f, 0.3f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind Texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureAtlas);

        // Use Shaders
        ourShader.use();
        cam.setProjection(ourShader);
        cam.setCamera(ourShader);
        ourShader.setFloat("opacity", opacity);


        for (int chunkRow = 0; chunkRow < WORLD_SIZE_X; chunkRow++) {
            for (int chunkCell = 0; chunkCell < WORLD_SIZE_Z; chunkCell++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(chunkRow * CHUNK_SIZE_X, 0, chunkCell * CHUNK_SIZE_Z));
                ourShader.setMatrix("model", model);

                Chunk& chunk = chunks[chunkRow][chunkCell];
                chunk.render();
            }
        }

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(Camera& cam, GLFWwindow* window) {
    // Close Window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Opacity
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        opacity = std::max(opacity - 0.01f, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        opacity = std::min(opacity + 0.01f, 1.0f);

    // Cam input
    cam.processCameraInput(window, deltaTime);

    // Toggle WireFrame
    bool qKeyPressed = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
    if (qKeyPressed && !qKeyPressedLastFrame) {
        wireFramed = !wireFramed;
        glPolygonMode(GL_FRONT_AND_BACK, wireFramed ? GL_LINE : GL_FILL);
    }
    qKeyPressedLastFrame = qKeyPressed;
}


unsigned int loadTexture(const char* path) {
    unsigned int texID;

    glGenTextures(1, &texID);

    int width, height, nrChannels;

    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    else {
        std::cout << "FAILED TO LOAD TEXTURE" << std::endl;
    }

    stbi_image_free(data);
    return texID;
}