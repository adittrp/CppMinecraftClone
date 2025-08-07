#ifndef WORLD_HPP
#define WORLD_HPP

#include "Chunk.hpp"
#include "Constants.hpp"
#include "Shader.hpp"

extern Chunk chunks[WORLD_SIZE_X][WORLD_SIZE_Z];

void generateWorld();
void renderWorld(Shader& ourShader);

#endif