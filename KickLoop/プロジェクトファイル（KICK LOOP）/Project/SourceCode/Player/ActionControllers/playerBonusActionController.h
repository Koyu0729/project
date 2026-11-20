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

	int m_Frame = 0;  // ダッシュの残りフレーム数

	// ダッシュ中に毎フレーム加える移動量
	float m_DashMoveX = 0.0f;        // X（左右）
	float m_DashForwardMove = 0.0f;  // Z（前後）

public:
	void Init();

	// 入力とゲージ残量から、ダッシュと空中ジャンプの移動を要求する
	void BuildAction(
		PlayerRuntimeState& state, 
		PlayerBonusActionParameter& parameter, 
		PlayerRunController& runController,
		PlayerSoundController& soundController,
		PlayerActionRequest& action
	);


private:
	// ゲージを消費して視線方向へのダッシュを開始する
	void StartViewDash(
		PlayerBonusActionParameter& parameter,
		PlayerRunController& runController,
		PlayerSoundController& soundController
	);

	void UpdateViewDash(
		PlayerBonusActionParameter& parameter, 
		PlayerActionRequest& action
	);

	// 空中ジャンプが可能ならゲージを消費して上向きの速度を要求する
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
