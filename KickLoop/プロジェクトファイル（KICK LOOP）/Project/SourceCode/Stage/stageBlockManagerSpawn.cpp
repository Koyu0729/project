#include "stageBlockManager.h"

#include "goalBuilding.h"
#include "groundBlockManager.h"
#include "jumpPad.h"
#include "laserWall.h"
#include "manager.h"
#include "player.h"
#include "polygon3D.h"
#include "scene.h"
#include "stageBlock.h"
#include "stageMaskBlock.h"
#include "timeCoin.h"

#include <cstdlib>

// 進行距離に合わせて次の区間を配置する
void StageBlockManager::UpdateStageModuleSpawn()
{
	float progressDistance = GetProgressDistance() + 100.0f;

	while (progressDistance >= m_NextStageModuleSpawnDistance)
	{
		const StageModuleData* module = nullptr;

		bool isFinalModule = false;

		int moduleID = -1;

		if (m_NextStageModuleSpawnDistance >= m_StageGenerateEndDistance)
		{
			if (m_IsFinalStageModuleSpawned)
			{
				return;
			}

			module = m_StageModuleManager.GetFinalJumpPadRoom();

			m_IsFinalStageModuleSpawned = true;

			isFinalModule = true;
		}
		else
		{
			const std::vector<StageModuleData>& modules = m_StageModuleManager.GetModules();

			if (!modules.empty())
			{
				moduleID = rand() % (int)modules.size();

				module = m_StageModuleManager.GetModule(moduleID);
			}

			if (module != nullptr)
			{
				if (m_NextStageModuleSpawnDistance + module->length > m_StageGenerateEndDistance)
				{
					m_NextStageModuleSpawnDistance = m_StageGenerateEndDistance;

					continue;
				}
			}
		}

		if (module != nullptr)
		{
			SpawnStageModule(
				*module,
				isFinalModule,
				moduleID
			);

			m_NextStageModuleSpawnDistance += module->length;
		}
		else
		{
			m_NextStageModuleSpawnDistance += m_StageModuleSpawnInterval;
		}
	}
}

void StageBlockManager::AddRoomRecord(int id, float distance)
{
	if (id < 0)
	{
		return;
	}

	StageRoomRecord record;

	record.id = id;

	record.distance = distance;

	m_RoomRecords.push_back(
		record
	);
}

void StageBlockManager::SpawnStageModule(
	const StageModuleData& module,
	bool isFinalModule,
	int moduleID
)
{
	float roomStartZ = m_NextStageModuleSpawnDistance + m_SpawnZ;

	float roomEndZ = roomStartZ + module.length;

	for (const StageModuleObjectData& objectData : module.objects)
	{
		float objectEndZ = roomStartZ + objectData.localPosition.z + objectData.size.z;

		if (objectEndZ > roomEndZ)
		{
			roomEndZ = objectEndZ;
		}
	}

	AddRoomProgressArea(
		roomStartZ,
		roomEndZ
	);

	if (!isFinalModule)
	{
		AddRoomRecord(
			moduleID,
			m_NextStageModuleSpawnDistance
		);
	}

	if (isFinalModule)
	{
		m_FinalClampZ = roomEndZ;

		m_HasFinalClampZ = true;

		Scene* scene = Manager::GetScene();

		if (scene != nullptr)
		{
			GoalBuilding* goalBuilding = scene->GetGameObject<GoalBuilding>();

			if (goalBuilding != nullptr)
			{
				goalBuilding->LockCurrentPosition();
			}
		}
	}

	for (const StageModuleObjectData& objectData : module.objects)
	{
		SpawnStageModuleObject(
			objectData
		);
	}
}

void StageBlockManager::SpawnStageModuleBlockAtDistance(
	const StageModuleObjectData& objectData,
	float spawnDistance
)
{
	if (objectData.objectType != StageModuleObjectType::Block)
	{
		return;
	}

	Vector3 spawnPosition = objectData.localPosition;

	spawnPosition.z = spawnDistance + m_SpawnZ + objectData.localPosition.z;

	StageMaskBlock* obstacle = GetObstacleFromPool();

	if (obstacle == nullptr)
	{
		return;
	}

	obstacle->SetPosition(
		spawnPosition
	);

	obstacle->SetSize(
		objectData.size
	);

	obstacle->SetBlockRotation(
		objectData.rotation
	);

	obstacle->SetUseOBBHit(
		objectData.useOBBHit
	);

	obstacle->SetWallKickGroupId(
		objectData.wallKickGroupId
	);

	obstacle->SetCanAddWallKickGauge(
		objectData.canAddWallKickGauge
	);

	obstacle->SetCanWallKick(
		objectData.canWallKick
	);

	obstacle->SetCanWallSlide(
		objectData.canWallSlide
	);

	obstacle->SetCanClimb(
		objectData.canClimb
	);

	obstacle->SetUVScroll(false);

	obstacle->SetUVRepeat(
		objectData.uvRepeat
	);

	obstacle->SetTexture(
		objectData.texturePath.c_str()
	);

	obstacle->SetMaskTexture(
		objectData.maskTexturePath.c_str()
	);

	obstacle->SetColor(
		objectData.color.x,
		objectData.color.y,
		objectData.color.z,
		objectData.alpha
	);

	obstacle->SetEmissivePower(
		objectData.emissivePower
	);

	obstacle->SetMaskColorRange(
		objectData.maskColorStart,
		objectData.maskColorEnd
	);

	obstacle->SetMaskCoreRange(
		objectData.maskCoreStart,
		objectData.maskCoreEnd
	);

	obstacle->SetRainbow(
		objectData.isRainbow
	);

	obstacle->SetRainbowSpeed(
		objectData.rainbowSpeed
	);

	obstacle->SetAlpha(
		objectData.alpha
	);

	obstacle->SetActive(true);
	obstacle->SetVisible(true);

	AddStageBlock(
		obstacle
	);

	m_Obstacles.push_back(
		obstacle
	);
}

void StageBlockManager::SpawnStageModuleObject(
	const StageModuleObjectData& objectData
)
{
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	if (objectData.objectType == StageModuleObjectType::LaserWall)
	{
		SpawnStageModuleLaserWall(
			objectData
		);

		return;
	}

	if (objectData.objectType == StageModuleObjectType::JumpPad)
	{
		SpawnStageModuleJumpPad(
			objectData
		);

		return;
	}

	if (objectData.objectType == StageModuleObjectType::TimeCoin)
	{
		SpawnStageModuleTimeCoin(
			objectData
		);

		return;
	}

	if (objectData.objectType == StageModuleObjectType::CommentUI)
	{
		SpawnStageModuleCommentUI(
			objectData
		);

		return;
	}

	Vector3 spawnPosition = objectData.localPosition;

	spawnPosition.z = m_NextStageModuleSpawnDistance + m_SpawnZ + objectData.localPosition.z;

	StageMaskBlock* obstacle = GetObstacleFromPool();

	if (obstacle == nullptr)
	{
		return;
	}

	obstacle->SetPosition(
		spawnPosition
	);

	obstacle->SetSize(
		objectData.size
	);

	obstacle->SetBlockRotation(
		objectData.rotation
	);

	obstacle->SetUseOBBHit(
		objectData.useOBBHit
	);

	obstacle->SetWallKickGroupId(
		objectData.wallKickGroupId
	);

	obstacle->SetCanAddWallKickGauge(
		objectData.canAddWallKickGauge
	);

	obstacle->SetCanWallKick(
		objectData.canWallKick
	);

	obstacle->SetCanWallSlide(
		objectData.canWallSlide
	);

	obstacle->SetCanClimb(
		objectData.canClimb
	);

	obstacle->SetUVScroll(false);

	obstacle->SetUVRepeat(
		objectData.uvRepeat
	);

	obstacle->SetTexture(
		objectData.texturePath.c_str()
	);

	obstacle->SetMaskTexture(
		objectData.maskTexturePath.c_str()
	);

	obstacle->SetColor(
		objectData.color.x,
		objectData.color.y,
		objectData.color.z,
		objectData.alpha
	);

	obstacle->SetEmissivePower(
		objectData.emissivePower
	);

	obstacle->SetMaskColorRange(
		objectData.maskColorStart,
		objectData.maskColorEnd
	);

	obstacle->SetMaskCoreRange(
		objectData.maskCoreStart,
		objectData.maskCoreEnd
	);

	obstacle->SetRainbow(
		objectData.isRainbow
	);

	obstacle->SetRainbowSpeed(
		objectData.rainbowSpeed
	);

	obstacle->SetAlpha(
		objectData.alpha
	);

	obstacle->SetActive(true);
	obstacle->SetVisible(true);

	AddStageBlock(
		obstacle
	);

	m_Obstacles.push_back(
		obstacle
	);
}

void StageBlockManager::SpawnStageModuleLaserWall(
	const StageModuleObjectData& objectData
)
{
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Vector3 spawnPosition = objectData.localPosition;

	spawnPosition.z = m_NextStageModuleSpawnDistance + m_SpawnZ + objectData.localPosition.z;

	LaserWall* laserWall = GetLaserWallFromPool();

	if (laserWall == nullptr)
	{
		return;
	}

	laserWall->SetTexturePath(
		objectData.texturePath
	);

	laserWall->SetColor(
		objectData.color.x,
		objectData.color.y,
		objectData.color.z,
		objectData.alpha
	);

	if (objectData.size.z > 0.0f)
	{
		laserWall->SetLaserDepth(
			objectData.size.z
		);
	}

	if (objectData.laserSetting.planeType == StageModuleLaserPlaneType::HorizontalXZ)
	{
		laserWall->SetPlaneType(
			LaserWallPlaneType::HorizontalXZ
		);
	}
	else if (objectData.laserSetting.planeType == StageModuleLaserPlaneType::SideYZ)
	{
		laserWall->SetPlaneType(
			LaserWallPlaneType::SideYZ
		);
	}
	else
	{
		laserWall->SetPlaneType(
			LaserWallPlaneType::VerticalXY
		);
	}

	laserWall->SetGridLineCount(
		objectData.laserSetting.verticalLineCount,
		objectData.laserSetting.horizontalLineCount
	);

	laserWall->SetStartAnimation(
		objectData.laserSetting.useStartAnimation,
		objectData.laserSetting.appearStartDistance
	);

	laserWall->SetAppearAnimation(
		objectData.laserSetting.useStartAnimation,
		0.35f,
		0.08f
	);

	laserWall->SetPatternAnimation(
		objectData.laserSetting.enablePatternAnimation
	);

	laserWall->SetTimePenaltyEffect(
		objectData.laserSetting.useTimePenaltyEffect,
		objectData.laserSetting.timePenalty,
		objectData.laserSetting.timePenaltyCooldownFrame
	);

	laserWall->Create(
		scene,
		spawnPosition,
		objectData.size,
		!objectData.laserSetting.useStartAnimation
	);

	m_LaserWalls.push_back(
		laserWall
	);
}

void StageBlockManager::SpawnStageModuleJumpPad(
	const StageModuleObjectData& objectData
)
{
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Vector3 spawnPosition = objectData.localPosition;

	spawnPosition.z = m_NextStageModuleSpawnDistance + m_SpawnZ + objectData.localPosition.z;

	JumpPad* jumpPad = GetJumpPadFromPool();

	if (jumpPad == nullptr)
	{
		return;
	}

	jumpPad->SetJumpVelocity(
		objectData.jumpPadSetting.jumpVelocity
	);

	jumpPad->SetFloatPowerRate(
		objectData.jumpPadSetting.floatPowerRate
	);

	jumpPad->SetCooldownFrame(
		objectData.jumpPadSetting.cooldownFrame
	);

	jumpPad->SetRotationY(
		objectData.jumpPadSetting.rotationY
	);

	Vector3 goalTargetPosition = objectData.jumpPadSetting.goalTargetPosition;

	if (objectData.jumpPadSetting.useGoalLaunch)
	{
		goalTargetPosition.z =
			m_NextStageModuleSpawnDistance +
			m_SpawnZ +
			objectData.jumpPadSetting.goalTargetPosition.z;
	}

	jumpPad->SetGoalLaunch(
		objectData.jumpPadSetting.useGoalLaunch,
		goalTargetPosition,
		objectData.jumpPadSetting.goalLaunchFrameMax,
		objectData.jumpPadSetting.goalLaunchHeight
	);

	jumpPad->Create(
		scene,
		spawnPosition,
		objectData.size
	);

	jumpPad->SetHitOBB(
		spawnPosition,
		objectData.size,
		objectData.jumpPadSetting.rotationY
	);

	jumpPad->SetActive(true);
	jumpPad->SetVisible(true);

	m_JumpPads.push_back(
		jumpPad
	);
}

void StageBlockManager::SpawnStageModuleTimeCoin(
	const StageModuleObjectData& objectData
)
{
	Vector3 spawnPosition = objectData.localPosition;

	spawnPosition.z = m_NextStageModuleSpawnDistance + m_SpawnZ + objectData.localPosition.z;

	TimeCoin* timeCoin = GetTimeCoinFromPool();

	if (timeCoin == nullptr)
	{
		return;
	}

	timeCoin->SetAddTime(
		objectData.timeCoinSetting.addTime
	);

	Vector3 coinSize = objectData.size;

	if (coinSize.x <= 0.0f ||
		coinSize.y <= 0.0f ||
		coinSize.z <= 0.0f)
	{
		coinSize = objectData.timeCoinSetting.hitSize;
	}

	timeCoin->Create(
		spawnPosition,
		coinSize
	);

	timeCoin->SetActive(true);
	timeCoin->SetVisible(true);

	m_TimeCoins.push_back(
		timeCoin
	);

#ifdef _DEBUG
	printf(
		"After TimeCoin Create ptr=%p active=%d visible=%d pos %.2f %.2f %.2f\n",
		timeCoin,
		timeCoin->Active(),
		timeCoin->Visible(),
		spawnPosition.x,
		spawnPosition.y,
		spawnPosition.z
	);
#endif
}

void StageBlockManager::SpawnStageModuleCommentUI(
	const StageModuleObjectData& objectData
)
{
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Vector3 spawnPosition = objectData.localPosition;

	spawnPosition.z = m_NextStageModuleSpawnDistance + m_SpawnZ + objectData.localPosition.z;

	Polygon3D* commentUI = scene->AddGameObject<Polygon3D>(SceneLayer::SceneObjects);

	if (commentUI == nullptr)
	{
		return;
	}

	commentUI->SetTexture(
		objectData.texturePath.c_str()
	);

	commentUI->SetRect3D(
		spawnPosition.x,
		spawnPosition.y,
		spawnPosition.z,
		objectData.size.x,
		objectData.size.y
	);

	commentUI->SetColor(
		1.0f,
		1.0f,
		1.0f,
		objectData.alpha
	);

	commentUI->SetLookAtPlayer(true);
	commentUI->SetVisible(false);

	m_CommentUIs.push_back(
		commentUI
	);
}

// ゴール演出中にカメラ付近の区間のブロックを再生成する
void StageBlockManager::RebuildPassedStageBlocks(float cameraZ)
{
	float rebuildBackDistance = 100.0f;
	float rebuildFrontDistance = 800.0f;

	float rebuildMinZ = cameraZ - rebuildBackDistance;
	float rebuildMaxZ = cameraZ + rebuildFrontDistance;

	for (StageRoomRecord& record : m_RoomRecords)
	{
		if (record.isGoalCameraRebuilt)
		{
			continue;
		}

		const StageModuleData* module = m_StageModuleManager.GetModule(record.id);

		if (module == nullptr)
		{
			continue;
		}

		float roomStartZ = record.distance + m_SpawnZ;

		float roomEndZ = roomStartZ + module->length;

		if (roomEndZ < rebuildMinZ)
		{
			continue;
		}

		if (roomStartZ > rebuildMaxZ)
		{
			continue;
		}

		for (const StageModuleObjectData& objectData : module->objects)
		{
			if (objectData.objectType != StageModuleObjectType::Block)
			{
				continue;
			}

			SpawnStageModuleBlockAtDistance(
				objectData,
				record.distance
			);
		}

		record.isGoalCameraRebuilt = true;
	}
}
