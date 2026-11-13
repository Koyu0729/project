#include "playerClimbController.h"

#include "manager.h"
#include "scene.h"
#include "stageBlock.h"
#include "stageBlockManager.h"

#include "playerActionRequest.h"
#include "playerMoveParameter.h"
#include "playerSoundController.h"

#include <cmath>

// 壁の高さと入力からよじ登り可否を調べる
bool PlayerClimbController::CanStartLedgeClimb(
	const Vector3& position,
	PlayerRuntimeState& state,
	const PlayerActionRequest& action
)
{
	if (!m_ClimbParameter.enable)
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

	StageBlock* climbBlock = state.blockRayHitBlock;

	if (climbBlock == nullptr)
	{
		return false;
	}

	if (!climbBlock->CanClimb())
	{
		return false;
	}

	float blockTopY = climbBlock->GetMax().y;

	float bodyY = position.y + m_ClimbParameter.bodyCheckOffsetY;

	float overTop = bodyY - blockTopY;

	if (overTop < m_ClimbParameter.climbMinOverTop)
	{
		return false;
	}

	if (overTop > m_ClimbParameter.climbMaxOverTop)
	{
		return false;
	}

	state.currentWallBlock = climbBlock;

	return true;
}

// 周囲のブロックをレイで調べる
void PlayerClimbController::UpdateBlockRay(
	Vector3 position,
	PlayerRuntimeState& state,
	const PlayerMoveParameter& moveParameter
)
{
	state.isBlockRayHit = false;
	state.blockRayHitBlock = nullptr;

	state.blockRayHitPosition = Vector3(0.0f, 0.0f, 0.0f);

	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	StageBlockManager* stageBlockManager = scene->GetGameObject<StageBlockManager>();

	if (stageBlockManager == nullptr)
	{
		return;
	}

	float minX = moveParameter.minMoveX;

	float maxX = moveParameter.maxMoveX;

	stageBlockManager->GetWalkableXRange(
		position.z,
		moveParameter.hitSize.x,
		minX,
		maxX
	);

	float roomCenterX = (minX + maxX) * 0.5f;

	Vector3 rayDirection = Vector3(roomCenterX - position.x, 0.0f, 0.0f);

	float length = fabsf(rayDirection.x);

	if (length <= 0.001f)
	{
		return;
	}

	rayDirection.x /= length;

	Vector3 checkSize =
		Vector3(
			m_ClimbParameter.rayCheckWidth,
			moveParameter.hitSize.y + m_ClimbParameter.rayCheckHeightExpand,
			m_ClimbParameter.rayCheckDepth
		);

	float step = m_ClimbParameter.rayStep;

	float maxDistance = m_ClimbParameter.rayMaxDistance;

	for (float distance = 0.0f; distance <= maxDistance; distance += step)
	{
		Vector3 checkPosition = position + rayDirection * distance;

		StageBlock* hitBlock = stageBlockManager->GetHitStageBlock(checkPosition, checkSize);

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

// よじ登りの開始位置と到達位置を決める。
void PlayerClimbController::Start(
	Vector3& position,
	PlayerRuntimeState& state,
	StageBlock* wallBlock,
	PlayerSoundController* soundController
)
{
	if (wallBlock == nullptr || !wallBlock->CanClimb() || state.isLedgeClimb)
	{
		return;
	}

	Vector3 blockMin = wallBlock->GetMin();
	Vector3 blockMax = wallBlock->GetMax();
	Vector3 blockCenter = Vector3(
		(blockMin.x + blockMax.x) * 0.5f,
		(blockMin.y + blockMax.y) * 0.5f,
		(blockMin.z + blockMax.z) * 0.5f
	);

	Vector3 forward = NormalizeXZ(Vector3(
		position.x - blockCenter.x,
		0.0f,
		position.z - blockCenter.z
	));

	if (fabsf(forward.x) < 0.0001f && fabsf(forward.z) < 0.0001f)
	{
		forward = NormalizeXZ(state.wallNormal);
	}

	if (fabsf(forward.x) < 0.0001f && fabsf(forward.z) < 0.0001f)
	{
		forward = Vector3(0.0f, 0.0f, 1.0f);
	}

	state.isLedgeClimb = true;
	state.ledgeClimbFrame = 0;
	state.ledgeClimbStartPosition = position;
	state.ledgeClimbForward = forward;

	state.ledgeClimbEndPosition = position;
	state.ledgeClimbEndPosition.y = blockMax.y + m_ClimbParameter.endYOffset;
	state.ledgeClimbEndPosition.x += forward.x * m_ClimbParameter.forwardDistance;
	state.ledgeClimbEndPosition.z += forward.z * m_ClimbParameter.forwardDistance;

	state.velocity = Vector3(0.0f, 0.0f, 0.0f);
	state.wallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);
	state.isGround = false;
	state.isCrouchRide = false;
	ClearWallContact(state);

	if (soundController != nullptr)
	{
		soundController->PlayGrab();
	}
}

// 補間中は通常移動を止め、最終フレームもResolverへ処理済みと返す。
bool PlayerClimbController::Update(
	Vector3& position,
	PlayerRuntimeState& state
)
{
	if (!state.isLedgeClimb)
	{
		return false;
	}

	int frameMax = m_ClimbParameter.climbFrameMax;
	if (frameMax < 1)
	{
		frameMax = 1;
	}

	state.ledgeClimbFrame++;
	float t = (float)state.ledgeClimbFrame / (float)frameMax;
	if (t > 1.0f)
	{
		t = 1.0f;
	}

	if (m_ClimbParameter.useSmoothStep)
	{
		t = SmoothStep(t);
	}

	position.x = state.ledgeClimbStartPosition.x + (state.ledgeClimbEndPosition.x - state.ledgeClimbStartPosition.x) * t;
	position.y = state.ledgeClimbStartPosition.y + (state.ledgeClimbEndPosition.y - state.ledgeClimbStartPosition.y) * t;
	position.z = state.ledgeClimbStartPosition.z + (state.ledgeClimbEndPosition.z - state.ledgeClimbStartPosition.z) * t;

	state.velocity = Vector3(0.0f, 0.0f, 0.0f);
	state.wallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);

	if (state.ledgeClimbFrame >= frameMax)
	{
		position = state.ledgeClimbEndPosition;
		End(state);
	}

	return true;
}

void PlayerClimbController::End(PlayerRuntimeState& state)
{
	state.isLedgeClimb = false;
	state.ledgeClimbFrame = 0;
	state.velocity = Vector3(0.0f, 0.0f, 0.0f);
	state.wallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);
	ClearWallContact(state);
}

// 登り終えた壁に再度吸着したり、離脱猶予で壁キックしたりしないようにする。
void PlayerClimbController::ClearWallContact(PlayerRuntimeState& state)
{
	state.isTouchWall = false;
	state.isWallKick = false;
	state.wallNormal = Vector3(0.0f, 0.0f, 0.0f);
	state.currentWallBlock = nullptr;

	state.isWallSlideLocked = false;
	state.wallSlideLockNormal = Vector3(0.0f, 0.0f, 0.0f);
	state.wallSlideLockPositionX = 0.0f;
	state.wallSlideLockPositionZ = 0.0f;
	state.wallSlideLockBlock = nullptr;

	state.wallKickCoyoteFrame = 0;
	state.wallKickCoyoteNormal = Vector3(0.0f, 0.0f, 0.0f);
	state.wallKickCoyoteBlock = nullptr;
}

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
