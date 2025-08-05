#ifndef UVHELPER_HPP
#define UVHELPER_HPP

enum BlockType {
    AIR,
    GRASS,
    DIRT,
    STONE
};

struct BlockUV {
    int top;
    int side;
    int bottom;
};

extern BlockUV blockTextures[];

struct UVCoords {
    float uMin, vMin;
    float uMax, vMax;
};

UVCoords getUVCoords(int tileIndex, int tilesPerRow, int tileSize, int atlasSize);

#endif