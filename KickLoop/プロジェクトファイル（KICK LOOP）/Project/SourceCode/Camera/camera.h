#pragma once

#include "gameObject.h"
#include "cameraParameter.h"

class Player;

enum class CameraFOV
{
	Normal,
	Impact,
};

// Default : 一定高度でゴール付近まで進む。
// Last    : ゴール付近で上昇し、ゴール床を見る。
enum class GoalCameraMode
{
	Default,
	Last,
};

// ゴール演出用カメラの移動開始位置、終了位置、補間用パラメータ。
struct GoalCameraData
{
	bool isActive = false;

	GoalCameraMode mode = GoalCameraMode::Default;

	Vector3 startPosition = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 endPosition = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 goalBlockPosition = Vector3(25.0f, 180.0f, 5400.0f);

	Vector3 moveDirection = Vector3(0.0f, 0.0f, 1.0f);

	Vector3 lastStartPosition = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 lastEndPosition = Vector3(0.0f, 0.0f, 0.0f);

	float moveSpeed = 1.5f;
	float heightOffset = 30.0f;
	float lookAhead = 30.0f;
	float targetDownOffset = 8.0f;

	float lastStartDistance = 120.0f;
	float lastTopOffset = 35.0f;
	float lastBackDistance = 45.0f;

	float lastRate = 0.0f;
	float lastRateSpeed = 0.015f;
};

// FPS視点の通常カメラ、壁アクション演出、落下演出、ゴール演出用カメラ。
class Camera : public GameObject
{
private:
	XMMATRIX m_Projection;
	XMMATRIX m_View;

	Vector3 m_Target = Vector3(0.0f, 0.0f, 0.0f);

	CameraParameter m_Parameter;

	CameraFOV m_CameraFOV = CameraFOV::Normal;

	float m_Yaw = 0.0f;
	float m_Pitch = 0.0f;

	float m_BaseFOV = 1.771f;
	float m_FOV = 1.771f;
	float m_FOVImpact = 0.0f;

	float m_Roll = 0.0f;

	// 壁スライド中の傾き、横寄せ、壁キック時のフリーズ演出。
	float m_LastWallSlideRollSide = 0.0f;
	int m_WallSlideRollKeepFrame = 0;

	float m_WallSlidePositionOffset = -1.0f;
	float m_WallSlidePositionOffsetPower = -1.0f;
	float m_WallSlidePositionOffsetLerpRate = 0.15f;

	float m_WallKickPitchImpact = 0.0f;
	float m_WallKickRollImpact = 0.0f;

	// 通常時としゃがみ滑走中の目線高さ。
	float m_CurrentEyeHeight = 1.2f;

	// 走行、壁スライド、落下、壁キック衝撃用のカメラ揺れ。
	Vector3 m_ShakeOffset = Vector3(0.0f, 0.0f, 0.0f);

	float m_RunShakeTime = 0.0f;
	float m_WallSlideShakeTime = 0.0f;

	float m_ImpactShakePower = 0.0f;

	float m_FallShakeTimer = 0.0f;
	float m_FallShakePower = 0.0f;

	// ゴール後に通常操作カメラを止め、専用カメラシーケンスを再生する。
	GoalCameraData m_GoalCamera;

	bool m_IsGoalCameraFinished = false;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void PlayWallKickFOVImpact();

	Vector3 GetTarget()
	{
		return m_Target;
	}

	XMMATRIX GetProjectionMatrix()
	{
		return m_Projection;
	}

	XMMATRIX GetViewMatrix()
	{
		return m_View;
	}

	bool IsGoalCameraFinished() const
	{
		return m_IsGoalCameraFinished;
	}

	void SetYawPitch(
		float yaw,
		float pitch
	);

	void SetYaw(float yaw);
	void SetPitch(float pitch);

	void StartGoalCameraSequence(
		Vector3 playerStartPosition,
		Vector3 goalPosition,
		Vector3 goalBlockPosition
	);

private:
	float LerpFloat(
		float current,
		float target,
		float rate
	);

	float ClampFloat(
		float value,
		float minValue,
		float maxValue
	);

	float SignFloat(float value);

	void ClampPitch();

	void UpdateFallCamera(Player* player);
	void UpdateWallKickPitchImpact();
	void UpdateFOV(Player* player);
	void UpdateCrouchEyeHeight(Player* player);

	Vector3 BuildForward(float pitch);
	Vector3 BuildCameraRight(Vector3 forward);

	void SyncGameObjectRotation(float pitch);

	void UpdateWallSlidePositionOffset(
		Player* player,
		Vector3 forward,
		Vector3& basePosition
	);

	void UpdateWallSlideRoll(
		Player* player,
		Vector3 forward
	);

	void UpdateCameraShake(
		Player* player,
		Vector3 forward
	);

	bool IsCameraHitBlockByPlayerHit(
		Vector3 position,
		Vector3 playerHitSize
	);

	void UpdateGoalCameraSequence();
	void UpdateGoalCameraDefault();
	void UpdateGoalCameraLast();
};
