#include "playerMovementList.h"
#include "player.h"

// 向いている方向へ進む
void PlayerMovementList::PlayerForward()
{
	if (!m_Player)
	{
		return;
	}

	Vector3 forward = m_Player->GetForward();

	forward.y = 0;

	forward.normalize();

	m_Player->SetPosition(m_Player->GetPosition() + forward * m_Player->GetMoveSpeed());
}

// 向いている方向と反対に下がる
void PlayerMovementList::PlayerBack()
{
	if (!m_Player)
	{
		return;
	}

	Vector3 forward = m_Player->GetForward();

	forward.y = 0;

	forward.normalize();

	m_Player->SetPosition(m_Player->GetPosition() - forward * m_Player->GetMoveSpeed());
}

// 移動しない
void PlayerMovementList::PlayerStop()
{
}

// 相手へ近づき、設定した距離で止まる
void PlayerMovementList::PlayerToTarget()
{
	if (!m_Player)
	{
		return;
	}

	auto *target = m_Player->GetTargetObject();

	if (!target)
	{
		return;
	}

	Vector3 dir = target->GetPosition() - m_Player->GetPosition();

	dir.y = 0;

	float distance = dir.length();

	float stop = m_Player->GetNodeMoveDistance();

	if (distance <= stop || distance < 0.0001f)
	{
		return;
	}

	dir /= distance;

	float step = (std::min)(m_Player->GetMoveSpeed() * 1.2f, distance - stop);

	m_Player->SetPosition(m_Player->GetPosition() + dir * step);

	Vector3 rot = m_Player->GetRotation();

	rot.y = atan2f(dir.x, dir.z);

	m_Player->SetRotation(rot);
}

// 相手から設定した距離まで離れる
void PlayerMovementList::PlayerEscape()
{
	if (!m_Player)
	{
		return;
	}

	auto *target = m_Player->GetTargetObject();

	if (!target)
	{
		return;
	}

	Vector3 dir = m_Player->GetPosition() - target->GetPosition();

	dir.y = 0;

	float distance = dir.length();

	float safe = m_Player->GetNodeMoveDistance();

	if (distance >= safe)
	{
		return;
	}

	if (distance < 0.0001f)
	{
		dir = Vector3(0, 0, -1);
	}
	else
	{
		dir /= distance;
	}

	float step = (std::min)(m_Player->GetMoveSpeed(), safe - distance);

	m_Player->SetPosition(m_Player->GetPosition() + dir * step);
}

// 向きを変えずに横へ移動する
void PlayerMovementList::PlayerSide(float side)
{
	if (!m_Player)
	{
		return;
	}

	Vector3 forward = m_Player->GetForward();

	forward.y = 0;

	forward.normalize();

	Vector3 right(forward.z, 0, -forward.x);

	m_Player->SetPosition(m_Player->GetPosition() + right * m_Player->GetMoveSpeed() * side);
}
