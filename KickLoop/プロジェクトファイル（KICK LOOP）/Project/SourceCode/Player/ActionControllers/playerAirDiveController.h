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
	int m_Frame = 0;
	bool m_CanDive = true;

public:
	void Init();

	void BuildAction(
		PlayerRuntimeState& state,
		PlayerDiveParameter& parameter,
		PlayerActionRequest& action
	);

private:
	// 空中急降下に必要な浮き上がり要求と急降下要求を作る。
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
