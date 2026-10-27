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

// 移動の要求を順番に反映する
void PlayerActionResolver::Resolve(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerJumpParameter& jumpParameter,
	PlayerClimbParameter& climbParameter,
	PlayerSlopeRideController& slopeRideController,
	PlayerSlopeRideParameter& slopeRideParameter,
	PlayerActionRequest& action,
	PlayerEffectController& effectController,
	PlayerStateController& stateController,
	PlayerSoundController* soundController
)
{
	bool wasGround = state.isGround;

	if (UpdateLedgeClimb(
		position,
		state,
		moveParameter,
		climbParameter
	))
	{
		UpdateSpeedLineEffect(
			0.0f,
			moveParameter,
			effectController
		);

		UpdateWallSlideEffect(
			state,
			effectController
		);
#ifdef DEBUG
		if (Input::GetKeyTrigger('P'))
		{
			DebugPlayerStatus(
				position,
				state,
				moveParameter,
				action
			);
		}
#endif // DEBUG
		return;
	}

	UpdateBlockRay(
		position,
		state,
		moveParameter,
		climbParameter
	);

	ApplySpeedRequest(
		moveParameter,
		action
	);

	ApplyStateRequest(
		state,
		action,
		effectController,
		stateController
	);

	ApplyVerticalRequest(
		state,
		action
	);

	ApplyAirDiveRequest(
		state,
		action
	);

	ApplyStartWallKickVelocity(
		state,
		action
	);

	UpdateGroundSupport(
		position,
		state,
		moveParameter,
		jumpParameter,
		action
	);

	ApplyHorizontalVelocityRequest(
		state,
		moveParameter,
		action
	);

	slopeRideController.Apply(
		position,
		state,
		moveParameter,
		slopeRideParameter,
		action,
		soundController
	);

	float forwardMove =
		ApplyHorizontalMove(
			position,
			state,
			moveParameter,
			action
		);

	SnapToSlopeGround(
		position,
		state,
		moveParameter
	);

	if (CanStartLedgeClimb(
		position,
		state,
		moveParameter,
		action,
		climbParameter
	))
	{
		StartLedgeClimb(
			position,
			state,
			climbParameter,
			soundController
		);

		UpdateSpeedLineEffect(
			0.0f,
			moveParameter,
			effectController
		);

		UpdateWallSlideEffect(
			state,
			effectController
		);
#ifdef DEBUG
		if (Input::GetKeyTrigger('P'))
		{
			DebugPlayerStatus(
				position,
				state,
				moveParameter,
				action
			);
		}
#endif // DEBUG
		return;
	}

	ApplyGravity(
		state,
		jumpParameter,
		action
	);

	ApplyVerticalMove(
		position,
		state,
		moveParameter
	);

	UpdateGround(
		position,
		state,
		jumpParameter
	);

	if (!wasGround && state.isGround)
	{
		if (soundController != nullptr)
		{
			soundController->PlayLanding();
		}

		effectController.EmitLandingSpark(position);
	}

	UpdateSpeedLineEffect(
		forwardMove,
		moveParameter,
		effectController
	);

	UpdateWallSlideEffect(
		state,
		effectController
	);

#ifdef DEBUG
	if (Input::GetKeyTrigger('P'))
	{
		DebugPlayerStatus(
			position,
			state,
			moveParameter,
			action
		);
	}
#endif // DEBUG
}

