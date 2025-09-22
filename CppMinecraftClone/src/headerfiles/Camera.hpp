#pragma once

#include "Shader.hpp"
#include "Player.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

class Camera {
public:
    Camera(GLFWwindow* window);

    Player curPlayer;

    void processCameraInput(GLFWwindow* window, float& deltaTime, bool sprinting);
    void setCamera(Shader& ourShader);
    void setProjection(Shader& ourShader);
    glm::vec3 getCamPos();
    glm::vec3 getCamTarget();
    glm::vec3 getCamUp();

    float getYaw();
    float getPitch();
    
    void updateFOV(bool isSprinting, float deltaTime);

    float Camera::horizontalCollision(float move);
    float groundedCheck(float currentVelocity);

private:
    // Gravity stuff
    bool isGrounded;
    float velocity;
    // ----

    glm::vec3 cameraPos;
    glm::vec3 cameraTarget;
    glm::vec3 cameraUp;

    float yaw;
    float pitch;

    float baseFOV;
    float fov;

    float lastX;
    float lastY;
    bool firstMouse;

    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    void processMouse(double xpos, double ypos);
    void processScroll(double yoffset);
};