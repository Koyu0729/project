#pragma once

#include "main.h"

#include "playerRuntimeState.h"
#include "playerDiveParameter.h"
#include "playerActionRequest.h"

enum class PlayerAirDiveState
{
	None,
	FloatUp,
	DiveDown,
};

// 空中急降下の開始判定と、急降下中の状態遷移を管理するクラス
class PlayerAirDiveController
{
private:
	// FloatUpで一度浮かせてからDiveDownへ移行する。
	PlayerAirDiveState m_State = PlayerAirDiveState::None;
	int m_Frame = 0;  // 急降下に移るまで浮き上がりを続ける残りフレーム数
	bool m_CanDive = true;

public:
	void Init();

	// 急降下の段階に合わせて、浮き上がりと落下の速度を要求する
	void BuildAction(
		PlayerRuntimeState& state,
		PlayerDiveParameter& parameter,
		PlayerActionRequest& action
	);

private:
	// 急降下前の浮き上がりを開始する
	void StartFloatUp(
		PlayerDiveParameter& parameter,
		PlayerActionRequest& action
	);

	void UpdateFloatUp(
		PlayerDiveParameter& parameter,
		PlayerActionRequest& action
	);

	void UpdateDiveDown(
		PlayerDiveParameter& parameter,
		PlayerActionRequest& action
	);
};
