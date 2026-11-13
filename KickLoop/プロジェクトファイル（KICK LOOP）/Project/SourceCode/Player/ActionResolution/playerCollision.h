#pragma once

#include "vector3.h"

class StageBlock;
class StageBlockManager;
struct PlayerMoveParameter;

struct PlayerWallContact
{
	Vector3 normal = Vector3(0.0f, 0.0f, 0.0f);
	StageBlock* block = nullptr;
};

struct PlayerHorizontalCollisionResult
{
	Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
	PlayerWallContact wall;
	bool stopX = false;
	bool stopZ = false;
};

enum class PlayerVerticalContact
{
	None,
	Floor,
	Ceiling,
};

struct PlayerVerticalCollisionResult
{
	float positionY = 0.0f;
	PlayerVerticalContact contact = PlayerVerticalContact::None;
};

// ステージとの接触を調べ、補正後の位置と接触結果を返す。
class PlayerCollision
{
private:
	StageBlockManager* m_StageBlockManager = nullptr;

public:
	void SetStageBlockManager(StageBlockManager* stageBlockManager) { m_StageBlockManager = stageBlockManager; }

	PlayerHorizontalCollisionResult MoveHorizontal(
		Vector3 position,
		Vector3 move,
		const PlayerMoveParameter& parameter,
		bool ignoreFloorContact
	) const;

	PlayerVerticalCollisionResult MoveVertical(Vector3 position, float velocityY, Vector3 hitSize) const;
	bool IsStandingOnStageBlock(Vector3 position, Vector3 hitSize) const;
	bool TryGetSlopeGroundY(Vector3 position, Vector3 hitSize, float& outSurfaceY) const;

private:
	void MoveX(PlayerHorizontalCollisionResult& result, float moveX, Vector3 hitSize, bool ignoreFloorContact) const;
	void MoveZ(PlayerHorizontalCollisionResult& result, float moveZ, Vector3 hitSize, bool ignoreFloorContact) const;
	void ClampX(PlayerHorizontalCollisionResult& result, const PlayerMoveParameter& parameter) const;
	void ClampZ(PlayerHorizontalCollisionResult& result, Vector3 hitSize) const;
};
