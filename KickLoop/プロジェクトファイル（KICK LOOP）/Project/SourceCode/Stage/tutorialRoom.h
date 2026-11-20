#pragma once

#include "stageModuleData.h"

class JumpPad;
class Player;
class Scene;
class StageBlockManager;

class TutorialRoom
{
private:
	const StageModuleData* m_Data = nullptr;
	const JumpPad* m_JumpPad = nullptr;
	float m_EndZ = 0.0f;

public:
	// 部屋番号に対応するステージ配置から練習用の部屋と床を作る
	void Build(Scene& scene, StageBlockManager& stage, int roomIndex);

	StageRoomType GetType() const
	{
		return m_Data->roomType;
	}

	bool IsFinal() const
	{
		return GetType() == StageRoomType::FinalJumpPadRoom;
	}

	bool IsAtExit(const Player& player) const;
	bool HasLandedOnLastBlock(Player& player) const;
	bool IsJumpPadOnCooldown() const;
};
