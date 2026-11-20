#pragma once

#include "UIQuadBase.h"

#include <array>

class StageBlockManager;
struct GroundLoadData;

class StageOuterFloorAndWalls : public UIQuadBase
{
private:
	static constexpr int TileCount = 13;  // カメラの前後で使い回す外周床の枚数
	static constexpr float TileLength = 192.0f;
	static constexpr float FloorWidth = 2304.0f;
	static constexpr float TextureRepeatLength = 16.0f;  // 床模様を1回繰り返すワールド上の長さ
	static constexpr float WallHeight = 5.0f;

	std::array<float, TileCount> m_TileCenters{};
	int m_CameraTile = 0;  // 最後に床を並べたときのカメラの区画番号
	bool m_HasPlacedTiles = false;

	float m_FloorCenterX = 25.0f;
	float m_FloorY = 0.61f;
	float m_LeftWallX = 15.0f;
	float m_RightWallX = 35.0f;

	StageBlockManager* m_Stage = nullptr;
	ID3D11RasterizerState* m_TwoSidedRasterizer = nullptr;

	// カメラの前後に外周の床タイルを並べ直す
	void RecycleTiles(float cameraZ);
	void DrawFloor();
	void DrawSideWalls(float cameraX);
	void DrawWall(float x, float minZ, float maxZ);

public:
	void Init() override;
	void Uninit() override;
	void Draw() override;

	void SetLayout(const GroundLoadData& ground, StageBlockManager& stage);
};
