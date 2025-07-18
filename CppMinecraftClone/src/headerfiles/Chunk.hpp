#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <cstdint>
#include <vector>

#include "Constants.hpp"

// Vertex data
float vertexData[6][30] = {
    // Back face (z = -0.5)
    {-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,

     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f},

    // Front face (z = +0.5)
    {-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f },

    // Left face (x = -0.5)
    {-0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f},

    // Right face (x = +0.5)
    {0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,

     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f},

     // Bottom face (y = -0.5)
     {-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
      0.5f, -0.5f,  0.5f,  1.0f, 0.0f,

      0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     -0.5f, -0.5f, -0.5f,  0.0f, 1.0f},

     // Top face (y = +0.5), normal points upwards (0, 1, 0)
     {-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      0.5f,  0.5f, -0.5f,  1.0f, 1.0f,

      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     -0.5f,  0.5f,  0.5f,  0.0f, 0.0f}
};

class Chunk {
public:

	~Chunk() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

	void Add(int x, int y, int z) {
		if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z) 
			return;
		ChunkData[x][y][z] = 1;
	}

	uint8_t getBlock(int x, int y, int z) {
		if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z) 
			return 0;
		return ChunkData[x][y][z];
	}

    void buildMesh(Chunk(&chunks)[WORLD_SIZE_X][WORLD_SIZE_Z]) {
        if (VAO != 0) glDeleteVertexArrays(1, &VAO);
        if (VBO != 0) glDeleteBuffers(1, &VBO);
        if (EBO != 0) glDeleteBuffers(1, &EBO);

        vertices.clear();
        indices.clear();
        unsigned int indexCount = 0;

        for (int x = 0; x < CHUNK_SIZE_X; ++x) {
            for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
                for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
                    if (getBlock(x, y, z) == 0) continue;

                    for (int face = 0; face < 6; face++) {
                        int nx = x, ny = y, nz = z;

                        // Back
                        if (face == 0) nz--;
                        // Front
                        if (face == 1) nz++;
                        // Left
                        if (face == 2) nx--;
                        // Right
                        if (face == 3) nx++;
                        // Bottom
                        if (face == 4) ny--;
                        // Top
                        if (face == 5) ny++;

                        bool shouldRenderFace = false;
                        if (nx < 0) {
                            if (chunkNumberX > 0) {
                                Chunk& neighbor = chunks[chunkNumberX - 1][chunkNumberZ];
                                shouldRenderFace = neighbor.getBlock(CHUNK_SIZE_X - 1, y, z) == 0;
                            }
                            else {
                                shouldRenderFace = true;
                            }
                        }
                        else if (nx >= CHUNK_SIZE_X) {
                            if (chunkNumberX + 1 < WORLD_SIZE_X) {
                                Chunk& neighbor = chunks[chunkNumberX + 1][chunkNumberZ];
                                shouldRenderFace = neighbor.getBlock(0, y, z) == 0;
                            }
                            else {
                                shouldRenderFace = true;
                            }
                        }
                        else if (nz < 0) {
                            if (chunkNumberZ > 0) {
                                Chunk& neighbor = chunks[chunkNumberX][chunkNumberZ - 1];
                                shouldRenderFace = neighbor.getBlock(x, y, CHUNK_SIZE_Z - 1) == 0;
                            }
                            else {
                                shouldRenderFace = true;
                            }
                        }
                        else if (nz >= CHUNK_SIZE_Z) {
                            if (chunkNumberZ + 1 < WORLD_SIZE_Z) {
                                Chunk& neighbor = chunks[chunkNumberX][chunkNumberZ + 1];
                                shouldRenderFace = neighbor.getBlock(x, y, 0) == 0;
                            }
                            else {
                                shouldRenderFace = true;
                            }
                        }
                        else {
                            shouldRenderFace = getBlock(nx, ny, nz) == 0;
                        }


                        if (shouldRenderFace) {
                            // 6 vertices per face
                            for (int vertex = 0; vertex < 6; ++vertex) {
                                // 5 values per vertex (x, y, z, u, v) first 3 for pos, last 2 for texture
                                for (int value = 0; value < 5; ++value) {
                                    float vertexValue = vertexData[face][vertex * 5 + value];

                                    if (value == 0) vertexValue += x;
                                    if (value == 1) vertexValue += y;
                                    if (value == 2) vertexValue += z;

                                    vertices.push_back(vertexValue);
                                }
                                indices.push_back(indexCount++);
                            }
                        }
                    }
                }
            }
        }

        if (VAO == 0) glGenVertexArrays(1, &VAO);
        if (VBO == 0) glGenBuffers(1, &VBO);
        if (EBO == 0) glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        meshGenerated = true;
    }

    void render() {
        if (!meshGenerated) return;
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }

    int chunkNumberX;
    int chunkNumberZ;

private:
	uint8_t ChunkData[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z] = {};

	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	unsigned int VAO = 0, VBO = 0, EBO = 0;
	bool meshGenerated = true;
};


#endif