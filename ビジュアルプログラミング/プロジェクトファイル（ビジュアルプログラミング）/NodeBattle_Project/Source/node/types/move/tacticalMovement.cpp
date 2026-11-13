#include "main.h"
#include "tacticalMovement.h"
#include "player.h"
#include <sstream>
#include <random>

// 距離を保つ時間と、回り込みの制限時間を用意する
TacticalMovement::TacticalMovement(int id, NodeType type)
	: SingleOutputNode(id, NodeCategory::Movement, type)
{
	m_Settings.distance = 2.0f;

	if (type == NodeType::Movement_KeepDistance)
	{
		m_Settings.duration = 1.0f;
	}
	else
	{
		m_Settings.duration = 5.0f;
	}
}

// 敵の横を決める基準の向きを、開始時に保存する
void TacticalMovement::Begin()
{
	NodeBase::Begin();

	if (m_GameChara && m_GameChara->GetTargetObject())
	{
		m_EnemyHeading = m_GameChara->GetTargetObject()->GetRotation().y;
	}
}

// 相手の位置に合わせて移動し、到着または制限時間で次へ進む
NodeStatus TacticalMovement::Tick(float dt)
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

	// 移動できる時間と、現在位置から目標までの距離
	float used = (std::min)(dt, (std::max)(0.0f, m_Settings.duration - m_Elapsed));

	m_Elapsed += used;

	m_GameChara->SetNodeDeltaSeconds(used);

	Vector3 position = m_GameChara->GetPosition();

	Vector3 center = target->GetPosition();

	Vector3 destination = center;

	bool keepDistance = m_Type == NodeType::Movement_KeepDistance;

	if (keepDistance)
	{
		Vector3 offset = position - center;

		offset.y = 0;

		float radius = offset.length();

		Vector3 away;

		if (radius > 0.0001f)
		{
			away = offset * (1.0f / radius);
		}
		else
		{
			away = m_GameChara->GetForward() * -1.0f;
		}

		destination += away * m_Settings.distance;
	}
	else
	{
		float side;

		if (m_Type == NodeType::Movement_FlankLeft)
		{
			side = -1.0f;
		}
		else
		{
			side = 1.0f;
		}

		destination += Vector3(std::cos(m_EnemyHeading), 0, -std::sin(m_EnemyHeading)) * (side * m_Settings.distance);
	}

	destination.y = position.y;

	Vector3 delta = destination - position;

	float distance = delta.length();

	if (!keepDistance && distance <= 0.15f)
	{
		m_ExecutionDetail = L"敵の横に到着";

		return NodeStatus::Completed;
	}

	if (distance > 0.05f)
	{
		float step = (std::min)(distance, m_GameChara->GetMoveSpeed());

		m_GameChara->SetPosition(position + delta * (step / distance));
	}

	// 移動中は敵を向くが、攻撃の判定は出さない
	Vector3 towards = center - m_GameChara->GetPosition();

	if (towards.x * towards.x + towards.z * towards.z > 0.000001f)
	{
		Vector3 rotation = m_GameChara->GetRotation();

		rotation.y = std::atan2(towards.x, towards.z);

		m_GameChara->SetRotation(rotation);
	}

	if (m_Elapsed + 0.00001f >= m_Settings.duration)
	{
		if (keepDistance)
		{
			m_ExecutionDetail = L"距離を保つ時間が終了";
		}
		else
		{
			m_ExecutionDetail = L"制限時間：次へ";
		}

		return NodeStatus::Completed;
	}

	return NodeStatus::Running;
}

// 次につながるノードを返す
NodeBase *TacticalMovement::Execute()
{
	return GetNext();
}
