#pragma once

constexpr unsigned int WORLD_SIZE_X = 16;
constexpr unsigned int WORLD_SIZE_Z = 16;

constexpr unsigned int CHUNK_SIZE_X = 32;
constexpr unsigned int CHUNK_SIZE_Y = 255;
constexpr unsigned int CHUNK_SIZE_Z = 32;

constexpr float vertexData[6][36] = {
    // left face
    {-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f},

    // right face
    {-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

     0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 
    -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f},

    // front face
    {-0.5f, 0.5f, 0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, 0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, 0.5f, -1.0f,  0.0f,  0.0f, 
    -0.5f, 0.5f, 0.5f, -1.0f,  0.0f,  0.0f},

    // back face
    {0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f, 1.0f,  0.0f,  0.0f,

     0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 
     0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f, 1.0f,  0.0f,  0.0f},

     // bottom face
     {-0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f,
      0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f,
      0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f,

      0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f,
     -0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f, 
     -0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f},

     // top face
     {-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 
      0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,

      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f}
};