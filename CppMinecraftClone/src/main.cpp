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
#include "headerfiles/Player.hpp"

struct AppState {
    Camera cam;
    Player player;

    bool wireFramed = false;
    bool qKeyPressedLastFrame = false;
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    unsigned int invSlotVAOs[9];
    unsigned int invSlotVBOs[9];

    AppState(GLFWwindow* window):
        cam(window)
    {
        cam.curPlayer = player;
    }
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(AppState& app, GLFWwindow* window);

unsigned int loadTexture(const char* path);

void crosshairSetUp(unsigned int& VAO, unsigned int& VBO);
void updateSlots(AppState& app);
void invSlotSetUp(unsigned int& VAO, unsigned int& VBO, int index, bool selected);
void highlightBlockSetUp(unsigned int& VAO, unsigned int& VBO);
//

int main(void)
{
    // Initialize GLFW
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // Create Window
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Hello World", primary, NULL);
    AppState app(window);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, &app);
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

    // Set up invSlot
    Shader invSlotShader("src/shaders/vs/invslot.vs", "src/shaders/fs/invslot.fs");

    for (int i = 0; i < 9; ++i) {
        unsigned int invSlotVAO, invSlotVBO;
        invSlotSetUp(invSlotVAO, invSlotVBO, i, i == app.player.currentInventorySlot);
        app.invSlotVAOs[i] = invSlotVAO;
        app.invSlotVBOs[i] = invSlotVBO;
    }

    // --------------------

    Shader highlightShader("src/shaders/vs/highlight.vs", "src/shaders/fs/highlight.fs");
    unsigned int highlightVAO, highlightVBO;

    highlightBlockSetUp(highlightVAO, highlightVBO);
    // -------------

    generateWorld();

    while (!glfwWindowShouldClose(window))
    {
        float time = glfwGetTime();
        app.deltaTime = time - app.lastFrame;
        app.lastFrame = time;

        float fps = 1.0f / app.deltaTime;
        if (fps < 50)
            std::cout <<  " FPS: " << fps << std::endl;

        processInput(app, window);
        glClearColor(0.2f, 0.3f, 0.3f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind Texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureAtlas);

        // Use Shaders
        ourShader.use();
        app.cam.setProjection(ourShader);
        app.cam.setCamera(ourShader);

        int playerChunkX = app.cam.getCamPos().x / CHUNK_SIZE_X;
        int playerChunkZ = app.cam.getCamPos().z / CHUNK_SIZE_Z;

        int renderBoundXLeft = std::max(0, playerChunkX - app.player.renderDistance);
        int renderBoundXRight = std::min((int)WORLD_SIZE_X, playerChunkX + app.player.renderDistance + 1);
        int renderBoundZLeft = std::max(0, playerChunkZ - app.player.renderDistance);
        int renderBoundZRight = std::min((int)WORLD_SIZE_Z, playerChunkZ + app.player.renderDistance + 1);

        for (int chunkRow = renderBoundXLeft; chunkRow < renderBoundXRight; chunkRow++) {
            for (int chunkCell = renderBoundZLeft; chunkCell < renderBoundZRight; chunkCell++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(chunkRow * CHUNK_SIZE_X, 0, chunkCell * CHUNK_SIZE_Z));
                ourShader.setMatrix("model", model);

                Chunk& chunk = chunks[chunkRow][chunkCell];
                chunk.render(ourShader);
            }
        }

        // Highlight if looking at a block
        highlightBlock(app.cam, highlightShader, highlightVAO);

        // Cross hair stuff
        glDisable(GL_DEPTH_TEST);

        crosshairShader.use();
        glBindVertexArray(crossVAO);

        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, 4);
        glBindVertexArray(0);
        // -------------------------

        // Inv. Slot stuff
        invSlotShader.use();
        for (int i = 0; i < 9; i++) {
            if (i == app.player.currentInventorySlot)
                continue;

            glm::vec3 slotColor(0.75f, 0.75f, 0.75f);
            invSlotShader.setVec3("objectColor", slotColor);
            glBindVertexArray(app.invSlotVAOs[i]);
            glLineWidth(3.0f);
            glDrawArrays(GL_LINES, 0, 8);
        }

        // Selected Slot
        glm::vec3 slotColor(1.0f, 1.0f, 1.0f);
        invSlotShader.setVec3("objectColor", slotColor);
        glBindVertexArray(app.invSlotVAOs[app.player.currentInventorySlot]);
        glLineWidth(5.0f);
        glDrawArrays(GL_LINES, 0, 8);


        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
        // -------------------------

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &crossVAO);
    glDeleteBuffers(1, &crossVBO);

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    AppState* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    if (!app) return;

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
                neighborChunk.regenMesh();
            }

            int neighborZ = -1;
            if (localPos.z == 0) {
                neighborZ = chunkZ - 1;
            } else if (localPos.z == CHUNK_SIZE_Z - 1) {
                neighborZ = chunkZ + 1;
            }
            if (neighborZ >= 0 && neighborZ < WORLD_SIZE_Z) {
                Chunk& neighborChunk = chunks[chunkX][neighborZ];
                neighborChunk.regenMesh();
            }
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        if (currentRayResult.hit) {
            glm::ivec3 blockPos = currentRayResult.blockPos + currentRayResult.faceNormal;

            int chunkX = blockPos.x / CHUNK_SIZE_X;
            int chunkZ = blockPos.z / CHUNK_SIZE_Z;
            if (chunkX < 0 || chunkX >= WORLD_SIZE_X || chunkZ < 0 || chunkZ >= WORLD_SIZE_Z) return;

            Chunk& chunk = chunks[chunkX][chunkZ];
            glm::ivec3 localPos = blockPos - glm::ivec3(chunkX * CHUNK_SIZE_X, 0, chunkZ * CHUNK_SIZE_Z);

            if (localPos.x < 0 || localPos.y < 0 || localPos.z < 0 ||
                localPos.x >= CHUNK_SIZE_X || localPos.y >= CHUNK_SIZE_Y || localPos.z >= CHUNK_SIZE_Z) return;
            chunk.Add(localPos.x, localPos.y, localPos.z, app->player.heldBlock, true);

            int neighborX = -1;
            if (localPos.x == 0) {
                neighborX = chunkX - 1;
            }
            else  if (localPos.x == CHUNK_SIZE_X - 1) {
                neighborX = chunkX + 1;
            }
            if (neighborX >= 0 && neighborX < WORLD_SIZE_X) {
                Chunk& neighborChunk = chunks[neighborX][chunkZ];
                neighborChunk.regenMesh();
            }

            int neighborZ = -1;
            if (localPos.z == 0) {
                neighborZ = chunkZ - 1;
            }
            else if (localPos.z == CHUNK_SIZE_Z - 1) {
                neighborZ = chunkZ + 1;
            }
            if (neighborZ >= 0 && neighborZ < WORLD_SIZE_Z) {
                Chunk& neighborChunk = chunks[chunkX][neighborZ];
                neighborChunk.regenMesh();
            }
        }
    }
}

void processInput(AppState& app, GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        app.player.heldBlock = UVHelper::BlockType::GRASS;
        app.player.currentInventorySlot = 0;
        updateSlots(app);
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        app.player.heldBlock = UVHelper::BlockType::DIRT;
        app.player.currentInventorySlot = 1;
        updateSlots(app);
    }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        app.player.heldBlock = UVHelper::BlockType::STONE;
        app.player.currentInventorySlot = 2;
        updateSlots(app);
    }
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        app.player.heldBlock = UVHelper::BlockType::OAKLOG;
        app.player.currentInventorySlot = 3;
        updateSlots(app);
    }
    else if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
        app.player.heldBlock = UVHelper::BlockType::OAKLEAVES;
        app.player.currentInventorySlot = 4;
        updateSlots(app);
    }
    else if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
        app.player.heldBlock = UVHelper::BlockType::WATER;
        app.player.currentInventorySlot = 5;
        updateSlots(app);
    }
    else if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
        //app.player.heldBlock = UVHelper::BlockType::WATER;
        app.player.currentInventorySlot = 6;
        updateSlots(app);
    }
    else if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
        //app.player.heldBlock = UVHelper::BlockType::WATER;
        app.player.currentInventorySlot = 7;
        updateSlots(app);
    }
    else if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
        //app.player.heldBlock = UVHelper::BlockType::WATER;
        app.player.currentInventorySlot = 8;
        updateSlots(app);
    }

    bool sprinting = false;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        sprinting = true;
    app.cam.updateFOV(sprinting, app.deltaTime);
    app.cam.processCameraInput(window, app.deltaTime, sprinting);

    bool qKeyPressed = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
    if (qKeyPressed && !app.qKeyPressedLastFrame) {
        app.wireFramed = !app.wireFramed;
        glPolygonMode(GL_FRONT_AND_BACK, app.wireFramed ? GL_LINE : GL_FILL);
    }
    app.qKeyPressedLastFrame = qKeyPressed;
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

void updateSlots(AppState& app) {
    for (int i = 0; i < 9; ++i) {
        unsigned int invSlotVAO, invSlotVBO;
        invSlotSetUp(invSlotVAO, invSlotVBO, i, i == app.player.currentInventorySlot);
        app.invSlotVAOs[i] = invSlotVAO;
        app.invSlotVBOs[i] = invSlotVBO;
    }
}
void invSlotSetUp(unsigned int& VAO, unsigned int& VBO, int index, bool selected) {
    const float leftX = -0.3f;
    const float rightX = 0.3f;

    const float bottomY = -1.0f;
    const float topY = -0.88f;

    float leftOffset{};
    float rightOffset{};
    float verticalOffset{};
    if (selected) {
        leftOffset = 0.0025f;
        rightOffset = 0.0025f;
        verticalOffset = 0.005f;
    }
    else {
        leftOffset = index == 0 ? 0.0015f : 0.0f;
        rightOffset = index == 8 ? 0.0015f : 0.0f;
        verticalOffset = 0.003f;
    }
    const float invSlotXJumps = 0.6f / 9;

    float invSlotVertices[] = {
        leftX + (invSlotXJumps * index), topY,
        leftX + (invSlotXJumps * index), bottomY,

        leftX + (invSlotXJumps * (index+1)), topY,
        leftX + (invSlotXJumps * (index+1)), bottomY,

        leftX + (invSlotXJumps * index), topY - verticalOffset,
        leftX + (invSlotXJumps * (index+1)), topY - verticalOffset,

        leftX + (invSlotXJumps * index), bottomY + verticalOffset,
        leftX + (invSlotXJumps * (index + 1)), bottomY + verticalOffset
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(invSlotVertices), invSlotVertices, GL_STATIC_DRAW);

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