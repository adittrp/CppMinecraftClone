#include "headerfiles/Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Camera::Camera(GLFWwindow* window) :
    cameraPos(glm::vec3(0.0f, 100.0f, 3.0f)),
    cameraTarget(glm::vec3(0.0f, 0.0f, -1.0f)),
    cameraUp(glm::vec3(0.0f, 1.0f, 0.0f)),
    yaw(-90.0f),
    pitch(0.0f),
    fov(80.0f),
    lastX(SCR_WIDTH / 2.0f),
    lastY(SCR_HEIGHT / 2.0f),
    firstMouse(true)
{
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, this);
}

void Camera::processCameraInput(GLFWwindow* window, float& deltaTime) {
    const float camSpeed = 50.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += camSpeed * cameraTarget;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= camSpeed * cameraTarget;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= camSpeed * glm::normalize(glm::cross(cameraTarget, cameraUp));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += camSpeed * glm::normalize(glm::cross(cameraTarget, cameraUp));
}

void Camera::setCamera(Shader& ourShader) {
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraTarget, cameraUp);
    ourShader.setMatrix("view", view);
}

void Camera::setProjection(Shader& ourShader) {
    glm::mat4 projection = glm::perspective(glm::radians(fov),
        (float)SCR_WIDTH / (float)SCR_HEIGHT,
        0.1f,
        1000.f);
    ourShader.setMatrix("projection", projection);
}

glm::vec3 Camera::getCamPos() {
    return cameraPos;
}

void Camera::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (cam) cam->processMouse(xpos, ypos);
}

void Camera::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (cam) cam->processScroll(yoffset);
}

void Camera::processMouse(double xpos, double ypos) {
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

void Camera::processScroll(double yoffset) {
    fov -= (float)yoffset;
    fov = glm::clamp(fov, 1.0f, 90.0f);
}
