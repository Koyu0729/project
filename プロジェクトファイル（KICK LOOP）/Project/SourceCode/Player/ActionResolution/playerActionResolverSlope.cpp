#include "playerActionResolver.h"

#include "manager.h"
#include "scene.h"
#include "stageBlockManager.h"
#include "stageBlock.h"
#include "playerEffectController.h"
#include "playerStateController.h"
#include "playerSoundController.h"
#include "playerSlopeRideController.h"
#include "playerSlopeRideParameter.h"
#include "camera.h"
#include "input.h"

#include <cmath>
#include <cstdio>

// 斜面上の接地位置を求める
bool PlayerActionResolver::TryGetSlopeRideGroundY(
	Vector3 position,
	PlayerMoveParameter& moveParameter,
	float& outSurfaceY,
	Vector3& outNormal,
	StageBlock*& outBlock
)
{
	outSurfaceY = 0.0f;

	outNormal =
		Vector3(
			0.0f,
			1.0f,
			0.0f
		);

	outBlock = nullptr;

	Scene* scene =
		Manager::GetScene();

	if (scene == nullptr)
	{
		return false;
	}

	StageBlockManager* stageBlockManager =
		scene->GetGameObject<StageBlockManager>();

	if (stageBlockManager == nullptr)
	{
		return false;
	}

	Vector3 checkPosition =
		position;

	checkPosition.y -= 0.05f;

	StageBlock* hitBlock =
		stageBlockManager->GetHitStageBlock(
			checkPosition,
			moveParameter.hitSize
		);

	if (hitBlock == nullptr)
	{
		return false;
	}

	float surfaceY = 0.0f;

	Vector3 slopeNormal =
		Vector3(
			0.0f,
			1.0f,
			0.0f
		);

	if (!hitBlock->GetSlopeRideSurfaceY(
		checkPosition,
		moveParameter.hitSize,
		surfaceY,
		slopeNormal
	))
	{
		return false;
	}

	outSurfaceY = surfaceY;
	outNormal = slopeNormal;
	outBlock = hitBlock;

	return true;
}

// プレイヤーの高さを斜面に合わせる
void PlayerActionResolver::SnapToSlopeGround(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter
)
{
	if (state.velocity.y > 0.0f)
	{
		return;
	}

	float slopeSurfaceY = 0.0f;

	Vector3 slopeNormal =
		Vector3(
			0.0f,
			1.0f,
			0.0f
		);

	StageBlock* slopeBlock = nullptr;

	if (!TryGetSlopeRideGroundY(
		position,
		moveParameter,
		slopeSurfaceY,
		slopeNormal,
		slopeBlock
	))
	{
		return;
	}

	float playerBottomY =
		position.y -
		moveParameter.hitSize.y;

	float distance =
		playerBottomY -
		slopeSurfaceY;

	float maxSnapUp = 0.35f;
	float maxSnapDown = 0.75f;

	if (distance > maxSnapUp)
	{
		return;
	}

	if (distance < -maxSnapDown)
	{
		return;
	}

	position.y =
		slopeSurfaceY +
		moveParameter.hitSize.y;

	state.velocity.y = 0.0f;
	state.isGround = true;

	state.wallKickVelocity.x = 0.0f;
	state.wallKickVelocity.z = 0.0f;

	state.isTouchWall = false;
	state.wallNormal =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	state.currentWallBlock = nullptr;
	state.oldBonusWallBlock = nullptr;

	ClearWallSlideLock(
		state
	);
}


