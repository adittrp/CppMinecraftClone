/*#include "headerfiles/Chunk.hpp"

#define STB_PERLIN_IMPLEMENTATION
#include "includes/stb_perlin.h"

#include <cstdint>

#include <glad/glad.h>
#include <iostream>

#include <glm/glm.hpp>

#include "headerfiles/World.hpp"


Chunk::~Chunk() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Chunk::regenMesh() {
    if (!meshFuture.valid()) {
        meshFuture = std::async(std::launch::async, [this] { this->generateMesh(); });
    }
}

void Chunk::Add(int x, int y, int z, UVHelper::BlockType blockType, bool regenerateMesh) {
    if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z)
        return;
    ChunkData[x][y][z] = blockType;

    if (regenerateMesh) {
        regenMesh();
    }
}

void Chunk::removeBlock(int x, int y, int z) {
    Add(x, y, z, UVHelper::BlockType::AIR, true);
}

UVHelper::BlockType Chunk::getBlock(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z)
        return UVHelper::BlockType::AIR;
    return ChunkData[x][y][z];
}

float Chunk::fbm(float x, float z) {
    float total = 0.0f;
    float frequency = 0.01f; 
    float amplitude = 1.0f;
    float persistence = 0.5f;    
    float lacunarity = 2.0f; 
    int octaves = 6;
    float maxAmplitude = 0.0f;

    for (int i = 0; i < octaves; i++) {
        total += stb_perlin_noise3(x * frequency, 0, z * frequency, 0, 0, 0) * amplitude;
        maxAmplitude += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    total /= maxAmplitude; 
    return total;
}


void Chunk::generateChunk() {
    for (int x = 0; x < CHUNK_SIZE_X; ++x) {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
            float xPos = (x + chunkNumberX * CHUNK_SIZE_X);
            float zPos = (z + chunkNumberZ * CHUNK_SIZE_Z);

            float noiseValue = fbm(xPos, zPos);
            noiseValue = (noiseValue + 1.0f) / 2.0f;

            int minHeight = CHUNK_SIZE_Y / 8;
            int maxHeight = CHUNK_SIZE_Y / 2;

            int height = minHeight + static_cast<int>(noiseValue * (maxHeight - minHeight));

            for (int y = 0; y < height; ++y) {
                if (y == height - 1) {
                    Add(x, y, z, UVHelper::BlockType::GRASS);
                }
                else if (y > height - 5) {
                    Add(x, y, z, UVHelper::BlockType::DIRT);
                }
                else {
                    Add(x, y, z, UVHelper::BlockType::STONE);
                }
            }
        }
    }
}


bool Chunk::isNeighborClear(UVHelper::BlockType neighborBlock) const {
    if (neighborBlock == UVHelper::BlockType::AIR ||
        neighborBlock == UVHelper::BlockType::WATER ||
        neighborBlock == UVHelper::BlockType::OAKLEAVES) return true;

    return false;
}

bool Chunk::isTransparent(UVHelper::BlockType blockType) const {
    if (blockType == UVHelper::BlockType::WATER ||
        blockType == UVHelper::BlockType::OAKLEAVES) return true;

    return false;
}

void Chunk::generateMesh() {
    std::vector<float> newVertices;
    std::vector<unsigned int> newIndices;
    unsigned int indexCount = 0;

    for (int x = 0; x < CHUNK_SIZE_X; ++x) {
        for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
            for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
                UVHelper::BlockType type = getBlock(x, y, z);
                if (type == UVHelper::BlockType::AIR) continue;

                for (int face = 0; face < 6; face++) {
                    int nx = x, ny = y, nz = z;

                    // Left
                    if (face == 0) nz--;
                    // Right 
                    if (face == 1) nz++;
                    // Front
                    if (face == 2) nx--;
                    // Back
                    if (face == 3) nx++;
                    // Bottom 
                    if (face == 4) ny--;
                    // Top
                    if (face == 5) ny++;

                    bool shouldRenderFace = true;
                    if (nx < 0) {
                        if (chunkNumberX > 0) {
                            Chunk& neighbor = chunks[chunkNumberX - 1][chunkNumberZ];
                            shouldRenderFace = isNeighborClear(neighbor.getBlock(CHUNK_SIZE_X - 1, y, z));
                        }
                    }
                    else if (nx >= CHUNK_SIZE_X) {
                        if (chunkNumberX + 1 < WORLD_SIZE_X) {
                            Chunk& neighbor = chunks[chunkNumberX + 1][chunkNumberZ];
                            shouldRenderFace = isNeighborClear(neighbor.getBlock(0, y, z));
                        }
                    }
                    else if (nz < 0) {
                        if (chunkNumberZ > 0) {
                            Chunk& neighbor = chunks[chunkNumberX][chunkNumberZ - 1];
                            shouldRenderFace = isNeighborClear(neighbor.getBlock(x, y, CHUNK_SIZE_Z - 1));
                        }
                    }
                    else if (nz >= CHUNK_SIZE_Z) {
                        if (chunkNumberZ + 1 < WORLD_SIZE_Z) {
                            Chunk& neighbor = chunks[chunkNumberX][chunkNumberZ + 1];
                            shouldRenderFace = isNeighborClear(neighbor.getBlock(x, y, 0));
                        }
                    }
                    else {
                        shouldRenderFace = isNeighborClear(getBlock(nx, ny, nz));
                    }

                    if (shouldRenderFace) {
                        UVHelper::BlockUV uvSet = UVHelper::blockTextures[type];
                        int tileIndex = 0;

                        if (face == 5) {
                            tileIndex = uvSet.top;
                        }
                        else if (face == 4) {
                            tileIndex = uvSet.bottom;
                        }
                        else {
                            tileIndex = uvSet.side;
                        }
                        UVHelper::UVCoords uv = UVHelper::getUVCoords(tileIndex, 3, 16, 48);

                        glm::vec2 uvCoords[6];

                        switch (face) {
                        case 0: // left
                            uvCoords[0] = { uv.uMin, uv.vMax };
                            uvCoords[1] = { uv.uMax, uv.vMin };
                            uvCoords[2] = { uv.uMax, uv.vMax };

                            uvCoords[3] = { uv.uMax, uv.vMin };
                            uvCoords[4] = { uv.uMin, uv.vMax };
                            uvCoords[5] = { uv.uMin, uv.vMin };
                            break;
                        case 1: // right
                            uvCoords[0] = { uv.uMin, uv.vMax };
                            uvCoords[1] = { uv.uMax, uv.vMax };
                            uvCoords[2] = { uv.uMax, uv.vMin };

                            uvCoords[3] = { uv.uMax, uv.vMin };
                            uvCoords[4] = { uv.uMin, uv.vMin };
                            uvCoords[5] = { uv.uMin, uv.vMax };
                            break;
                        case 2: // front
                            uvCoords[0] = { uv.uMin, uv.vMin };
                            uvCoords[1] = { uv.uMax, uv.vMin };
                            uvCoords[2] = { uv.uMax, uv.vMax };
                            uvCoords[3] = { uv.uMax, uv.vMax };
                            uvCoords[4] = { uv.uMin, uv.vMax };
                            uvCoords[5] = { uv.uMin, uv.vMin };
                            break;
                        case 3: // back
                            uvCoords[0] = { uv.uMin, uv.vMin };
                            uvCoords[1] = { uv.uMax, uv.vMax };
                            uvCoords[2] = { uv.uMax, uv.vMin };

                            uvCoords[3] = { uv.uMax, uv.vMax };
                            uvCoords[4] = { uv.uMin, uv.vMin };
                            uvCoords[5] = { uv.uMin, uv.vMax };
                            break;
                        case 4: // bottom 
                            uvCoords[0] = { uv.uMin, uv.vMax };
                            uvCoords[1] = { uv.uMax, uv.vMax };
                            uvCoords[2] = { uv.uMax, uv.vMin };

                            uvCoords[3] = { uv.uMax, uv.vMin };
                            uvCoords[4] = { uv.uMin, uv.vMin };
                            uvCoords[5] = { uv.uMin, uv.vMax };
                            break;
                        case 5: // top
                            uvCoords[0] = { uv.uMin, uv.vMin };
                            uvCoords[1] = { uv.uMax, uv.vMax };
                            uvCoords[2] = { uv.uMax, uv.vMin };

                            uvCoords[3] = { uv.uMax, uv.vMax };
                            uvCoords[4] = { uv.uMin, uv.vMin };
                            uvCoords[5] = { uv.uMin, uv.vMax };
                            break;
                        }


                        for (int vertex = 0; vertex < 6; ++vertex) {
                            int base = vertex * 6;

                            newVertices.push_back(vertexData[face][base + 0] + x);
                            newVertices.push_back(vertexData[face][base + 1] + y);
                            newVertices.push_back(vertexData[face][base + 2] + z);

                            // Normal
                            newVertices.push_back(vertexData[face][base + 3]);
                            newVertices.push_back(vertexData[face][base + 4]);
                            newVertices.push_back(vertexData[face][base + 5]);

                            // UV from atlas
                            newVertices.push_back(uvCoords[vertex].x);
                            newVertices.push_back(uvCoords[vertex].y);

                            newIndices.push_back((indexCount)++);
                        }
                    }
                }
            }
        }
    }

    vertices = std::move(newVertices);
    indices = std::move(newIndices);
}

void Chunk::buildMesh() {
    // Solid
    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }
    else {
        glBindVertexArray(VAO);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_DYNAMIC_DRAW);
}


void Chunk::render() {
    if (VAO == 0) {
        regenMesh();
    }

    if (meshFuture.valid() && meshFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
        meshFuture.get();
        buildMesh();
        meshFuture = std::future<void>();
    }
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
}
*/

#include "headerfiles/Chunk.hpp"

#define STB_PERLIN_IMPLEMENTATION
#include "includes/stb_perlin.h"

#include <cstdint>

#include <glad/glad.h>
#include <iostream>

#include <glm/glm.hpp>

#include "headerfiles/World.hpp"


Chunk::~Chunk() {
    glDeleteVertexArrays(1, &solidVAO);
    glDeleteBuffers(1, &solidVBO);
    glDeleteBuffers(1, &solidEBO);

    glDeleteVertexArrays(1, &transparentVAO);
    glDeleteBuffers(1, &transparentVBO);
    glDeleteBuffers(1, &transparentEBO);
}

void Chunk::regenMesh() {
    if (!meshFuture.valid()) {
        meshFuture = std::async(std::launch::async, [this] { this->generateMesh(); });
    }
}

void Chunk::Add(int x, int y, int z, UVHelper::BlockType blockType, bool regenerateMesh) {
    if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z)
        return;
    ChunkData[x][y][z] = blockType;

    if (regenerateMesh) {
        regenMesh();
    }
}

void Chunk::removeBlock(int x, int y, int z) {
    Add(x, y, z, UVHelper::BlockType::AIR, true);
}

UVHelper::BlockType Chunk::getBlock(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z)
        return UVHelper::BlockType::AIR;
    return ChunkData[x][y][z];
}

float Chunk::fbm(float x, float z) {
    float total = 0.0f;
    float frequency = 0.01f; 
    float amplitude = 1.0f;
    float persistence = 0.5f;    
    float lacunarity = 2.0f; 
    int octaves = 6;
    float maxAmplitude = 0.0f;

    for (int i = 0; i < octaves; i++) {
        total += stb_perlin_noise3(x * frequency, 0, z * frequency, 0, 0, 0) * amplitude;
        maxAmplitude += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    total /= maxAmplitude; 
    return total;
}


void Chunk::generateChunk() {
    for (int x = 0; x < CHUNK_SIZE_X; ++x) {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
            float xPos = (x + chunkNumberX * CHUNK_SIZE_X);
            float zPos = (z + chunkNumberZ * CHUNK_SIZE_Z);

            float noiseValue = fbm(xPos, zPos);
            noiseValue = (noiseValue + 1.0f) / 2.0f;

            int minHeight = CHUNK_SIZE_Y / 8;
            int maxHeight = CHUNK_SIZE_Y / 2;

            int height = minHeight + static_cast<int>(noiseValue * (maxHeight - minHeight));

            for (int y = 0; y < height; ++y) {
                if (y == height - 1) {
                    Add(x, y, z, UVHelper::BlockType::GRASS);
                }
                else if (y > height - 5) {
                    Add(x, y, z, UVHelper::BlockType::DIRT);
                }
                else {
                    Add(x, y, z, UVHelper::BlockType::STONE);
                }
            }
        }
    }
}


bool Chunk::isNeighborClear(UVHelper::BlockType neighborBlock, UVHelper::BlockType currentBlock) const {
    if (isTransparent(currentBlock) && neighborBlock == currentBlock) return false;


    if (neighborBlock == UVHelper::BlockType::AIR ||
        neighborBlock == UVHelper::BlockType::WATER ||
        neighborBlock == UVHelper::BlockType::OAKLEAVES) return true;

    return false;
}

bool Chunk::isTransparent(UVHelper::BlockType blockType) const {
    if (blockType == UVHelper::BlockType::WATER ||
        blockType == UVHelper::BlockType::OAKLEAVES) return true;

    return false;
}

void Chunk::generateMesh() {
    std::vector<float> newSolidVertices;
    std::vector<unsigned int> newSolidIndices;

    std::vector<float> newTransparentVertices;
    std::vector<unsigned int> newTransparentIndices;

    unsigned int solidIndexCount = 0;
    unsigned int transparentIndexCount = 0;

    for (int x = 0; x < CHUNK_SIZE_X; ++x) {
        for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
            for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
                UVHelper::BlockType type = getBlock(x, y, z);
                if (type == UVHelper::BlockType::AIR) continue;

                for (int face = 0; face < 6; face++) {
                    int nx = x, ny = y, nz = z;

                    // Left
                    if (face == 0) nz--;
                    // Right 
                    if (face == 1) nz++;
                    // Front
                    if (face == 2) nx--;
                    // Back
                    if (face == 3) nx++;
                    // Bottom 
                    if (face == 4) ny--;
                    // Top
                    if (face == 5) ny++;

                    bool shouldRenderFace = true;
                    if (nx < 0) {
                        if (chunkNumberX > 0) {
                            Chunk& neighbor = chunks[chunkNumberX - 1][chunkNumberZ];
                            shouldRenderFace = isNeighborClear(neighbor.getBlock(CHUNK_SIZE_X - 1, y, z), type);
                        }
                    }
                    else if (nx >= CHUNK_SIZE_X) {
                        if (chunkNumberX + 1 < WORLD_SIZE_X) {
                            Chunk& neighbor = chunks[chunkNumberX + 1][chunkNumberZ];
                            shouldRenderFace = isNeighborClear(neighbor.getBlock(0, y, z), type);
                        }
                    }
                    else if (nz < 0) {
                        if (chunkNumberZ > 0) {
                            Chunk& neighbor = chunks[chunkNumberX][chunkNumberZ - 1];
                            shouldRenderFace = isNeighborClear(neighbor.getBlock(x, y, CHUNK_SIZE_Z - 1), type);
                        }
                    }
                    else if (nz >= CHUNK_SIZE_Z) {
                        if (chunkNumberZ + 1 < WORLD_SIZE_Z) {
                            Chunk& neighbor = chunks[chunkNumberX][chunkNumberZ + 1];
                            shouldRenderFace = isNeighborClear(neighbor.getBlock(x, y, 0), type);
                        }
                    }
                    else {
                        shouldRenderFace = isNeighborClear(getBlock(nx, ny, nz), type);
                    }

                    if (shouldRenderFace) {
                        UVHelper::BlockUV uvSet = UVHelper::blockTextures[type];
                        int tileIndex = 0;

                        if (face == 5) {
                            tileIndex = uvSet.top;
                        }
                        else if (face == 4) {
                            tileIndex = uvSet.bottom;
                        }
                        else {
                            tileIndex = uvSet.side;
                        }
                        UVHelper::UVCoords uv = UVHelper::getUVCoords(tileIndex, 3, 16, 48);

                        glm::vec2 uvCoords[6];

                        switch (face) {
                        case 0: // left
                            uvCoords[0] = { uv.uMin, uv.vMax };
                            uvCoords[1] = { uv.uMax, uv.vMin };
                            uvCoords[2] = { uv.uMax, uv.vMax };

                            uvCoords[3] = { uv.uMax, uv.vMin };
                            uvCoords[4] = { uv.uMin, uv.vMax };
                            uvCoords[5] = { uv.uMin, uv.vMin };
                            break;
                        case 1: // right
                            uvCoords[0] = { uv.uMin, uv.vMax };
                            uvCoords[1] = { uv.uMax, uv.vMax };
                            uvCoords[2] = { uv.uMax, uv.vMin };

                            uvCoords[3] = { uv.uMax, uv.vMin };
                            uvCoords[4] = { uv.uMin, uv.vMin };
                            uvCoords[5] = { uv.uMin, uv.vMax };
                            break;
                        case 2: // front
                            uvCoords[0] = { uv.uMin, uv.vMin };
                            uvCoords[1] = { uv.uMax, uv.vMin };
                            uvCoords[2] = { uv.uMax, uv.vMax };
                            uvCoords[3] = { uv.uMax, uv.vMax };
                            uvCoords[4] = { uv.uMin, uv.vMax };
                            uvCoords[5] = { uv.uMin, uv.vMin };
                            break;
                        case 3: // back
                            uvCoords[0] = { uv.uMin, uv.vMin };
                            uvCoords[1] = { uv.uMax, uv.vMax };
                            uvCoords[2] = { uv.uMax, uv.vMin };

                            uvCoords[3] = { uv.uMax, uv.vMax };
                            uvCoords[4] = { uv.uMin, uv.vMin };
                            uvCoords[5] = { uv.uMin, uv.vMax };
                            break;
                        case 4: // bottom 
                            uvCoords[0] = { uv.uMin, uv.vMax };
                            uvCoords[1] = { uv.uMax, uv.vMax };
                            uvCoords[2] = { uv.uMax, uv.vMin };

                            uvCoords[3] = { uv.uMax, uv.vMin };
                            uvCoords[4] = { uv.uMin, uv.vMin };
                            uvCoords[5] = { uv.uMin, uv.vMax };
                            break;
                        case 5: // top
                            uvCoords[0] = { uv.uMin, uv.vMin };
                            uvCoords[1] = { uv.uMax, uv.vMax };
                            uvCoords[2] = { uv.uMax, uv.vMin };

                            uvCoords[3] = { uv.uMax, uv.vMax };
                            uvCoords[4] = { uv.uMin, uv.vMin };
                            uvCoords[5] = { uv.uMin, uv.vMax };
                            break;
                        }

                        std::vector<float>* newVertices; 
                        std::vector<unsigned int>* newIndices;
                        unsigned int* indexCount;
                        if (isTransparent(type)) {
                            newVertices = &newTransparentVertices;
                            newIndices = &newTransparentIndices;
                            indexCount = &transparentIndexCount;
                        }
                        else {
                            newVertices = &newSolidVertices;
                            newIndices = &newSolidIndices;
                            indexCount = &solidIndexCount;
                        }

                        for (int vertex = 0; vertex < 6; ++vertex) {
                            int base = vertex * 6;

                            newVertices->push_back(vertexData[face][base + 0] + x);
                            newVertices->push_back(vertexData[face][base + 1] + y);
                            newVertices->push_back(vertexData[face][base + 2] + z);

                            // Normal
                            newVertices->push_back(vertexData[face][base + 3]);
                            newVertices->push_back(vertexData[face][base + 4]);
                            newVertices->push_back(vertexData[face][base + 5]);

                            // UV from atlas
                            newVertices->push_back(uvCoords[vertex].x);
                            newVertices->push_back(uvCoords[vertex].y);

                            newIndices->push_back((*indexCount)++);
                        }
                    }
                }
            }
        }
    }

    solidVertices = std::move(newSolidVertices);
    solidIndices = std::move(newSolidIndices);

    transparentVertices = std::move(newTransparentVertices);
    transparentIndices = std::move(newTransparentIndices);
}

void Chunk::buildMesh() {
    // Solid
    if (solidVAO == 0) {
        glGenVertexArrays(1, &solidVAO);
        glGenBuffers(1, &solidVBO);
        glGenBuffers(1, &solidEBO);

        glBindVertexArray(solidVAO);
        glBindBuffer(GL_ARRAY_BUFFER, solidVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, solidEBO);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }
    else {
        glBindVertexArray(solidVAO);
    }

    glBindBuffer(GL_ARRAY_BUFFER, solidVBO);
    glBufferData(GL_ARRAY_BUFFER, solidVertices.size() * sizeof(float), solidVertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, solidEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, solidIndices.size() * sizeof(unsigned), solidIndices.data(), GL_DYNAMIC_DRAW);

    // Transparent
    if (transparentVAO == 0) {
        glGenVertexArrays(1, &transparentVAO);
        glGenBuffers(1, &transparentVBO);
        glGenBuffers(1, &transparentEBO);

        glBindVertexArray(transparentVAO);
        glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, transparentEBO);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }
    else {
        glBindVertexArray(transparentVAO);
    }

    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, transparentVertices.size() * sizeof(float), transparentVertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, transparentEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, transparentIndices.size() * sizeof(unsigned), transparentIndices.data(), GL_DYNAMIC_DRAW);
}


void Chunk::render(Shader& ourShader) {
    if (solidVAO == 0 || transparentVAO == 0) {
        regenMesh();
    }

    if (meshFuture.valid() && meshFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
        meshFuture.get();
        buildMesh();
        meshFuture = std::future<void>();
    }
    ourShader.setFloat("opacity", 1.0f);
    glBindVertexArray(solidVAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(solidIndices.size()), GL_UNSIGNED_INT, 0);

    glDisable(GL_CULL_FACE);

    ourShader.setFloat("opacity", 0.85f);
    glBindVertexArray(transparentVAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(transparentIndices.size()), GL_UNSIGNED_INT, 0);

    glEnable(GL_CULL_FACE);
}
