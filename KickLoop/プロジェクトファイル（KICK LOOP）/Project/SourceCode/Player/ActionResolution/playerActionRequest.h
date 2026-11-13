#pragma once

#include "main.h"
#include "playerMoveParameter.h"

#include <cmath>
#include <string>

// 入力と外部効果から集める、そのフレームの移動要求。
// Resolverが合成し、接触状態やアクション固有の状態は各Controllerでも更新する。

enum class PlayerActionMoveMode
{
	None,
	Instant,
	Decay,
	Timed,
};

enum class PlayerActionEffectType
{
	None,
	JumpPad,
	LaserSlow,
	GameOver,
};

struct PlayerActionRequest
{
	// そのフレームで追加する移動量。
	// ブリンクやスライドなど、通常移動とは別に加える速度を入れる。
	Vector3 moveVelocity = Vector3(0.0f, 0.0f, 0.0f);

	// ランゲーム用の前進量。
	// 通常の速度とは別に、Z方向へ足す移動量として扱う。
	float forwardMove = 0.0f;

	// 正規化前の移動入力の長さ。0.0fなら無入力。
	// 壁滑りの自動前進中は1.0fを使う。
	float inputLength = 0.0f;

	// 入力やカメラ方向から計算した、目標水平速度。
	bool useHorizontalTargetVelocity = false;
	Vector3 horizontalTargetVelocity = Vector3(0.0f, 0.0f, 0.0f);

	// 走行速度をこのフレーム用に上書きする。
	bool useResolvedMoveSpeed = false;
	float resolvedMoveSpeed = 0.0f;

	// 旧レーザー減速用の要求。現在の移動処理では未使用。
	bool useLaserSlowPenalty = false;
	float laserSlowPenalty = 0.0f;

	// ジャンプ、壁キック、ジャンプ台などで使う縦方向速度。
	bool useVerticalVelocity = false;
	float verticalVelocity = 0.0f;

	// 重力倍率。
	// ブリンク中やジャンプ台で浮いている間に重力を変更する。
	bool useGravityRate = false;
	float gravityRate = 1.0f;

	// 最大落下速度の上書き。
	bool useMaxFallSpeed = false;
	float maxFallSpeed = -999.0f;

	// 壁キック開始時に追加する専用の水平速度。
	bool useStartWallKickVelocity = false;
	Vector3 startWallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);

	// 空中急降下の前段階として、少し浮かせる要求。
	bool requestAirFloat = false;

	// 空中急降下要求。
	bool requestAirDive = false;

	float airFloatVelocity = 0.0f;
	float airDiveVelocity = 0.0f;
	float airDiveHorizontalBrakeRate = 1.0f;

	// 壁キック成功時の状態変更要求。
	bool requestWallKickSuccess = false;

	// 左クリックでのよじ登り要求。
	bool requestLedgeClimb = false;


	bool requestCrouchRide = false;
	bool requestSlideStart = false;
	bool isCrouchHold = false;

	// 一瞬停止演出。
	// 壁キックなどで操作感にメリハリを出すために使う。
	bool requestFreeze = false;
	int freezeFrame = 0;

	// スピードラインなどのブースト演出要求。
	bool requestBoostEffect = false;

	// ゲームオーバー要求。
	bool requestGameOver = false;

	void Clear()
	{
		moveVelocity = Vector3(0.0f, 0.0f, 0.0f);

		forwardMove = 0.0f;

		inputLength = 0.0f;

		useHorizontalTargetVelocity = false;
		horizontalTargetVelocity = Vector3(0.0f, 0.0f, 0.0f);

		useResolvedMoveSpeed = false;
		resolvedMoveSpeed = 0.0f;

		useLaserSlowPenalty = false;
		laserSlowPenalty = 0.0f;

		useVerticalVelocity = false;
		verticalVelocity = 0.0f;

		useGravityRate = false;
		gravityRate = 1.0f;

		useMaxFallSpeed = false;
		maxFallSpeed = -999.0f;

		useStartWallKickVelocity = false;
		startWallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);

		requestAirFloat = false;
		requestAirDive = false;

		airFloatVelocity = 0.0f;
		airDiveVelocity = 0.0f;
		airDiveHorizontalBrakeRate = 1.0f;

		requestWallKickSuccess = false;

		requestLedgeClimb = false;

		requestCrouchRide = false;
		requestSlideStart = false;
		isCrouchHold = false;

		requestFreeze = false;
		freezeFrame = 0;

		requestBoostEffect = false;

		requestGameOver = false;
	}

	void AddMoveVelocity(Vector3 velocity)
	{
		moveVelocity.x += velocity.x;
		moveVelocity.y += velocity.y;
		moveVelocity.z += velocity.z;
	}

	void AddForwardMove(float move)
	{
		forwardMove += move;
	}

	void SetInputLength(float value)
	{
		inputLength = value;
	}

	void SetHorizontalTargetVelocity(Vector3 velocity)
	{
		useHorizontalTargetVelocity = true;
		horizontalTargetVelocity = velocity;
	}

	void SetResolvedMoveSpeed(float speed)
	{
		useResolvedMoveSpeed = true;
		resolvedMoveSpeed = speed;
	}

	float GetMoveSpeed(PlayerMoveParameter& moveParameter)
	{
		if (useResolvedMoveSpeed)
		{
			return resolvedMoveSpeed;
		}

		return moveParameter.moveSpeed;
	}

	void SetLaserSlowPenalty(float penalty)
	{
		useLaserSlowPenalty = true;

		if (penalty < 0.0f)
		{
			penalty = 0.0f;
		}

		// 減速要求の最大値を保持する。
		if (penalty > laserSlowPenalty)
		{
			laserSlowPenalty = penalty;
		}
	}

	void SetVerticalVelocity(float velocity)
	{
		useVerticalVelocity = true;
		verticalVelocity = velocity;
	}

	void SetGravityRate(float rate)
	{
		useGravityRate = true;
		gravityRate = rate;

		if (gravityRate < 0.0f)
		{
			gravityRate = 0.0f;
		}
	}

	void SetMaxFallSpeed(float speed)
	{
		useMaxFallSpeed = true;
		maxFallSpeed = speed;
	}

	void SetStartWallKickVelocity(Vector3 velocity)
	{
		useStartWallKickVelocity = true;
		startWallKickVelocity = velocity;
	}

	void RequestAirFloat(float velocity, float horizontalBrakeRate)
	{
		requestAirFloat = true;

		airFloatVelocity = velocity;

		airDiveHorizontalBrakeRate = horizontalBrakeRate;

		if (airDiveHorizontalBrakeRate < 0.0f)
		{
			airDiveHorizontalBrakeRate = 0.0f;
		}

		if (airDiveHorizontalBrakeRate > 1.0f)
		{
			airDiveHorizontalBrakeRate = 1.0f;
		}
	}

	void RequestAirDive(float velocity, float horizontalBrakeRate)
	{
		requestAirDive = true;

		airDiveVelocity = velocity;

		airDiveHorizontalBrakeRate = horizontalBrakeRate;

		if (airDiveHorizontalBrakeRate < 0.0f)
		{
			airDiveHorizontalBrakeRate = 0.0f;
		}

		if (airDiveHorizontalBrakeRate > 1.0f)
		{
			airDiveHorizontalBrakeRate = 1.0f;
		}
	}

	void SetWallKickSuccess()
	{
		requestWallKickSuccess = true;
	}

	void RequestLedgeClimb()
	{
		requestLedgeClimb = true;
	}

	void RequestCrouchRide()
	{
		requestCrouchRide = true;
		isCrouchHold = true;
	}

	void RequestSlideStart()
	{
		requestSlideStart = true;
		requestCrouchRide = true;
	}

	void SetCrouchHold(bool isHold)
	{
		isCrouchHold = isHold;

		if (isCrouchHold)
		{
			requestCrouchRide = true;
		}
	}

	void RequestFreeze(int frame)
	{
		requestFreeze = true;
		freezeFrame = frame;

		if (freezeFrame < 0)
		{
			freezeFrame = 0;
		}
	}

	void RequestBoostEffect()
	{
		requestBoostEffect = true;
	}

	void RequestGameOver()
	{
		requestGameOver = true;
	}
};

// 外部ギミックの効果を保持し、適用中は毎フレームPlayerActionRequestに反映する。
// ジャンプ台の移動量と浮遊効果をここで管理する。
struct PlayerActionEffect
{
	PlayerActionEffectType effectType = PlayerActionEffectType::None;

	// 外部効果を識別するためのキー。現在は照合に使っていない。
	std::string effectKey = "";

	Vector3 moveVelocity = Vector3(0.0f, 0.0f, 0.0f);

	PlayerActionMoveMode moveMode = PlayerActionMoveMode::None;

	float moveDecay = 0.95f;
	float moveMin = 0.01f;

	int moveFrame = 0;

	// 開始時に一度だけ縦速度を与える。
	// ジャンプ台など、発動瞬間だけY速度を入れたい効果に使う。
	bool useStartVerticalVelocity = false;
	float startVerticalVelocity = 0.0f;
	bool hasAppliedStartVerticalVelocity = false;

	int floatFrame = 0;

	bool useGravityRate = false;
	float gravityRate = 1.0f;

	bool useMaxFallSpeed = false;
	float maxFallSpeed = -999.0f;

	bool requestBoostEffect = false;

	// 着地したら効果を終了するか。
	bool stopOnGround = false;
	bool wasGround = false;

	// 旧レーザー減速用の減速量と継続フレーム。
	float slowPenalty = 0.0f;
	int activeFrame = 0;

	bool requestGameOver = false;

	bool isActive = true;

	void Refresh(PlayerActionEffect effect)
	{
		if (effect.activeFrame > activeFrame)
		{
			activeFrame = effect.activeFrame;
		}

		if (effect.slowPenalty > slowPenalty)
		{
			slowPenalty = effect.slowPenalty;
		}

		if (effect.requestGameOver)
		{
			requestGameOver = true;
		}

		isActive = true;
	}

	void UpdateGroundEnd(bool isGround)
	{
		if (!stopOnGround)
		{
			wasGround = isGround;
			return;
		}

		// 開始時の縦速度をまだ適用していない場合は、着地終了判定を待つ。
		if (useStartVerticalVelocity && !hasAppliedStartVerticalVelocity)
		{
			wasGround = isGround;
			return;
		}

		if (isGround)
		{
			isActive = false;
			wasGround = isGround;
			return;
		}

		wasGround = isGround;
	}

	void ApplyStart(PlayerActionRequest& action)
	{
		if (!useStartVerticalVelocity)
		{
			return;
		}

		if (hasAppliedStartVerticalVelocity)
		{
			return;
		}

		action.SetVerticalVelocity(startVerticalVelocity);

		hasAppliedStartVerticalVelocity = true;
	}

	void Apply(PlayerActionRequest& action)
	{
		if (!isActive)
		{
			return;
		}

		if (effectType == PlayerActionEffectType::LaserSlow)
		{
			action.SetLaserSlowPenalty(slowPenalty);
			return;
		}

		if (effectType == PlayerActionEffectType::GameOver)
		{
			action.RequestGameOver();
			isActive = false;
			return;
		}

		ApplyStart(action);

		if (moveMode != PlayerActionMoveMode::None)
		{
			action.AddMoveVelocity(moveVelocity);
		}

		if (floatFrame > 0)
		{
			if (useGravityRate)
			{
				action.SetGravityRate(gravityRate);
			}

			if (useMaxFallSpeed)
			{
				action.SetMaxFallSpeed(maxFallSpeed);
			}
		}

		if (requestBoostEffect)
		{
			action.RequestBoostEffect();
		}
	}

	void Update()
	{
		if (!isActive)
		{
			return;
		}

		if (effectType == PlayerActionEffectType::LaserSlow)
		{
			activeFrame--;

			if (activeFrame <= 0)
			{
				activeFrame = 0;
				isActive = false;
			}

			return;
		}

		if (effectType == PlayerActionEffectType::GameOver)
		{
			isActive = false;
			return;
		}

		if (moveMode == PlayerActionMoveMode::Decay)
		{
			moveVelocity.x *= moveDecay;
			moveVelocity.y *= moveDecay;
			moveVelocity.z *= moveDecay;
		}

		if (moveMode == PlayerActionMoveMode::Timed)
		{
			moveFrame--;
		}

		if (floatFrame > 0)
		{
			floatFrame--;
		}

		if (IsFinished())
		{
			isActive = false;
		}
	}

	bool IsMoveFinished()
	{
		if (moveMode == PlayerActionMoveMode::None)
		{
			return true;
		}

		if (moveMode == PlayerActionMoveMode::Instant)
		{
			return true;
		}

		if (moveMode == PlayerActionMoveMode::Timed)
		{
			return moveFrame <= 0;
		}

		if (moveMode == PlayerActionMoveMode::Decay)
		{
			float speed =
				sqrtf(
					moveVelocity.x * moveVelocity.x +
					moveVelocity.y * moveVelocity.y +
					moveVelocity.z * moveVelocity.z
				);

			return speed <= moveMin;
		}

		return true;
	}

	bool IsFloatFinished()
	{
		return floatFrame <= 0;
	}

	bool IsStartFinished()
	{
		if (!useStartVerticalVelocity)
		{
			return true;
		}

		return hasAppliedStartVerticalVelocity;
	}

	bool IsFinished()
	{
		if (!isActive)
		{
			return true;
		}

		if (effectType == PlayerActionEffectType::LaserSlow)
		{
			return activeFrame <= 0;
		}

		if (effectType == PlayerActionEffectType::GameOver)
		{
			return true;
		}

		if (!IsStartFinished())
		{
			return false;
		}

		if (!IsMoveFinished())
		{
			return false;
		}

		if (!IsFloatFinished())
		{
			return false;
		}

		return true;
	}
};