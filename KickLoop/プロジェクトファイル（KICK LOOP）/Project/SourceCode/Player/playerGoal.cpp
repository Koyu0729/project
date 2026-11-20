#include "main.h"
#include "player.h"

#include "stageBlockManager.h"
#include "stageBlock.h"
#include "gameTime.h"

// 補間値を滑らかにする
float Player::SmoothStep01(float value)
{
	if (value < 0.0f)
	{
		value = 0.0f;
	}

	if (value > 1.0f)
	{
		value = 1.0f;
	}

	return value * value * (3.0f - 2.0f * value);
}

void Player::StartGoalLaunch(
	Vector3 targetPosition,
	int frameMax,
	float height
)
{
	if (IsDead() || IsGoalReached() || IsGoalLaunching())
	{
		return;
	}

	m_GoalLaunchStartPosition = m_Position;

	m_GoalLaunchTargetPosition = targetPosition;

	m_GoalLaunchFrame = 0;

	m_GoalLaunchFrameMax = frameMax;

	if (m_GoalLaunchFrameMax <= 1)
	{
		m_GoalLaunchFrameMax = 1;
	}

	m_GoalLaunchHeight = height;

	if (m_GoalLaunchHeight < 0.0f)
	{
		m_GoalLaunchHeight = 0.0f;
	}

	SetMainState(PlayerMainState::GoalLaunch);
}

// ゴールへ向かう放物線上に移動する
void Player::UpdateGoalLaunch()
{
	m_GoalLaunchFrame++;

	float t = (float)m_GoalLaunchFrame / (float)m_GoalLaunchFrameMax;

	if (t > 1.0f)
	{
		t = 1.0f;
	}

	float smoothT = SmoothStep01(t);

	Vector3 basePosition;

	basePosition.x =
		m_GoalLaunchStartPosition.x +
		(m_GoalLaunchTargetPosition.x - m_GoalLaunchStartPosition.x) *
		smoothT;

	basePosition.y =
		m_GoalLaunchStartPosition.y +
		(m_GoalLaunchTargetPosition.y - m_GoalLaunchStartPosition.y) *
		smoothT;

	basePosition.z =
		m_GoalLaunchStartPosition.z +
		(m_GoalLaunchTargetPosition.z - m_GoalLaunchStartPosition.z) *
		smoothT;

	// 放物線の高さ。
	// t=0 と t=1 では0、t=0.5で最大。
	float arc = 4.0f * t * (1.0f - t);

	basePosition.y += arc * m_GoalLaunchHeight;

	m_Position = basePosition;

	if (m_GoalLaunchFrame >= m_GoalLaunchFrameMax)
	{
		m_Position = m_GoalLaunchTargetPosition;

		SetMainState(PlayerMainState::Playing);

		m_GoalLaunchFrame = 0;

		m_State.velocity = Vector3(0.0f, 0.0f, 0.0f);

		m_State.wallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);

		m_State.isGround = true;
		m_State.isTouchWall = false;
		m_State.isWallKick = false;

		m_State.wallNormal = Vector3(0.0f, 0.0f, 0.0f);

		m_State.currentWallBlock = nullptr;
	}
}

bool Player::IsGoalLaunching() const
{
	return m_State.mainState == PlayerMainState::GoalLaunch;
}

// ゴール建物の上面への到達を調べる
void Player::CheckGoalTopBlock()
{
	if (IsGoalReached() || IsDead())
	{
		return;
	}

	if (m_StageBlockManager == nullptr)
	{
		return;
	}

	StageBlock* hitBlock =
		m_StageBlockManager->GetHitStageBlock(
			m_Position,
			m_MovementParameters.move.hitSize
		);

	if (hitBlock == nullptr)
	{
		return;
	}

	if (!hitBlock->IsGoalTopBlock())
	{
		return;
	}

	if (m_GameTime != nullptr)
	{
		m_GameTime->StopTime();
	}

	StartGoalSequence();
}

// ゴール演出を開始する
void Player::StartGoalSequence()
{
	if (IsGoalReached() || IsDead())
	{
		return;
	}

	SetMainState(PlayerMainState::Goal);
}
