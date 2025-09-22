#include "headerfiles/Player.hpp"

Player::Player() :
	physicalState(true),
	currentInventorySlot(0),
	renderDistance(4),
	heldBlock(UVHelper::BlockType::GRASS)
{
}