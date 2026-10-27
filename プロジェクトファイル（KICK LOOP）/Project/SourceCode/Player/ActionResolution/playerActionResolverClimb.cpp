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

// 壁の上端を調べてよじ登りを進める
bool PlayerActionResolver::UpdateLedgeClimb(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerClimbParameter& climbParameter
)
{
	if (!state.isLedgeClimb)
	{
		return false;
	}

	state.ledgeClimbFrame++;

	float t =
		(float)state.ledgeClimbFrame /
		(float)climbParameter.climbFrameMax;

	if (t > 1.0f)
	{
		t = 1.0f;
	}

	if (climbParameter.useSmoothStep)
	{
		t =
			SmoothStep(
				t
			);
	}

	float nextX =
		state.ledgeClimbStartPosition.x +
		(state.ledgeClimbEndPosition.x - state.ledgeClimbStartPosition.x) *
		t;

	float nextY =
		state.ledgeClimbStartPosition.y +
		(state.ledgeClimbEndPosition.y - state.ledgeClimbStartPosition.y) *
		t;

	float nextZ =
		state.ledgeClimbStartPosition.z +
		(state.ledgeClimbEndPosition.z - state.ledgeClimbStartPosition.z) *
		t;

	position.x = nextX;
	position.y = nextY;
	position.z = nextZ;

	state.velocity =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	state.wallKickVelocity =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	if (state.ledgeClimbFrame >= climbParameter.climbFrameMax)
	{
		state.isLedgeClimb = false;
		state.ledgeClimbFrame = 0;

		state.isTouchWall = false;
		state.isWallKick = false;

		state.velocity =
			Vector3(
				0.0f,
				0.0f,
				0.0f
			);

		state.wallKickVelocity =
			Vector3(
				0.0f,
				0.0f,
				0.0f
			);

		ClearWallSlideLock(
			state
		);
	}

	return true;
}


// 壁の高さと入力からよじ登り可否を調べる
bool PlayerActionResolver::CanStartLedgeClimb(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerActionRequest& action,
	PlayerClimbParameter& climbParameter
)
{
	if (!climbParameter.enable)
	{
		return false;
	}

	if (state.isLedgeClimb)
	{
		return false;
	}

	if (!action.requestLedgeClimb)
	{
		return false;
	}

	if (state.isGround)
	{
		return false;
	}

	if (!state.isBlockRayHit)
	{
		return false;
	}

	StageBlock* climbBlock =
		state.blockRayHitBlock;

	if (climbBlock == nullptr)
	{
		return false;
	}

	if (!climbBlock->CanClimb())
	{
		return false;
	}

	float blockTopY =
		climbBlock->GetMax().y;

	float bodyY =
		position.y +
		climbParameter.bodyCheckOffsetY;

	float overTop =
		bodyY -
		blockTopY;

	if (overTop < climbParameter.climbMinOverTop)
	{
		return false;
	}

	if (overTop > climbParameter.climbMaxOverTop)
	{
		return false;
	}

	state.currentWallBlock =
		climbBlock;

	return true;
}

// よじ登りの補間を開始する
void PlayerActionResolver::StartLedgeClimb(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerClimbParameter& climbParameter,
	PlayerSoundController* soundController
)
{
	if (state.currentWallBlock == nullptr)
	{
		return;
	}

	if (!state.currentWallBlock->CanClimb())
	{
		return;
	}

	state.isLedgeClimb = true;
	state.ledgeClimbFrame = 0;

	state.ledgeClimbStartPosition = position;

	Vector3 blockMin =
		state.currentWallBlock->GetMin();

	Vector3 blockMax =
		state.currentWallBlock->GetMax();

	Vector3 blockCenter =
		Vector3(
			(blockMin.x + blockMax.x) * 0.5f,
			(blockMin.y + blockMax.y) * 0.5f,
			(blockMin.z + blockMax.z) * 0.5f
		);

	Vector3 forward =
		Vector3(
			position.x - blockCenter.x,
			0.0f,
			position.z - blockCenter.z
		);

	forward =
		NormalizeXZ(
			forward
		);

	if (
		fabsf(forward.x) < 0.0001f &&
		fabsf(forward.z) < 0.0001f
		)
	{
		forward =
			NormalizeXZ(
				state.wallNormal
			);
	}

	if (
		fabsf(forward.x) < 0.0001f &&
		fabsf(forward.z) < 0.0001f
		)
	{
		forward =
			Vector3(
				0.0f,
				0.0f,
				1.0f
			);
	}

	state.ledgeClimbForward =
		forward;

	float blockTopY =
		state.currentWallBlock->GetMax().y;

	state.ledgeClimbEndPosition =
		position;

	state.ledgeClimbEndPosition.y =
		blockTopY +
		climbParameter.endYOffset;

	state.ledgeClimbEndPosition.x +=
		forward.x *
		climbParameter.forwardDistance;

	state.ledgeClimbEndPosition.z +=
		forward.z *
		climbParameter.forwardDistance;

	state.velocity =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	state.wallKickVelocity =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	state.isWallKick = false;

	ClearWallSlideLock(
		state
	);

	if (soundController != nullptr)
	{
		soundController->PlayGrab();
	}

#ifdef _DEBUG
	printf(
		"StartLedgeClimb start(%.2f %.2f %.2f) end(%.2f %.2f %.2f) forward(%.2f %.2f %.2f)\n",
		state.ledgeClimbStartPosition.x,
		state.ledgeClimbStartPosition.y,
		state.ledgeClimbStartPosition.z,
		state.ledgeClimbEndPosition.x,
		state.ledgeClimbEndPosition.y,
		state.ledgeClimbEndPosition.z,
		forward.x,
		forward.y,
		forward.z
	);
#endif
}

// 周囲のブロックをレイで調べる
void PlayerActionResolver::UpdateBlockRay(
	Vector3 position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerClimbParameter& climbParameter
)
{
	state.isBlockRayHit = false;
	state.blockRayHitBlock = nullptr;

	state.blockRayHitPosition =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

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

	float minX =
		moveParameter.minMoveX;

	float maxX =
		moveParameter.maxMoveX;

	stageBlockManager->GetWalkableXRange(
		position.z,
		moveParameter.hitSize.x,
		minX,
		maxX
	);

	float roomCenterX =
		(minX + maxX) *
		0.5f;

	Vector3 rayDirection =
		Vector3(
			roomCenterX - position.x,
			0.0f,
			0.0f
		);

	float length =
		fabsf(
			rayDirection.x
		);

	if (length <= 0.001f)
	{
		return;
	}

	rayDirection.x /=
		length;

	Vector3 checkSize =
		Vector3(
			climbParameter.rayCheckWidth,
			moveParameter.hitSize.y + climbParameter.rayCheckHeightExpand,
			climbParameter.rayCheckDepth
		);

	float step =
		climbParameter.rayStep;

	float maxDistance =
		climbParameter.rayMaxDistance;

	for (float distance = 0.0f; distance <= maxDistance; distance += step)
	{
		Vector3 checkPosition =
			position +
			rayDirection *
			distance;

		StageBlock* hitBlock =
			stageBlockManager->GetHitStageBlock(
				checkPosition,
				checkSize
			);

		if (hitBlock == nullptr)
		{
			continue;
		}

		state.isBlockRayHit = true;
		state.blockRayHitBlock = hitBlock;
		state.blockRayHitPosition = checkPosition;

		return;
	}
}

