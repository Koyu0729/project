#pragma once

#include "main.h"

#include "playerRuntimeState.h"
#include "playerBonusActionParameter.h"
#include "playerActionRequest.h"

class PlayerRunController;
class PlayerSoundController;

enum class PlayerBonusActionMode
{
	None,
	ViewDash,
};

// ボーナスゲージを消費する追加アクションを管理するクラス。
// 通常移動とは別に、視点方向ダッシュや空中ジャンプの要求をPlayerActionRequestへ追加する。
class PlayerBonusActionController
{
private:
	PlayerBonusActionMode m_Mode = PlayerBonusActionMode::None;

	int m_Frame = 0;

	float m_DashMoveX = 0.0f;
	float m_DashForwardMove = 0.0f;

public:
	void Init();

	void BuildAction(
		PlayerRuntimeState& state, 
		PlayerBonusActionParameter& parameter, 
		PlayerRunController& runController,
		PlayerSoundController& soundController,
		PlayerActionRequest& action
	);


private:
	// カメラの向いている方向へ一定フレーム加速する。
	// 移動量は直接座標へ反映せず、PlayerActionRequestへ追加移動として渡す。
	void StartViewDash(
		PlayerBonusActionParameter& parameter,
		PlayerRunController& runController,
		PlayerSoundController& soundController
	);

	void UpdateViewDash(
		PlayerBonusActionParameter& parameter, 
		PlayerActionRequest& action
	);

	// 空中でゲージを消費して追加ジャンプする。
	// 壁接触中や壁キック猶予中は壁キックを優先するため、通常の空中ジャンプは行わない。
	void TryAirJump(
		PlayerRuntimeState& state, 
		PlayerBonusActionParameter& parameter, 
		PlayerRunController& runController,
		PlayerSoundController& soundController, 
		PlayerActionRequest& action
	);

	// カメラの前方向をXZ平面に正規化して取得する。
	Vector3 GetCameraForwardXZ();

	void End();
};
