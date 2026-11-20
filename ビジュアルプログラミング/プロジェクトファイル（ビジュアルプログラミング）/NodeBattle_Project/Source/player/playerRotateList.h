#pragma once
#include "nodeList.h"

class Player;

class PlayerRotateList
{
private:
	Player* m_Player = nullptr;
	float m_RemitRotate = 0.0f;
public:
	PlayerRotateList(Player* player) : m_Player(player) {}

	void PlayerFaceLeft();
	void PlayerFaceRight();
	void PlayerTurnLeft();
	void PlayerTurnRight();
};
