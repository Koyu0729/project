#include "main.h"
#include "spinRotation.h"
#include "player.h"
#include <sstream>
#include <random>

SpinRotation::SpinRotation(int id, NodeType type)
	: SingleOutputNode(id, NodeCategory::Rotate, type)
{
	m_Settings.duration = 1;

	m_Settings.angleDegrees = 360;
}

NodeStatus SpinRotation::Tick(float dt)
{
	if (!m_GameChara)
	{
		m_ExecutionDetail = L"キャラクターがいません";

		return NodeStatus::Failed;
	}

	// 残り時間の分だけ動かして、設定時間で止める
	float used = (std::min)(dt, (std::max)(0.0f, m_Settings.duration - m_Elapsed));

	m_Elapsed += used;

	Vector3 rotation = m_GameChara->GetRotation();

	int spinDirection;

	if (m_Type == NodeType::Rotate_SpinLeft)
	{
		spinDirection = -1;
	}
	else
	{
		spinDirection = 1;
	}

	rotation.y += (spinDirection)*m_Settings.angleDegrees * 3.141592654f / 180.0f * used;

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

NodeBase *SpinRotation::Execute()
{
	return GetNext();
}
