#include "main.h"

#include "groundBlockManager.h"

#include "manager.h"
#include "scene.h"

#include "stageBlock.h"
#include "stageBlockManager.h"
#include "player.h"

void GroundBlockManager::Init()
{
	m_DanceFloorBlocks.clear();

	m_DanceFloorColorPattern.clear();
	m_DanceFloorColorPatternIndex = 0;

	m_IsGoalCameraRebuildMode = false;

	m_UseCollision = true;

	m_BlockHalfSizeX = m_FloorHalfSizeX;
	m_BlockHalfSizeY = m_TileHalfSizeY;
	m_BlockHalfSizeZ = m_FloorHalfSizeX;
	m_BlockFullSizeZ = m_BlockHalfSizeZ * 2.0f;

	m_NextSpawnZ = 0.0f;
}

void GroundBlockManager::Uninit()
{
	Scene* scene = Manager::GetScene();

	StageBlockManager* stageBlockManager = nullptr;

	if (scene != nullptr)
	{
		stageBlockManager = scene->GetGameObject<StageBlockManager>();
	}

	for (StageBlock* block : m_DanceFloorBlocks)
	{
		if (block == nullptr)
		{
			continue;
		}

		if (stageBlockManager != nullptr)
		{
			stageBlockManager->RemoveStageBlock(block);
		}

		block->SetActive(false);
		block->SetVisible(false);
		block->SetDestroy();
	}

	m_DanceFloorBlocks.clear();

	m_DanceFloorColorPattern.clear();
	m_DanceFloorColorPatternIndex = 0;

	m_NextSpawnZ = 0.0f;
}

void GroundBlockManager::Update()
{
	if (m_IsGoalCameraRebuildMode)
	{
		return;
	}

	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Player* player = scene->GetGameObject<Player>();

	if (player == nullptr)
	{
		return;
	}

	StageBlockManager* stageBlockManager = scene->GetGameObject<StageBlockManager>();

	float playerZ = player->GetPosition().z;

	RecyclePassedFloorTiles(
		playerZ,
		stageBlockManager
	);
}

void GroundBlockManager::Draw()
{
}

void GroundBlockManager::SetGroundData(const GroundLoadData& groundData)
{
	m_FloorCenterX = groundData.center.x;
	m_FloorCenterY = groundData.center.y;
	m_FloorCenterZ = groundData.center.z;

	m_FloorHalfSizeX = groundData.halfSize.x;
	m_FloorHalfSizeY = groundData.halfSize.y;
	m_FloorHalfSizeZ = groundData.halfSize.z;

	m_TileSize = groundData.tileSize;
	m_TileHalfSizeY = groundData.tileHalfSizeY;

	SetDanceFloorColorPattern(groundData.colorPattern);
}

void GroundBlockManager::CreateFloorTiles(
	Scene* scene,
	StageBlockManager* stageBlockManager,
	bool useCollision
)
{
	if (scene == nullptr)
	{
		return;
	}

	m_UseCollision = useCollision;

	// すでに床がある場合は作り直す。
	for (StageBlock* block : m_DanceFloorBlocks)
	{
		if (block == nullptr)
		{
			continue;
		}

		if (stageBlockManager != nullptr)
		{
			stageBlockManager->RemoveStageBlock(block);
		}

		block->SetActive(false);
		block->SetVisible(false);
		block->SetDestroy();
	}

	m_DanceFloorBlocks.clear();

	m_DanceFloorColorPatternIndex = 0;

	m_BlockHalfSizeX = m_FloorHalfSizeX;
	m_BlockHalfSizeY = m_TileHalfSizeY;

	// 床1枚を正方形にして、奥行き方向へ連続配置する。
	m_BlockHalfSizeZ = m_FloorHalfSizeX;
	m_BlockFullSizeZ = m_BlockHalfSizeZ * 2.0f;

	float startZ = m_FloorCenterZ - m_BlockHalfSizeZ;

	for (int i = 0; i < INITIAL_BLOCK_COUNT; i++)
	{
		float positionZ = startZ + m_BlockFullSizeZ * (float)i;

		CreateFloorTile(
			scene,
			stageBlockManager,
			positionZ
		);
	}

	m_NextSpawnZ = startZ + m_BlockFullSizeZ * (float)INITIAL_BLOCK_COUNT;
}

void GroundBlockManager::CreateFloorTile(
	Scene* scene,
	StageBlockManager* stageBlockManager,
	float positionZ
)
{
	if (scene == nullptr)
	{
		return;
	}

	StageBlock* block = scene->AddGameObject<StageBlock>(1);

	if (block == nullptr)
	{
		return;
	}

	float blockY = m_FloorCenterY + m_FloorHalfSizeY + m_BlockHalfSizeY + 0.01f;

	block->SetPosition(Vector3(m_FloorCenterX, blockY, positionZ));
	block->SetSize(Vector3(m_BlockHalfSizeX, m_BlockHalfSizeY, m_BlockHalfSizeZ));

	block->SetTexture(m_TileTexturePath.c_str());

	block->SetUVScroll(false);
	block->SetUVFlipZ(true);

	block->SetRainbow(false);
	block->SetRainbowSpeed(1.0f);

	block->SetColor(
		1.0f,
		1.0f,
		1.0f,
		1.0f
	);

	block->SetActive(true);
	block->SetVisible(true);

	AddDanceFloorBlock(block);

	if (m_UseCollision && stageBlockManager != nullptr)
	{
		stageBlockManager->AddStageBlock(block);
	}
}

// 通過した床を前方へ回して再利用する
void GroundBlockManager::RecyclePassedFloorTiles(
	float playerZ,
	StageBlockManager* stageBlockManager
)
{
	float recycleLimitZ = playerZ - m_DestroyBehindDistance;
	float spawnLimitZ = 999999.0f;

	if (stageBlockManager != nullptr)
	{
		spawnLimitZ = stageBlockManager->GetStageGenerateEndWorldZ();
	}

	for (StageBlock* block : m_DanceFloorBlocks)
	{
		if (block == nullptr)
		{
			continue;
		}

		if (block->Destroy())
		{
			continue;
		}

		Vector3 blockPosition = block->GetPosition();

		float blockBackZ = blockPosition.z + m_BlockHalfSizeZ;

		if (blockBackZ >= recycleLimitZ)
		{
			continue;
		}

		// ゴール用の終端を越える位置には床を送らず、Pool待機のように非表示にする。
		if (m_NextSpawnZ > spawnLimitZ)
		{
			if (stageBlockManager != nullptr)
			{
				stageBlockManager->RemoveStageBlock(block);
			}

			block->SetActive(false);
			block->SetVisible(false);

			continue;
		}

		// 前方へ移動する前に、当たり判定リストから一度外して二重登録を防ぐ。
		if (stageBlockManager != nullptr)
		{
			stageBlockManager->RemoveStageBlock(block);
		}

		blockPosition.z = m_NextSpawnZ;

		block->SetPosition(blockPosition);

		block->SetActive(true);
		block->SetVisible(true);

		if (m_UseCollision && stageBlockManager != nullptr)
		{
			stageBlockManager->AddStageBlock(block);
		}

		m_NextSpawnZ += m_BlockFullSizeZ;
	}
}

void GroundBlockManager::AddDanceFloorBlock(StageBlock* stageBlock)
{
	if (stageBlock == nullptr)
	{
		return;
	}

	m_DanceFloorBlocks.push_back(stageBlock);

	ApplyDanceFloorColor(stageBlock);
}

XMFLOAT4 GroundBlockManager::GetDanceFloorColorByNumber(int colorNumber)
{
	static const XMFLOAT4 colors[] =
	{
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 0.1f, 0.1f, 1.0f),
		XMFLOAT4(0.1f, 0.4f, 1.0f, 1.0f),
		XMFLOAT4(0.55f, 0.1f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 0.1f, 0.85f, 1.0f),
		XMFLOAT4(0.1f, 1.0f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 0.9f, 0.1f, 1.0f),
		XMFLOAT4(0.2f, 1.0f, 0.3f, 1.0f),
		XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f),
		XMFLOAT4(0.08f, 0.02f, 0.12f, 1.0f),
		XMFLOAT4(0.02f, 0.06f, 0.10f, 1.0f),
		XMFLOAT4(0.10f, 0.04f, 0.02f, 1.0f),
	};

	if (colorNumber < 1 || colorNumber > 11)
	{
		return colors[0];
	}

	return colors[colorNumber];
}

void GroundBlockManager::ApplyDanceFloorColor(StageBlock* stageBlock)
{
	if (stageBlock == nullptr)
	{
		return;
	}

	stageBlock->SetRainbow(false);

	stageBlock->SetColor(
		1.0f,
		1.0f,
		1.0f,
		1.0f
	);
}

void GroundBlockManager::SetDanceFloorColorPattern(const std::vector<int>& colorPattern)
{
	m_DanceFloorColorPattern = colorPattern;
	m_DanceFloorColorPatternIndex = 0;

	ApplyDanceFloorColors();
}

void GroundBlockManager::ApplyDanceFloorColors()
{
	m_DanceFloorColorPatternIndex = 0;

	for (StageBlock* danceFloorBlock : m_DanceFloorBlocks)
	{
		ApplyDanceFloorColor(danceFloorBlock);
	}
}

void GroundBlockManager::DestroyBlocksBehindZ(float deleteZ)
{
	Scene* scene = Manager::GetScene();

	StageBlockManager* stageBlockManager = nullptr;

	if (scene != nullptr)
	{
		stageBlockManager = scene->GetGameObject<StageBlockManager>();
	}

	for (StageBlock* block : m_DanceFloorBlocks)
	{
		if (block == nullptr)
		{
			continue;
		}

		if (block->Destroy())
		{
			continue;
		}

		Vector3 blockPosition = block->GetPosition();
		float blockBackZ = blockPosition.z + m_BlockHalfSizeZ;

		// 1枚だけ後ろに残すため、deleteZよりさらに1枚分後ろの床だけ再利用する。
		float safeDeleteZ = deleteZ - m_BlockFullSizeZ;

		if (blockBackZ >= safeDeleteZ)
		{
			continue;
		}

		if (stageBlockManager != nullptr)
		{
			stageBlockManager->RemoveStageBlock(block);
		}

		blockPosition.z = m_NextSpawnZ;

		block->SetPosition(blockPosition);
		block->SetActive(true);
		block->SetVisible(true);

		if (m_UseCollision && stageBlockManager != nullptr)
		{
			stageBlockManager->AddStageBlock(block);
		}

		m_NextSpawnZ += m_BlockFullSizeZ;
	}
}

// 進行位置に合わせて床を再構築する
void GroundBlockManager::RebuildPassedGroundBlocks()
{
	m_IsGoalCameraRebuildMode = true;

	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Player* player = scene->GetGameObject<Player>();

	if (player == nullptr)
	{
		return;
	}

	StageBlockManager* stageBlockManager = scene->GetGameObject<StageBlockManager>();

	float startZ = 13.0f;
	float endZ = player->GetPosition().z - 15.0f;

	if (startZ > endZ)
	{
		float temp = startZ;
		startZ = endZ;
		endZ = temp;
	}

	float rebuildStartZ = startZ;
	float rebuildEndZ = endZ + m_BlockFullSizeZ;

	int blockIndex = 0;

	for (float positionZ = rebuildStartZ; positionZ <= rebuildEndZ; positionZ += m_BlockFullSizeZ)
	{
		StageBlock* block = nullptr;

		if (blockIndex < (int)m_DanceFloorBlocks.size())
		{
			block = m_DanceFloorBlocks[blockIndex];
		}
		else
		{
			CreateFloorTile(
				scene,
				stageBlockManager,
				positionZ
			);

			blockIndex++;

			continue;
		}

		blockIndex++;

		if (block == nullptr)
		{
			continue;
		}

		if (stageBlockManager != nullptr)
		{
			stageBlockManager->RemoveStageBlock(block);
		}

		float blockY = m_FloorCenterY + m_FloorHalfSizeY + m_BlockHalfSizeY + 0.01f;

		block->SetPosition(Vector3(m_FloorCenterX, blockY, positionZ));
		block->SetSize(Vector3(m_BlockHalfSizeX, m_BlockHalfSizeY, m_BlockHalfSizeZ));

		block->SetTexture(m_TileTexturePath.c_str());

		block->SetUVScroll(false);
		block->SetUVFlipZ(true);

		block->SetRainbow(false);
		block->SetRainbowSpeed(1.0f);

		block->SetColor(
			1.0f,
			1.0f,
			1.0f,
			1.0f
		);

		block->SetActive(true);
		block->SetVisible(true);

		if (m_UseCollision && stageBlockManager != nullptr)
		{
			stageBlockManager->AddStageBlock(block);
		}
	}

	for (int i = blockIndex; i < (int)m_DanceFloorBlocks.size(); i++)
	{
		StageBlock* block = m_DanceFloorBlocks[i];

		if (block == nullptr)
		{
			continue;
		}

		if (stageBlockManager != nullptr)
		{
			stageBlockManager->RemoveStageBlock(block);
		}

		block->SetActive(false);
		block->SetVisible(false);
	}

	m_NextSpawnZ = rebuildEndZ + m_BlockFullSizeZ;
}
