#include "main.h"
#include "orbitMovement.h"
#include "player.h"
#include <sstream>
#include <random>

// ノードの設定を初期化する
OrbitMovement::OrbitMovement(int id, NodeType type)
	: SingleOutputNode(id, NodeCategory::Movement, type)
{
	m_Settings.duration = 1;
}

// 設定時間や条件を見ながら、ノードを進める
NodeStatus OrbitMovement::Tick(float dt)
{
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

	// 敵を中心に回るための、自分の位置と敵からの距離
	Vector3 position = m_GameChara->GetPosition();

	Vector3 center = target->GetPosition();

	Vector3 offset = position - center;

	offset.y = 0;

	float radius = offset.length();

	if (radius < 0.1f)
	{
		m_ExecutionDetail = L"敵に近すぎるため次へ";

		return NodeStatus::Completed;
	}

	// 残り時間の分だけ動かして、設定時間で止める
	float used = (std::min)(dt, (std::max)(0.0f, m_Settings.duration - m_Elapsed));

	m_Elapsed += used;

	m_GameChara->SetNodeDeltaSeconds(used);

	int orbitDirection;

	if (m_Type == NodeType::Movement_OrbitLeft)
	{
		orbitDirection = 1;
	}
	else
	{
		orbitDirection = -1;
	}

	float angle = m_GameChara->GetMoveSpeed() / radius * (orbitDirection);

	float cosine = std::cos(angle), sine = std::sin(angle);

	Vector3 next(center.x + offset.x * cosine + offset.z * sine, position.y, center.z - offset.x * sine + offset.z * cosine);

	m_GameChara->SetPosition(next);

	Vector3 rotation = m_GameChara->GetRotation();

	rotation.y = std::atan2(center.x - next.x, center.z - next.z);

	m_GameChara->SetRotation(rotation);

	if (m_Elapsed + 0.00001f >= m_Settings.duration)
	{
		return NodeStatus::Completed;
	}
	else
	{
		return NodeStatus::Running;
	}
}

// プレイヤーへ行動を渡して、次のノードを返す
NodeBase *OrbitMovement::Execute()
{
	return GetNext();
}
