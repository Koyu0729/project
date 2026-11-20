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

	// ゴール後、自動カメラが動き出すまでの待ち時間（5秒）。
	int m_GoalCameraStartFrame = 5 * GAME_FPS;

	bool m_IsGoalCameraStarted = false;

private:
	// Scene上の演出用オブジェクトを取得する。
	SpeedLineEffect* GetSpeedLineEffect();
	ParticleEffectManager* GetParticleEffectManager();

	// ゴールしたプレイヤーの周囲に花火を円形に打ち上げる
	void SpawnGoalFireworks(Vector3 playerPosition);
	void StartGoalCameraSequence();

public:
	// 画面のスピードラインを止める
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
	void EmitWallKickSpark(Vector3 position, Vector3 wallNormal);

	// ゴール演出を開始し、最初の花火を出す
	void StartGoalEffect(Vector3 playerPosition);
	// 花火を繰り返し打ち上げ、待ち時間が過ぎたらゴールカメラを動かす
	void UpdateGoalEffect(Vector3 playerPosition);

};
