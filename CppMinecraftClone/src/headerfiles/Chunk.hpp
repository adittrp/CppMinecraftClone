#pragma once

#include <vector>

#include "Constants.hpp"
#include "headerfiles/UVHelper.hpp"


class Chunk {
public:
    ~Chunk();

    void Add(int x, int y, int z, BlockType blockType, bool regenerateMesh = false);
    void removeBlock(int x, int y, int z);

    BlockType getBlock(int x, int y, int z);

    float fbm(float x, float z);


    void generateChunk();

    void buildMesh();

    void render();

    int chunkNumberX;
    int chunkNumberZ;

private:
	BlockType ChunkData[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z] = {BlockType::AIR};

	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	unsigned int VAO = 0, VBO = 0, EBO = 0;
	bool meshGenerated = false;
};
