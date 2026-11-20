#include "playerStateController.h"

void PlayerStateController::Init(PlayerRuntimeState& state)
{
	state.mainState = PlayerMainState::Wait;
	state.freezeFrame = 0;
	state.isFallCameraMode = false;
	m_FreezeReturnState = PlayerMainState::Playing;
	EnterState(state);
}

void PlayerStateController::UpdateState(
	PlayerRuntimeState& state,
	const Vector3& position,
	float fallCameraStartY,
	float fallSceneChangeY,
	bool isInputBlocked
)
{
	switch (state.mainState)
	{
	case PlayerMainState::Wait:
		if (!isInputBlocked)
		{
			ChangeState(state, PlayerMainState::Playing);
		}
		break;

	case PlayerMainState::Freeze:
		UpdateFreezeState(state);
		break;

	default:
		break;
	}

	CheckGameplayState(state, position, fallCameraStartY, fallSceneChangeY, isInputBlocked);
}

void PlayerStateController::CheckGameplayState(
	PlayerRuntimeState& state,
	const Vector3& position,
	float fallCameraStartY,
	float fallSceneChangeY,
	bool isInputBlocked
)
{
	if (state.mainState == PlayerMainState::Playing)
	{
		if (isInputBlocked)
		{
			ChangeState(state, PlayerMainState::Wait);
		}
		else if (position.y <= fallSceneChangeY)
		{
			ChangeState(state, PlayerMainState::Dead);
		}
		else if (position.y <= fallCameraStartY)
		{
			ChangeState(state, PlayerMainState::Falling);
		}
	}
	else if (state.mainState == PlayerMainState::Falling && position.y <= fallSceneChangeY)
	{
		ChangeState(state, PlayerMainState::Dead);
	}
}

void PlayerStateController::StartFreeze(PlayerRuntimeState& state, int frame)
{
	if (frame <= 0 || (!IsPlaying(state) && state.mainState != PlayerMainState::Freeze))
	{
		return;
	}

	if (state.mainState != PlayerMainState::Freeze)
	{
		m_FreezeReturnState = state.mainState;
		ChangeState(state, PlayerMainState::Freeze);
	}

	if (state.freezeFrame < frame)
	{
		state.freezeFrame = frame;
	}
}

void PlayerStateController::UpdateFreezeState(PlayerRuntimeState& state)
{
	// 残り時間のあるフレームは停止したままにし、次のフレームから再開する。
	if (state.freezeFrame > 0)
	{
		state.freezeFrame--;
		return;
	}

	ChangeState(state, m_FreezeReturnState);
}

void PlayerStateController::ChangeState(PlayerRuntimeState& state, PlayerMainState nextState)
{
	if (state.mainState == nextState)
	{
		return;
	}

	ExitState(state);
	state.mainState = nextState;
	EnterState(state);
}

void PlayerStateController::EnterState(PlayerRuntimeState& state)
{
	switch (state.mainState)
	{
	case PlayerMainState::Falling:
		state.isFallCameraMode = true;
		break;

	case PlayerMainState::GoalLaunch:
	case PlayerMainState::Goal:
		state.isFallCameraMode = false;
		ResetMotion(state);
		state.isGround = state.mainState == PlayerMainState::Goal;
		break;

	case PlayerMainState::Dead:
		ResetMotion(state);
		break;

	default:
		break;
	}
}

void PlayerStateController::ExitState(PlayerRuntimeState& state)
{
	if (state.mainState == PlayerMainState::Freeze)
	{
		state.freezeFrame = 0;
	}
}

void PlayerStateController::ResetMotion(PlayerRuntimeState& state)
{
	state.velocity = Vector3(0.0f, 0.0f, 0.0f);
	state.wallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);
	state.isGround = false;
	state.isTouchWall = false;
	state.isWallKick = false;
	state.isCrouchRide = false;
	state.isLedgeClimb = false;
	state.ledgeClimbFrame = 0;
	state.wallNormal = Vector3(0.0f, 0.0f, 0.0f);
	state.currentWallBlock = nullptr;
	state.oldBonusWallBlock = nullptr;
	state.wallKickCoyoteFrame = 0;
	state.wallKickCoyoteNormal = Vector3(0.0f, 0.0f, 0.0f);
	state.wallKickCoyoteBlock = nullptr;
	state.isWallSlideLocked = false;
	state.wallSlideLockNormal = Vector3(0.0f, 0.0f, 0.0f);
	state.wallSlideLockPositionX = 0.0f;
	state.wallSlideLockPositionZ = 0.0f;
	state.wallSlideLockBlock = nullptr;
	state.isBlockRayHit = false;
	state.blockRayHitBlock = nullptr;
	state.blockRayHitPosition = Vector3(0.0f, 0.0f, 0.0f);
}

bool PlayerStateController::IsPlaying(const PlayerRuntimeState& state)
{
	return state.mainState == PlayerMainState::Playing || state.mainState == PlayerMainState::Falling;
}

bool PlayerStateController::IsDead(const PlayerRuntimeState& state)
{
	return state.mainState == PlayerMainState::Dead;
}
