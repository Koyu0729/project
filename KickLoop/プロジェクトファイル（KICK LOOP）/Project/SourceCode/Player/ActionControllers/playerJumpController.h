#pragma once

#include "main.h"

#include "playerRuntimeState.h"
#include "playerJumpParameter.h"
#include "playerActionRequest.h"

class PlayerSoundController;

// 通常ジャンプの入力受付とジャンプバッファを管理するクラス。
class PlayerJumpController
{
private:
	// ジャンプ入力を数フレーム保持し、接地直後でもジャンプできるようにする。
	int m_JumpBufferFrame = 0;

public:
	void BuildAction(
		PlayerRuntimeState& state,
		PlayerJumpParameter& jumpParameter, 
		PlayerSoundController& soundController, 
		PlayerActionRequest& action
	);

private:
	// ジャンプの先行入力を記録し、受付の残り時間を更新する
	void UpdateJumpBuffer(
		PlayerRuntimeState& state, 
		PlayerJumpParameter& jumpParameter
	);

	// 接地中かつジャンプバッファが残っている場合、縦速度変更の要求を作る。
	void BuildJumpInputAction(
		PlayerRuntimeState& state,
		PlayerJumpParameter& jumpParameter,
		PlayerSoundController& soundController,
		PlayerActionRequest& action
	);
};
