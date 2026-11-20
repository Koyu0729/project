#include "tutorialRoom.h"

#include "gameConfigData.h"
#include "goalBuilding.h"
#include "jumpPad.h"
#include "player.h"
#include "scene.h"
#include "stageBlock.h"
#include "stageBlockManager.h"
#include "stageOuterFloorAndWalls.h"

#include <cmath>

namespace
{
	constexpr float StartZ = 3.0f;
	constexpr float RoomStartZ = 30.0f;
	constexpr float ExitTolerance = 0.5f;  // 部屋の終端のどれだけ手前から出口とみなすか
}

void TutorialRoom::Build(Scene& scene, StageBlockManager& stage, int roomIndex)
{
	const StageModuleManager& modules = stage.GetStageModules();
	int roomCount = (int)modules.GetModules().size() + 1;

	if (roomIndex == roomCount - 1)
	{
		m_Data = modules.GetFinalJumpPadRoom();
	}
	else
	{
		m_Data = modules.GetModule(roomIndex);
	}

	GroundLoadData ground;
	bool isFinalRoom = IsFinal();

	if (isFinalRoom)
	{
		scene.AddGameObject<GoalBuilding>(SceneLayer::SceneObjects);
	}

	m_EndZ = stage.SpawnFixedRoom(*m_Data, RoomStartZ, isFinalRoom);
	m_JumpPad = scene.GetGameObject<JumpPad>();

	float floorCenterY = ground.center.y + ground.halfSize.y + ground.tileHalfSizeY + 0.01f;
	float floorHalfLength = (m_EndZ - StartZ) * 0.5f;

	StageBlock* floor = scene.AddGameObject<StageBlock>(SceneLayer::DepthSorted);
	floor->SetPosition(Vector3(ground.center.x, floorCenterY, StartZ + floorHalfLength));
	floor->SetSize(Vector3(ground.halfSize.x, ground.tileHalfSizeY, floorHalfLength));

	floor->SetTexture("asset\\texture\\cyberGroundRain.png");
	floor->SetUVFlipZ(true);
	floor->SetUVRepeat(XMFLOAT2(1.0f, floorHalfLength / ground.halfSize.x));

	floor->SetCanWallKick(false);
	floor->SetCanWallSlide(false);
	floor->SetCanClimb(false);
	stage.AddStageBlock(floor);

	StageOuterFloorAndWalls* outerFloorAndWalls = scene.AddGameObject<StageOuterFloorAndWalls>(SceneLayer::StageOuterFloorAndWalls);
	outerFloorAndWalls->SetLayout(ground, stage);
}

bool TutorialRoom::IsAtExit(const Player& player) const
{
	return player.GetPosition().z >= m_EndZ - ExitTolerance;
}

bool TutorialRoom::HasLandedOnLastBlock(Player& player) const
{
	if (!player.IsGround() || m_Data->objects.empty())
	{
		return false;
	}

	const StageModuleObjectData& target = m_Data->objects.back();
	Vector3 position = player.GetPosition();
	Vector3 hitSize = player.GetHitSize();

	float topY = target.localPosition.y + target.size.y;
	float worldZ = RoomStartZ + target.localPosition.z;

	// 足裏と足場の上面は、高さのずれを0.2まで許す
	return fabsf(position.y - hitSize.y - topY) <= 0.2f &&
		fabsf(position.x - target.localPosition.x) <= target.size.x + hitSize.x &&
		fabsf(position.z - worldZ) <= target.size.z + hitSize.z;
}

bool TutorialRoom::IsJumpPadOnCooldown() const
{
	return m_JumpPad != nullptr && m_JumpPad->IsOnCooldown();
}
