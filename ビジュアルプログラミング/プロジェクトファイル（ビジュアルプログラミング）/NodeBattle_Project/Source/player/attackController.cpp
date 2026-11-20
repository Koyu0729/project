#include "main.h"
#include "attackController.h"
#include "player.h"
#include "manager.h"
#include "stageObstacle.h"
#include "combatBalance.h"
#include <algorithm>
#include <cmath>

// プレイヤーと弾の生成処理を結びつける
AttackController::AttackController(Player &player)
	: m_Player(player), m_RangeList(&player)
{
}

// すべての攻撃状態を初期化する
void AttackController::Reset()
{
	CancelMeleeAttack();
	CancelRangeAttack();
	m_AttackCooldown = 0;
	m_SwordContacts.clear();
}

// クールタイムと攻撃の動作を同じ時間で更新する
void AttackController::Update(float dt)
{
	m_AttackCooldown = (std::max)(0.0f, m_AttackCooldown - dt);
	m_RangeAttackTime = (std::max)(0.0f, m_RangeAttackTime - dt);
	UpdateMelee(dt);
	UpdateRangeAttack(dt);
}

// 剣に触れ始めた相手だけにダメージを入れる
void AttackController::UpdateSword(bool damageAllowed)
{
	if (!m_SwordEnabled || !Manager::GetScene())
	{
		return;
	}

	std::unordered_set<Player *> contacts;

	for (auto *target : Manager::GetScene()->GetGameObjects<Player>())
	{
		if (target == &m_Player || target->Destroy() || target->GetHP() <= 0 || (m_Player.GetTeam() != Team::Team_Null && target->GetTeam() == m_Player.GetTeam()))
		{
			continue;
		}

		if (!SwordTouches(target, m_Player.GetPosition(), m_Player.GetRotation().y))
		{
			continue;
		}

		contacts.insert(target);

		if (damageAllowed && m_SwordDamageEnabled && !m_SwordContacts.count(target))
		{
			const Vector3 origin = m_Player.GetPosition();
			target->TakeDamage(CombatBalance::ContactDamage, &origin);
		}
	}

	m_SwordContacts = std::move(contacts);
}

// 槍または剣の攻撃を始め、攻撃の向きを固定する
void AttackController::StartMelee(NodeType type)
{
	if ((type != NodeType::Attack_Melee && type != NodeType::Attack_Sword) || m_AttackCooldown > 0 || m_MeleePending)
	{
		return;
	}

	m_ActiveMeleeType = type;

	m_AttackCooldown = CombatBalance::AttackCooldown;

	m_MeleePending = true;

	m_MeleeElapsed = 0;

	m_MeleeHits.clear();

	m_MeleeHeading = m_Player.GetRotation().y;

	m_MeleeOrigin = m_Player.GetPosition();
}

// 射撃の構えを始め、敵へ軸を合わせる（この時点では弾を出さない）
void AttackController::StartRange(NodeType type)
{
	if (type != NodeType::Attack_Gun || m_AttackCooldown > 0 || m_RangePending)
	{
		return;
	}

	auto *target = m_Player.GetTargetObject();

	if (!target)
	{
		return;
	}

	Vector3 direction = target->GetPosition() - m_Player.GetPosition();

	direction.y = 0;

	if (direction.length() < CombatBalance::ShotMinimumDistance)
	{
		return;
	}

	m_RangeHeading = std::atan2(direction.x, direction.z);

	m_RangeOrigin = m_Player.GetPosition();

	SetHeading(m_RangeHeading);

	m_RangePending = true;

	m_RangeElapsed = 0;

	m_AttackCooldown = CombatBalance::AttackCooldown;
}

// 近接攻撃の動作と命中済みの記録を消す
void AttackController::CancelMeleeAttack()
{
	m_MeleePending = false;
	m_ActiveMeleeType = NodeType::Type_Null;
	m_MeleeElapsed = 0;
	m_MeleeHits.clear();
}

// 射撃の構えと発射直後の状態を消す
void AttackController::CancelRangeAttack()
{
	m_RangePending = false;
	m_RangeElapsed = 0;
	m_RangeAttackTime = 0;
}

// 正面の武器を表示するか設定する
void AttackController::SetSwordEnabled(bool enabled)
{
	m_SwordEnabled = enabled;
	m_SwordContacts.clear();
}

// 継続接触の記録を消し、接触ダメージの有効状態を設定する
void AttackController::SetSwordDamageEnabled(bool enabled)
{
	m_SwordDamageEnabled = enabled;
	m_SwordContacts.clear();
}

// 構え・突き・戻りに合わせて、棒の長さを決める
float AttackController::GetMeleeExtension() const
{
	if (m_ActiveMeleeType == NodeType::Attack_Sword)
	{
		return 0.8f;
	}

	if (!m_MeleePending)
	{
		return 1.2f;
	}

	if (m_MeleeElapsed <= 0.2f)
	{
		return 1.2f;
	}

	if (m_MeleeElapsed <= 0.3f)
	{
		return 1.2f + 1.2f * (m_MeleeElapsed - 0.2f) / 0.1f;
	}

	return (std::max)(1.2f, 2.4f - 1.2f * (m_MeleeElapsed - 0.3f) / 0.1f);
}

// 体の向きを固定したまま、剣だけを右から左へ振る
float AttackController::GetMeleeSwingHeading() const
{
	if (!m_MeleePending)
	{
		return m_Player.GetRotation().y;
	}

	if (m_ActiveMeleeType == NodeType::Attack_Sword)
	{
		return m_MeleeHeading + 0.9f - 1.8f * (std::min)(m_MeleeElapsed / 0.16f, 1.0f);
	}

	return m_MeleeHeading;
}

// 正面の棒と、相手の体が重なっているか調べる
bool AttackController::SwordTouches(Player *target, const Vector3 &origin, float heading, float length) const
{
	if (!target || std::fabs(target->GetPosition().y - origin.y) > 0.65f)
	{
		return false;
	}

	// 棒の根元から先端までと、相手の体の距離を調べる
	Vector3 forward(std::sin(heading), 0, std::cos(heading));

	// 見た目と同じ右側の位置から、正面の当たり判定を出す
	Vector3 right(std::cos(heading), 0, -std::sin(heading));

	Vector3 start = origin + right * 0.22f + forward * 0.35f;

	Vector3 delta = target->GetPosition() - start;

	delta.y = 0;

	float along = (std::max)(0.0f, (std::min)(length, Vector3::dot(delta, forward)));

	Vector3 nearest = start + forward * along;

	// 壁の向こう側には、剣や槍のダメージを入れない
	if (Manager::GetScene())
	{
		for (auto *wall : Manager::GetScene()->GetGameObjects<StageObstacle>())
		{
			if (!wall->GetVisible())
			{
				continue;
			}

			if (wall->BlocksPath(start, nearest))
			{
				return false;
			}
		}
	}

	Vector3 offset = target->GetPosition() - nearest;

	offset.y = 0;

	return offset.length() <= 0.39f;
}

// キャラクターの向きを、固定した攻撃軸へ合わせる
void AttackController::SetHeading(float heading)
{
	Vector3 rotation = m_Player.GetRotation();
	rotation.y = heading;
	m_Player.SetRotation(rotation);
}

// 剣を振る、または槍を伸ばす（1回の攻撃で相手ごとに1回だけ当てる）
void AttackController::UpdateMelee(float dt)
{
	if (!m_MeleePending)
	{
		return;
	}

	SetHeading(m_MeleeHeading);

	// 前回から今回までに伸びた部分を見て、突きの当たり判定を出す
	float previous = m_MeleeElapsed;

	m_MeleeElapsed += dt;

	if (m_ActiveMeleeType == NodeType::Attack_Sword)
	{
		// フレームの間に刃が通った範囲も調べて、高速な振りの見逃しを防ぐ
		float firstAngle = 0.9f - 1.8f * (std::min)(previous / 0.16f, 1.0f);

		float lastAngle = 0.9f - 1.8f * (std::min)(m_MeleeElapsed / 0.16f, 1.0f);

		int samples = (std::max)(1, static_cast<int>(std::ceil(std::fabs(lastAngle - firstAngle) / 0.08f)));

		if (previous < 0.16f && Manager::GetScene())
		{
			for (auto *target : Manager::GetScene()->GetGameObjects<Player>())
			{
				if (target == &m_Player || target->Destroy() || target->GetHP() <= 0 || m_MeleeHits.count(target) || (m_Player.GetTeam() != Team::Team_Null && target->GetTeam() == m_Player.GetTeam()))
				{
					continue;
				}

				for (int i = 0; i <= samples; ++i)
				{
					float heading = m_MeleeHeading + firstAngle + (lastAngle - firstAngle) * i / samples;

					if (SwordTouches(target, m_MeleeOrigin, heading, 0.8f))
					{
						target->TakeDamage(CombatBalance::MeleeDamage, &m_MeleeOrigin);

						m_MeleeHits.insert(target);

						break;
					}
				}
			}
		}

		if (m_MeleeElapsed + 0.00001f >= 0.2f)
		{
			CancelMeleeAttack();
		}

		return;
	}

	// 槍を前へ突き出している間だけ攻撃判定を出し、引き戻し中や向き合わせ中はダメージを与えない。

	if (m_MeleeElapsed > 0.2f && previous < 0.3f && Manager::GetScene())
	{
		float sweptLength = 1.2f + 1.2f * ((std::min)(m_MeleeElapsed, 0.3f) - 0.2f) / 0.1f;

		for (auto *target : Manager::GetScene()->GetGameObjects<Player>())
		{
			if (target == &m_Player || target->Destroy() || target->GetHP() <= 0 || m_MeleeHits.count(target) || (m_Player.GetTeam() != Team::Team_Null && target->GetTeam() == m_Player.GetTeam()))
			{
				continue;
			}

			if (SwordTouches(target, m_MeleeOrigin, m_MeleeHeading, sweptLength))
			{
				target->TakeDamage(CombatBalance::MeleeDamage, &m_MeleeOrigin);

				m_MeleeHits.insert(target);
			}
		}
	}

	if (m_MeleeElapsed + 0.00001f >= 0.4f)
	{
		CancelMeleeAttack();
	}
}

// 構えた軸を固定し、敵が横へ避けても撃つ直前に追い直さない
void AttackController::UpdateRangeAttack(float dt)
{
	if (!m_RangePending)
	{
		return;
	}

	SetHeading(m_RangeHeading);

	m_RangeElapsed += dt;

	if (m_RangeElapsed + 0.00001f >= CombatBalance::ShotWindup)
	{
		if (m_RangeList.PlayerGun(m_RangeOrigin, m_RangeHeading))
		{
			m_RangeAttackTime = 0.15f;
		}

		m_RangePending = false;
	}
}
