#include "headerfiles/Camera.hpp"
#include "headerfiles/Constants.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "headerfiles/Chunk.hpp"
#include "headerfiles/World.hpp"

Camera::Camera(GLFWwindow* window) :
    cameraPos(glm::vec3(WORLD_SIZE_X / 2 * CHUNK_SIZE_X, 90.0f, WORLD_SIZE_Z / 2 * CHUNK_SIZE_Z)),
    cameraTarget(glm::vec3(0.0f, 0.0f, -1.0f)),
    cameraUp(glm::vec3(0.0f, 1.0f, 0.0f)),
    yaw(45.0f),
    pitch(0.0f),
    fov(90.0f),
    baseFOV(90.0f),
    lastX(SCR_WIDTH / 2.0f),
    lastY(SCR_HEIGHT / 2.0f),
    firstMouse(true),
    isGrounded(false),
    velocity(0.0f)
{
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, this);
}

void Camera::processCameraInput(GLFWwindow* window, float& deltaTime, bool sprinting) {
    const float camSpeed = (curPlayer.physicalState ? 5.0f : 15.0f) * deltaTime * (sprinting ? 1.35 : 1);

    glm::vec3 forward = glm::normalize(glm::vec3(cameraTarget.x, 0.0f, cameraTarget.z));
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

    glm::vec3 move(0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= forward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += right;

    if (glm::dot(move, move) > 0.0f) {
        move = glm::normalize(move);
        cameraPos += camSpeed * move;
    }

    if (curPlayer.physicalState) {
        float maxDistance = groundedCheck(velocity);

        if (!isGrounded || velocity > 0.0f) {
            velocity = std::max(velocity - (deltaTime / 4.0f), -0.5f);
            cameraPos.y += velocity;
        }
        else {
            if (maxDistance <= -0.001f) cameraPos.y += std::max(velocity, maxDistance);
            else velocity = 0.0f;

            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
                velocity = 0.1f;
        }
    }
    else {
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            cameraPos.y += camSpeed / 1.25f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cameraPos.y -= camSpeed / 1.25f;
    }
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

glm::vec3 Camera::getCamTarget() {
    return cameraTarget;
}

glm::vec3 Camera::getCamUp() {
    return cameraUp;
}

float Camera::getYaw() {
    return yaw;
}

float Camera::getPitch() {
    return pitch;
}

void Camera::updateFOV(bool isSprinting, float deltaTime) {
    const double sprintScale = 1.12;
    const double targetFov = isSprinting ? (baseFOV * sprintScale) : baseFOV;

    const double smoothingSpeed = isSprinting ? 12.0 : 8.0;


    const double alpha = 1.0 - std::exp(-smoothingSpeed * std::max(0.0f, deltaTime));

    fov += (targetFov - fov) * alpha;

    float minFov = baseFOV;
    float maxFov = baseFOV * 1.10f;
    fov = glm::clamp(fov, minFov, maxFov);
}

float Camera::horizontalCollision(float move) {

}

float Camera::groundedCheck(float currentVelocity) {
    isGrounded = false;
    float maxDistance = 0.0f;

    float playerFeet = cameraPos.y - 1.8f + currentVelocity;

    int chunkX = round(cameraPos.x) / CHUNK_SIZE_X;
    int chunkZ = round(cameraPos.z) / CHUNK_SIZE_Z;
    if (chunkX < 0 || chunkX >= WORLD_SIZE_X || chunkZ < 0 || chunkZ >= WORLD_SIZE_Z)
        return 0.0f;

    Chunk& chunk = chunks[chunkX][chunkZ];
    glm::ivec3 localPos = glm::ivec3(round(cameraPos.x), round(playerFeet), round(cameraPos.z))
        - glm::ivec3(chunkX * CHUNK_SIZE_X, 0, chunkZ * CHUNK_SIZE_Z);

    if (localPos.x < 0 || localPos.y < 0 || localPos.z < 0 ||
        localPos.x >= CHUNK_SIZE_X || localPos.y >= CHUNK_SIZE_Y || localPos.z >= CHUNK_SIZE_Z) {
        std::cout << "ah2h" << std::endl;
        std::cout << localPos.x << " " << localPos.y << " " << localPos.z << std::endl;
        return 0.0f;
    }

    if (chunk.getBlock(localPos.x, localPos.y, localPos.z) != UVHelper::BlockType::AIR) {
        isGrounded = true;
        float blockTop = round(playerFeet) + 1.0f;
        maxDistance = blockTop - playerFeet;
    }

    return maxDistance;
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
    baseFOV -= (float)yoffset;
    baseFOV = glm::clamp(baseFOV, 1.0f, 90.0f);

    fov = baseFOV;
}