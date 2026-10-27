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

// 移動速度と壁滑りに合わせて速度線を更新する
void PlayerActionResolver::UpdateSpeedLineEffect(
	float forwardMove,
	PlayerMoveParameter& moveParameter,
	PlayerEffectController& effectController
)
{
	float moveAmount =
		fabsf(forwardMove);

	float maxSpeed =
		moveParameter.maxMoveSpeed;

	if (maxSpeed <= 0.001f)
	{
		effectController.SetSpeedRate(0.0f);
		return;
	}

	float speedRate =
		moveAmount /
		maxSpeed;

	effectController.SetSpeedRate(
		speedRate
	);
}

// 壁滑り中の演出を更新する
void PlayerActionResolver::UpdateWallSlideEffect(
	PlayerRuntimeState& state,
	PlayerEffectController& effectController
)
{
	bool isWallSlideEffect =
		!state.isGround &&
		state.isTouchWall &&
		state.velocity.y <= 0.0f &&
		fabsf(state.wallNormal.x) > 0.1f;

	effectController.SetWallSlideEffect(
		isWallSlideEffect,
		state.wallNormal.x
	);
}

float PlayerActionResolver::SmoothStep(float t)
{
	if (t < 0.0f)
	{
		t = 0.0f;
	}

	if (t > 1.0f)
	{
		t = 1.0f;
	}

	return
		t *
		t *
		(3.0f - 2.0f * t);
}

Vector3 PlayerActionResolver::NormalizeXZ(Vector3 vector)
{
	vector.y = 0.0f;

	float length =
		sqrtf(
			vector.x * vector.x +
			vector.z * vector.z
		);

	if (length <= 0.0001f)
	{
		return
			Vector3(
				0.0f,
				0.0f,
				0.0f
			);
	}

	vector.x /=
		length;

	vector.z /=
		length;

	return vector;
}

// 位置・速度・状態をデバッグ出力する
void PlayerActionResolver::DebugPlayerStatus(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerActionRequest& action
)
{
#ifdef _DEBUG
	printf(
		"Pos(%.2f, %.2f, %.2f) "
		"Vel(%.2f, %.2f, %.2f) "
		"WallKick(%.2f, %.2f, %.2f) "
		"MoveSpeed %.2f "
		"ActionMove(%.2f, %.2f, %.2f) "
		"ForwardMove %.2f "
		"Ground %d Wall %d WallNormal(%.2f, %.2f, %.2f) "
		"CurrentWall %p OldBonusWall %p "
		"AirFloat %d AirDive %d "
		"LedgeClimb %d LedgeFrame %d "
		"WallSlideLock %d LockNormal(%.2f, %.2f, %.2f) LockBlock %p\n",
		position.x,
		position.y,
		position.z,

		state.velocity.x,
		state.velocity.y,
		state.velocity.z,

		state.wallKickVelocity.x,
		state.wallKickVelocity.y,
		state.wallKickVelocity.z,

		moveParameter.moveSpeed,

		action.moveVelocity.x,
		action.moveVelocity.y,
		action.moveVelocity.z,

		action.forwardMove,

		state.isGround ? 1 : 0,
		state.isTouchWall ? 1 : 0,

		state.wallNormal.x,
		state.wallNormal.y,
		state.wallNormal.z,

		state.currentWallBlock,
		state.oldBonusWallBlock,

		action.requestAirFloat ? 1 : 0,
		action.requestAirDive ? 1 : 0,

		state.isLedgeClimb ? 1 : 0,
		state.ledgeClimbFrame,

		state.isWallSlideLocked ? 1 : 0,

		state.wallSlideLockNormal.x,
		state.wallSlideLockNormal.y,
		state.wallSlideLockNormal.z,

		state.wallSlideLockBlock
	);
#endif
}