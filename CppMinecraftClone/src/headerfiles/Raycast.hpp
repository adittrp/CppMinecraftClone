#pragma once

#include <glm/glm.hpp>
#include "Camera.hpp"

struct RaycastResult {
    bool hit = false;
    glm::ivec3 blockPos = {};
    glm::ivec3 faceNormal = {};
};

extern RaycastResult currentRayResult;

bool isBlockSolid(glm::ivec3 pos);
RaycastResult raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance);
void highlightBlock(Camera& cam, Shader& highlightShader, unsigned int& highlightVAO);