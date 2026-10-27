#pragma once

#include <string>
#include <vector>

#include "main.h"

// 障害物用のプリセットデータ
struct ObstaclePresetData
{
	Vector3 size = Vector3(0.0f, 0.0f, 0.0f);

	float centerX = 0.0f;
	float centerY = 0.0f;

	std::string texturePath = "asset\\texture\\siro.png";
	
	XMFLOAT2 uvRepeat = XMFLOAT2(1.0f, 1.0f);

	bool isRainbow = false;
	float rainbowSpeed = 1.0f;
	float alpha = 1.0f;

	// 空文字ならコメントUIなし
	std::string commentTexturePath = "";

	Vector3 commentOffset = Vector3(0.0f, 0.0f, 0.0f);
};

// 地面のロードデータ
struct GroundLoadData
{
	Vector3 center = Vector3(25.0f, 0.0f, 23.0f);
	Vector3 halfSize = Vector3(10.0f, 0.5f, 100.0f);

	float tileSize = 4.0f;
	float tileHalfSizeY = 0.05f;

	std::string texturePath = "asset\\texture\\frameBox.png";

	// 床タイルの色番号パターン
	std::vector<int> colorPattern =
	{
		1, 2, 3, 4, 5,
		2, 3, 4, 5, 1,
		3, 4, 5, 1, 2,
		4, 5, 1, 2, 3,
		5, 1, 2, 3, 4
	};
};

// ゲーム全体のロードデータ
struct GameLoadData
{
	GroundLoadData ground;

	std::vector<std::string> texturePaths;

	// JSONの配列数で障害物種類を決める
	std::vector<ObstaclePresetData> floorObstaclePresets;
	std::vector<ObstaclePresetData> wallObstaclePresets;
};