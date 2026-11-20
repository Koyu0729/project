#pragma once
#include "singleOutputNode.h"
#include "combatBalance.h"
#include <algorithm>

class AttackGun : public SingleOutputNode
{
  public:
	AttackGun(int id)
		: SingleOutputNode(id, NodeCategory::AttackRange, NodeType::Attack_Gun)
	{
	}

	// 攻撃開始の要求をプレイヤーへ渡す
	NodeBase *Execute() override;

	// 射撃の構えが終わる前に次へ進まないようにする
	NodeStatus Tick(float dt) override
	{
		float duration = m_Settings.duration;

		m_Settings.duration = (std::max)(CombatBalance::ShotActionTime, duration);

		NodeStatus status = NodeBase::Tick(dt);

		m_Settings.duration = duration;

		return status;
	}
};
