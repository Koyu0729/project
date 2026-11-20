#include "main.h"
#include "targetCondition.h"
#include "player.h"
#include <sstream>
#include <random>

// ノードの設定を初期化する
TargetCondition::TargetCondition(int id, NodeType type)
	: BranchBase(id, type)
{
	if (type == NodeType::If_TargetFar)
	{
		m_Settings.distance = 6;
	}

	if (type == NodeType::If_TargetAhead)
	{
		m_Settings.angleDegrees = 90;
	}
}

// 条件を調べて、「はい」か「いいえ」を決める
bool TargetCondition::Branch()
{
	GameObject3D *target;

	if (m_GameChara)
	{
		target = m_GameChara->GetTargetObject();
	}
	else
	{
		target = nullptr;
	}

	bool result = false;

	std::wostringstream detail;

	if (m_Type == NodeType::If_TargetExists)
	{
		result = target != nullptr;

		const wchar_t *targetText;

		if (target)
		{
			targetText = L"あり";
		}
		else
		{
			targetText = L"なし";
		}

		detail << L"敵" << (targetText);
	}
	else if (!target)
	{
		detail << L"敵なし";
	}
	else
	{

		// 敵の位置から、向く方向や距離を調べる
		Vector3 direction = target->GetPosition() - m_GameChara->GetPosition();

		direction.y = 0;

		float distance = direction.length();

		if (m_Type == NodeType::If_TargetFar)
		{
			result = distance >= m_Settings.distance;

			detail << L"距離 " << distance;
		}
		else if (m_Type == NodeType::If_TargetHP)
		{
			auto *enemy = dynamic_cast<Player *>(target);

			if (enemy && enemy->GetStats()->maxHp > 0)
			{
				float hp = 100.0f * enemy->GetHP() / enemy->GetStats()->maxHp;

				result = hp <= m_Settings.hpPercent;

				detail << L"敵HP " << hp << L"%";
			}
			else
			{
				detail << L"敵HPを取得できません";
			}
		}
		else if (m_Type == NodeType::If_TargetAhead)
		{
			if (distance > 0.0001f)
			{
				float angle = std::fabs(std::remainder(std::atan2(direction.x, direction.z) - m_GameChara->GetRotation().y, 6.283185307f));

				result = angle <= m_Settings.angleDegrees * 0.5f * 3.141592654f / 180.0f + 0.00001f;

				detail << L"正面から " << angle * 180.0f / 3.141592654f << L"度";
			}
			else
			{
				detail << L"敵と同じ位置";
			}
		}
	}

	const wchar_t *answerText;

	if (result)
	{
		answerText = L"はい";
	}
	else
	{
		answerText = L"いいえ";
	}

	detail << L" → " << (answerText);

	m_ExecutionDetail = detail.str();

	return result;
}
