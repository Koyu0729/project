#pragma once

#include <vector>

#include "stageModuleData.h"

// StageModuleDataを作成・保持するクラス。
// 各ルームのブロック、レーザー、ジャンプ台などの配置データを作り、
// StageBlockManagerが進行距離に応じて生成できるようにする。
class StageModuleManager
{
private:
	std::vector<StageModuleData> m_Modules;

	StageModuleData m_FinalJumpPadRoom;

public:
	void InitDefaultModules();

	const std::vector<StageModuleData>& GetModules() const;

	const StageModuleData* GetModule(int index) const;

	const StageModuleData* GetRandomModule() const;

	const StageModuleData* GetFinalJumpPadRoom() const;

private:
	// ルームごとの配置データを作成する。
	StageModuleData CreateJumpOnlyRoom();

	StageModuleData CreateZigzagRoom();

	StageModuleData CreateWallKickWaveRoom();
	StageModuleData CreateWallKickRoom();

	StageModuleData CreateLaserWallSlideRoom();

	StageModuleData CreateWallSlideRoom();

	StageModuleData CreateJumpPadConnectRoom();

	StageModuleData CreateFinalJumpPadRoom();
};