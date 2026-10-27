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
	PlayerSlideState m_SlideState = PlayerSlideState::None;

	int m_SlideFrame = 0;
	int m_SlideCooldownFrame = 0;

	float m_SlideSpeed = 0.0f;

	Vector3 m_SlideDirection =
		Vector3(
			0.0f,
			0.0f,
			1.0f
		);

private:
	// スライド開始、終了、キャンセル、クールダウンを管理する。
	void UpdateCooldown();

	bool CanStartSlide(
		Vector3 position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerSlopeRideParameter& slopeRideParameter
	);

	void StartSlide(
		Vector3 position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerSlopeRideParameter& slopeRideParameter,
		PlayerSoundController* soundController
	);

	void UpdateSlide(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerSlopeRideParameter& slopeRideParameter,
		PlayerActionRequest& action
	);

	void EndSlide(PlayerSlopeRideParameter& slopeRideParameter);

	void CancelSlide(
		PlayerRuntimeState& state,
		PlayerSlopeRideParameter& slopeRideParameter
	);

	void ResetSlide();

	// 現在の移動速度や入力方向から、スライドの進行方向を決める。
	float GetHorizontalSpeed(PlayerRuntimeState& state);

	Vector3 GetHorizontalVelocityDirection(PlayerRuntimeState& state);

	Vector3 GetInputDirection(PlayerActionRequest& action);

	void ApplySteer(
		PlayerRuntimeState& state,
		PlayerSlopeRideParameter& slopeRideParameter,
		PlayerActionRequest& action
	);

	// 坂の法線から下り方向と傾斜の強さを計算し、スライド速度へ反映する。
	void ApplySlope(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerSlopeRideParameter& slopeRideParameter
	);

	bool GetSlopeDownInfo(
		Vector3 position,
		PlayerMoveParameter& moveParameter,
		PlayerSlopeRideParameter& slopeRideParameter,
		Vector3& outDirection,
		float& outSlopeRate
	);

	void LimitSlideSpeed(PlayerSlopeRideParameter& slopeRideParameter);

	Vector3 NormalizeXZ(Vector3 vector);

public:
	void Apply(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerSlopeRideParameter& slopeRideParameter,
		PlayerActionRequest& action,
		PlayerSoundController* soundController = nullptr
	);
};
