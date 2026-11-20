#pragma once
#include "nodeList.h"

class Player;

class PlayerMovementList
{
  private:
	Player *m_Player = nullptr;

  public:
	PlayerMovementList(Player *player)
		: m_Player(player)
	{
	}

	// 向いている方向へ進む
	void PlayerForward();

	// 向いている方向と反対に下がる
	void PlayerBack();

	// 移動しない
	void PlayerStop();

	// 向きを変えずに横へ移動する
	void PlayerSide(float side);

	// 相手へ近づき、設定した距離で止まる
	void PlayerToTarget();

	// 相手から設定した距離まで離れる
	void PlayerEscape();
};
