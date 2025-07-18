#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "shaders/shader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

class Camera {
public:
    Camera(GLFWwindow* window) {
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetWindowUserPointer(window, this);
    }

    void processCameraInput(GLFWwindow* window, float &deltaTime) {
        const float camSpeed = 10.0f * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += camSpeed * cameraTarget;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= camSpeed * cameraTarget;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= camSpeed * glm::normalize(glm::cross(cameraTarget, cameraUp));
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += camSpeed * glm::normalize(glm::cross(cameraTarget, cameraUp));
    }

    void setCamera(Shader& ourShader) {
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraTarget, cameraUp);
        ourShader.setMatrix("view", view);
    }

    void setProjection(Shader& ourShader) {
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.f);
        ourShader.setMatrix("projection", projection);
    }

private:
    glm::vec3 cameraPos = glm::vec3(0.0f, 20.0f, 3.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float yaw = -90.0f;
    float pitch = 0.0f;
    float fov = 80.0f;

    float lastX = SCR_WIDTH / 2.0f;
    float lastY = SCR_HEIGHT / 2.0f;
    bool firstMouse = true;

    static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
        Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        if (cam) cam->processMouse(xpos, ypos);
    }

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        if (cam) cam->processScroll(yoffset);
    }

    void processMouse(double xpos, double ypos) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;

        lastX = xpos;
        lastY = ypos;

        const float sens = 0.05f;
        xoffset *= sens;
        yoffset *= sens;

        yaw += xoffset;
        pitch += yoffset;

        pitch = glm::clamp(pitch, -89.0f, 89.0f);

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        cameraTarget = glm::normalize(direction);
    }

    void processScroll(double yoffset) {
        fov -= (float)yoffset;
        fov = glm::clamp(fov, 1.0f, 90.0f);
    }
};
#endif