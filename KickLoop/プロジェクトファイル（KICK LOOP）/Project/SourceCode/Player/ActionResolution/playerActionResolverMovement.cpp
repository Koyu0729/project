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

// 入力に応じて加速か摩擦を適用する
void PlayerActionResolver::ApplyHorizontalVelocityRequest(
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerActionRequest& action
)
{
	if (action.useHorizontalTargetVelocity)
	{
		ApplyAcceleration(
			state,
			moveParameter,
			action.horizontalTargetVelocity
		);
	}
	else
	{
		ApplyFriction(
			state,
			moveParameter
		);
	}

	if (fabsf(state.velocity.x) < moveParameter.stopVelocity)
	{
		state.velocity.x = 0.0f;
	}

	if (fabsf(state.velocity.z) < moveParameter.stopVelocity)
	{
		state.velocity.z = 0.0f;
	}
}

// 目標の水平速度に近づける
void PlayerActionResolver::ApplyAcceleration(
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	Vector3 targetVelocity
)
{
	float acceleration = moveParameter.airAcceleration;

	if (state.isGround)
	{
		acceleration = moveParameter.groundAcceleration;
	}

	state.velocity.x +=
		(targetVelocity.x - state.velocity.x) *
		acceleration;

	state.velocity.z +=
		(targetVelocity.z - state.velocity.z) *
		acceleration;
}

// 入力のない水平速度を減らす
void PlayerActionResolver::ApplyFriction(
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter
)
{
	float friction = moveParameter.airFriction;

	if (state.isGround)
	{
		friction = moveParameter.groundFriction;
	}

	state.velocity.x *= friction;
	state.velocity.z *= friction;
}

// 壁キック直後の追加速度を設定する
void PlayerActionResolver::ApplyStartWallKickVelocity(
	PlayerRuntimeState& state,
	PlayerActionRequest& action
)
{
	if (!action.useStartWallKickVelocity)
	{
		return;
	}

	state.wallKickVelocity = action.startWallKickVelocity;
}

Vector3 PlayerActionResolver::BuildFinalHorizontalMove(
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerActionRequest& action
)
{
	Vector3 move =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	move.x += state.velocity.x;
	move.z += state.velocity.z;

	move.x += state.wallKickVelocity.x;
	move.z += state.wallKickVelocity.z;

	move.x += action.moveVelocity.x;
	move.z += action.moveVelocity.z;

	ApplyWallKickDecay(
		state,
		moveParameter
	);

	return move;
}

// 壁キックの追加速度を徐々に減らす
void PlayerActionResolver::ApplyWallKickDecay(
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter
)
{
	state.wallKickVelocity.x *= moveParameter.wallKickDecay;
	state.wallKickVelocity.z *= moveParameter.wallKickDecay;

	if (fabsf(state.wallKickVelocity.x) < 0.001f)
	{
		state.wallKickVelocity.x = 0.0f;
	}

	if (fabsf(state.wallKickVelocity.z) < 0.001f)
	{
		state.wallKickVelocity.z = 0.0f;
	}
}

// 壁との接触を調べて水平移動を補正する
float PlayerActionResolver::ApplyHorizontalMove(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerActionRequest& action
)
{
	state.currentWallBlock = nullptr;

	Vector3 move =
		BuildFinalHorizontalMove(
			state,
			moveParameter,
			action
		);

	bool dummyTouchWall = false;

	Vector3 dummyWallNormal =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	StageBlock* dummyHitWallBlock = nullptr;

	UpdateWallSlideLock(
		position,
		move,
		moveParameter.hitSize,
		state,
		dummyTouchWall,
		dummyWallNormal,
		dummyHitWallBlock
	);

	float forwardMove =
		move.z +
		action.forwardMove;

	move.z =
		forwardMove;

	bool isTouchWall = false;

	Vector3 wallNormal =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	StageBlock* hitWallBlock = nullptr;

	MoveWithWallSlide(
		position,
		move,
		moveParameter.hitSize,
		state,
		isTouchWall,
		wallNormal,
		hitWallBlock
	);

	ClampMoveX(
		position,
		state,
		moveParameter
	);

	ClampMoveZ(
		position,
		state,
		moveParameter
	);

	UpdateTouchWallState(
		state,
		isTouchWall,
		wallNormal,
		hitWallBlock
	);

	UpdateWallSlideLock(
		position,
		move,
		moveParameter.hitSize,
		state,
		isTouchWall,
		wallNormal,
		hitWallBlock
	);

	return forwardMove;
}

