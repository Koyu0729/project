#pragma once
#include "nodeList.h"
#include "vector3.h"

class Player;

class PlayerRangeList
{
  private:
	Player *m_Player = nullptr;

  public:
	PlayerRangeList(Player *player)
		: m_Player(player)
	{
	}

	// 指定された位置と向きで弾を生成し、発射できたか返す
	bool PlayerGun(const Vector3 &origin, float heading);
};
