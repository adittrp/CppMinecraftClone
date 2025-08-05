#include "headerfiles/UVHelper.hpp"

BlockUV blockTextures[] = {
    { -1, -1, -1 },       // AIR
    { 2, 1, 0 },          // GRASS
    { 0, 0, 0 },          // DIRT
    { 3, 3, 3 }           // STONE
};

UVCoords getUVCoords(int tileIndex, int tilesPerRow, int tileSize, int atlasSize) {
    int x = tileIndex % tilesPerRow;
    int y = tileIndex / tilesPerRow;

    float uvTile = (float)tileSize / (float)atlasSize;

    UVCoords uv;
    uv.uMin = x * uvTile;
    uv.vMin = y * uvTile;
    uv.uMax = (x + 1) * uvTile;
    uv.vMax = (y + 1) * uvTile;
    return uv;
}