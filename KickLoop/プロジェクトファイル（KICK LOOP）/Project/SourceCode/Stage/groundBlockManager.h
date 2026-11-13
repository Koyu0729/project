#pragma once

#include <vector>
#include <string>

#include "gameObject.h"
#include "main.h"
#include "gameConfigData.h"

class Scene;
class StageBlock;
class StageBlockManager;

// プレイヤーの進行に合わせて、床ブロックを前方へ再利用する管理クラス。
class GroundBlockManager : public GameObject
{
private:
	// 現在床として使用しているStageBlock。
	std::vector<StageBlock*> m_DanceFloorBlocks;

	// ゴール演出用に床を再構築した後、通常の床リサイクルを止める。
	bool m_IsGoalCameraRebuildMode = false;

	// 床色パターン用の値。現在は固定白色で表示している。
	std::vector<int> m_DanceFloorColorPattern;
	int m_DanceFloorColorPatternIndex = 0;

	// ロードデータから設定される床全体の中心と半径サイズ。
	float m_FloorCenterX = 25.0f;
	float m_FloorCenterY = 0.0f;
	float m_FloorCenterZ = 23.0f;

	float m_FloorHalfSizeX = 10.0f;
	float m_FloorHalfSizeY = 0.5f;
	float m_FloorHalfSizeZ = 100.0f;

	float m_TileSize = 4.0f;
	float m_TileHalfSizeY = 0.05f;

	std::string m_TileTexturePath = "asset\\texture\\cyberGroundRain.png";

	// 床を一定枚数だけ作成し、後方へ流れたものを前方へ移動して再利用する。
	bool m_UseCollision = true;

	float m_BlockHalfSizeX = 10.0f;
	float m_BlockHalfSizeY = 0.05f;
	float m_BlockHalfSizeZ = 10.0f;
	float m_BlockFullSizeZ = 20.0f;

	float m_NextSpawnZ = 0.0f;

	float m_GenerateAheadDistance = 220.0f;
	float m_DestroyBehindDistance = 140.0f;

	static constexpr int INITIAL_BLOCK_COUNT = 50;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetGroundData(const GroundLoadData& groundData);

	void CreateFloorTiles(
		Scene* scene,
		StageBlockManager* stageBlockManager,
		bool useCollision
	);

	void SetDanceFloorColorPattern(const std::vector<int>& colorPattern);

	void DestroyBlocksBehindZ(float deleteZ);

	void RebuildPassedGroundBlocks();

private:
	void AddDanceFloorBlock(StageBlock* stageBlock);

	XMFLOAT4 GetDanceFloorColorByNumber(int colorNumber);

	void ApplyDanceFloorColor(StageBlock* stageBlock);
	void ApplyDanceFloorColors();

	void CreateFloorTile(
		Scene* scene,
		StageBlockManager* stageBlockManager,
		float positionZ
	);

	void RecyclePassedFloorTiles(
		float playerZ,
		StageBlockManager* stageBlockManager
	);
};
