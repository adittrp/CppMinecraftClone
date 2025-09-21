#pragma once

#include <cstdint>

namespace UVHelper {
    enum BlockType : uint8_t {
        AIR,
        GRASS,
        DIRT,
        STONE,
        OAKLOG,
        OAKLEAVES,
        WATER
    };

    struct BlockUV {
        int top;
        int side;
        int bottom;
    };

    // TOP, SIDE, BOTTOM
    constexpr BlockUV blockTextures[] = {
        { -1, -1, -1 }, // AIR
        { 3, 1, 0 }, // GRASS
        { 0, 0, 0 }, // DIRT
        { 4, 4, 4 }, // STONE
        { 2, 6, 2 }, // OAK LOG
        { 5, 5, 5 }, // OAK LEAVES
        { 7, 7, 7 } // WATER
    };

    struct UVCoords {
        float uMin, vMin;
        float uMax, vMax;
    };

    UVCoords getUVCoords(int tileIndex, int tilesPerRow, int tileSize, int atlasSize);
}