#pragma once

#include "backgroundObject.h"

class StageBlock;

// プレイヤーの進行距離に合わせて位置と大きさを変えるゴール用背景オブジェクト。
class GoalBuilding : public BackgroundObject
{
private:
	// 基準位置
	Vector3 m_BasePosition = Vector3(25.0f, 0.0f, 500.0f);

	// 位置オフセット
	float m_PositionOffsetRate = 1.0f;

	// スケール
	float m_StartScale = 8.0f;
	float m_ScalePerDistance = 0.01f;
	float m_MaxScale = 32.0f;

	// 位置ロック
	// FinalJumpPadRoom生成後、位置とスケール更新を止める。
	bool m_IsLocked = false;

	// ゴール床
	// GoalLaunchの着地点と、ゴール判定用の頂上StageBlock。
	float m_ModelTopLocalY = 12.0f;

	StageBlock* m_TopStageBlock = nullptr;

	Vector3 m_TopStageBlockSize = Vector3(10.0f, 1.0f, 10.0f);

public:
	void Init() override;
	void Update() override;

	void SetBasePosition(Vector3 position);
	void SetPositionOffsetRate(float rate);

	void SetStartScale(float scale);
	void SetScalePerDistance(float scalePerDistance);
	void SetMaxScale(float maxScale);

	void SetModelTopLocalY(float localY);

	// FinalJumpPadRoom生成時に呼び、GoalBuildingを現在位置で固定する。
	void LockCurrentPosition();

	bool IsLocked() const;

	// JumpPadのGoalLaunch飛び先。
	Vector3 GetTopPosition() const;

private:
	void UpdateByPlayerProgress();

	void CreateTopStageBlock();
};
