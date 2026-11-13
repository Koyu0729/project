#include "main.h"
#include "playerWallSlideController.h"

#include "camera.h"
#include "playerCollision.h"
#include "playerRuntimeState.h"
#include "stageBlock.h"
#include "stageBlockManager.h"

#include <cmath>

void PlayerWallSlideController::SetSceneReferences(Camera* camera, StageBlockManager* stageBlockManager)
{
	m_Camera = camera;
	m_StageBlockManager = stageBlockManager;
}

void PlayerWallSlideController::ApplyContact(PlayerRuntimeState& state, const PlayerWallContact& wall)
{
	state.isTouchWall = wall.block != nullptr;

	if (state.isTouchWall)
	{
		state.wallNormal = wall.normal;
		state.currentWallBlock = wall.block;
	}
	else if (!state.isWallSlideLocked)
	{
		state.wallNormal = Vector3(0.0f, 0.0f, 0.0f);
		state.currentWallBlock = nullptr;
	}
}

void PlayerWallSlideController::Update(
	Vector3& position,
	Vector3& move,
	Vector3 hitSize,
	PlayerRuntimeState& state,
	PlayerWallContact& wall
)
{
	if (state.isGround)
	{
		ClearLock(state);
		return;
	}

	if (state.isWallSlideLocked)
	{
		if (!UpdateLockedContact(position, hitSize, state) || ShouldReleaseByCamera(state))
		{
			ClearLock(state);
			wall = {};
			ApplyContact(state, wall);
			return;
		}

		position.x = state.wallSlideLockPositionX;
		move.x = 0.0f;
		state.velocity.x = 0.0f;
		state.wallKickVelocity.x = 0.0f;

		wall.normal = state.wallSlideLockNormal;
		wall.block = state.wallSlideLockBlock;
		ApplyContact(state, wall);
		return;
	}

	if (wall.block == nullptr || state.velocity.y > 0.0f || fabsf(wall.normal.x) <= 0.1f)
	{
		return;
	}

	if (wall.block->CanWallSlide())
	{
		StartLock(position, state, wall);
	}
}

void PlayerWallSlideController::StartLock(Vector3 position, PlayerRuntimeState& state, const PlayerWallContact& wall)
{
	state.isWallSlideLocked = true;
	state.wallSlideLockNormal = wall.normal;
	state.wallSlideLockPositionX = position.x;
	state.wallSlideLockPositionZ = position.z;
	state.wallSlideLockBlock = wall.block;

	ApplyContact(state, wall);
	state.velocity.x = 0.0f;
	state.wallKickVelocity.x = 0.0f;
}

void PlayerWallSlideController::ClearLock(PlayerRuntimeState& state)
{
	state.isWallSlideLocked = false;
	state.wallSlideLockNormal = Vector3(0.0f, 0.0f, 0.0f);
	state.wallSlideLockPositionX = 0.0f;
	state.wallSlideLockPositionZ = 0.0f;
	state.wallSlideLockBlock = nullptr;
}

// ŒÅ’èæ‚Ì•Ç‚ÉG‚ê‚Ä‚¢‚é‚©’²‚×‚é
bool PlayerWallSlideController::UpdateLockedContact(
	Vector3 position,
	Vector3 hitSize,
	PlayerRuntimeState& state
)
{
	if (m_StageBlockManager == nullptr || !m_StageBlockManager->HasActiveStageBlock(state.wallSlideLockBlock))
	{
		return false;
	}

	if (!state.wallSlideLockBlock->CanWallSlide())
	{
		return false;
	}

	Vector3 checkPosition = position;

	float checkDistance = 0.20f;

	checkPosition.x -= state.wallSlideLockNormal.x * checkDistance;

	StageBlock* hitBlock = m_StageBlockManager->GetHitStageBlock(checkPosition, hitSize);

	if (hitBlock == nullptr)
	{
		return false;
	}

	if (!hitBlock->CanWallSlide())
	{
		return false;
	}

	Vector3 hitNormal = Vector3(0.0f, 0.0f, 0.0f);

	if (!hitBlock->GetHitNormal(
		checkPosition,
		hitSize,
		hitNormal
	))
	{
		return false;
	}

	if (fabsf(hitNormal.x) <= 0.30f)
	{
		return false;
	}

	state.wallSlideLockBlock = hitBlock;
	state.currentWallBlock = hitBlock;
	state.wallSlideLockNormal = hitNormal;

	return true;
}

// Ž‹ü‚ÌŒü‚«‚©‚ç•ÇŠŠ‚è‚Ì‰ðœ‚ð”»’è‚·‚é
bool PlayerWallSlideController::ShouldReleaseByCamera(const PlayerRuntimeState& state) const
{
	Vector3 cameraForward = GetCameraForwardXZ();

	float cameraForwardLength = sqrtf(cameraForward.x * cameraForward.x + cameraForward.z * cameraForward.z);

	if (cameraForwardLength <= 0.001f)
	{
		return false;
	}

	Vector3 wallNormal = state.wallSlideLockNormal;

	float wallNormalLength = sqrtf(wallNormal.x * wallNormal.x + wallNormal.z * wallNormal.z);

	if (wallNormalLength <= 0.001f)
	{
		return false;
	}

	wallNormal.x /= wallNormalLength;

	wallNormal.z /= wallNormalLength;

	float dot = cameraForward.x * wallNormal.x + cameraForward.z * wallNormal.z;

	if (dot >= state.wallSlideLookReleaseDot)
	{
		return true;
	}

	return false;
}

// ƒJƒƒ‰‚Ì‘O•ûŒü‚ð…•½–Ê‚É“Š‰e‚·‚é
Vector3 PlayerWallSlideController::GetCameraForwardXZ() const
{
	Vector3 result = Vector3(0.0f, 0.0f, 0.0f);

	if (m_Camera == nullptr)
	{
		return result;
	}

	result = m_Camera->GetTarget() - m_Camera->GetPosition();

	result.y = 0.0f;

	float length = sqrtf(result.x * result.x + result.z * result.z);

	if (length <= 0.001f)
	{
		return Vector3(0.0f, 0.0f, 0.0f);
	}

	result.x /= length;

	result.z /= length;

	return result;
}
