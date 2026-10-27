#pragma once

#include <vector>
#include <string>

#include "main.h"
#include "gameObject.h"

#include "stageModuleManager.h"

class Scene;
class StageBlock;
class StageMaskBlock;
class Polygon3D;
class LaserWall;
class JumpPad;
class TimeCoin;

struct ObstaclePresetData;

// 床や壁に生成する障害物の見た目と配置情報。
struct ObstaclePreset
{
	Vector3 size = Vector3(0.0f, 0.0f, 0.0f);

	float centerX = 0.0f;
	float centerY = 0.0f;

	std::string texturePath = "asset\\texture\\siro.png";

	XMFLOAT2 uvRepeat = XMFLOAT2(1.0f, 1.0f);

	bool isRainbow = false;
	float rainbowSpeed = 1.0f;
	float alpha = 1.0f;
};

// 一定距離を越えたら削除するStageBlockの記録。
// 通常の進行削除とは別に、個別タイミングで消したい床や足場に使う。
struct DistanceDeleteStageBlock
{
	StageBlock* stageBlock = nullptr;
	float deleteDistance = 0.0f;
	bool isDeleted = false;
};

// 生成済みRoomのZ範囲。
// プレイヤーが通過したRoomを判定し、Z戻り防止用のClamp更新に使う。
struct StageRoomProgressArea
{
	float startZ = 0.0f;
	float endZ = 0.0f;
};

// 生成済みRoomのIDと出現距離。
// リザルトやステージ再構築で、どのRoomを通ったか復元するために使う。
struct StageRoomRecord
{
	int id = -1;
	float distance = 0.0f;
	bool isGoalCameraRebuilt = false;
};

// 取得済みTimeCoinの記録。
struct CollectedTimeCoinRecord
{
	Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 size = Vector3(0.7f, 0.7f, 0.7f);

	float addTime = 0.0f;
};

// ステージ上のブロック、障害物、レーザー、ジャンプ台、TimeCoinを管理するクラス。
class StageBlockManager : public GameObject
{
private:
	// 現在ステージ上で有効なブロックと障害物。
	std::vector<StageBlock*> m_StageBlocks;
	std::vector<StageBlock*> m_Obstacles;

	// ステージ中で頻繁に生成・削除されるオブジェクトを再利用する。
	bool m_IsPoolPreCreated = false;

	int m_InitialObstaclePoolCount = 50;
	std::vector<StageMaskBlock*> m_ObstaclePool;

	std::vector<LaserWall*> m_LaserWalls;
	std::vector<LaserWall*> m_LaserWallPool;

	std::vector<JumpPad*> m_JumpPads;
	std::vector<JumpPad*> m_JumpPadPool;

	int m_InitialTimeCoinPoolCount = 10;
	std::vector<TimeCoin*> m_TimeCoins;
	std::vector<TimeCoin*> m_TimeCoinPool;

	// 進行中に発生したステージ情報。
	// リザルトや再構築で同じ状態を復元するために保持する。
	std::vector<CollectedTimeCoinRecord> m_CollectedTimeCoinRecords;
	std::vector<StageRoomRecord> m_RoomRecords;

	std::vector<Polygon3D*> m_CommentUIs;
	std::vector<DistanceDeleteStageBlock> m_DistanceDeleteStageBlocks;

	// Room通過判定用のZ範囲。
	// オブジェクトをRoom単位で持たず、Clamp更新タイミングだけ管理する。
	std::vector<StageRoomProgressArea> m_RoomProgressAreas;

	// 床と壁にランダム生成する障害物の候補。
	std::vector<ObstaclePreset> m_FloorObstaclePresets;
	std::vector<ObstaclePreset> m_WallObstaclePresets;

	// Roomデータを保持し、進行距離に応じて次のRoomを生成する。
	StageModuleManager m_StageModuleManager;

	// 障害物、壁、Roomを生成する間隔と進行距離。
	float m_CommentVisibleDistance = 35.0f;

	int m_FloorSpawnCount = 1;
	int m_WallSpawnCount = 1;

	float m_NextFloorSpawnDistance = 40.0f;
	float m_NextWallSpawnDistance = 20.0f;

	float m_FloorSpawnInterval = 40.0f;
	float m_WallSpawnInterval = 40.0f;

	float m_NextStageModuleSpawnDistance = 40.0f;
	float m_StageModuleSpawnInterval = 40.0f;

	float m_SpawnZ = 90.0f;
	float m_PlayerBaseZ = 13.0f;

	// プレイヤーより後ろになったオブジェクトを消す位置。
	float m_DestroyObstacleZ = -160.0f;

	// プレイヤーが移動できるX範囲。
	float m_DefaultWalkMinX = 15.0f;
	float m_DefaultWalkMaxX = 35.0f;

	// プレイヤーが戻れない最小Z座標。
	float m_StartClampBackOffset = 10.0f;
	float m_MinClampZ = 0.0f;

	// Room終端をどれだけ越えたら、そのRoomを通過済みにするか。
	float m_RoomClampPassedOffset = 0.0f;

	// FinalJumpPadRoomの終端Z。
	float m_FinalClampZ = 0.0f;
	bool m_HasFinalClampZ = false;

	float m_StageGenerateEndDistance = 5000.0f;
	bool m_IsFinalStageModuleSpawned = false;

	// ゴール演出用、Z制限を解除。
	bool m_IsGoalZClampReleased = false;

private:
	void SpawnFloorObstacle();
	void SpawnWallObstacle();

	void SpawnObstacleFromPreset(const ObstaclePreset& preset);

	void UpdateStageModuleSpawn();

	void AddRoomRecord(
		int id,
		float distance
	);

	void SpawnStageModule(
		const StageModuleData& module,
		bool isFinalModule,
		int moduleID
	);

	void SpawnStageModuleBlockAtDistance(
		const StageModuleObjectData& objectData,
		float spawnDistance
	);

	void SpawnStageModuleObject(const StageModuleObjectData& objectData);

	void SpawnStageModuleLaserWall(const StageModuleObjectData& objectData);
	void SpawnStageModuleJumpPad(const StageModuleObjectData& objectData);
	void SpawnStageModuleTimeCoin(const StageModuleObjectData& objectData);
	void SpawnStageModuleCommentUI(const StageModuleObjectData& objectData);

	void DestroyPassedObstacles();
	void DestroyPassedLaserWalls();
	void DestroyPassedJumpPads();
	void DestroyPassedTimeCoins();
	void DestroyPassedCommentUIs();

	void ClearLaserWalls();
	void ClearJumpPads();
	void ClearTimeCoins();
	void ClearCommentUIs();

	ObstaclePreset ConvertObstaclePresetData(const ObstaclePresetData& presetData);

	void UpdateCommentVisible();
	void UpdateDistanceDeleteStageBlocks();

	float GetProgressDistance();

	void AddRoomProgressArea(
		float roomStartZ,
		float roomEndZ
	);

	void DeletePassedRoomProgressAreas(float playerZ);

	void DestroyObjectsBehindMinClampZ();

	void PreCreateObjectPools();

	StageMaskBlock* GetObstacleFromPool();
	void ReturnObstacleToPool(StageMaskBlock* obstacle);

	LaserWall* GetLaserWallFromPool();
	void ReturnLaserWallToPool(LaserWall* laserWall);

	JumpPad* GetJumpPadFromPool();
	void ReturnJumpPadToPool(JumpPad* jumpPad);

	TimeCoin* GetTimeCoinFromPool();
	void ReturnTimeCoinToPool(TimeCoin* timeCoin);

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void AddStageBlock(StageBlock* stageBlock);

	void RemoveStageBlock(StageBlock* stageBlock);

	bool IsHitAnyStageBlock(
		Vector3 position,
		Vector3 size
	);

	StageBlock* GetHitStageBlock(
		Vector3 position,
		Vector3 size
	);

	bool IsHitAnyObstacleExcept(
		StageBlock* ignoreBlock,
		Vector3 position,
		Vector3 size
	);

	bool IsHitAnyLaser(
		Vector3 position,
		Vector3 size
	);

	void SpawnObstacle();

	void SetFloorSpawnCountAtSamePoint(int count);
	void SetWallSpawnCountAtSamePoint(int count);

	void SetObstaclePresets(
		const std::vector<ObstaclePresetData>& floorObstaclePresets,
		const std::vector<ObstaclePresetData>& wallObstaclePresets
	);

	void AddDistanceDeleteStageBlock(
		StageBlock* stageBlock,
		float deleteDistance
	);

	// 現在のX範囲はZに依存しないため、playerZは未使用。
	bool GetWalkableXRange(
		float playerZ,
		float playerHalfSizeX,
		float& outMinX,
		float& outMaxX
	);

	bool GetStageZClampRange(
		float playerHalfSizeZ,
		float& outMinZ,
		float& outMaxZ
	);

	void SetStageGenerateEndDistance(float distance);
	float GetStageGenerateEndDistance() const;
	float GetStageGenerateEndWorldZ() const;

	bool IsFinalStageModuleSpawned() const;

	void ReleaseGoalZClamp();
	bool IsGoalZClampReleased() const;

	void AddCollectedTimeCoinRecord(
		Vector3 position,
		Vector3 size,
		float addTime
	);

	void RebuildPassedStageBlocks(float cameraZ);

	void UpdateGoalCameraStage(float cameraZ);

	const std::vector<StageRoomRecord>& GetRoomRecords() const
	{
		return m_RoomRecords;
	}

	const std::vector<CollectedTimeCoinRecord>& GetCollectedTimeCoinRecords() const
	{
		return m_CollectedTimeCoinRecords;
	}
};