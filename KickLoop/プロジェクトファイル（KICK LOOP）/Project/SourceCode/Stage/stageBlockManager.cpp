#include "stageBlockManager.h"

#include "groundBlockManager.h"
#include "laserWall.h"
#include "manager.h"
#include "player.h"
#include "polygon3D.h"
#include "scene.h"
#include "stageBlock.h"

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

	// 取得したコインと生成済み区間の記録。
	m_CollectedTimeCoinRecords.clear();
	m_RoomRecords.clear();

	m_DistanceDeleteStageBlocks.clear();
	m_RoomProgressAreas.clear();

	m_NextStageModuleSpawnDistance = 40.0f;
	m_StageModuleSpawnInterval = 40.0f;

	m_StageGenerateEndDistance = 5000.0f;
	m_IsFinalStageModuleSpawned = false;

	m_DefaultWalkMinX = 15.0f;
	m_DefaultWalkMaxX = 35.0f;

	m_StartClampBackOffset = 10.0f;

	m_RoomClampPassedOffset = 0.0f;

	m_MinClampZ = m_PlayerBaseZ - m_StartClampBackOffset;

	m_FinalClampZ = 0.0f;
	m_HasFinalClampZ = false;

	m_IsGoalZClampReleased = false;

	m_StageModuleManager.InitDefaultModules();

	m_IsPoolPreCreated = false;
}

void StageBlockManager::Update()
{
	if (!m_IsPoolPreCreated)
	{
		PreCreateObjectPools();

		m_IsPoolPreCreated = true;

		return;
	}

	UpdateStageModuleSpawn();

	UpdateDistanceDeleteStageBlocks();

	Scene* scene = Manager::GetScene();

	Player* player = nullptr;

	if (scene != nullptr)
	{
		player = scene->GetGameObject<Player>();
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

bool StageBlockManager::HasActiveStageBlock(const StageBlock* stageBlock) const
{
	if (stageBlock == nullptr)
	{
		return false;
	}

	for (const StageBlock* block : m_StageBlocks)
	{
		if (block == stageBlock)
		{
			return block->Active() && !block->Destroy();
		}
	}

	return false;
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

void StageBlockManager::UpdateCommentVisible()
{
	Scene* scene = Manager::GetScene();

	Player* player = nullptr;

	if (scene != nullptr)
	{
		player = scene->GetGameObject<Player>();
	}

	float playerZ = m_PlayerBaseZ;

	if (player != nullptr)
	{
		playerZ = player->GetPosition().z;
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

		Vector3 uiPosition = commentUI->GetPosition();

		float distanceZ = uiPosition.z - playerZ;

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

	data.stageBlock = stageBlock;

	data.deleteDistance = deleteDistance;

	data.isDeleted = false;

	m_DistanceDeleteStageBlocks.push_back(
		data
	);
}

void StageBlockManager::UpdateDistanceDeleteStageBlocks()
{
	float progressDistance = GetProgressDistance();

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

float StageBlockManager::GetProgressDistance()
{
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return 0.0f;
	}

	Player* player = scene->GetGameObject<Player>();

	if (player == nullptr)
	{
		return 0.0f;
	}

	return player->GetProgressDistance();
}

void StageBlockManager::AddRoomProgressArea(
	float roomStartZ,
	float roomEndZ
)
{
	if (roomEndZ < roomStartZ)
	{
		float temp = roomStartZ;

		roomStartZ = roomEndZ;

		roomEndZ = temp;
	}

	if (roomEndZ <= roomStartZ)
	{
		return;
	}

	StageRoomProgressArea area;

	area.startZ = roomStartZ;

	area.endZ = roomEndZ;

	m_RoomProgressAreas.push_back(
		area
	);
}

// 取得したタイムコインの位置、サイズ、加算秒数を記録する
void StageBlockManager::AddCollectedTimeCoinRecord(
	Vector3 position,
	Vector3 size,
	float addTime
)
{
	CollectedTimeCoinRecord record;

	record.position = position;

	record.size = size;

	record.addTime = addTime;

	m_CollectedTimeCoinRecords.push_back(
		record
	);
}

void StageBlockManager::UpdateGoalCameraStage(float cameraZ)
{
	m_MinClampZ = cameraZ - 300.0f;

	DestroyObjectsBehindMinClampZ();

	RebuildPassedStageBlocks(cameraZ);
}

void StageBlockManager::DeletePassedRoomProgressAreas(float playerZ)
{
	float passedZ = playerZ - m_RoomClampPassedOffset;

	bool isDeletedRoom = false;

	float latestDeletedRoomStartZ = m_MinClampZ + m_StartClampBackOffset;

	for (int i = (int)m_RoomProgressAreas.size() - 1; i >= 0; i--)
	{
		if (m_RoomProgressAreas[i].endZ > passedZ)
		{
			continue;
		}

		if (!isDeletedRoom)
		{
			latestDeletedRoomStartZ = m_RoomProgressAreas[i].startZ;
		}
		else
		{
			if (m_RoomProgressAreas[i].startZ > latestDeletedRoomStartZ)
			{
				latestDeletedRoomStartZ = m_RoomProgressAreas[i].startZ;
			}
		}

		m_RoomProgressAreas.erase(
			m_RoomProgressAreas.begin() + i
		);

		isDeletedRoom = true;
	}

	if (!isDeletedRoom)
	{
		return;
	}

	float nextMinClampZ = latestDeletedRoomStartZ - m_StartClampBackOffset;

	if (nextMinClampZ > m_MinClampZ)
	{
		m_MinClampZ = nextMinClampZ;
	}

	DestroyObjectsBehindMinClampZ();

	Scene* scene = Manager::GetScene();

	if (scene != nullptr)
	{
		GroundBlockManager* groundBlockManager = scene->GetGameObject<GroundBlockManager>();

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

	outMinX = m_DefaultWalkMinX + playerHalfSizeX;

	outMaxX = m_DefaultWalkMaxX - playerHalfSizeX;

	if (outMaxX < outMinX)
	{
		float center = (outMinX + outMaxX) * 0.5f;

		outMinX = center;

		outMaxX = center;
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
		outMinZ = -999999.0f;

		outMaxZ = 999999.0f;

		return true;
	}

	outMinZ = m_MinClampZ;

	if (m_HasFinalClampZ)
	{
		outMaxZ = m_FinalClampZ;
	}
	else
	{
		outMaxZ = GetStageGenerateEndWorldZ();
	}

	if (outMaxZ < outMinZ)
	{
		float center = (outMinZ + outMaxZ) * 0.5f;

		outMinZ = center;

		outMaxZ = center;
	}

	return true;
}

void StageBlockManager::SetStageGenerateEndDistance(float distance)
{
	if (distance < 0.0f)
	{
		distance = 0.0f;
	}

	m_StageGenerateEndDistance = distance;
}

float StageBlockManager::GetStageGenerateEndDistance() const
{
	return m_StageGenerateEndDistance;
}

float StageBlockManager::GetStageGenerateEndWorldZ() const
{
	float finalRoomLength = 0.0f;

	const StageModuleData* finalRoom = m_StageModuleManager.GetFinalJumpPadRoom();

	if (finalRoom != nullptr)
	{
		finalRoomLength = finalRoom->length;
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
	m_IsGoalZClampReleased = true;
}

bool StageBlockManager::IsGoalZClampReleased() const
{
	return m_IsGoalZClampReleased;
}
