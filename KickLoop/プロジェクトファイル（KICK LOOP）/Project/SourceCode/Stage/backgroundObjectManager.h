#pragma once

#include <vector>
#include <string>

#include "main.h"
#include "gameObject.h"

class BackgroundObject;

// 生成済み背景オブジェクトを管理するための記録。
struct BackgroundObjectData
{
	BackgroundObject* object = nullptr;
};

// プレイヤーの進行に合わせて、左右に背景オブジェクトを生成・再利用する管理クラス。
class BackgroundObjectManager : public GameObject
{
private:
	// 現在表示中の背景オブジェクト。
	std::vector<BackgroundObjectData> m_BackgroundObjects;

	// 背景オブジェクトをDestroyせず再利用するためのPool。
	std::vector<BackgroundObject*> m_BackgroundObjectPool;

	bool m_IsPoolPreCreated = false;
	int m_InitialBackgroundObjectPoolCount = 40;

	// 背景生成に使うOBJモデルの候補。
	std::vector<std::string> m_ModelPaths;

	// ゴールカメラ用に背景を再構築した後、通常の背景生成を止める。
	bool m_IsGoalCameraRebuildMode = false;

	// プレイヤー前方への生成距離と、後方オブジェクトをPoolへ戻す距離。
	float m_NextSpawnZ = 0.0f;
	float m_SpawnInterval = 150.0f;

	float m_GenerateAheadDistance = 800.0f;
	float m_DestroyBehindDistance = 220.0f;

	// 開始直後に背景密度を出すため、同じZに複数組配置する。
	int m_InitialSameZObjectCount = 3;
	float m_InitialSameZXSpacing = 35.0f;

	// Roomの歩けるX範囲を基準に、左右の背景配置位置を決める。
	float m_LeftX = -85.0f;
	float m_RightX = 135.0f;

	float m_SideOffsetFromRoom = 100.0f;

	bool m_HasRoomSideX = false;
	float m_RoomLeftBackgroundX = -85.0f;
	float m_RoomRightBackgroundX = 135.0f;

	bool m_HasSpawnInitialObjects = false;

	// 背景OBJの見た目用スケール、ランダム幅、左右の向き。
	float m_BackgroundObjectScale = 12.0f;

	float m_SpawnRandomZRange = 0.0f;
	float m_SpawnRandomXRange = 0.0f;

	float m_LeftRotationY = 1.5707963f;
	float m_RightRotationY = -1.5707963f;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void RebuildPassedBackgroundObjects();

private:
	void InitModelPaths();

	void PreCreateObjectPools();

	BackgroundObject* GetBackgroundObjectFromPool();
	void ReturnBackgroundObjectToPool(BackgroundObject* object);

	void ClearActiveObjectsToPool();

	bool UpdateRoomSideX();

	void SpawnInitialObjects();
	void SpawnObjectsAhead(float playerZ);

	void SpawnPair(
		float z,
		float extraSideOffsetX = 0.0f
	);

	void DestroyPassedObjects(float playerZ);

	float CalculatePerspectiveScale(
		float z,
		float extraSideOffsetX
	);

	float GetRandomRange(
		float minValue,
		float maxValue
	);

	float GetPlayerZ() const;
};
