#include "headerfiles/Camera.hpp"
#include "headerfiles/Constants.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "headerfiles/Chunk.hpp"
#include "headerfiles/World.hpp"

Camera::Camera(GLFWwindow* window) :
    cameraPos(glm::vec3(WORLD_SIZE_X / 2 * CHUNK_SIZE_X, 90.0f, WORLD_SIZE_Z / 2 * CHUNK_SIZE_Z)),
    //cameraPos(glm::vec3(0, 90.0f, 0)),
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
    const float camSpeed = (curPlayer.physicalState ? 5.0f : 15.0f) * deltaTime * (sprinting ? 1.25f : 1);

    glm::vec3 forward = glm::normalize(glm::vec3(cameraTarget.x, 0.0f, cameraTarget.z));
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

    glm::vec3 move(0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= forward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += right;


    if (curPlayer.physicalState) {
        float maxDistance = groundedCheck(velocity);

        if (!isGrounded || velocity > 0.0f) {
            velocity = std::max(velocity - (deltaTime / 4.0f), -0.5f);
            cameraPos.y += velocity;
        }
        else {
            if (maxDistance <= -0.00001f) cameraPos.y += std::max(velocity, maxDistance);
            else velocity = 0.0f;

            // TODO (Need to check if there is a block above the head
            // TODO FIX DO IMMEDIETLy
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
                velocity = 0.1f;
        }

        if (glm::dot(move, move) > 0.0f) {
            move = glm::normalize(move) * camSpeed;
            horizontalCollision(move);

            if (!collidingX) cameraPos.x += move.x;
            if (!collidingZ) cameraPos.z += move.z;
        }
    }
    else {
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            cameraPos.y += camSpeed / 1.25f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cameraPos.y -= camSpeed / 1.25f;

        if (glm::dot(move, move) > 0.0f) {
            move = glm::normalize(move);
            cameraPos += camSpeed * move;
        }
    }
}

void Camera::setCamera(Shader& ourShader) {
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraTarget, cameraUp);
    ourShader.setMatrix("view", view);
}

void Camera::setProjection(Shader& ourShader) {
    glm::mat4 projection = glm::perspective(glm::radians(fov),
        (double)SCR_WIDTH / (double)SCR_HEIGHT,
        0.1,
        1000.0);
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

double Camera::getYaw() {
    return yaw;
}

double Camera::getPitch() {
    return pitch;
}

void Camera::updateFOV(bool isSprinting, float deltaTime) {
    const double sprintScale = 1.12f;
    const double targetFov = isSprinting ? (baseFOV * sprintScale) : baseFOV;

    const double smoothingSpeed = isSprinting ? 12.0 : 8.0;


    const double alpha = 1.0f - std::exp(-smoothingSpeed * std::max(0.0f, deltaTime));

    fov += (targetFov - fov) * alpha;

    double minFov = baseFOV;
    double maxFov = baseFOV * 1.10f;
    fov = glm::clamp(fov, minFov, maxFov);
}

void Camera::horizontalCollision(glm::vec3 move) {
    collidingX = false;
    collidingZ = false;
    glm::vec3 maxDistance = move;


    // IMPROVE (When you move exclusively in the x direction, there technically is no z width which causes some clipping) FIX TODO
    // TODO
    const float playerWidthX = 0.2f * (move.x < 0 ? -1 : 1);
    const float playerWidthZ = 0.2f * (move.z < 0 ? -1 : 1);

    glm::vec3 newPos = cameraPos + move + glm::vec3(playerWidthX, 0.0f, playerWidthZ);

    // Check X
    int chunkX = round(newPos.x) / CHUNK_SIZE_X;
    int chunkZ = round(cameraPos.z) / CHUNK_SIZE_Z;
    if (chunkX < 0 || chunkX >= WORLD_SIZE_X || chunkZ < 0 || chunkZ >= WORLD_SIZE_Z) return;

    Chunk& rChunkX = chunks[chunkX][chunkZ];
    glm::ivec3 localPos = glm::ivec3(round(newPos.x), round(cameraPos.y - 1.79f), round(cameraPos.z))
        - glm::ivec3(chunkX * CHUNK_SIZE_X, 0, chunkZ * CHUNK_SIZE_Z);

    if (rChunkX.getBlock(localPos.x, localPos.y, localPos.z) != UVHelper::BlockType::AIR) collidingX = true;

    // Check Z
    chunkX = round(cameraPos.x) / CHUNK_SIZE_X;
    chunkZ = round(newPos.z) / CHUNK_SIZE_Z;
    if (chunkX < 0 || chunkX >= WORLD_SIZE_X || chunkZ < 0 || chunkZ >= WORLD_SIZE_Z) return;

    Chunk& rChunkZ = chunks[chunkX][chunkZ];
    localPos = glm::ivec3(round(cameraPos.x), round(cameraPos.y - 1.79f), round(newPos.z))
        - glm::ivec3(chunkX * CHUNK_SIZE_X, 0, chunkZ * CHUNK_SIZE_Z);

    if (rChunkZ.getBlock(localPos.x, localPos.y, localPos.z) != UVHelper::BlockType::AIR) collidingZ = true;

    // Check X 2
    chunkX = round(newPos.x) / CHUNK_SIZE_X;
    chunkZ = round(cameraPos.z) / CHUNK_SIZE_Z;
    if (chunkX < 0 || chunkX >= WORLD_SIZE_X || chunkZ < 0 || chunkZ >= WORLD_SIZE_Z) return;

    Chunk& rChunkX2 = chunks[chunkX][chunkZ];
    localPos = glm::ivec3(round(newPos.x), round(cameraPos.y - 0.79f), round(cameraPos.z))
        - glm::ivec3(chunkX * CHUNK_SIZE_X, 0, chunkZ * CHUNK_SIZE_Z);

    if (rChunkX2.getBlock(localPos.x, localPos.y, localPos.z) != UVHelper::BlockType::AIR) collidingX = true;

    // Check Z 2
    chunkX = round(cameraPos.x) / CHUNK_SIZE_X;
    chunkZ = round(newPos.z) / CHUNK_SIZE_Z;
    if (chunkX < 0 || chunkX >= WORLD_SIZE_X || chunkZ < 0 || chunkZ >= WORLD_SIZE_Z) return;

    Chunk& rChunkZ2 = chunks[chunkX][chunkZ];
    localPos = glm::ivec3(round(cameraPos.x), round(cameraPos.y - 0.79f), round(newPos.z))
        - glm::ivec3(chunkX * CHUNK_SIZE_X, 0, chunkZ * CHUNK_SIZE_Z);

    if (rChunkZ2.getBlock(localPos.x, localPos.y, localPos.z) != UVHelper::BlockType::AIR) collidingZ = true;
}

float Camera::groundedCheck(float currentVelocity) {
    isGrounded = false;
    float maxDistance = 0.0f;

    const float playerWidth = 0.2f;
    const float playerFeet = cameraPos.y - 1.8f + currentVelocity;

    for (int x = 0; x <= 2; x++) {
        for (int z = 0; z <= 2; z++) {
            glm::ivec3 newPos(round(cameraPos.x + playerWidth * (x == 0 ? -1 : 1)), round(playerFeet), round(cameraPos.z + playerWidth * (z == 0 ? -1 : 1)));

            int chunkX = newPos.x / CHUNK_SIZE_X;
            int chunkZ = newPos.z / CHUNK_SIZE_Z;
            if (chunkX < 0 || chunkX >= WORLD_SIZE_X || chunkZ < 0 || chunkZ >= WORLD_SIZE_Z) return 0.0f;

            Chunk& chunk = chunks[chunkX][chunkZ];
            glm::ivec3 localPos = newPos - glm::ivec3(chunkX * CHUNK_SIZE_X, 0, chunkZ * CHUNK_SIZE_Z);

            if (chunk.getBlock(localPos.x, localPos.y, localPos.z) != UVHelper::BlockType::AIR) {
                isGrounded = true;
                goto end;
            }
        }
    }

end:
    if (isGrounded) {
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

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    const float sens = 0.05f;
    xoffset *= sens;
    yoffset *= sens;

    yaw += xoffset;
    pitch += yoffset;

    pitch = glm::clamp(pitch, -89.0, 89.0);

    glm::dvec3 direction = {
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))
    };

    cameraTarget = glm::normalize(direction);
}

void Camera::processScroll(double yoffset) {
    baseFOV -= yoffset;
    baseFOV = glm::clamp(baseFOV, 1.0, 90.0);

    fov = baseFOV;
}