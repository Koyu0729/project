#pragma once
#include "singleOutputNode.h"

class AttackMelee : public SingleOutputNode
{
  public:
	// 武器に合わせて攻撃の種類と標準時間を決める
	AttackMelee(int id, NodeType type = NodeType::Attack_Melee)
		: SingleOutputNode(id, NodeCategory::AttackMelee, type)
	{

		if (type == NodeType::Attack_Sword)
		{
			m_Settings.duration = 0.2f;
		}
	}

	// プレイヤーへ行動を渡して、次のノードを返す
	NodeBase *Execute() override;

	// 設定時間や条件を見ながら、ノードを進める
	NodeStatus Tick(float dt) override
	{
		float duration = m_Settings.duration;

		float minimumDuration;

		if (m_Type == NodeType::Attack_Sword)
		{
			minimumDuration = 0.2f;
		}
		else
		{
			minimumDuration = 0.4f;
		}

		m_Settings.duration = (std::max)(minimumDuration, duration);

		auto status = NodeBase::Tick(dt);

		m_Settings.duration = duration;

		return status;
	}
};
