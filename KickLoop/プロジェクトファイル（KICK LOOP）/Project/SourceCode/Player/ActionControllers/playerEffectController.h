#pragma once

#include "main.h"

class SpeedLineEffect;
class ParticleEffectManager;

// プレイヤーに紐づく画面演出とパーティクル演出をまとめて扱うクラス。
class PlayerEffectController
{
private:
	bool m_IsGoalEffect = false;

	int m_GoalEffectFrame = 0;

	// 60fps想定。30f = 0.5秒。
	int m_GoalFireworkInterval = 30;

	// 60fps想定。600f = 10秒。
	int m_GoalCameraStartFrame = 600;

	bool m_IsGoalCameraStarted = false;

private:
	// Scene上の演出用オブジェクトを取得する。
	SpeedLineEffect* GetSpeedLineEffect();
	ParticleEffectManager* GetParticleEffectManager();

	// ゴール後の花火発生とカメラ演出開始。
	void SpawnGoalFireworks(Vector3 playerPosition);
	void StartGoalCameraSequence();

public:
	// プレイヤー速度や壁スライド状態に合わせて、画面上の速度演出を制御する。
	void Disable(bool resetSpeedRate);

	void SetSpeedRate(float speedRate);


	void SetSpeedRateFromVelocity(
		Vector3 velocity,
		float appearSpeed,
		float maxSpeed
	);

	void SetSpeedRateFromVelocity(
		Vector3 velocity,
		float virtualForwardMove,
		float appearSpeed,
		float maxSpeed
	);


	void SetWallSlideEffect(
		bool isActive,
		float directionX
	);

	void PlayBoost();

	// 着地、壁キック、壁スライド。
	void EmitSpark(
		Vector3 position,
		int count
	);

	void EmitLandingSpark(Vector3 position);
	void EmitWallSlidSpark(Vector3 position);
	void EmitWallKickSpark(Vector3 position);

	// ゴール後の花火とカメラ演出の開始、更新、停止。
	void StartGoalEffect(Vector3 playerPosition);
	void UpdateGoalEffect(Vector3 playerPosition);

};
