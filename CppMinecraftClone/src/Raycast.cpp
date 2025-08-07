#include "headerfiles/Raycast.hpp"
#include "headerfiles/Chunk.hpp"
#include "headerfiles/World.hpp"

RaycastResult currentRayResult = { false, {} };

bool isBlockSolid(glm::ivec3 pos) {
    int chunkX = pos.x / CHUNK_SIZE_X;
    int chunkZ = pos.z / CHUNK_SIZE_Z;
    if (chunkX < 0 || chunkX >= WORLD_SIZE_X || chunkZ < 0 || chunkZ >= WORLD_SIZE_Z) return false;

    Chunk& chunk = chunks[chunkX][chunkZ];
    glm::ivec3 localPos = pos - glm::ivec3(chunkX * CHUNK_SIZE_X, 0, chunkZ * CHUNK_SIZE_Z);

    if (localPos.x < 0 || localPos.y < 0 || localPos.z < 0 ||
        localPos.x >= CHUNK_SIZE_X || localPos.y >= CHUNK_SIZE_Y || localPos.z >= CHUNK_SIZE_Z) return false;

    return chunk.getBlock(localPos.x, localPos.y, localPos.z) != BlockType::AIR;
}

RaycastResult raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) {
    glm::vec3 rayDir = glm::normalize(direction);
    glm::vec3 resPos;

    float distanceTraveled = 0.0f;
    float stepSize = 0.05f;

    while (distanceTraveled <= maxDistance) {
        distanceTraveled += stepSize;
        resPos = origin + rayDir * distanceTraveled;

        resPos.x = round(resPos.x);
        resPos.y = round(resPos.y);
        resPos.z = round(resPos.z);

        if (isBlockSolid(resPos)) {
            currentRayResult = { true, (glm::ivec3)resPos };
            return currentRayResult;
        }
        else {
            currentRayResult = { false, {} };
        }
    }

    return { false, {} };
}

void highlightBlock(Camera& cam, Shader& highlightShader, unsigned int& highlightVAO) {
    glm::vec3 rayOrigin = cam.getCamPos();
    glm::vec3 rayDirection = cam.getCamTarget();

    RaycastResult result = raycast(rayOrigin, rayDirection, 8.0f);

    if (result.hit) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(result.blockPos));

        highlightShader.use();
        cam.setProjection(highlightShader);
        cam.setCamera(highlightShader);
        highlightShader.setMatrix("model", model);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(highlightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}