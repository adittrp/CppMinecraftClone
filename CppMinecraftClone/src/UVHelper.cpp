#include "headerfiles/UVHelper.hpp"

namespace UVHelper {
    UVCoords getUVCoords(int tileIndex, int tilesPerRow, int tileSize, int atlasSize) {
        int x = tileIndex % tilesPerRow;
        int y = tileIndex / tilesPerRow;

        float uvTile = (float)tileSize / (float)atlasSize;

        UVCoords uv {};
        uv.uMin = x * uvTile;
        uv.vMin = y * uvTile;
        uv.uMax = (x + 1) * uvTile;
        uv.vMax = (y + 1) * uvTile;
        return uv;
    }
}