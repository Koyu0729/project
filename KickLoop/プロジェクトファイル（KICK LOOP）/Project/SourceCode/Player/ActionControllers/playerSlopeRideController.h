#pragma once

#include "main.h"

#include "playerRuntimeState.h"
#include "playerMoveParameter.h"
#include "playerSlopeRideParameter.h"
#include "playerActionRequest.h"

enum class PlayerSlideState
{
	None,
	Sliding,
	Cooldown,
};

class PlayerSoundController;

// 坂やしゃがみ入力を使ったスライディングを管理するクラス。
class PlayerSlopeRideController
{
private:
	PlayerSlopeRideParameter m_SlopeRideParameter;

	PlayerSlideState m_SlideState = PlayerSlideState::None;

	int m_SlideFrame = 0;
	int m_SlideCooldownFrame = 0;

	float m_SlideSpeed = 0.0f;

	Vector3 m_SlideDirection = Vector3(0.0f, 0.0f, 1.0f);

private:
	// 次のスライドが可能になるまでの残り時間を更新する
	void UpdateCooldown();

	bool CanStartSlide(
		Vector3 position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter
	);

	void StartSlide(
		Vector3 position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerSoundController* soundController
	);

	void UpdateSlide(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerActionRequest& action
	);

	void EndSlide();

	void CancelSlide(
		PlayerRuntimeState& state
	);

	void ResetSlide();

	// 左右と前後の速度から水平速度の大きさを求める
	float GetHorizontalSpeed(PlayerRuntimeState& state);

	Vector3 GetHorizontalVelocityDirection(PlayerRuntimeState& state);

	Vector3 GetInputDirection(PlayerActionRequest& action);

	void ApplySteer(
		PlayerRuntimeState& state,
		PlayerActionRequest& action
	);

	// 滑る向きと坂の傾きに応じて、下りで加速し上りで減速する
	void ApplySlope(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter
	);

	// 足元の斜面から下り方向と傾きの強さを求める
	bool GetSlopeDownInfo(
		Vector3 position,
		PlayerMoveParameter& moveParameter,
		Vector3& outDirection,
		float& outSlopeRate
	);

	void LimitSlideSpeed();

	Vector3 NormalizeXZ(Vector3 vector);

public:
	void Reset(PlayerRuntimeState& state);

	// しゃがみ入力と接地状態に応じてスライドの開始・移動・終了を処理する
	void Apply(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerActionRequest& action,
		PlayerSoundController* soundController = nullptr
	);
};
