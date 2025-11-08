#include "headerfiles/Player.hpp"
#include "iostream"

Player::Player() :
	physicalState(true),
	currentInventorySlot(0),
	renderDistance(4),
	heldBlock(UVHelper::BlockType::GRASS)
{
    gamemodeDescription();
}

void Player::gamemodeDescription() {
    if (physicalState) {
        std::cout << "Current mode: Survival Mode" << std::endl;
        std::cout << " - WASD for basic movement (forward, backwards, left, and right)" << std::endl;
        std::cout << " - Space to jump" << std::endl;
        std::cout << " - Left CRTL for Sprinting" << std::endl;
        std::cout << " - Left Click to break blocks, Right Click to place blocks" << std::endl;
        std::cout << " - Choose hotbar tabs with numbers (1-9)\n" << std::endl;
    }
    else {
        std::cout << "Current mode: Spectator Mode" << std::endl;
        std::cout << " - WASD for basic movement (forward, backwards, left, and right)" << std::endl;
        std::cout << " - Space to fly upwards, and Shift to fly down" << std::endl;
        std::cout << " - Left CRTL for Sprinting" << std::endl;
        std::cout << " - Left Click to break blocks, Right Click to place blocks" << std::endl;
        std::cout << " - Choose hotbar tabs with numbers (1-9)\n" << std::endl;
    }
}