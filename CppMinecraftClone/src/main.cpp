#define STB_IMAGE_IMPLEMENTATION
#include "includes/stb_image.h"

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
#include "headerfiles/Raycast.hpp"
#include "headerfiles/World.hpp"
#include "headerfiles/Shader.hpp"

bool wireFramed = false;
bool qKeyPressedLastFrame = false;

float opacity = 1.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Func def
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(Camera& cam, GLFWwindow* window);

unsigned int loadTexture(const char* path);

void crosshairSetUp(unsigned int& VAO, unsigned int& VBO);
void highlightBlockSetUp(unsigned int& VAO, unsigned int& VBO);

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
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
   
    glEnable(GL_DEPTH_TEST);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

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


    // Set up crosshair
    Shader crosshairShader("src/shaders/vs/crosshair.vs", "src/shaders/fs/crosshair.fs");
    unsigned int crossVAO, crossVBO;

    crosshairSetUp(crossVAO, crossVBO);

    // --------------------
    Shader highlightShader("src/shaders/vs/highlight.vs", "src/shaders/fs/highlight.fs");
    unsigned int highlightVAO, highlightVBO;

    highlightBlockSetUp(highlightVAO, highlightVBO);
    // -------------

    generateWorld();

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

        // Chunk Rendering
        renderWorld(ourShader);

        // Highlight if looking at a block
        highlightBlock(cam, highlightShader, highlightVAO);

        // Cross hair stuff
        glDisable(GL_DEPTH_TEST);

        crosshairShader.use();
        glBindVertexArray(crossVAO);

        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, 4);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);
        // -------------------------

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // clean up
    glDeleteVertexArrays(1, &crossVAO);
    glDeleteBuffers(1, &crossVBO);

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (currentRayResult.hit) {
            int chunkX = currentRayResult.blockPos.x / CHUNK_SIZE_X;
            int chunkZ = currentRayResult.blockPos.z / CHUNK_SIZE_Z;
            if (chunkX < 0 || chunkX >= WORLD_SIZE_X || chunkZ < 0 || chunkZ >= WORLD_SIZE_Z) return;

            Chunk& chunk = chunks[chunkX][chunkZ];
            glm::ivec3 localPos = currentRayResult.blockPos - glm::ivec3(chunkX * CHUNK_SIZE_X, 0, chunkZ * CHUNK_SIZE_Z);

            if (localPos.x < 0 || localPos.y < 0 || localPos.z < 0 ||
                localPos.x >= CHUNK_SIZE_X || localPos.y >= CHUNK_SIZE_Y || localPos.z >= CHUNK_SIZE_Z) return;

            chunk.removeBlock(localPos.x, localPos.y, localPos.z);

            int neighborX = -1;
            if (localPos.x == 0) {
                neighborX = chunkX - 1;
            } else  if (localPos.x == CHUNK_SIZE_X - 1) {
                neighborX = chunkX + 1;
            }
            if (neighborX >= 0 && neighborX < WORLD_SIZE_X) {
                Chunk& neighborChunk = chunks[neighborX][chunkZ];
                neighborChunk.buildMesh(chunks);
            }

            int neighborZ = -1;
            if (localPos.z == 0) {
                neighborZ = chunkZ - 1;
            } else if (localPos.z == CHUNK_SIZE_Z - 1) {
                neighborZ = chunkZ + 1;
            }
            if (neighborZ >= 0 && neighborZ < WORLD_SIZE_Z) {
                Chunk& neighborChunk = chunks[chunkX][neighborZ];
                neighborChunk.buildMesh(chunks);
            }
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        std::cout << "Right mouse button clicked!" << std::endl;
    }
}

void processInput(Camera& cam, GLFWwindow* window) {
    // Close Window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Opacity
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        opacity = std::max(opacity - 0.001f, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        opacity = std::min(opacity + 0.001f, 1.0f);

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

void crosshairSetUp(unsigned int& VAO, unsigned int& VBO) {
    float crossSizeVertical = 1920.0 / 100000.0;
    float crossSizeHorizontal = 1080.0 / 100000.0;

    float crosshairVertices[] = {
        -crossSizeHorizontal, 0.0f,
         crossSizeHorizontal, 0.0f,
         0.0f, -crossSizeVertical,
         0.0f,  crossSizeVertical
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void highlightBlockSetUp(unsigned int& VAO, unsigned int& VBO) {
    constexpr float vertices[] = {
        // left face
        -0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,

         0.5f,  0.5f, -0.5f, 
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,

        // right face
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        // front face
        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,

        // back face
        0.5f,  0.5f,  0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,

         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,

         // bottom face
         -0.5f, -0.5f, -0.5f,
          0.5f, -0.5f, -0.5f,
          0.5f, -0.5f,  0.5f,

          0.5f, -0.5f,  0.5f,
         -0.5f, -0.5f,  0.5f,
         -0.5f, -0.5f, -0.5f,

         // top face
         -0.5f,  0.5f, -0.5f, 
          0.5f,  0.5f,  0.5f,  
          0.5f,  0.5f, -0.5f,  

          0.5f,  0.5f,  0.5f,  
         -0.5f,  0.5f, -0.5f,  
         -0.5f,  0.5f,  0.5f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}