#include "main.h"
#include "tacticalCondition.h"
#include "player.h"
#include <sstream>
#include <random>

// 条件の種類を保存し、標準の距離や確率を使う
TacticalCondition::TacticalCondition(int id, NodeType type)
	: BranchBase(id, type)
{
}

// 分岐に到達するたびに一度だけ条件を決める
bool TacticalCondition::Branch()
{
	bool result = false;

	if (m_Type == NodeType::If_Random)
	{
		// 毎フレームで引き直さず、実行順や左右の選択をばらけさせる
		static std::mt19937 engine(std::random_device{}());

		std::uniform_real_distribution<float> draw(0, 100);

		result = draw(engine) < m_Settings.hpPercent;
	}
	else if (m_GameChara)
	{
		if (m_Type == NodeType::If_ObstacleAhead)
		{
			result = m_GameChara->HasObstacleAhead(m_Settings.distance);
		}
		else
		{
			auto *enemy = dynamic_cast<Player *>(m_GameChara->GetTargetObject());

			result = enemy && enemy->IsAttacking();
		}
	}

	if (result)
	{
		m_ExecutionDetail = L"はい";
	}
	else
	{
		m_ExecutionDetail = L"いいえ";
	}

	return result;
}
