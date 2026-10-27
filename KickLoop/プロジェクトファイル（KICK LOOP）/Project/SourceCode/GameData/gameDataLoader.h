#pragma once

#include <string>

#include "gameConfigData.h"

class GameDataLoader
{
public:
	static bool SaveStageData(const std::string& path, const GameLoadData& data);
	static bool LoadStageData(const std::string& path, GameLoadData& outData);

	// 初期ステージの設定データを作る
	static GameLoadData CreateDefaultGameLoadData();
};