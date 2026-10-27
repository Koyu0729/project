#include "main.h"
#include "targetRotation.h"
#include "player.h"
#include <sstream>
#include <random>

// ノードの設定を初期化する
TargetRotation::TargetRotation(int id, NodeType type)
	: SingleOutputNode(id, NodeCategory::Rotate, type)
{
	if (type == NodeType::Rotate_TurnBack)
	{
		m_Settings.angleDegrees = 180;
	}
}

// 設定時間や条件を見ながら、ノードを進める
NodeStatus TargetRotation::Tick(float dt)
{
	if (m_Type != NodeType::Rotate_ToTarget)
	{
		return NodeBase::Tick(dt);
	}

	if (!m_GameChara)
	{
		m_ExecutionDetail = L"キャラクターがいません";

		return NodeStatus::Failed;
	}

	auto *target = m_GameChara->GetTargetObject();

	if (!target)
	{
		m_ExecutionDetail = L"敵なし：次へ";

		return NodeStatus::Completed;
	}

	// 敵の位置から、向く方向や距離を調べる
	Vector3 direction = target->GetPosition() - m_GameChara->GetPosition();

	direction.y = 0;

	if (direction.length() < 0.0001f)
	{
		return NodeStatus::Completed;
	}

	Vector3 rotation = m_GameChara->GetRotation();

	float difference = std::remainder(std::atan2(direction.x, direction.z) - rotation.y, 6.283185307f);

	float step = (std::max)(-dt * 3.0f, (std::min)(dt * 3.0f, difference));

	rotation.y += step;

	m_GameChara->SetRotation(rotation);

	if (std::fabs(difference - step) < 0.0001f)
	{
		return NodeStatus::Completed;
	}
	else
	{
		return NodeStatus::Running;
	}
}

// プレイヤーへ行動を渡して、次のノードを返す
NodeBase *TargetRotation::Execute()
{
	return GetNext();
}
