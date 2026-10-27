#pragma once

#include "main.h"

class PlayerEffectController;
class PlayerSoundController;

struct PlayerRuntimeState;
struct PlayerMoveParameter;
struct PlayerActionRequest;

// 入力とカメラ方向から、プレイヤーの通常移動要求を作るクラス。
class PlayerMoveController
{
private:
	// 壁スライド開始音と火花演出の連続発生を抑えるための状態。
	bool m_PrevWallSliding = false;
	int m_WallSlideSparkTimer = 0;

public:
	void BuildAction(
		Vector3 playerPosition,
		Vector3& rotation,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerEffectController& effectController,
		PlayerSoundController& soundController,
		PlayerActionRequest& action);

private:
	// WASD入力を取得し、斜め移動で速度が増えないように正規化する。
	void GetMoveInput(
		float& inputX,
		float& inputZ,
		float& inputLength
	);

	// 入力方向をカメラ基準の移動方向へ変換する。
	Vector3 GetCameraMoveDirection(
		float inputX,
		float inputZ,
		Vector3& rotation,
		PlayerEffectController& effectController
	);
};
