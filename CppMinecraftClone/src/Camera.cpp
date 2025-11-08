#include "headerfiles/Camera.hpp"
#include "headerfiles/Constants.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "headerfiles/Chunk.hpp"
#include "headerfiles/World.hpp"

Camera::Camera(GLFWwindow* window, Player& player) :
    cameraPos(glm::vec3(WORLD_SIZE_X / 2 * CHUNK_SIZE_X, 90.0f, WORLD_SIZE_Z / 2 * CHUNK_SIZE_Z)),
    //cameraPos(glm::vec3(1.0f, 90.0f, 1.0f)),
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
    velocity(0.0f),
    curPlayer(player)
{
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, this);
}

void Camera::processCameraInput(GLFWwindow* window, float& deltaTime, bool sprinting) {
    const float camSpeed = (curPlayer.physicalState ? 4.0f : 15.0f) * deltaTime * (sprinting ? 1.25f : 1);

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
            velocity = std::max(velocity - (deltaTime / 3.0f), -0.5f);
            if (velocity > 0.0f) headerCheck(velocity);

            cameraPos.y += velocity;
        }
        else {
            if (maxDistance <= -0.00001f) cameraPos.y += std::max(velocity, maxDistance);
            else velocity = 0.0f;

            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
                velocity = 0.1f;
        }

        if (glm::dot(move, move) > 0.0f) {
            move = glm::normalize(move) * camSpeed;

            glm::vec3 tryMove = move;

            if (move.x != 0.0f) {
                glm::vec3 moveX{ move.x, 0.0f, 0.0f };
                if (horizontalCollision(moveX)) tryMove.x = 0.0f;
            }

            if (move.z != 0.0f) {
                glm::vec3 moveZ{ 0.0f, 0.0f, move.z };
                if (horizontalCollision(moveZ)) tryMove.z = 0.0f;
            }

            cameraPos += tryMove;
        }
    }
    else {
        velocity = 0.0f;

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

bool Camera::horizontalCollision(glm::vec3 move) {
    const float playerWidth{ 0.2f };

    for (int dx = -1; dx <= 1; dx += 2) {
        for (int dz = -1; dz <= 1; dz += 2) {
            glm::vec3 offset = glm::vec3(playerWidth * dx, 0.0f, playerWidth * dz);
            glm::vec3 newPos = cameraPos + move + offset;

            int chunkX = round(newPos.x) / CHUNK_SIZE_X;
            int chunkZ = round(newPos.z) / CHUNK_SIZE_Z;
            if (chunkX < 0 || chunkX >= WORLD_SIZE_X || chunkZ < 0 || chunkZ >= WORLD_SIZE_Z) continue;

            Chunk& chunk = chunks[chunkX][chunkZ];
            for (float yOffset : { -1.79f, -0.5f }) {
                glm::ivec3 localPos = glm::ivec3(round(newPos.x), round(cameraPos.y + yOffset), round(newPos.z))
                    - glm::ivec3(chunkX * CHUNK_SIZE_X, 0, chunkZ * CHUNK_SIZE_Z);

                if (chunk.getBlock(localPos.x, localPos.y, localPos.z) != UVHelper::BlockType::AIR) return true;
            }
        }
    }

    return false;
}

float Camera::groundedCheck(float currentVelocity) {
    isGrounded = false;
    float maxDistance = 0.0f;

    const float playerWidth = 0.2f;
    const float playerFeet = cameraPos.y - 1.8f + currentVelocity;

    for (int x = 0; x < 2; x++) {
        for (int z = 0; z < 2; z++) {
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

void Camera::headerCheck(float& currentVelocity) {
    const float playerWidth = 0.2f;
    const float playerHead = cameraPos.y + 0.1f + currentVelocity;

    for (int x = 0; x < 2; x++) {
        for (int z = 0; z < 2; z++) {
            glm::ivec3 newPos(round(cameraPos.x + playerWidth * (x == 0 ? -1 : 1)), round(playerHead), round(cameraPos.z + playerWidth * (z == 0 ? -1 : 1)));

            int chunkX = newPos.x / CHUNK_SIZE_X;
            int chunkZ = newPos.z / CHUNK_SIZE_Z;
            if (chunkX < 0 || chunkX >= WORLD_SIZE_X || chunkZ < 0 || chunkZ >= WORLD_SIZE_Z) return;

            Chunk& chunk = chunks[chunkX][chunkZ];
            glm::ivec3 localPos = newPos - glm::ivec3(chunkX * CHUNK_SIZE_X, 0, chunkZ * CHUNK_SIZE_Z);

            if (chunk.getBlock(localPos.x, localPos.y, localPos.z) != UVHelper::BlockType::AIR) {
                velocity = 0.0f;
                return;
            }
        }
    }
}

bool Camera::blockPlaceCheck(glm::ivec3 blockPlacePos) {
    const float playerWidth = 0.2f;

    for (int x = 0; x < 2; x++) {
        for (int z = 0; z < 2; z++) {
            if (!(round(cameraPos.x + playerWidth * (x == 0 ? -1 : 1)) == blockPlacePos.x && round(cameraPos.z + playerWidth * (z == 0 ? -1 : 1)) == blockPlacePos.z)) continue;
            if (round(cameraPos.y) == blockPlacePos.y || round(cameraPos.y - 1.0f) == blockPlacePos.y) return false;
        }
    }

    return true;
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