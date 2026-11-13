#include "stageBlockManager.h"

#include "jumpPad.h"
#include "laserWall.h"
#include "manager.h"
#include "polygon3D.h"
#include "scene.h"
#include "stageMaskBlock.h"
#include "timeCoin.h"

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


	m_IsFinalStageModuleSpawned = false;
	m_IsGoalZClampReleased = false;
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

void StageBlockManager::DestroyObjectsBehindMinClampZ()
{
	float deleteZ = m_MinClampZ;

	for (int i = (int)m_Obstacles.size() - 1; i >= 0; i--)
	{
		StageBlock* obstacle = m_Obstacles[i];

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

		Vector3 position = obstacle->GetPosition();

		if (position.z >= deleteZ)
		{
			continue;
		}

		StageMaskBlock* maskBlock = (StageMaskBlock*)obstacle;

		ReturnObstacleToPool(
			maskBlock
		);

		m_Obstacles.erase(
			m_Obstacles.begin() + i
		);
	}

	for (int i = (int)m_LaserWalls.size() - 1; i >= 0; i--)
	{
		LaserWall* laserWall = m_LaserWalls[i];

		if (laserWall == nullptr)
		{
			m_LaserWalls.erase(
				m_LaserWalls.begin() + i
			);

			continue;
		}

		Vector3 position = laserWall->GetHitCenter();

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
		JumpPad* jumpPad = m_JumpPads[i];

		if (jumpPad == nullptr)
		{
			m_JumpPads.erase(
				m_JumpPads.begin() + i
			);

			continue;
		}

		Vector3 position = jumpPad->GetHitCenter();

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
		TimeCoin* timeCoin = m_TimeCoins[i];

		if (timeCoin == nullptr)
		{
			m_TimeCoins.erase(
				m_TimeCoins.begin() + i
			);

			continue;
		}

		Vector3 position = timeCoin->GetHitCenter();

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
		Polygon3D* commentUI = m_CommentUIs[i];

		if (commentUI == nullptr)
		{
			m_CommentUIs.erase(
				m_CommentUIs.begin() + i
			);

			continue;
		}

		Vector3 position = commentUI->GetPosition();

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
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	for (int i = 0; i < m_InitialObstaclePoolCount; i++)
	{
		StageMaskBlock* obstacle = scene->AddGameObject<StageMaskBlock>(SceneLayer::SceneObjects);

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
		TimeCoin* timeCoin = scene->AddGameObject<TimeCoin>(SceneLayer::SceneObjects);

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

		obstacle = scene->AddGameObject<StageMaskBlock>(SceneLayer::SceneObjects);
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
	LaserWall* laserWall = nullptr;

	if (!m_LaserWallPool.empty())
	{
		laserWall = m_LaserWallPool.back();

		m_LaserWallPool.pop_back();
	}
	else
	{
		Scene* scene = Manager::GetScene();

		if (scene == nullptr)
		{
			return nullptr;
		}

		laserWall = scene->AddGameObject<LaserWall>(SceneLayer::SceneObjects);
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
	JumpPad* jumpPad = nullptr;

	if (!m_JumpPadPool.empty())
	{
		jumpPad = m_JumpPadPool.back();

		m_JumpPadPool.pop_back();
	}
	else
	{
		Scene* scene = Manager::GetScene();

		if (scene == nullptr)
		{
			return nullptr;
		}

		jumpPad = scene->AddGameObject<JumpPad>(SceneLayer::SceneObjects);
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
	TimeCoin* timeCoin = nullptr;

	if (!m_TimeCoinPool.empty())
	{
		timeCoin = m_TimeCoinPool.back();

		m_TimeCoinPool.pop_back();
	}
	else
	{
		Scene* scene = Manager::GetScene();

		if (scene == nullptr)
		{
			return nullptr;
		}

		timeCoin = scene->AddGameObject<TimeCoin>(SceneLayer::SceneObjects);
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
