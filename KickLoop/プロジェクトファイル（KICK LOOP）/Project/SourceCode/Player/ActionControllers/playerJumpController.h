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
	// 空中で壁に触れている間と、壁を離れた直後は壁キックを優先する。
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
