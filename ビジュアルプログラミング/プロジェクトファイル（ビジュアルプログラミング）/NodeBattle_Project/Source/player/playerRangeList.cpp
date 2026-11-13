#include "playerRangeList.h"
#include "bullet.h"
#include "player.h"
#include "manager.h"
#include "stageObstacle.h"

// 指定された位置と向きで弾を生成し、発射できたか返す
bool PlayerRangeList::PlayerGun(const Vector3 &origin, float heading)
{
	if (!m_Player || !Manager::GetScene())
	{
		return false;
	}

	// 構え始めた向きと、体の前の銃口から弾を出す
	Vector3 direction(std::sin(heading), 0, std::cos(heading));

	Vector3 start = origin + direction * 0.55f;

	for (auto *wall : Manager::GetScene()->GetGameObjects<StageObstacle>())
	{
		if (wall->GetVisible() && wall->BlocksPath(origin, start))
		{
			return false;
		}
	}

	Bullet *bullet = Manager::GetScene()->AddGameObject3D<Bullet>(5);

	bullet->SetOwner(m_Player);

	bullet->SetPosition(start);

	bullet->SetVelocity(direction * CombatBalance::BulletSpeed);

	return true;
}
