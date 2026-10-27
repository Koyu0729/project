#include "backgroundObjectManager.h"

#include "backgroundObject.h"

#include "manager.h"
#include "scene.h"
#include "player.h"
#include "stageBlockManager.h"

#include <cstdlib>

void BackgroundObjectManager::Init()
{
	m_BackgroundObjects.clear();
	m_BackgroundObjectPool.clear();
	m_ModelPaths.clear();

	m_IsPoolPreCreated = false;
	m_InitialBackgroundObjectPoolCount = 40;

	m_IsGoalCameraRebuildMode = false;

	m_NextSpawnZ = 0.0f;
	m_SpawnInterval = 150.0f;

	m_GenerateAheadDistance = 800.0f;
	m_DestroyBehindDistance = 220.0f;

	m_InitialSameZObjectCount = 3;
	m_InitialSameZXSpacing = 35.0f;

	m_LeftX = -85.0f;
	m_RightX = 135.0f;

	m_SideOffsetFromRoom = 100.0f;

	m_HasRoomSideX = false;
	m_RoomLeftBackgroundX = m_LeftX;
	m_RoomRightBackgroundX = m_RightX;

	m_HasSpawnInitialObjects = false;

	m_BackgroundObjectScale = 12.0f;

	m_SpawnRandomZRange = 0.0f;
	m_SpawnRandomXRange = 0.0f;

	m_LeftRotationY = 1.5707963f;
	m_RightRotationY = -1.5707963f;

	InitModelPaths();
}

void BackgroundObjectManager::Uninit()
{
	for (BackgroundObjectData& data : m_BackgroundObjects)
	{
		if (data.object == nullptr)
		{
			continue;
		}

		data.object->SetActive(false);
		data.object->SetVisible(false);
		data.object->SetDestroy();
		data.object = nullptr;
	}

	m_BackgroundObjects.clear();

	for (BackgroundObject* object : m_BackgroundObjectPool)
	{
		if (object == nullptr)
		{
			continue;
		}

		object->SetActive(false);
		object->SetVisible(false);
		object->SetDestroy();
	}

	m_BackgroundObjectPool.clear();
	m_ModelPaths.clear();

	m_IsPoolPreCreated = false;

	m_HasRoomSideX = false;
	m_HasSpawnInitialObjects = false;

	m_IsGoalCameraRebuildMode = false;
}

void BackgroundObjectManager::Update()
{
	if (!m_IsPoolPreCreated)
	{
		PreCreateObjectPools();

		if (!m_IsPoolPreCreated)
		{
			return;
		}
	}

	if (m_IsGoalCameraRebuildMode)
	{
		return;
	}

	if (!m_HasRoomSideX)
	{
		if (!UpdateRoomSideX())
		{
			return;
		}
	}

	if (!m_HasSpawnInitialObjects)
	{
		SpawnInitialObjects();

		m_HasSpawnInitialObjects = true;
	}

	float playerZ = GetPlayerZ();

	SpawnObjectsAhead(playerZ);
	DestroyPassedObjects(playerZ);
}

void BackgroundObjectManager::Draw()
{
}

void BackgroundObjectManager::InitModelPaths()
{
	m_ModelPaths.clear();

	m_ModelPaths.push_back("asset\\model\\cyberTower.obj");
}

// 背景モデルの再利用分を先に作る
void BackgroundObjectManager::PreCreateObjectPools()
{
	if (m_IsPoolPreCreated)
	{
		return;
	}

	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	for (int i = 0; i < m_InitialBackgroundObjectPoolCount; i++)
	{
		BackgroundObject* object = scene->AddGameObject<BackgroundObject>(3);

		if (object == nullptr)
		{
			continue;
		}

		object->SetActive(false);
		object->SetVisible(false);

		m_BackgroundObjectPool.push_back(object);
	}

	if (!m_BackgroundObjectPool.empty())
	{
		m_IsPoolPreCreated = true;
	}
}

BackgroundObject* BackgroundObjectManager::GetBackgroundObjectFromPool()
{
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return nullptr;
	}

	while (!m_BackgroundObjectPool.empty())
	{
		BackgroundObject* object = m_BackgroundObjectPool.back();

		m_BackgroundObjectPool.pop_back();

		if (object == nullptr)
		{
			continue;
		}

		if (object->Destroy())
		{
			continue;
		}

		object->SetActive(true);
		object->SetVisible(true);

		return object;
	}

	BackgroundObject* object = scene->AddGameObject<BackgroundObject>(3);

	if (object == nullptr)
	{
		return nullptr;
	}

	object->SetActive(true);
	object->SetVisible(true);

	return object;
}

void BackgroundObjectManager::ReturnBackgroundObjectToPool(BackgroundObject* object)
{
	if (object == nullptr)
	{
		return;
	}

	if (object->Destroy())
	{
		return;
	}

	object->SetActive(false);
	object->SetVisible(false);

	m_BackgroundObjectPool.push_back(object);
}

void BackgroundObjectManager::ClearActiveObjectsToPool()
{
	for (BackgroundObjectData& data : m_BackgroundObjects)
	{
		if (data.object == nullptr)
		{
			continue;
		}

		ReturnBackgroundObjectToPool(data.object);

		data.object = nullptr;
	}

	m_BackgroundObjects.clear();
}

bool BackgroundObjectManager::UpdateRoomSideX()
{
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return false;
	}

	StageBlockManager* stageBlockManager = scene->GetGameObject<StageBlockManager>();

	if (stageBlockManager == nullptr)
	{
		m_RoomLeftBackgroundX = m_LeftX;
		m_RoomRightBackgroundX = m_RightX;
		m_HasRoomSideX = true;

		return true;
	}

	float walkMinX = 0.0f;
	float walkMaxX = 0.0f;

	if (!stageBlockManager->GetWalkableXRange(
		0.0f,
		0.0f,
		walkMinX,
		walkMaxX
	))
	{
		m_RoomLeftBackgroundX = m_LeftX;
		m_RoomRightBackgroundX = m_RightX;
		m_HasRoomSideX = true;

		return true;
	}

	m_RoomLeftBackgroundX = walkMinX - m_SideOffsetFromRoom;
	m_RoomRightBackgroundX = walkMaxX + m_SideOffsetFromRoom;
	m_HasRoomSideX = true;

	return true;
}

void BackgroundObjectManager::SpawnInitialObjects()
{
	float playerZ = GetPlayerZ();

	for (int i = 0; i < m_InitialSameZObjectCount; i++)
	{
		float extraSideOffsetX = m_InitialSameZXSpacing * (float)i;

		SpawnPair(
			playerZ,
			extraSideOffsetX
		);
	}

	m_NextSpawnZ = playerZ + m_SpawnInterval;

	SpawnObjectsAhead(playerZ);
}

void BackgroundObjectManager::SpawnObjectsAhead(float playerZ)
{
	float generateLimitZ = playerZ + m_GenerateAheadDistance;

	while (m_NextSpawnZ <= generateLimitZ)
	{
		SpawnPair(m_NextSpawnZ);

		m_NextSpawnZ += m_SpawnInterval;
	}
}

void BackgroundObjectManager::SpawnPair(
	float z,
	float extraSideOffsetX
)
{
	if (m_ModelPaths.empty())
	{
		return;
	}

	if (!m_HasRoomSideX)
	{
		if (!UpdateRoomSideX())
		{
			m_RoomLeftBackgroundX = m_LeftX;
			m_RoomRightBackgroundX = m_RightX;
			m_HasRoomSideX = true;
		}
	}

	float leftX = m_RoomLeftBackgroundX;
	float rightX = m_RoomRightBackgroundX;

	for (int side = 0; side < 2; side++)
	{
		BackgroundObject* object = GetBackgroundObjectFromPool();

		if (object == nullptr)
		{
			continue;
		}

		int modelIndex = rand() % (int)m_ModelPaths.size();

		float sideX = 0.0f;

		if (side == 0)
		{
			sideX = leftX - extraSideOffsetX;
		}
		else
		{
			sideX = rightX + extraSideOffsetX;
		}

		float randomX = GetRandomRange(
			-m_SpawnRandomXRange,
			m_SpawnRandomXRange
		);

		float randomZ = GetRandomRange(
			-m_SpawnRandomZRange,
			m_SpawnRandomZRange
		);

		float scale = CalculatePerspectiveScale(
			z,
			extraSideOffsetX
		);

		object->LoadModel(m_ModelPaths[modelIndex]);

		object->SetPosition(Vector3(sideX + randomX, 0.0f, z + randomZ));
		object->SetScale(Vector3(scale, scale, scale));

		if (side == 0)
		{
			object->SetRotation(Vector3(0.0f, m_LeftRotationY, 0.0f));
		}
		else
		{
			object->SetRotation(Vector3(0.0f, m_RightRotationY, 0.0f));
		}

		object->SetActive(true);
		object->SetVisible(true);

		BackgroundObjectData data;

		data.object = object;

		m_BackgroundObjects.push_back(data);
	}
}

// 通過した背景を再利用待ちに戻す
void BackgroundObjectManager::DestroyPassedObjects(float playerZ)
{
	float destroyZ = playerZ - m_DestroyBehindDistance;

	for (int i = (int)m_BackgroundObjects.size() - 1; i >= 0; i--)
	{
		BackgroundObject* object = m_BackgroundObjects[i].object;

		if (object == nullptr)
		{
			m_BackgroundObjects.erase(m_BackgroundObjects.begin() + i);

			continue;
		}

		if (object->Destroy())
		{
			m_BackgroundObjects.erase(m_BackgroundObjects.begin() + i);

			continue;
		}

		if (object->GetPosition().z > destroyZ)
		{
			continue;
		}

		ReturnBackgroundObjectToPool(object);

		m_BackgroundObjects.erase(m_BackgroundObjects.begin() + i);
	}
}

// 進行位置に合わせて背景を再配置する
void BackgroundObjectManager::RebuildPassedBackgroundObjects()
{
	m_IsGoalCameraRebuildMode = true;

	ClearActiveObjectsToPool();

	if (!m_HasRoomSideX)
	{
		if (!UpdateRoomSideX())
		{
			m_RoomLeftBackgroundX = m_LeftX;
			m_RoomRightBackgroundX = m_RightX;
			m_HasRoomSideX = true;
		}
	}

	float startZ = 13.0f;
	float endZ = GetPlayerZ() - 15.0f;

	if (startZ > endZ)
	{
		float temp = startZ;
		startZ = endZ;
		endZ = temp;
	}

	for (int i = 0; i < m_InitialSameZObjectCount; i++)
	{
		float extraSideOffsetX = m_InitialSameZXSpacing * (float)i;

		SpawnPair(
			startZ,
			extraSideOffsetX
		);
	}

	float spawnZ = startZ + m_SpawnInterval;
	float rebuildEndZ = endZ + m_SpawnInterval;

	while (spawnZ <= rebuildEndZ)
	{
		SpawnPair(spawnZ);

		spawnZ += m_SpawnInterval;
	}

	m_NextSpawnZ = rebuildEndZ + m_SpawnInterval;
	m_HasSpawnInitialObjects = true;
}

// 距離に応じた背景モデルの大きさを求める
float BackgroundObjectManager::CalculatePerspectiveScale(
	float z,
	float extraSideOffsetX
)
{
	return m_BackgroundObjectScale;
}

float BackgroundObjectManager::GetRandomRange(
	float minValue,
	float maxValue
)
{
	if (maxValue < minValue)
	{
		float temp = minValue;
		minValue = maxValue;
		maxValue = temp;
	}

	float rate = (float)rand() / (float)RAND_MAX;

	return minValue + (maxValue - minValue) * rate;
}

float BackgroundObjectManager::GetPlayerZ() const
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

	return player->GetPosition().z;
}
