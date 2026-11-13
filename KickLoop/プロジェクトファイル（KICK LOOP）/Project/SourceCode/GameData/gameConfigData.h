#pragma once

#include <string>
#include <vector>

#include "main.h"

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
};