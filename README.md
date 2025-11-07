# **C++ Minecraft Clone**
Voxel-based Minecraft Clone made using C++ and OpenGL

## Major Features
- Chunked voxel world with adjustable size
- Efficient meshing algorithm to minimize drawn vertices
- Multithreaded meshing -> terrain meshing runs on a separate thread from the main game loop
- Visual hotbar that lets you see which slot is currently selected
- Basic perlin noise terrain generation
- First-person camera movement
- Block placing and breaking using raycasts
- Highlight system for selected blocks
- Collision detection/Survival mode type movement
- Custom block UV mapping

## Todo/In progress
- Better terrain generation
- Fully functional inventory system
- Basic crafting system
- Better lighting system
- More optimized chunk meshing (greedy meshing)
- Saving/loading worlds
- More advanced UI (Hearts, hunger, text, etc.)

## How to Run
1. Download the repository as a ZIP file or clone it
2. Open the CppMinecraftClone.sln file in Visual Studio
3. Run 'main.cpp' found in the src folder
