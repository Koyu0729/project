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

// 落下速度を重力で増やす
void PlayerActionResolver::ApplyGravity(
	PlayerRuntimeState& state,
	PlayerJumpParameter& jumpParameter,
	PlayerActionRequest& action
)
{
	if (state.isGround)
	{
		return;
	}

	if (action.useGravityRate)
	{
		float gravity =
			jumpParameter.gravity *
			action.gravityRate;

		if (state.velocity.y <= 0.0f)
		{
			gravity *= 0.55f;
		}

		state.velocity.y -= gravity;

		if (action.useMaxFallSpeed)
		{
			if (state.velocity.y < action.maxFallSpeed)
			{
				state.velocity.y = action.maxFallSpeed;
			}
		}

		return;
	}

	if (state.isTouchWall && state.velocity.y <= 0.0f)
	{
		state.velocity.y -= jumpParameter.wallSlideGravity;

		if (state.velocity.y < jumpParameter.wallSlideMaxFallSpeed)
		{
			state.velocity.y = jumpParameter.wallSlideMaxFallSpeed;
		}

		return;
	}

	state.velocity.y -= jumpParameter.gravity;
}

// 足元のブロックが接地を支えているか調べる
void PlayerActionResolver::UpdateGroundSupport(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerJumpParameter& jumpParameter,
	PlayerActionRequest& action
)
{
	if (action.useVerticalVelocity)
	{
		return;
	}

	if (action.requestAirFloat || action.requestAirDive)
	{
		state.isGround = false;
		return;
	}

	if (state.velocity.y > 0.0f)
	{
		state.isGround = false;
		return;
	}

	if (position.y <= jumpParameter.groundY)
	{
		state.isGround = true;
		return;
	}

	if (IsStandingOnStageBlock(
		position,
		moveParameter
	))
	{
		state.isGround = true;
		return;
	}

	state.isGround = false;
}

// 指定したブロック上に立っているか調べる
bool PlayerActionResolver::IsStandingOnStageBlock(
	Vector3& position,
	PlayerMoveParameter& moveParameter
)
{
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

	float slopeSurfaceY = 0.0f;

	Vector3 slopeNormal =
		Vector3(
			0.0f,
			1.0f,
			0.0f
		);

	StageBlock* slopeBlock = nullptr;

	if (TryGetSlopeRideGroundY(
		position,
		moveParameter,
		slopeSurfaceY,
		slopeNormal,
		slopeBlock
	))
	{
		float playerBottomY =
			position.y -
			moveParameter.hitSize.y;

		float distance =
			playerBottomY -
			slopeSurfaceY;

		if (distance < 0.0f)
		{
			distance = -distance;
		}

		if (distance <= 0.20f)
		{
			return true;
		}
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

	Vector3 blockMax =
		hitBlock->GetMax();

	float playerBottomY =
		position.y -
		moveParameter.hitSize.y;

	float distance =
		playerBottomY -
		blockMax.y;

	if (distance < 0.0f)
	{
		distance = -distance;
	}

	if (distance > 0.15f)
	{
		return false;
	}

	return true;
}

// 床と天井との接触に合わせて上下移動を補正する
void PlayerActionResolver::ApplyVerticalMove(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter
)
{
	Scene* scene =
		Manager::GetScene();

	StageBlockManager* stageBlockManager = nullptr;

	if (scene != nullptr)
	{
		stageBlockManager =
			scene->GetGameObject<StageBlockManager>();
	}

	float oldY =
		position.y;

	position.y +=
		state.velocity.y;

	if (stageBlockManager == nullptr)
	{
		return;
	}

	StageBlock* hitBlock =
		stageBlockManager->GetHitStageBlock(
			position,
			moveParameter.hitSize
		);

	if (hitBlock == nullptr)
	{
		return;
	}

	if (state.velocity.y < 0.0f)
	{
		float slopeSurfaceY = 0.0f;

		Vector3 slopeNormal =
			Vector3(
				0.0f,
				1.0f,
				0.0f
			);

		if (hitBlock->GetSlopeRideSurfaceY(
			position,
			moveParameter.hitSize,
			slopeSurfaceY,
			slopeNormal
		))
		{
			position.y =
				slopeSurfaceY +
				moveParameter.hitSize.y;
		}
		else
		{
			Vector3 blockMax =
				hitBlock->GetMax();

			position.y =
				blockMax.y +
				moveParameter.hitSize.y;
		}

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

		return;
	}

	if (state.velocity.y > 0.0f)
	{
		Vector3 blockMin =
			hitBlock->GetMin();

		position.y =
			blockMin.y -
			moveParameter.hitSize.y;

		state.velocity.y = 0.0f;

		return;
	}

	position.y =
		oldY;
}

// 足元の判定から接地状態を更新する
void PlayerActionResolver::UpdateGround(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerJumpParameter& jumpParameter
)
{
	if (position.y > jumpParameter.groundY)
	{
		return;
	}

	position.y = jumpParameter.groundY;
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

