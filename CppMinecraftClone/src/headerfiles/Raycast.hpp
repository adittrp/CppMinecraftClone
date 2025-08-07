#ifndef RAYCAST_HPP
#define RAYCAST_HPP

#include <glm/glm.hpp>
#include "Camera.hpp"

struct RaycastResult {
    bool hit;
    glm::ivec3 blockPos;
};

extern RaycastResult currentRayResult;

bool isBlockSolid(glm::ivec3 pos);
RaycastResult raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance);
void highlightBlock(Camera& cam, Shader& highlightShader, unsigned int& highlightVAO);

#endif