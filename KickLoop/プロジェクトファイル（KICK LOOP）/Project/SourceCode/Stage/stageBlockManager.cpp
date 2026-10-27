#include "main.h"

#include "stageBlockManager.h"
#include "stageBlock.h"
#include "stageMaskBlock.h"

#include "groundBlockManager.h"

#include "polygon3D.h"

#include "laserWall.h"

#include "jumpPad.h"

#include "timeCoin.h"
#include "player.h"
#include "goalBuilding.h"	

#include "manager.h"
#include "scene.h"

#include "gameConfigData.h"

#include <cstdlib>

#ifdef _DEBUG
#include <cstdio>
#endif

void StageBlockManager::Init()
{
	m_StageBlocks.clear();
	m_Obstacles.clear();

	// Pool管理リスト。
	m_ObstaclePool.clear();

	m_LaserWalls.clear();
	m_LaserWallPool.clear();

	m_JumpPads.clear();
	m_JumpPadPool.clear();

	m_TimeCoins.clear();
	m_TimeCoinPool.clear();


	m_CommentUIs.clear();

	// リザルトや再構築で使う記録。
	m_CollectedTimeCoinRecords.clear();
	m_RoomRecords.clear();

	m_DistanceDeleteStageBlocks.clear();
	m_RoomProgressAreas.clear();

	m_FloorObstaclePresets.clear();
	m_WallObstaclePresets.clear();


	m_NextFloorSpawnDistance = 40.0f;
	m_NextWallSpawnDistance = 20.0f;

	m_NextStageModuleSpawnDistance = 40.0f;
	m_StageModuleSpawnInterval = 40.0f;

	m_StageGenerateEndDistance = 5000.0f;
	m_IsFinalStageModuleSpawned = false;

	m_DefaultWalkMinX = 15.0f;
	m_DefaultWalkMaxX = 35.0f;

	m_StartClampBackOffset = 10.0f;

	m_RoomClampPassedOffset = 0.0f;

	m_MinClampZ =
		m_PlayerBaseZ -
		m_StartClampBackOffset;

	m_FinalClampZ = 0.0f;
	m_HasFinalClampZ = false;

	m_IsGoalZClampReleased = false;

	m_StageModuleManager.InitDefaultModules();

	m_IsPoolPreCreated = false;
}

void StageBlockManager::Uninit()
{
	ClearLaserWalls();
	ClearJumpPads();
	ClearTimeCoins();
	ClearCommentUIs();

	for (StageMaskBlock* obstacle : m_ObstaclePool)
	{
		if (obstacle == nullptr)
		{
			continue;
		}

		obstacle->SetActive(false);
		obstacle->SetVisible(false);
		obstacle->SetDestroy();
	}

	for (LaserWall* laserWall : m_LaserWallPool)
	{
		if (laserWall == nullptr)
		{
			continue;
		}

		laserWall->SleepForPool();
		laserWall->SetDestroy();
	}

	for (JumpPad* jumpPad : m_JumpPadPool)
	{
		if (jumpPad == nullptr)
		{
			continue;
		}

		jumpPad->SleepForPool();
		jumpPad->SetDestroy();
	}

	for (TimeCoin* timeCoin : m_TimeCoinPool)
	{
		if (timeCoin == nullptr)
		{
			continue;
		}

		timeCoin->SetActive(false);
		timeCoin->SetVisible(false);
		timeCoin->SetDestroy();
	}

	m_ObstaclePool.clear();
	m_LaserWallPool.clear();
	m_JumpPadPool.clear();
	m_TimeCoinPool.clear();

	m_StageBlocks.clear();
	m_Obstacles.clear();
	m_LaserWalls.clear();
	m_JumpPads.clear();
	m_TimeCoins.clear();
	m_CollectedTimeCoinRecords.clear();
	m_CommentUIs.clear();

	m_DistanceDeleteStageBlocks.clear();
	m_RoomProgressAreas.clear();

	m_FloorObstaclePresets.clear();
	m_WallObstaclePresets.clear();

	m_IsFinalStageModuleSpawned = false;
	m_IsGoalZClampReleased = false;
}

void StageBlockManager::Update()
{
	if (!m_IsPoolPreCreated)
	{
		PreCreateObjectPools();

		m_IsPoolPreCreated =
			true;

		return;
	}

	UpdateStageModuleSpawn();

	UpdateDistanceDeleteStageBlocks();

	Scene* scene =
		Manager::GetScene();

	Player* player = nullptr;

	if (scene != nullptr)
	{
		player =
			scene->GetGameObject<Player>();
	}

	if (player != nullptr)
	{
		DeletePassedRoomProgressAreas(
			player->GetPosition().z
		);
	}

	UpdateCommentVisible();

}

void StageBlockManager::Draw()
{
}

void StageBlockManager::AddStageBlock(StageBlock* stageBlock)
{
	if (stageBlock == nullptr)
	{
		return;
	}

	m_StageBlocks.push_back(
		stageBlock
	);
}

void StageBlockManager::RemoveStageBlock(StageBlock* stageBlock)
{
	if (stageBlock == nullptr)
	{
		return;
	}

	for (int i = (int)m_StageBlocks.size() - 1; i >= 0; i--)
	{
		if (m_StageBlocks[i] != stageBlock)
		{
			continue;
		}

		m_StageBlocks.erase(
			m_StageBlocks.begin() + i
		);
	}

	for (DistanceDeleteStageBlock& data : m_DistanceDeleteStageBlocks)
	{
		if (data.stageBlock != stageBlock)
		{
			continue;
		}

		data.stageBlock = nullptr;
		data.isDeleted = true;
	}
}

bool StageBlockManager::IsHitAnyStageBlock(Vector3 position, Vector3 size)
{
	for (StageBlock* stageBlock : m_StageBlocks)
	{
		if (stageBlock == nullptr)
		{
			continue;
		}

		if (stageBlock->Destroy())
		{
			continue;
		}

		if (!stageBlock->Active())
		{
			continue;
		}

		if (stageBlock->IsHit(position, size))
		{
			return true;
		}
	}

	return false;
}

StageBlock* StageBlockManager::GetHitStageBlock(Vector3 position, Vector3 size)
{
	for (StageBlock* stageBlock : m_StageBlocks)
	{
		if (stageBlock == nullptr)
		{
			continue;
		}

		if (stageBlock->Destroy())
		{
			continue;
		}

		if (!stageBlock->Active())
		{
			continue;
		}

		if (stageBlock->IsHit(position, size))
		{
			return stageBlock;
		}
	}

	return nullptr;
}

bool StageBlockManager::IsHitAnyObstacleExcept(
	StageBlock* ignoreBlock,
	Vector3 position,
	Vector3 size
)
{
	for (StageBlock* obstacle : m_Obstacles)
	{
		if (obstacle == nullptr)
		{
			continue;
		}

		if (obstacle->Destroy())
		{
			continue;
		}

		if (!obstacle->Active())
		{
			continue;
		}

		if (obstacle == ignoreBlock)
		{
			continue;
		}

		if (obstacle->IsHit(position, size))
		{
			return true;
		}
	}

	return false;
}

bool StageBlockManager::IsHitAnyLaser(Vector3 position, Vector3 size)
{
	for (LaserWall* laserWall : m_LaserWalls)
	{
		if (laserWall == nullptr)
		{
			continue;
		}

		if (!laserWall->Active())
		{
			continue;
		}

		if (laserWall->IsHitAABB(position, size))
		{
			return true;
		}
	}

	return false;
}

void StageBlockManager::SpawnObstacle()
{
	SpawnFloorObstacle();
	SpawnWallObstacle();
}

void StageBlockManager::SpawnFloorObstacle()
{
	if (m_FloorObstaclePresets.empty())
	{
		return;
	}

	for (int i = 0; i < m_FloorSpawnCount; i++)
	{
		int typeIndex =
			rand() %
			(int)m_FloorObstaclePresets.size();

		SpawnObstacleFromPreset(
			m_FloorObstaclePresets[typeIndex]
		);
	}
}

void StageBlockManager::SpawnWallObstacle()
{
	if (m_WallObstaclePresets.empty())
	{
		return;
	}

	for (int i = 0; i < m_WallSpawnCount; i++)
	{
		int typeIndex =
			rand() %
			(int)m_WallObstaclePresets.size();

		SpawnObstacleFromPreset(
			m_WallObstaclePresets[typeIndex]
		);
	}
}

void StageBlockManager::SpawnObstacleFromPreset(const ObstaclePreset& preset)
{
	Scene* scene =
		Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Player* player =
		scene->GetGameObject<Player>();

	StageMaskBlock* obstacle =
		GetObstacleFromPool();

	if (obstacle == nullptr)
	{
		return;
	}

	float spawnZ =
		m_SpawnZ;

	if (player != nullptr)
	{
		spawnZ =
			player->GetPosition().z +
			(m_SpawnZ - m_PlayerBaseZ);
	}

	obstacle->SetPosition(
		Vector3(
			preset.centerX,
			preset.centerY,
			spawnZ
		)
	);

	obstacle->SetSize(
		preset.size
	);

	obstacle->SetBlockRotation(
		Vector3(
			0.0f,
			0.0f,
			0.0f
		)
	);

	obstacle->SetUseOBBHit(false);

	obstacle->SetWallKickGroupId(-1);
	obstacle->SetCanAddWallKickGauge(false);

	obstacle->SetCanWallKick(true);
	obstacle->SetCanWallSlide(true);
	obstacle->SetCanClimb(false);

	obstacle->SetUVScroll(false);
	obstacle->SetUVRepeat(preset.uvRepeat);

	obstacle->SetTexture(
		preset.texturePath.c_str()
	);

	obstacle->SetMaskTexture(
		"asset\\texture\\cyberWallMask.png"
	);

	obstacle->SetColor(
		0.0f,
		0.8f,
		1.0f,
		preset.alpha
	);

	obstacle->SetEmissivePower(1.5f);

	obstacle->SetMaskColorRange(
		0.0f,
		1.0f
	);

	obstacle->SetMaskCoreRange(
		0.0f,
		1.0f
	);

	obstacle->SetRainbow(preset.isRainbow);
	obstacle->SetRainbowSpeed(preset.rainbowSpeed);
	obstacle->SetAlpha(preset.alpha);

	obstacle->SetActive(true);
	obstacle->SetVisible(true);

	AddStageBlock(
		obstacle
	);

	m_Obstacles.push_back(
		obstacle
	);
}

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

			module =
				m_StageModuleManager.GetFinalJumpPadRoom();

			m_IsFinalStageModuleSpawned = true;

			isFinalModule = true;
		}
		else
		{
			const std::vector<StageModuleData>& modules =
				m_StageModuleManager.GetModules();

			if (!modules.empty())
			{
				moduleID =
					rand() %
					(int)modules.size();

				module =
					m_StageModuleManager.GetModule(
						moduleID
					);
			}

			if (module != nullptr)
			{
				if (m_NextStageModuleSpawnDistance + module->length > m_StageGenerateEndDistance)
				{
					m_NextStageModuleSpawnDistance =
						m_StageGenerateEndDistance;

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

			m_NextStageModuleSpawnDistance +=
				module->length;
		}
		else
		{
			m_NextStageModuleSpawnDistance +=
				m_StageModuleSpawnInterval;
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

	record.distance =
		distance;

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
	float roomStartZ =
		m_NextStageModuleSpawnDistance +
		m_SpawnZ;

	float roomEndZ =
		roomStartZ +
		module.length;

	for (const StageModuleObjectData& objectData : module.objects)
	{
		float objectEndZ =
			roomStartZ +
			objectData.localPosition.z +
			objectData.size.z;

		if (objectEndZ > roomEndZ)
		{
			roomEndZ =
				objectEndZ;
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
		m_FinalClampZ =
			roomEndZ;

		m_HasFinalClampZ =
			true;

		Scene* scene =
			Manager::GetScene();

		if (scene != nullptr)
		{
			GoalBuilding* goalBuilding =
				scene->GetGameObject<GoalBuilding>();

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

	Vector3 spawnPosition =
		objectData.localPosition;

	spawnPosition.z =
		spawnDistance +
		m_SpawnZ +
		objectData.localPosition.z;

	StageMaskBlock* obstacle =
		GetObstacleFromPool();

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
	Scene* scene =
		Manager::GetScene();

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

	Vector3 spawnPosition =
		objectData.localPosition;

	spawnPosition.z =
		m_NextStageModuleSpawnDistance +
		m_SpawnZ +
		objectData.localPosition.z;

	StageMaskBlock* obstacle =
		GetObstacleFromPool();

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
	Scene* scene =
		Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Vector3 spawnPosition =
		objectData.localPosition;

	spawnPosition.z =
		m_NextStageModuleSpawnDistance +
		m_SpawnZ +
		objectData.localPosition.z;

	LaserWall* laserWall =
		GetLaserWallFromPool();

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
	Scene* scene =
		Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Vector3 spawnPosition =
		objectData.localPosition;

	spawnPosition.z =
		m_NextStageModuleSpawnDistance +
		m_SpawnZ +
		objectData.localPosition.z;

	JumpPad* jumpPad =
		GetJumpPadFromPool();

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

	Vector3 goalTargetPosition =
		objectData.jumpPadSetting.goalTargetPosition;

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
	Vector3 spawnPosition =
		objectData.localPosition;

	spawnPosition.z =
		m_NextStageModuleSpawnDistance +
		m_SpawnZ +
		objectData.localPosition.z;

	TimeCoin* timeCoin =
		GetTimeCoinFromPool();

	if (timeCoin == nullptr)
	{
		return;
	}

	timeCoin->SetAddTime(
		objectData.timeCoinSetting.addTime
	);

	Vector3 coinSize =
		objectData.size;

	if (coinSize.x <= 0.0f ||
		coinSize.y <= 0.0f ||
		coinSize.z <= 0.0f)
	{
		coinSize =
			objectData.timeCoinSetting.hitSize;
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
	Scene* scene =
		Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Vector3 spawnPosition =
		objectData.localPosition;

	spawnPosition.z =
		m_NextStageModuleSpawnDistance +
		m_SpawnZ +
		objectData.localPosition.z;

	Polygon3D* commentUI =
		scene->AddGameObject<Polygon3D>(3);

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

// 進行位置に応じて案内表示を切り替える
void StageBlockManager::UpdateCommentVisible()
{
	Scene* scene =
		Manager::GetScene();

	Player* player = nullptr;

	if (scene != nullptr)
	{
		player =
			scene->GetGameObject<Player>();
	}

	float playerZ =
		m_PlayerBaseZ;

	if (player != nullptr)
	{
		playerZ =
			player->GetPosition().z;
	}

	for (Polygon3D* commentUI : m_CommentUIs)
	{
		if (commentUI == nullptr)
		{
			continue;
		}

		if (commentUI->Destroy())
		{
			continue;
		}

		Vector3 uiPosition =
			commentUI->GetPosition();

		float distanceZ =
			uiPosition.z -
			playerZ;

		if (distanceZ >= 0.0f && distanceZ <= m_CommentVisibleDistance)
		{
			commentUI->SetVisible(true);
		}
		else
		{
			commentUI->SetVisible(false);
		}
	}
}

void StageBlockManager::AddDistanceDeleteStageBlock(
	StageBlock* stageBlock,
	float deleteDistance
)
{
	if (stageBlock == nullptr)
	{
		return;
	}

	DistanceDeleteStageBlock data;

	data.stageBlock =
		stageBlock;

	data.deleteDistance =
		deleteDistance;

	data.isDeleted =
		false;

	m_DistanceDeleteStageBlocks.push_back(
		data
	);
}

void StageBlockManager::UpdateDistanceDeleteStageBlocks()
{
	float progressDistance =
		GetProgressDistance();

	for (DistanceDeleteStageBlock& data : m_DistanceDeleteStageBlocks)
	{
		if (data.stageBlock == nullptr)
		{
			continue;
		}

		if (data.isDeleted)
		{
			continue;
		}

		if (progressDistance < data.deleteDistance)
		{
			continue;
		}

		data.stageBlock->SetActive(false);
		data.stageBlock->SetVisible(false);

		data.isDeleted = true;
	}
}

void StageBlockManager::SetFloorSpawnCountAtSamePoint(int count)
{
	if (count < 1)
	{
		count = 1;
	}

	m_FloorSpawnCount =
		count;
}

void StageBlockManager::SetWallSpawnCountAtSamePoint(int count)
{
	if (count < 1)
	{
		count = 1;
	}

	m_WallSpawnCount =
		count;
}

void StageBlockManager::SetObstaclePresets(
	const std::vector<ObstaclePresetData>& floorObstaclePresets,
	const std::vector<ObstaclePresetData>& wallObstaclePresets
)
{
	m_FloorObstaclePresets.clear();
	m_WallObstaclePresets.clear();

	for (const ObstaclePresetData& presetData : floorObstaclePresets)
	{
		m_FloorObstaclePresets.push_back(
			ConvertObstaclePresetData(
				presetData
			)
		);
	}

	for (const ObstaclePresetData& presetData : wallObstaclePresets)
	{
		m_WallObstaclePresets.push_back(
			ConvertObstaclePresetData(
				presetData
			)
		);
	}
}

ObstaclePreset StageBlockManager::ConvertObstaclePresetData(
	const ObstaclePresetData& presetData
)
{
	ObstaclePreset preset;

	preset.size =
		presetData.size;

	preset.centerX =
		presetData.centerX;

	preset.centerY =
		presetData.centerY;

	preset.uvRepeat =
		presetData.uvRepeat;

	preset.texturePath =
		presetData.texturePath;

	preset.isRainbow =
		presetData.isRainbow;

	preset.rainbowSpeed =
		presetData.rainbowSpeed;

	preset.alpha =
		presetData.alpha;

	return preset;
}

// 通過した障害物を再利用待ちに戻す
void StageBlockManager::DestroyPassedObstacles()
{
	Scene* scene =
		Manager::GetScene();

	Player* player = nullptr;

	if (scene != nullptr)
	{
		player =
			scene->GetGameObject<Player>();
	}

	float destroyZ =
		m_DestroyObstacleZ;

	if (player != nullptr)
	{
		destroyZ =
			player->GetPosition().z +
			m_DestroyObstacleZ;
	}

	for (int i = (int)m_Obstacles.size() - 1; i >= 0; i--)
	{
		StageBlock* obstacle =
			m_Obstacles[i];

		if (obstacle == nullptr)
		{
			m_Obstacles.erase(
				m_Obstacles.begin() + i
			);

			continue;
		}

		if (obstacle->Destroy())
		{
			RemoveStageBlock(
				obstacle
			);

			m_Obstacles.erase(
				m_Obstacles.begin() + i
			);

			continue;
		}

		Vector3 position =
			obstacle->GetPosition();

		if (position.z > destroyZ)
		{
			continue;
		}

		StageMaskBlock* maskBlock =
			(StageMaskBlock*)obstacle;

		ReturnObstacleToPool(
			maskBlock
		);

		m_Obstacles.erase(
			m_Obstacles.begin() + i
		);
	}
}

void StageBlockManager::DestroyPassedLaserWalls()
{
	Scene* scene =
		Manager::GetScene();

	Player* player = nullptr;

	if (scene != nullptr)
	{
		player =
			scene->GetGameObject<Player>();
	}

	float destroyZ =
		m_DestroyObstacleZ;

	if (player != nullptr)
	{
		destroyZ =
			player->GetPosition().z +
			m_DestroyObstacleZ;
	}

	for (int i = (int)m_LaserWalls.size() - 1; i >= 0; i--)
	{
		LaserWall* laserWall =
			m_LaserWalls[i];

		if (laserWall == nullptr)
		{
			m_LaserWalls.erase(
				m_LaserWalls.begin() + i
			);

			continue;
		}

		Vector3 position =
			laserWall->GetHitCenter();

		if (position.z > destroyZ)
		{
			continue;
		}

		ReturnLaserWallToPool(
			laserWall
		);

		m_LaserWalls.erase(
			m_LaserWalls.begin() + i
		);
	}
}

void StageBlockManager::DestroyPassedJumpPads()
{
	Scene* scene =
		Manager::GetScene();

	Player* player = nullptr;

	if (scene != nullptr)
	{
		player =
			scene->GetGameObject<Player>();
	}

	float destroyZ =
		m_DestroyObstacleZ;

	if (player != nullptr)
	{
		destroyZ =
			player->GetPosition().z +
			m_DestroyObstacleZ;
	}

	for (int i = (int)m_JumpPads.size() - 1; i >= 0; i--)
	{
		JumpPad* jumpPad =
			m_JumpPads[i];

		if (jumpPad == nullptr)
		{
			m_JumpPads.erase(
				m_JumpPads.begin() + i
			);

			continue;
		}

		if (jumpPad->Destroy())
		{
			m_JumpPads.erase(
				m_JumpPads.begin() + i
			);

			continue;
		}

		Vector3 position =
			jumpPad->GetHitCenter();

		if (position.z > destroyZ)
		{
			continue;
		}

		ReturnJumpPadToPool(
			jumpPad
		);

		m_JumpPads.erase(
			m_JumpPads.begin() + i
		);
	}
}

void StageBlockManager::DestroyPassedTimeCoins()
{
	Scene* scene =
		Manager::GetScene();

	Player* player = nullptr;

	if (scene != nullptr)
	{
		player =
			scene->GetGameObject<Player>();
	}

	float destroyZ =
		m_DestroyObstacleZ;

	if (player != nullptr)
	{
		destroyZ =
			player->GetPosition().z +
			m_DestroyObstacleZ;
	}

	for (int i = (int)m_TimeCoins.size() - 1; i >= 0; i--)
	{
		TimeCoin* timeCoin =
			m_TimeCoins[i];

		if (timeCoin == nullptr)
		{
			m_TimeCoins.erase(
				m_TimeCoins.begin() + i
			);

			continue;
		}

		if (!timeCoin->Active())
		{
			ReturnTimeCoinToPool(
				timeCoin
			);

			m_TimeCoins.erase(
				m_TimeCoins.begin() + i
			);

			continue;
		}

		Vector3 position =
			timeCoin->GetHitCenter();

		if (position.z > destroyZ)
		{
			continue;
		}

		ReturnTimeCoinToPool(
			timeCoin
		);

		m_TimeCoins.erase(
			m_TimeCoins.begin() + i
		);
	}
}

void StageBlockManager::DestroyPassedCommentUIs()
{
	Scene* scene =
		Manager::GetScene();

	Player* player = nullptr;

	if (scene != nullptr)
	{
		player =
			scene->GetGameObject<Player>();
	}

	float destroyZ =
		m_DestroyObstacleZ;

	if (player != nullptr)
	{
		destroyZ =
			player->GetPosition().z +
			m_DestroyObstacleZ;
	}

	for (int i = (int)m_CommentUIs.size() - 1; i >= 0; i--)
	{
		Polygon3D* commentUI =
			m_CommentUIs[i];

		if (commentUI == nullptr)
		{
			m_CommentUIs.erase(
				m_CommentUIs.begin() + i
			);

			continue;
		}

		if (commentUI->Destroy())
		{
			m_CommentUIs.erase(
				m_CommentUIs.begin() + i
			);

			continue;
		}

		Vector3 position =
			commentUI->GetPosition();

		if (position.z > destroyZ)
		{
			continue;
		}

		commentUI->SetVisible(false);
		commentUI->SetDestroy();

		m_CommentUIs.erase(
			m_CommentUIs.begin() + i
		);
	}
}

void StageBlockManager::ClearLaserWalls()
{
	for (LaserWall* laserWall : m_LaserWalls)
	{
		if (laserWall == nullptr)
		{
			continue;
		}

		ReturnLaserWallToPool(
			laserWall
		);
	}

	m_LaserWalls.clear();
}

void StageBlockManager::ClearJumpPads()
{
	for (JumpPad* jumpPad : m_JumpPads)
	{
		if (jumpPad == nullptr)
		{
			continue;
		}

		ReturnJumpPadToPool(
			jumpPad
		);
	}

	m_JumpPads.clear();
}

void StageBlockManager::ClearTimeCoins()
{
	for (TimeCoin* timeCoin : m_TimeCoins)
	{
		if (timeCoin == nullptr)
		{
			continue;
		}

		ReturnTimeCoinToPool(
			timeCoin
		);
	}

	m_TimeCoins.clear();
}

void StageBlockManager::ClearCommentUIs()
{
	for (Polygon3D* commentUI : m_CommentUIs)
	{
		if (commentUI == nullptr)
		{
			continue;
		}

		commentUI->SetVisible(false);
		commentUI->SetDestroy();
	}

	m_CommentUIs.clear();
}

float StageBlockManager::GetProgressDistance()
{
	Scene* scene =
		Manager::GetScene();

	if (scene == nullptr)
	{
		return 0.0f;
	}

	Player* player =
		scene->GetGameObject<Player>();

	if (player == nullptr)
	{
		return 0.0f;
	}

	return
		player->GetProgressDistance();
}

void StageBlockManager::AddRoomProgressArea(
	float roomStartZ,
	float roomEndZ
)
{
	if (roomEndZ < roomStartZ)
	{
		float temp =
			roomStartZ;

		roomStartZ =
			roomEndZ;

		roomEndZ =
			temp;
	}

	if (roomEndZ <= roomStartZ)
	{
		return;
	}

	StageRoomProgressArea area;

	area.startZ =
		roomStartZ;

	area.endZ =
		roomEndZ;

	m_RoomProgressAreas.push_back(
		area
	);
}

// 移動可能範囲より後ろの物体を片付ける
void StageBlockManager::DestroyObjectsBehindMinClampZ()
{
	float deleteZ =
		m_MinClampZ;

	for (int i = (int)m_Obstacles.size() - 1; i >= 0; i--)
	{
		StageBlock* obstacle =
			m_Obstacles[i];

		if (obstacle == nullptr)
		{
			m_Obstacles.erase(
				m_Obstacles.begin() + i
			);

			continue;
		}

		if (obstacle->Destroy())
		{
			RemoveStageBlock(
				obstacle
			);

			m_Obstacles.erase(
				m_Obstacles.begin() + i
			);

			continue;
		}

		Vector3 position =
			obstacle->GetPosition();

		if (position.z >= deleteZ)
		{
			continue;
		}

		StageMaskBlock* maskBlock =
			(StageMaskBlock*)obstacle;

		ReturnObstacleToPool(
			maskBlock
		);

		m_Obstacles.erase(
			m_Obstacles.begin() + i
		);
	}

	for (int i = (int)m_LaserWalls.size() - 1; i >= 0; i--)
	{
		LaserWall* laserWall =
			m_LaserWalls[i];

		if (laserWall == nullptr)
		{
			m_LaserWalls.erase(
				m_LaserWalls.begin() + i
			);

			continue;
		}

		Vector3 position =
			laserWall->GetHitCenter();

		if (position.z >= deleteZ)
		{
			continue;
		}

		ReturnLaserWallToPool(
			laserWall
		);

		m_LaserWalls.erase(
			m_LaserWalls.begin() + i
		);
	}

	for (int i = (int)m_JumpPads.size() - 1; i >= 0; i--)
	{
		JumpPad* jumpPad =
			m_JumpPads[i];

		if (jumpPad == nullptr)
		{
			m_JumpPads.erase(
				m_JumpPads.begin() + i
			);

			continue;
		}

		Vector3 position =
			jumpPad->GetHitCenter();

		if (position.z >= deleteZ)
		{
			continue;
		}

		ReturnJumpPadToPool(
			jumpPad
		);

		m_JumpPads.erase(
			m_JumpPads.begin() + i
		);
	}

	for (int i = (int)m_TimeCoins.size() - 1; i >= 0; i--)
	{
		TimeCoin* timeCoin =
			m_TimeCoins[i];

		if (timeCoin == nullptr)
		{
			m_TimeCoins.erase(
				m_TimeCoins.begin() + i
			);

			continue;
		}

		Vector3 position =
			timeCoin->GetHitCenter();

		if (position.z >= deleteZ)
		{
			continue;
		}

		ReturnTimeCoinToPool(
			timeCoin
		);

		m_TimeCoins.erase(
			m_TimeCoins.begin() + i
		);
	}

	for (int i = (int)m_CommentUIs.size() - 1; i >= 0; i--)
	{
		Polygon3D* commentUI =
			m_CommentUIs[i];

		if (commentUI == nullptr)
		{
			m_CommentUIs.erase(
				m_CommentUIs.begin() + i
			);

			continue;
		}

		Vector3 position =
			commentUI->GetPosition();

		if (position.z >= deleteZ)
		{
			continue;
		}

		commentUI->SetVisible(false);
		commentUI->SetDestroy();

		m_CommentUIs.erase(
			m_CommentUIs.begin() + i
		);
	}
}

// 障害物とギミックの再利用分を先に作る
void StageBlockManager::PreCreateObjectPools()
{
	Scene* scene =
		Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	for (int i = 0; i < m_InitialObstaclePoolCount; i++)
	{
		StageMaskBlock* obstacle =
			scene->AddGameObject<StageMaskBlock>(3);

		if (obstacle == nullptr)
		{
			continue;
		}

		obstacle->SetActive(false);
		obstacle->SetVisible(false);

#ifdef _DEBUG
		printf(
			"PreCreate MaskBlock index=%d ptr=%p active=%d visible=%d\n",
			i,
			obstacle,
			obstacle->Active(),
			obstacle->Visible()
		);
#endif

		m_ObstaclePool.push_back(
			obstacle
		);
	}

	for (int i = 0; i < m_InitialTimeCoinPoolCount; i++)
	{
		TimeCoin* timeCoin =
			scene->AddGameObject<TimeCoin>(3);

		if (timeCoin == nullptr)
		{
			continue;
		}

		timeCoin->SetActive(false);
		timeCoin->SetVisible(false);

#ifdef _DEBUG
		printf(
			"PreCreate TimeCoin index=%d ptr=%p active=%d visible=%d\n",
			i,
			timeCoin,
			timeCoin->Active(),
			timeCoin->Visible()
		);
#endif

		m_TimeCoinPool.push_back(
			timeCoin
		);
	}
}

// 待機中の障害物を取り出す
StageMaskBlock* StageBlockManager::GetObstacleFromPool()
{
	StageMaskBlock* obstacle = nullptr;

	if (!m_ObstaclePool.empty())
	{
		obstacle = m_ObstaclePool.back();

		m_ObstaclePool.pop_back();
	}
	else
	{
		Scene* scene = Manager::GetScene();

		if (scene == nullptr)
		{
			return nullptr;
		}

#ifdef _DEBUG
		printf(
			"[StageBlockManager] StageMaskBlockを新規生成します。 Active=%zu Pool=%zu\n",
			m_Obstacles.size(),
			m_ObstaclePool.size()
		);
#endif

		obstacle = scene->AddGameObject<StageMaskBlock>(3);
	}

	if (obstacle == nullptr)
	{
		return nullptr;
	}

	obstacle->ResetForReuse();

	return obstacle;
}

void StageBlockManager::ReturnObstacleToPool(StageMaskBlock* obstacle)
{
	if (obstacle == nullptr)
	{
		return;
	}

	RemoveStageBlock(
		obstacle
	);

	obstacle->SetActive(false);
	obstacle->SetVisible(false);

	m_ObstaclePool.push_back(
		obstacle
	);
}

// 待機中のレーザーを取り出す
LaserWall* StageBlockManager::GetLaserWallFromPool()
{
	LaserWall* laserWall =
		nullptr;

	if (!m_LaserWallPool.empty())
	{
		laserWall =
			m_LaserWallPool.back();

		m_LaserWallPool.pop_back();
	}
	else
	{
		Scene* scene =
			Manager::GetScene();

		if (scene == nullptr)
		{
			return nullptr;
		}

		laserWall =
			scene->AddGameObject<LaserWall>(3);
	}

	if (laserWall == nullptr)
	{
		return nullptr;
	}

	laserWall->ResetForReuse();

	return laserWall;
}

void StageBlockManager::ReturnLaserWallToPool(LaserWall* laserWall)
{
	if (laserWall == nullptr)
	{
		return;
	}

	laserWall->SleepForPool();

	m_LaserWallPool.push_back(
		laserWall
	);
}

// 待機中のジャンプ台を取り出す
JumpPad* StageBlockManager::GetJumpPadFromPool()
{
	JumpPad* jumpPad =
		nullptr;

	if (!m_JumpPadPool.empty())
	{
		jumpPad =
			m_JumpPadPool.back();

		m_JumpPadPool.pop_back();
	}
	else
	{
		Scene* scene =
			Manager::GetScene();

		if (scene == nullptr)
		{
			return nullptr;
		}

		jumpPad =
			scene->AddGameObject<JumpPad>(3);
	}

	if (jumpPad == nullptr)
	{
		return nullptr;
	}

	jumpPad->ResetForReuse();

	return jumpPad;
}

void StageBlockManager::ReturnJumpPadToPool(JumpPad* jumpPad)
{
	if (jumpPad == nullptr)
	{
		return;
	}

	jumpPad->SleepForPool();

	m_JumpPadPool.push_back(
		jumpPad
	);
}

// 待機中のコインを取り出す
TimeCoin* StageBlockManager::GetTimeCoinFromPool()
{
	TimeCoin* timeCoin =
		nullptr;

	if (!m_TimeCoinPool.empty())
	{
		timeCoin =
			m_TimeCoinPool.back();

		m_TimeCoinPool.pop_back();
	}
	else
	{
		Scene* scene =
			Manager::GetScene();

		if (scene == nullptr)
		{
			return nullptr;
		}

		timeCoin =
			scene->AddGameObject<TimeCoin>(3);
	}

	if (timeCoin == nullptr)
	{
		return nullptr;
	}

	timeCoin->SetActive(false);
	timeCoin->SetVisible(false);

#ifdef _DEBUG
	printf(
		"Use TimeCoin FromPool ptr=%p active=%d visible=%d\n",
		timeCoin,
		timeCoin->Active(),
		timeCoin->Visible()
	);
#endif

	return timeCoin;
}

void StageBlockManager::ReturnTimeCoinToPool(TimeCoin* timeCoin)
{
	if (timeCoin == nullptr)
	{
		return;
	}

	timeCoin->SetActive(false);
	timeCoin->SetVisible(false);

	m_TimeCoinPool.push_back(
		timeCoin
	);
}

void StageBlockManager::AddCollectedTimeCoinRecord(
	Vector3 position,
	Vector3 size,
	float addTime
)
{
	CollectedTimeCoinRecord record;

	record.position =
		position;

	record.size =
		size;

	record.addTime =
		addTime;

	m_CollectedTimeCoinRecords.push_back(
		record
	);
}

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

		const StageModuleData* module =
				m_StageModuleManager.GetModule(
				record.id
			);

		if (module == nullptr)
		{
			continue;
		}

		float roomStartZ =
			record.distance +
			m_SpawnZ;

		float roomEndZ =
			roomStartZ +
			module->length;

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

void StageBlockManager::UpdateGoalCameraStage(float cameraZ)
{
	m_MinClampZ = cameraZ - 300.0f;

	DestroyObjectsBehindMinClampZ();

	RebuildPassedStageBlocks(cameraZ);
}

void StageBlockManager::DeletePassedRoomProgressAreas(float playerZ)
{
	float passedZ =
		playerZ -
		m_RoomClampPassedOffset;

	bool isDeletedRoom = false;

	float latestDeletedRoomStartZ =
		m_MinClampZ +
		m_StartClampBackOffset;

	for (int i = (int)m_RoomProgressAreas.size() - 1; i >= 0; i--)
	{
		if (m_RoomProgressAreas[i].endZ > passedZ)
		{
			continue;
		}

		if (!isDeletedRoom)
		{
			latestDeletedRoomStartZ =
				m_RoomProgressAreas[i].startZ;
		}
		else
		{
			if (m_RoomProgressAreas[i].startZ > latestDeletedRoomStartZ)
			{
				latestDeletedRoomStartZ =
					m_RoomProgressAreas[i].startZ;
			}
		}

		m_RoomProgressAreas.erase(
			m_RoomProgressAreas.begin() + i
		);

		isDeletedRoom =
			true;
	}

	if (!isDeletedRoom)
	{
		return;
	}

	float nextMinClampZ =
		latestDeletedRoomStartZ -
		m_StartClampBackOffset;

	if (nextMinClampZ > m_MinClampZ)
	{
		m_MinClampZ =
			nextMinClampZ;
	}

	DestroyObjectsBehindMinClampZ();

	Scene* scene =
		Manager::GetScene();

	if (scene != nullptr)
	{
		GroundBlockManager* groundBlockManager =
			scene->GetGameObject<GroundBlockManager>();

		if (groundBlockManager != nullptr)
		{
			groundBlockManager->DestroyBlocksBehindZ(
				m_MinClampZ
			);
		}
	}
}

// 歩行可能な横方向の範囲を求める
bool StageBlockManager::GetWalkableXRange(
	float playerZ,
	float playerHalfSizeX,
	float& outMinX,
	float& outMaxX
)
{
	(void)playerZ;

	outMinX =
		m_DefaultWalkMinX +
		playerHalfSizeX;

	outMaxX =
		m_DefaultWalkMaxX -
		playerHalfSizeX;

	if (outMaxX < outMinX)
	{
		float center =
			(outMinX + outMaxX) * 0.5f;

		outMinX =
			center;

		outMaxX =
			center;
	}

	return true;
}

bool StageBlockManager::GetStageZClampRange(
	float playerHalfSizeZ,
	float& outMinZ,
	float& outMaxZ
)
{
	(void)playerHalfSizeZ;

	if (m_IsGoalZClampReleased)
	{
		outMinZ =
			-999999.0f;

		outMaxZ =
			999999.0f;

		return true;
	}

	outMinZ =
		m_MinClampZ;

	if (m_HasFinalClampZ)
	{
		outMaxZ =
			m_FinalClampZ;
	}
	else
	{
		outMaxZ =
			GetStageGenerateEndWorldZ();
	}

	if (outMaxZ < outMinZ)
	{
		float center =
			(outMinZ + outMaxZ) * 0.5f;

		outMinZ =
			center;

		outMaxZ =
			center;
	}

	return true;
}

void StageBlockManager::SetStageGenerateEndDistance(float distance)
{
	if (distance < 0.0f)
	{
		distance = 0.0f;
	}

	m_StageGenerateEndDistance =
		distance;
}

float StageBlockManager::GetStageGenerateEndDistance() const
{
	return m_StageGenerateEndDistance;
}

float StageBlockManager::GetStageGenerateEndWorldZ() const
{
	float finalRoomLength =
		0.0f;

	const StageModuleData* finalRoom =
		m_StageModuleManager.GetFinalJumpPadRoom();

	if (finalRoom != nullptr)
	{
		finalRoomLength =
			finalRoom->length;
	}

	return
		m_StageGenerateEndDistance +
		m_SpawnZ +
		finalRoomLength;
}

bool StageBlockManager::IsFinalStageModuleSpawned() const
{
	return m_IsFinalStageModuleSpawned;
}

void StageBlockManager::ReleaseGoalZClamp()
{
	m_IsGoalZClampReleased =
		true;
}

bool StageBlockManager::IsGoalZClampReleased() const
{
	return m_IsGoalZClampReleased;
}