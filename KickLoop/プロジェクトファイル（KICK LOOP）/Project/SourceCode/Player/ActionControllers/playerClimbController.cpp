#include "playerClimbController.h"

#include "stageBlock.h"
#include "playerSoundController.h"

#include <cmath>

void PlayerClimbController::Init()
{
}


// よじ登りの開始位置と到達位置を決める
void PlayerClimbController::Start(
	Vector3& position,
	PlayerRuntimeState& state,
	StageBlock* wallBlock,
	const Vector3& wallNormal,
	const PlayerClimbParameter& parameter,
	PlayerSoundController* soundController
)
{
	if (wallBlock == nullptr)
	{
		return;
	}

	state.isLedgeClimb = true;
	state.ledgeClimbFrame = 0;
	state.ledgeClimbStartPosition = position;

	Vector3 forward = NormalizeXZ(wallNormal);

	if (fabsf(forward.x) < 0.0001f && fabsf(forward.z) < 0.0001f)
	{
		forward = NormalizeXZ(state.wallNormal);
	}

	if (fabsf(forward.x) < 0.0001f && fabsf(forward.z) < 0.0001f)
	{
		forward = Vector3(0.0f, 0.0f, 1.0f);
	}

	state.ledgeClimbForward = forward;

	float blockTopY = wallBlock->GetMax().y;

	state.ledgeClimbEndPosition = position;
	state.ledgeClimbEndPosition.y = blockTopY + parameter.endYOffset;
	state.ledgeClimbEndPosition.x += forward.x * parameter.forwardDistance;
	state.ledgeClimbEndPosition.z += forward.z * parameter.forwardDistance;

	state.velocity = Vector3(0.0f, 0.0f, 0.0f);
	state.wallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);

	state.isWallKick = false;
	state.isTouchWall = false;

	if (soundController != nullptr)
	{
		soundController->PlayGrab();
	}
}

// 開始位置から到達位置へ補間する
bool PlayerClimbController::Update(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	const PlayerClimbParameter& parameter
)
{
	if (!state.isLedgeClimb)
	{
		return false;
	}

	state.ledgeClimbFrame++;

	float t = (float)state.ledgeClimbFrame / (float)parameter.climbFrameMax;

	if (t > 1.0f)
	{
		t = 1.0f;
	}

	if (parameter.useSmoothStep)
	{
		t = SmoothStep(t);
	}

	position.x = state.ledgeClimbStartPosition.x + (state.ledgeClimbEndPosition.x - state.ledgeClimbStartPosition.x) * t;
	position.y = state.ledgeClimbStartPosition.y + (state.ledgeClimbEndPosition.y - state.ledgeClimbStartPosition.y) * t;
	position.z = state.ledgeClimbStartPosition.z + (state.ledgeClimbEndPosition.z - state.ledgeClimbStartPosition.z) * t;

	state.velocity = Vector3(0.0f, 0.0f, 0.0f);
	state.wallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);

	if (state.ledgeClimbFrame >= parameter.climbFrameMax)
	{
		state.isLedgeClimb = false;
		state.ledgeClimbFrame = 0;

		state.isTouchWall = false;
		state.isWallKick = false;

		state.velocity = Vector3(0.0f, 0.0f, 0.0f);
		state.wallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);
	}

	return true;
}

// よじ登りの始まりと終わりを滑らかにする
float PlayerClimbController::SmoothStep(float t)
{
	if (t < 0.0f)
	{
		t = 0.0f;
	}

	if (t > 1.0f)
	{
		t = 1.0f;
	}

	return t * t * (3.0f - 2.0f * t);
}

Vector3 PlayerClimbController::NormalizeXZ(Vector3 vector)
{
	vector.y = 0.0f;

	float length = sqrtf(vector.x * vector.x + vector.z * vector.z);

	if (length <= 0.0001f)
	{
		return Vector3(0.0f, 0.0f, 0.0f);
	}

	vector.x /= length;
	vector.z /= length;

	return vector;
}
