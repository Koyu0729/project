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

// ブロックとの接触に合わせて移動を補正する
void PlayerActionResolver::MoveWithWallSlide(
	Vector3& position,
	Vector3 move,
	Vector3 playerHitSize,
	PlayerRuntimeState& state,
	bool& isTouchWall,
	Vector3& wallNormal,
	StageBlock*& hitWallBlock
)
{
	Scene* scene =
		Manager::GetScene();

	if (scene == nullptr)
	{
		position.x += move.x;
		position.z += move.z;
		return;
	}

	StageBlockManager* stageBlockManager =
		scene->GetGameObject<StageBlockManager>();

	if (stageBlockManager == nullptr)
	{
		position.x += move.x;
		position.z += move.z;
		return;
	}

	float maxMove =
		fabsf(move.x);

	if (fabsf(move.z) > maxMove)
	{
		maxMove =
			fabsf(move.z);
	}

	float stepUnit =
		playerHitSize.x;

	if (playerHitSize.z < stepUnit)
	{
		stepUnit =
			playerHitSize.z;
	}

	stepUnit *= 0.35f;

	if (stepUnit < 0.05f)
	{
		stepUnit = 0.05f;
	}

	int stepCount =
		(int)(maxMove / stepUnit) + 1;

	if (stepCount < 1)
	{
		stepCount = 1;
	}

	if (stepCount > 96)
	{
		stepCount = 96;
	}

	Vector3 stepMove =
		Vector3(
			move.x / (float)stepCount,
			0.0f,
			move.z / (float)stepCount
		);

	for (int step = 0; step < stepCount; step++)
	{
		float oldX =
			position.x;

		position.x +=
			stepMove.x;

		float minX =
			0.0f;

		float maxX =
			0.0f;

		if (stageBlockManager->GetWalkableXRange(
			position.z,
			playerHitSize.x,
			minX,
			maxX
		))
		{
			if (position.x < minX)
			{
				position.x =
					minX;

				state.velocity.x = 0.0f;
				state.wallKickVelocity.x = 0.0f;
			}

			if (position.x > maxX)
			{
				position.x =
					maxX;

				state.velocity.x = 0.0f;
				state.wallKickVelocity.x = 0.0f;
			}
		}

		StageBlock* hitBlockX =
			stageBlockManager->GetHitStageBlock(
				position,
				playerHitSize
			);

		if (hitBlockX != nullptr)
		{
			Vector3 hitNormal =
				Vector3(
					0.0f,
					0.0f,
					0.0f
				);

			hitBlockX->GetHitNormal(
				position,
				playerHitSize,
				hitNormal
			);

			bool isGroundOnlyContact =
				IsGroundOnlyHorizontalContact(
					position,
					playerHitSize,
					state,
					hitBlockX,
					hitNormal
				);

			if (!isGroundOnlyContact)
			{
				position.x =
					oldX;

				state.velocity.x = 0.0f;
				state.wallKickVelocity.x = 0.0f;

				if (fabsf(hitNormal.x) > fabsf(hitNormal.z))
				{
					if (fabsf(hitNormal.x) > 0.30f)
					{
						if (hitBlockX->CanWallSlide())
						{
							isTouchWall = true;
							hitWallBlock = hitBlockX;
							wallNormal = hitNormal;
						}
					}
				}
			}
		}

		float oldZ =
			position.z;

		position.z +=
			stepMove.z;

		StageBlock* hitBlockZ =
			stageBlockManager->GetHitStageBlock(
				position,
				playerHitSize
			);

		if (hitBlockZ != nullptr)
		{
			Vector3 hitNormal =
				Vector3(
					0.0f,
					0.0f,
					0.0f
				);

			hitBlockZ->GetHitNormal(
				position,
				playerHitSize,
				hitNormal
			);

			bool isGroundOnlyContact =
				IsGroundOnlyHorizontalContact(
					position,
					playerHitSize,
					state,
					hitBlockZ,
					hitNormal
				);

			if (!isGroundOnlyContact)
			{
				position.z =
					oldZ;

				state.velocity.z = 0.0f;
				state.wallKickVelocity.z = 0.0f;
			}
		}
	}
}

// 横方向の接触が床だけか調べる
bool PlayerActionResolver::IsGroundOnlyHorizontalContact(
	Vector3 position,
	Vector3 playerHitSize,
	PlayerRuntimeState& state,
	StageBlock* hitBlock,
	Vector3 hitNormal
)
{
	if (hitBlock == nullptr)
	{
		return false;
	}

	if (!state.isGround)
	{
		return false;
	}

	if (state.velocity.y > 0.0f)
	{
		return false;
	}

	if (hitNormal.y < 0.35f)
	{
		return false;
	}

	Vector3 blockMax =
		hitBlock->GetMax();

	float playerBottomY =
		position.y -
		playerHitSize.y;

	float distanceY =
		playerBottomY -
		blockMax.y;

	if (distanceY < 0.0f)
	{
		distanceY =
			-distanceY;
	}

	if (distanceY <= 0.20f)
	{
		return true;
	}

	return false;
}

// 横方向の位置をステージ内に収める
void PlayerActionResolver::ClampMoveX(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter
)
{
	float minX =
		moveParameter.minMoveX;

	float maxX =
		moveParameter.maxMoveX;

	Scene* scene =
		Manager::GetScene();

	StageBlockManager* stageBlockManager = nullptr;

	if (scene != nullptr)
	{
		stageBlockManager =
			scene->GetGameObject<StageBlockManager>();

		if (stageBlockManager != nullptr)
		{
			stageBlockManager->GetWalkableXRange(
				position.z,
				moveParameter.hitSize.x,
				minX,
				maxX
			);
		}
	}

	float beforeClampX =
		position.x;

	if (position.x < minX)
	{
		position.x =
			minX;
	}

	if (position.x > maxX)
	{
		position.x =
			maxX;
	}

	if (stageBlockManager == nullptr)
	{
		return;
	}

	if (!stageBlockManager->IsHitAnyStageBlock(
		position,
		moveParameter.hitSize
	))
	{
		return;
	}

	position.x =
		beforeClampX;

	state.velocity.x = 0.0f;
	state.wallKickVelocity.x = 0.0f;
}

// 前後方向の位置をステージ内に収める
void PlayerActionResolver::ClampMoveZ(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter
)
{
	Scene* scene =
		Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	StageBlockManager* stageBlockManager =
		scene->GetGameObject<StageBlockManager>();

	if (stageBlockManager == nullptr)
	{
		return;
	}

	float minZ = 0.0f;
	float maxZ = 0.0f;

	if (!stageBlockManager->GetStageZClampRange(
		moveParameter.hitSize.z,
		minZ,
		maxZ
	))
	{
		return;
	}

	if (position.z < minZ)
	{
		position.z =
			minZ;

		state.velocity.z = 0.0f;
		state.wallKickVelocity.z = 0.0f;
	}

	if (position.z > maxZ)
	{
		position.z =
			maxZ;

		state.velocity.z = 0.0f;
		state.wallKickVelocity.z = 0.0f;
	}
}

// 接触した壁と向きを記録する
void PlayerActionResolver::UpdateTouchWallState(
	PlayerRuntimeState& state,
	bool isTouchWall,
	Vector3 wallNormal,
	StageBlock* hitWallBlock
)
{
	state.isTouchWall = isTouchWall;

	if (state.isTouchWall)
	{
		state.wallNormal = wallNormal;
		state.currentWallBlock = hitWallBlock;
	}
	else
	{
		if (!state.isWallSlideLocked)
		{
			state.wallNormal =
				Vector3(
					0.0f,
					0.0f,
					0.0f
				);

			state.currentWallBlock = nullptr;
		}
	}
}

