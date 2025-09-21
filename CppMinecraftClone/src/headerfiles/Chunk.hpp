#pragma once

#include <vector>
#include <future>

#include "Constants.hpp"
#include "headerfiles/Shader.hpp"
#include "headerfiles/UVHelper.hpp"


class Chunk {
public:
    ~Chunk();

    void regenMesh();
    void Add(int x, int y, int z, UVHelper::BlockType blockType, bool regenerateMesh = false);
    void removeBlock(int x, int y, int z);

    UVHelper::BlockType getBlock(int x, int y, int z);

    float fbm(float x, float z);

    void generateChunk();

    bool isNeighborClear(UVHelper::BlockType neighborBlock, UVHelper::BlockType currentBlock) const;
    bool isTransparent(UVHelper::BlockType blockType) const;

    void generateMesh();
    void buildMesh();

    void render(Shader& ourShader);

    int chunkNumberX;
    int chunkNumberZ;

private:
    UVHelper::BlockType ChunkData[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z] = { UVHelper::BlockType::AIR};
    
    /*unsigned int VAO = 0, VBO = 0, EBO = 0;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;*/


	std::vector<float> solidVertices;
	std::vector<unsigned int> solidIndices;

    std::vector<float> transparentVertices;
	std::vector<unsigned int> transparentIndices;

	unsigned int solidVAO = 0, solidVBO = 0, solidEBO = 0;
	unsigned int transparentVAO = 0, transparentVBO = 0, transparentEBO = 0;
    bool meshBuilding = false;

    std::future<void> meshFuture;
};
