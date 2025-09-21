#pragma once

#include "UVHelper.hpp"

class Player {
public:
	UVHelper::BlockType heldBlock = UVHelper::BlockType::GRASS;
	int renderDistance = 4;
};