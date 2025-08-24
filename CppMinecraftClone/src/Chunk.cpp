#include "headerfiles/Chunk.hpp"

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

void Chunk::Add(int x, int y, int z, BlockType blockType, bool regenerateMesh) {
    if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z)
        return;
    ChunkData[x][y][z] = blockType;

    if (regenerateMesh) {
        meshGenerated = false;
        buildMesh(chunks);
    }
}

void Chunk::removeBlock(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z)
        return;
    ChunkData[x][y][z] = BlockType::AIR;
    meshGenerated = false;
    buildMesh(chunks);
}

BlockType Chunk::getBlock(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z)
        return BlockType::AIR;
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
                    Add(x, y, z, BlockType::GRASS);
                }
                else if (y > height - 5) {
                    Add(x, y, z, BlockType::DIRT);
                }
                else {
                    Add(x, y, z, BlockType::STONE);
                }
            }
        }
    }
}

void Chunk::buildMesh(Chunk(&chunks)[WORLD_SIZE_X][WORLD_SIZE_Z]) {
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (EBO != 0) glDeleteBuffers(1, &EBO);

    vertices.clear();
    indices.clear();
    unsigned int indexCount = 0;

    for (int x = 0; x < CHUNK_SIZE_X; ++x) {
        for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
            for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
                BlockType type = getBlock(x, y, z);
                if (type == BlockType::AIR) continue;

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

                    bool shouldRenderFace = false;
                    if (nx < 0) {
                        if (chunkNumberX > 0) {
                            Chunk& neighbor = chunks[chunkNumberX - 1][chunkNumberZ];
                            shouldRenderFace = neighbor.getBlock(CHUNK_SIZE_X - 1, y, z) == BlockType::AIR;
                        }
                        else {
                            shouldRenderFace = true;
                        }
                    }
                    else if (nx >= CHUNK_SIZE_X) {
                        if (chunkNumberX + 1 < WORLD_SIZE_X) {
                            Chunk& neighbor = chunks[chunkNumberX + 1][chunkNumberZ];
                            shouldRenderFace = neighbor.getBlock(0, y, z) == BlockType::AIR;
                        }
                        else {
                            shouldRenderFace = true;
                        }
                    }
                    else if (nz < 0) {
                        if (chunkNumberZ > 0) {
                            Chunk& neighbor = chunks[chunkNumberX][chunkNumberZ - 1];
                            shouldRenderFace = neighbor.getBlock(x, y, CHUNK_SIZE_Z - 1) == BlockType::AIR;
                        }
                        else {
                            shouldRenderFace = true;
                        }
                    }
                    else if (nz >= CHUNK_SIZE_Z) {
                        if (chunkNumberZ + 1 < WORLD_SIZE_Z) {
                            Chunk& neighbor = chunks[chunkNumberX][chunkNumberZ + 1];
                            shouldRenderFace = neighbor.getBlock(x, y, 0) == BlockType::AIR;
                        }
                        else {
                            shouldRenderFace = true;
                        }
                    }
                    else {
                        shouldRenderFace = getBlock(nx, ny, nz) == BlockType::AIR;
                    }

                    if (shouldRenderFace) {
                        BlockUV uvSet = blockTextures[type];
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
                        UVCoords uv = getUVCoords(tileIndex, 2, 16, 32);

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

                            // Position
                            vertices.push_back(vertexData[face][base + 0] + x);
                            vertices.push_back(vertexData[face][base + 1] + y);
                            vertices.push_back(vertexData[face][base + 2] + z);

                            // Normal
                            vertices.push_back(vertexData[face][base + 3]);
                            vertices.push_back(vertexData[face][base + 4]);
                            vertices.push_back(vertexData[face][base + 5]);

                            // UV from atlas
                            vertices.push_back(uvCoords[vertex].x);
                            vertices.push_back(uvCoords[vertex].y);

                            indices.push_back(indexCount++);
                        }
                    }
                }
            }
        }
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    meshGenerated = true;
}

void Chunk::render() {
    if (!meshGenerated) return;
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
}