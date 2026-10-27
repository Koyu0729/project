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

// 固定先の壁を調べて壁滑りを続ける
void PlayerActionResolver::UpdateWallSlideLock(
	Vector3& position,
	Vector3& move,
	Vector3 playerHitSize,
	PlayerRuntimeState& state,
	bool& isTouchWall,
	Vector3& wallNormal,
	StageBlock*& hitWallBlock
)
{
	if (state.isGround)
	{
		ClearWallSlideLock(
			state
		);

		return;
	}

	if (state.isWallSlideLocked)
	{
		if (!UpdateStillTouchingWallSlideLock(
			position,
			playerHitSize,
			state
		))
		{
			ClearWallSlideLock(
				state
			);

			isTouchWall = false;
			wallNormal =
				Vector3(
					0.0f,
					0.0f,
					0.0f
				);

			hitWallBlock = nullptr;

			state.isTouchWall = false;
			state.wallNormal =
				Vector3(
					0.0f,
					0.0f,
					0.0f
				);

			state.currentWallBlock = nullptr;

			return;
		}

		if (ShouldReleaseWallSlideLockByCamera(
			state
		))
		{
			ClearWallSlideLock(
				state
			);

			isTouchWall = false;
			wallNormal =
				Vector3(
					0.0f,
					0.0f,
					0.0f
				);

			hitWallBlock = nullptr;

			state.isTouchWall = false;
			state.wallNormal =
				Vector3(
					0.0f,
					0.0f,
					0.0f
				);

			state.currentWallBlock = nullptr;

			return;
		}

		position.x =
			state.wallSlideLockPositionX;

		move.x = 0.0f;

		state.velocity.x = 0.0f;
		state.wallKickVelocity.x = 0.0f;

		state.isTouchWall = true;
		state.wallNormal = state.wallSlideLockNormal;
		state.currentWallBlock = state.wallSlideLockBlock;

		isTouchWall = true;
		wallNormal = state.wallSlideLockNormal;
		hitWallBlock = state.wallSlideLockBlock;

		return;
	}

	if (!isTouchWall)
	{
		return;
	}

	if (state.velocity.y > 0.0f)
	{
		return;
	}

	if (fabsf(wallNormal.x) <= 0.1f)
	{
		return;
	}

	if (hitWallBlock == nullptr)
	{
		return;
	}

	if (!hitWallBlock->CanWallSlide())
	{
		return;
	}

	StartWallSlideLock(
		position,
		state,
		wallNormal,
		hitWallBlock
	);
}

// 壁滑りで使う壁と位置を記録する
void PlayerActionResolver::StartWallSlideLock(
	Vector3& position,
	PlayerRuntimeState& state,
	Vector3 wallNormal,
	StageBlock* hitWallBlock
)
{
	if (hitWallBlock == nullptr)
	{
		return;
	}

	if (!hitWallBlock->CanWallSlide())
	{
		return;
	}

	if (fabsf(wallNormal.x) <= 0.1f)
	{
		return;
	}

	state.isWallSlideLocked = true;

	state.wallSlideLockNormal = wallNormal;

	state.wallSlideLockPositionX = position.x;
	state.wallSlideLockPositionZ = position.z;

	state.wallSlideLockBlock = hitWallBlock;

	state.isTouchWall = true;
	state.wallNormal = wallNormal;
	state.currentWallBlock = hitWallBlock;

	state.velocity.x = 0.0f;
	state.wallKickVelocity.x = 0.0f;
}

// 壁滑りの固定情報を消す
void PlayerActionResolver::ClearWallSlideLock(
	PlayerRuntimeState& state
)
{
	state.isWallSlideLocked = false;

	state.wallSlideLockNormal =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	state.wallSlideLockPositionX = 0.0f;
	state.wallSlideLockPositionZ = 0.0f;

	state.wallSlideLockBlock = nullptr;
}

// 固定先の壁に触れているか調べる
bool PlayerActionResolver::UpdateStillTouchingWallSlideLock(
	Vector3 position,
	Vector3 playerHitSize,
	PlayerRuntimeState& state
)
{
	if (!state.isWallSlideLocked)
	{
		return false;
	}

	if (state.wallSlideLockBlock == nullptr)
	{
		return false;
	}

	if (!state.wallSlideLockBlock->CanWallSlide())
	{
		return false;
	}

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

	float checkDistance =
		0.20f;

	checkPosition.x -=
		state.wallSlideLockNormal.x *
		checkDistance;

	StageBlock* hitBlock =
		stageBlockManager->GetHitStageBlock(
			checkPosition,
			playerHitSize
		);

	if (hitBlock == nullptr)
	{
		return false;
	}

	if (!hitBlock->CanWallSlide())
	{
		return false;
	}

	Vector3 hitNormal =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	if (!hitBlock->GetHitNormal(
		checkPosition,
		playerHitSize,
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

// 視線の向きから壁滑りの解除を判定する
bool PlayerActionResolver::ShouldReleaseWallSlideLockByCamera(
	PlayerRuntimeState& state
)
{
	Vector3 cameraForward =
		GetCameraForwardXZ();

	float cameraForwardLength =
		sqrtf(
			cameraForward.x * cameraForward.x +
			cameraForward.z * cameraForward.z
		);

	if (cameraForwardLength <= 0.001f)
	{
		return false;
	}

	Vector3 wallNormal =
		state.wallSlideLockNormal;

	float wallNormalLength =
		sqrtf(
			wallNormal.x * wallNormal.x +
			wallNormal.z * wallNormal.z
		);

	if (wallNormalLength <= 0.001f)
	{
		return false;
	}

	wallNormal.x /=
		wallNormalLength;

	wallNormal.z /=
		wallNormalLength;

	float dot =
		cameraForward.x * wallNormal.x +
		cameraForward.z * wallNormal.z;

	if (dot >= state.wallSlideLookReleaseDot)
	{
		return true;
	}

	return false;
}

// カメラの前方向を水平面に投影する
Vector3 PlayerActionResolver::GetCameraForwardXZ()
{
	Vector3 result =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	Scene* scene =
		Manager::GetScene();

	if (scene == nullptr)
	{
		return result;
	}

	Camera* camera =
		scene->GetGameObject<Camera>();

	if (camera == nullptr)
	{
		return result;
	}

	result =
		camera->GetTarget() -
		camera->GetPosition();

	result.y = 0.0f;

	float length =
		sqrtf(
			result.x * result.x +
			result.z * result.z
		);

	if (length <= 0.001f)
	{
		return
			Vector3(
				0.0f,
				0.0f,
				0.0f
			);
	}

	result.x /=
		length;

	result.z /=
		length;

	return result;
}

