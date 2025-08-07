#include "headerfiles/World.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Chunk chunks[WORLD_SIZE_X][WORLD_SIZE_Z];

void generateWorld() {
    for (int chunkRow = 0; chunkRow < WORLD_SIZE_X; chunkRow++) {
        for (int chunkCell = 0; chunkCell < WORLD_SIZE_Z; chunkCell++) {
            Chunk& chunk = chunks[chunkRow][chunkCell];
            chunk.chunkNumberX = chunkRow;
            chunk.chunkNumberZ = chunkCell;
            chunk.generateChunk();
        }
    }

    for (int chunkRow = 0; chunkRow < WORLD_SIZE_X; chunkRow++) {
        for (int chunkCell = 0; chunkCell < WORLD_SIZE_Z; chunkCell++) {
            Chunk& chunk = chunks[chunkRow][chunkCell];
            chunk.buildMesh(chunks);
        }
    }
}

void renderWorld(Shader& ourShader) {
    for (int chunkRow = 0; chunkRow < WORLD_SIZE_X; chunkRow++) {
        for (int chunkCell = 0; chunkCell < WORLD_SIZE_Z; chunkCell++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(chunkRow * CHUNK_SIZE_X, 0, chunkCell * CHUNK_SIZE_Z));
            ourShader.setMatrix("model", model);

            Chunk& chunk = chunks[chunkRow][chunkCell];
            chunk.render();
        }
    }
}