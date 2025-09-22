#pragma once

#include "UVHelper.hpp"

class Player {
public:
	Player();

	int currentInventorySlot;
	bool physicalState;

	UVHelper::BlockType heldBlock;
	int renderDistance;
};