#include "playerCollision.h"

#include "playerMoveParameter.h"
#include "stageBlock.h"
#include "stageBlockManager.h"

#include <algorithm>
#include <cmath>

namespace
{
	int GetHorizontalStepCount(Vector3 move, Vector3 hitSize)
	{
		const float distance = std::max(fabsf(move.x), fabsf(move.z));
		const float stepUnit = std::max(std::min(hitSize.x, hitSize.z) * 0.35f, 0.05f);
		const int stepCount = (int)(distance / stepUnit) + 1;
		return std::min(std::max(stepCount, 1), 96);
	}

	bool IsFloorContact(Vector3 position, Vector3 hitSize, StageBlock& block, Vector3 normal)
	{
		if (normal.y < 0.35f)
		{
			return false;
		}

		float distanceY = position.y - hitSize.y - block.GetMax().y;
		return fabsf(distanceY) <= 0.20f;
	}
}

PlayerHorizontalCollisionResult PlayerCollision::MoveHorizontal(
	Vector3 position,
	Vector3 move,
	const PlayerMoveParameter& parameter,
	bool ignoreFloorContact
) const
{
	PlayerHorizontalCollisionResult result;
	result.position = position;

	if (m_StageBlockManager != nullptr)
	{
		const int stepCount = GetHorizontalStepCount(move, parameter.hitSize);
		const float stepX = move.x / (float)stepCount;
		const float stepZ = move.z / (float)stepCount;

		// 1ステップずつX、Zの順に動かす。
		for (int step = 0; step < stepCount; step++)
		{
			MoveX(result, stepX, parameter.hitSize, ignoreFloorContact);
			MoveZ(result, stepZ, parameter.hitSize, ignoreFloorContact);
		}
	}
	else
	{
		result.position.x += move.x;
		result.position.z += move.z;
	}

	ClampX(result, parameter);
	ClampZ(result, parameter.hitSize);
	return result;
}

void PlayerCollision::MoveX(
	PlayerHorizontalCollisionResult& result,
	float moveX,
	Vector3 hitSize,
	bool ignoreFloorContact
) const
{
	Vector3& position = result.position;
	float oldX = position.x;
	position.x += moveX;

	float minX = 0.0f;
	float maxX = 0.0f;
	if (m_StageBlockManager->GetWalkableXRange(position.z, hitSize.x, minX, maxX))
	{
		if (position.x < minX)
		{
			position.x = minX;
			result.stopX = true;
		}
		if (position.x > maxX)
		{
			position.x = maxX;
			result.stopX = true;
		}
	}

	StageBlock* hitBlock = m_StageBlockManager->GetHitStageBlock(position, hitSize);
	if (hitBlock == nullptr)
	{
		return;
	}

	Vector3 normal = Vector3(0.0f, 0.0f, 0.0f);
	hitBlock->GetHitNormal(position, hitSize, normal);

	if (ignoreFloorContact && IsFloorContact(position, hitSize, *hitBlock, normal))
	{
		return;
	}

	position.x = oldX;
	result.stopX = true;

	float normalX = fabsf(normal.x);
	if (normalX > fabsf(normal.z) && normalX > 0.30f && hitBlock->CanWallSlide())
	{
		result.wall.normal = normal;
		result.wall.block = hitBlock;
	}
}

void PlayerCollision::MoveZ(
	PlayerHorizontalCollisionResult& result,
	float moveZ,
	Vector3 hitSize,
	bool ignoreFloorContact
) const
{
	Vector3& position = result.position;
	float oldZ = position.z;
	position.z += moveZ;

	StageBlock* hitBlock = m_StageBlockManager->GetHitStageBlock(position, hitSize);
	if (hitBlock == nullptr)
	{
		return;
	}

	Vector3 normal = Vector3(0.0f, 0.0f, 0.0f);
	hitBlock->GetHitNormal(position, hitSize, normal);

	if (ignoreFloorContact && IsFloorContact(position, hitSize, *hitBlock, normal))
	{
		return;
	}

	position.z = oldZ;
	result.stopZ = true;
}

// X方向の補正先がブロックに重なったら、補正前の位置を使う。
void PlayerCollision::ClampX(PlayerHorizontalCollisionResult& result, const PlayerMoveParameter& parameter) const
{
	Vector3& position = result.position;
	float minX = parameter.minMoveX;
	float maxX = parameter.maxMoveX;

	if (m_StageBlockManager != nullptr)
	{
		m_StageBlockManager->GetWalkableXRange(position.z, parameter.hitSize.x, minX, maxX);
	}

	float oldX = position.x;
	if (position.x < minX)
	{
		position.x = minX;
	}
	if (position.x > maxX)
	{
		position.x = maxX;
	}

	if (m_StageBlockManager != nullptr && m_StageBlockManager->IsHitAnyStageBlock(position, parameter.hitSize))
	{
		position.x = oldX;
		result.stopX = true;
	}
}

void PlayerCollision::ClampZ(PlayerHorizontalCollisionResult& result, Vector3 hitSize) const
{
	if (m_StageBlockManager == nullptr)
	{
		return;
	}

	float minZ = 0.0f;
	float maxZ = 0.0f;
	if (!m_StageBlockManager->GetStageZClampRange(hitSize.z, minZ, maxZ))
	{
		return;
	}

	if (result.position.z < minZ)
	{
		result.position.z = minZ;
		result.stopZ = true;
	}
	if (result.position.z > maxZ)
	{
		result.position.z = maxZ;
		result.stopZ = true;
	}
}

PlayerVerticalCollisionResult PlayerCollision::MoveVertical(Vector3 position, float velocityY, Vector3 hitSize) const
{
	float oldY = position.y;
	position.y += velocityY;
	PlayerVerticalCollisionResult result;
	result.positionY = position.y;

	if (m_StageBlockManager == nullptr)
	{
		return result;
	}

	StageBlock* hitBlock = m_StageBlockManager->GetHitStageBlock(position, hitSize);
	if (hitBlock == nullptr)
	{
		return result;
	}

	if (velocityY < 0.0f)
	{
		float surfaceY = 0.0f;
		Vector3 slopeNormal = Vector3(0.0f, 1.0f, 0.0f);
		if (!hitBlock->GetSlopeRideSurfaceY(position, hitSize, surfaceY, slopeNormal))
		{
			surfaceY = hitBlock->GetMax().y;
		}
		result.positionY = surfaceY + hitSize.y;
		result.contact = PlayerVerticalContact::Floor;
	}
	else if (velocityY > 0.0f)
	{
		result.positionY = hitBlock->GetMin().y - hitSize.y;
		result.contact = PlayerVerticalContact::Ceiling;
	}
	else
	{
		result.positionY = oldY;
	}

	return result;
}

bool PlayerCollision::IsStandingOnStageBlock(Vector3 position, Vector3 hitSize) const
{
	if (m_StageBlockManager == nullptr)
	{
		return false;
	}

	float slopeSurfaceY = 0.0f;
	float playerBottomY = position.y - hitSize.y;
	if (TryGetSlopeGroundY(position, hitSize, slopeSurfaceY) && fabsf(playerBottomY - slopeSurfaceY) <= 0.20f)
	{
		return true;
	}

	Vector3 checkPosition = position;
	checkPosition.y -= 0.05f;
	StageBlock* hitBlock = m_StageBlockManager->GetHitStageBlock(checkPosition, hitSize);
	if (hitBlock == nullptr)
	{
		return false;
	}

	float distance = fabsf(playerBottomY - hitBlock->GetMax().y);
	if (distance > 0.15f)
	{
		return false;
	}
	return true;
}

bool PlayerCollision::TryGetSlopeGroundY(Vector3 position, Vector3 hitSize, float& outSurfaceY) const
{
	outSurfaceY = 0.0f;

	if (m_StageBlockManager == nullptr)
	{
		return false;
	}

	Vector3 checkPosition = position;
	checkPosition.y -= 0.05f;
	StageBlock* hitBlock = m_StageBlockManager->GetHitStageBlock(checkPosition, hitSize);
	if (hitBlock == nullptr)
	{
		return false;
	}

	float surfaceY = 0.0f;
	Vector3 slopeNormal = Vector3(0.0f, 1.0f, 0.0f);
	if (!hitBlock->GetSlopeRideSurfaceY(checkPosition, hitSize, surfaceY, slopeNormal))
	{
		return false;
	}

	outSurfaceY = surfaceY;
	return true;
}
