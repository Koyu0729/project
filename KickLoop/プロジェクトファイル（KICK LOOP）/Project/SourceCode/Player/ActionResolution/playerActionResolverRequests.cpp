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

// 走行速度の要求を上下限内に収める
void PlayerActionResolver::ApplySpeedRequest(
	PlayerMoveParameter& moveParameter,
	PlayerActionRequest& action
)
{
	if (action.useResolvedMoveSpeed)
	{
		moveParameter.moveSpeed = action.resolvedMoveSpeed;
	}
	else
	{
		moveParameter.moveSpeed = moveParameter.baseMoveSpeed;
	}

	if (moveParameter.moveSpeed > moveParameter.maxMoveSpeed)
	{
		moveParameter.moveSpeed = moveParameter.maxMoveSpeed;
	}

	if (moveParameter.moveSpeed < moveParameter.minMoveSpeed)
	{
		moveParameter.moveSpeed = moveParameter.minMoveSpeed;
	}
}

// 死亡・壁キック・停止の要求を反映する
void PlayerActionResolver::ApplyStateRequest(
	PlayerRuntimeState& state,
	PlayerActionRequest& action,
	PlayerEffectController& effectController,
	PlayerStateController& stateController
)
{
	if (action.requestGameOver)
	{
		state.mainState = PlayerMainState::Dead;
		return;
	}

	if (action.requestWallKickSuccess)
	{
		state.isWallKick = true;

		ClearWallSlideLock(
			state
		);
	}

	if (action.requestFreeze)
	{
		stateController.StartFreeze(
			state,
			action.freezeFrame
		);
	}

	if (action.requestBoostEffect)
	{
		effectController.PlayBoost();
	}
}

// ジャンプの要求を上向きの速度に反映する
void PlayerActionResolver::ApplyVerticalRequest(
	PlayerRuntimeState& state,
	PlayerActionRequest& action
)
{
	if (!action.useVerticalVelocity)
	{
		return;
	}

	state.velocity.y = action.verticalVelocity;
	state.isGround = false;
}

// 浮上や急降下の速度を反映する
void PlayerActionResolver::ApplyAirDiveRequest(
	PlayerRuntimeState& state,
	PlayerActionRequest& action
)
{
	if (action.requestAirFloat)
	{
		state.velocity.y = action.airFloatVelocity;

		state.velocity.x *= action.airDiveHorizontalBrakeRate;
		state.velocity.z *= action.airDiveHorizontalBrakeRate;

		state.wallKickVelocity.x *= action.airDiveHorizontalBrakeRate;
		state.wallKickVelocity.z *= action.airDiveHorizontalBrakeRate;

		state.isGround = false;
	}

	if (action.requestAirDive)
	{
		state.velocity.y = action.airDiveVelocity;

		state.velocity.x *= action.airDiveHorizontalBrakeRate;
		state.velocity.z *= action.airDiveHorizontalBrakeRate;

		state.wallKickVelocity.x *= action.airDiveHorizontalBrakeRate;
		state.wallKickVelocity.z *= action.airDiveHorizontalBrakeRate;

		state.isGround = false;
	}
}

