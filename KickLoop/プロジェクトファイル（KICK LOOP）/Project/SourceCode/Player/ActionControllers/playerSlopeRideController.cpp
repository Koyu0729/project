#include "playerSlopeRideController.h"

#include "manager.h"
#include "scene.h"

#include "stageBlock.h"
#include "stageBlockManager.h"

#include "playerSoundController.h"

#include <cmath>

#ifdef _DEBUG
#include <cstdio>
#endif

// 斜面滑りの開始と移動を処理する
void PlayerSlopeRideController::Apply(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerSlopeRideParameter& slopeRideParameter,
	PlayerActionRequest& action,
	PlayerSoundController* soundController
)
{
	state.isCrouchRide = false;

	UpdateCooldown();

	if (!slopeRideParameter.enable)
	{
		ResetSlide();
		return;
	}

	if (!state.isGround)
	{
		if (m_SlideState == PlayerSlideState::Sliding)
		{
			EndSlide(slopeRideParameter);
		}

		return;
	}

	if (m_SlideState == PlayerSlideState::Sliding)
	{
		state.isCrouchRide = true;

		if (!action.isCrouchHold)
		{
			CancelSlide(
				state,
				slopeRideParameter
			);

			return;
		}

		UpdateSlide(
			position,
			state,
			moveParameter,
			slopeRideParameter,
			action
		);

		return;
	}

	if (m_SlideState == PlayerSlideState::Cooldown)
	{
		return;
	}

	// しゃがみ入力中に条件を満たしていれば、自動でスライドを開始する。
	if (!action.isCrouchHold)
	{
		return;
	}

	if (!CanStartSlide(
		position,
		state,
		moveParameter,
		slopeRideParameter
	))
	{
		return;
	}

	StartSlide(
		position,
		state,
		moveParameter,
		slopeRideParameter,
		soundController
	);

	state.isCrouchRide = true;

	UpdateSlide(
		position,
		state,
		moveParameter,
		slopeRideParameter,
		action
	);
}

// スライド状態とクールダウンを更新する。
void PlayerSlopeRideController::UpdateCooldown()
{
	if (m_SlideState != PlayerSlideState::Cooldown)
	{
		return;
	}

	if (m_SlideCooldownFrame > 0)
	{
		m_SlideCooldownFrame--;
	}

	if (m_SlideCooldownFrame <= 0)
	{
		m_SlideCooldownFrame = 0;
		m_SlideState = PlayerSlideState::None;
	}
}

bool PlayerSlopeRideController::CanStartSlide(
	Vector3 position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerSlopeRideParameter& slopeRideParameter
)
{
	if (!state.isGround)
	{
		return false;
	}

	Vector3 slopeDownDirection =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	float slopeRate = 0.0f;

	bool isSlope =
		GetSlopeDownInfo(
			position,
			moveParameter,
			slopeRideParameter,
			slopeDownDirection,
			slopeRate
		);

	float speed = GetHorizontalSpeed(state);

#ifdef _DEBUG
	printf(
		"[SlideStartCheck] isSlope:%d dirX:%f dirZ:%f rate:%f minRate:%f speed:%f minSpeed:%f\n",
		isSlope ? 1 : 0,
		slopeDownDirection.x,
		slopeDownDirection.z,
		slopeRate,
		slopeRideParameter.minSlopeSlideRate,
		speed,
		slopeRideParameter.slideStartMinSpeed
	);
#endif

	// 坂の場合は、速度がなくても傾きに応じて滑り出せるようにする。
	if (isSlope)
	{
		if (slopeRate >= slopeRideParameter.minSlopeSlideRate)
		{
			return true;
		}
	}

	// 平地では、一定以上の速度が必要。
	if (speed < slopeRideParameter.slideStartMinSpeed)
	{
		return false;
	}

	return true;
}

void PlayerSlopeRideController::StartSlide(
	Vector3 position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerSlopeRideParameter& slopeRideParameter,
	PlayerSoundController* soundController
)
{
	m_SlideState = PlayerSlideState::Sliding;

	m_SlideFrame = 0;

	Vector3 slopeDownDirection =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	float slopeRate = 0.0f;

	bool isSlope =
		GetSlopeDownInfo(
			position,
			moveParameter,
			slopeRideParameter,
			slopeDownDirection,
			slopeRate
		);

	float speed = GetHorizontalSpeed(state);

	if (isSlope && slopeRate >= slopeRideParameter.minSlopeSlideRate)
	{
		// 坂では、下り方向へ滑り出す。
		// 傾きが強いほど初速も上げる。
		m_SlideDirection = slopeDownDirection;

		float slopeStartSpeed =
			slopeRideParameter.slopeStartMinSpeed +
			slopeRideParameter.slopeStartSpeedByAngle *
			slopeRate;

		if (speed < slopeStartSpeed)
		{
			speed = slopeStartSpeed;
		}
	}
	else
	{
		// 平地では、現在の水平移動方向を引き継いで滑る。
		m_SlideDirection =
			GetHorizontalVelocityDirection(
				state
			);

		speed += slopeRideParameter.slideStartBoost;
	}

	if (speed > slopeRideParameter.slideMaxSpeed)
	{
		speed = slopeRideParameter.slideMaxSpeed;
	}

	m_SlideSpeed = speed;

	// X方向はプレイヤーの移動速度として使う。
	state.velocity.x = m_SlideDirection.x * m_SlideSpeed;

	// このゲームではZ移動は前進量として扱うため、velocity.zには入れない。
	state.velocity.z = 0.0f;

	// スライディング開始音。
	// ワンショットSEなので、終了時に停止しない。
	if (soundController != nullptr)
	{
		soundController->PlaySlide();
	}
}

void PlayerSlopeRideController::UpdateSlide(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerSlopeRideParameter& slopeRideParameter,
	PlayerActionRequest& action
)
{
	if (m_SlideState != PlayerSlideState::Sliding)
	{
		return;
	}

	m_SlideFrame++;

	ApplySteer(
		state,
		slopeRideParameter,
		action
	);

	ApplySlope(
		position,
		state,
		moveParameter,
		slopeRideParameter
	);

	m_SlideSpeed *= slopeRideParameter.slideFriction;

	LimitSlideSpeed(slopeRideParameter);

	Vector3 slideMove =
		Vector3(
			m_SlideDirection.x * m_SlideSpeed,
			0.0f,
			m_SlideDirection.z * m_SlideSpeed
		);

	// ここでは直接positionを動かさない。
	action.AddMoveVelocity(slideMove);

	// 演出とデバッグ用に、現在のスライド速度をstateにも反映。
	state.velocity.x = slideMove.x;

	state.velocity.z = 0.0f;

	if (m_SlideFrame >= slopeRideParameter.slideMaxFrame)
	{
		EndSlide(slopeRideParameter);

		return;
	}

	if (m_SlideSpeed <= slopeRideParameter.slideEndSpeed)
	{
		EndSlide(slopeRideParameter);

		return;
	}
}

void PlayerSlopeRideController::EndSlide(
	PlayerSlopeRideParameter& slopeRideParameter
)
{
	m_SlideState = PlayerSlideState::Cooldown;

	m_SlideFrame = 0;

	m_SlideSpeed = 0.0f;

	m_SlideCooldownFrame = slopeRideParameter.slideCooldownFrame;

	m_SlideDirection =
		Vector3(
			0.0f,
			0.0f,
			1.0f
		);
}

// 斜面滑りを中断して減速する
void PlayerSlopeRideController::CancelSlide(
	PlayerRuntimeState& state,
	PlayerSlopeRideParameter& slopeRideParameter
)
{
	m_SlideSpeed *= slopeRideParameter.slideCancelBrakeRate;

	state.velocity.x *= slopeRideParameter.slideCancelBrakeRate;

	state.velocity.z = 0.0f;

	EndSlide(slopeRideParameter);
}

// 斜面滑りの状態と時間を初期化する。
void PlayerSlopeRideController::ResetSlide()
{
	m_SlideState = PlayerSlideState::None;

	m_SlideFrame = 0;
	m_SlideCooldownFrame = 0;

	m_SlideSpeed = 0.0f;

	m_SlideDirection =
		Vector3(
			0.0f,
			0.0f,
			1.0f
		);
}

// 水平方向の速さを計算する
float PlayerSlopeRideController::GetHorizontalSpeed(
	PlayerRuntimeState& state
)
{
	return sqrtf(
		state.velocity.x * state.velocity.x +
		state.velocity.z * state.velocity.z
	);
}

Vector3 PlayerSlopeRideController::GetHorizontalVelocityDirection(
	PlayerRuntimeState& state
)
{
	Vector3 direction =
		Vector3(
			state.velocity.x,
			0.0f,
			state.velocity.z
		);

	direction = NormalizeXZ(direction);

	if (
		fabsf(direction.x) < 0.0001f &&
		fabsf(direction.z) < 0.0001f
		)
	{
		direction =
			Vector3(
				0.0f,
				0.0f,
				1.0f
			);
	}

	return direction;
}

// 滑走中の操作方向を求める
Vector3 PlayerSlopeRideController::GetInputDirection(
	PlayerActionRequest& action
)
{
	if (!action.useHorizontalTargetVelocity)
	{
		return Vector3(
			0.0f,
			0.0f,
			0.0f
		);
	}

	Vector3 direction =
		Vector3(
			action.horizontalTargetVelocity.x,
			0.0f,
			action.horizontalTargetVelocity.z
		);

	return NormalizeXZ(direction);
}

// 入力に合わせて滑る方向を調整する
void PlayerSlopeRideController::ApplySteer(
	PlayerRuntimeState& state,
	PlayerSlopeRideParameter& slopeRideParameter,
	PlayerActionRequest& action
)
{
	Vector3 inputDirection = GetInputDirection(action);

	if (
		fabsf(inputDirection.x) < 0.0001f &&
		fabsf(inputDirection.z) < 0.0001f
		)
	{
		return;
	}

	m_SlideDirection.x += inputDirection.x * slopeRideParameter.slideSteerPower;

	m_SlideDirection.z += inputDirection.z * slopeRideParameter.slideSteerPower;

	m_SlideDirection = NormalizeXZ(m_SlideDirection);
}

// 斜面の下り方向へ加速する
void PlayerSlopeRideController::ApplySlope(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerSlopeRideParameter& slopeRideParameter
)
{
	Vector3 slopeDownDirection =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	float slopeRate = 0.0f;

	if (!GetSlopeDownInfo(
		position,
		moveParameter,
		slopeRideParameter,
		slopeDownDirection,
		slopeRate
	))
	{
		return;
	}

	if (slopeRate < slopeRideParameter.minSlopeSlideRate)
	{
		return;
	}

	float dot =
		m_SlideDirection.x * slopeDownDirection.x +
		m_SlideDirection.z * slopeDownDirection.z;

	if (dot >= 0.0f)
	{
		// 下り方向へ進んでいる。
		// 急坂ほど強く加速する。
		m_SlideSpeed += slopeRideParameter.slopeDownAccel * slopeRate;

		// 下り方向へ少し向きを寄せる。
		m_SlideDirection.x += slopeDownDirection.x * slopeRideParameter.slideSteerPower * slopeRate;

		m_SlideDirection.z += slopeDownDirection.z * slopeRideParameter.slideSteerPower * slopeRate;

		m_SlideDirection =
			NormalizeXZ(
				m_SlideDirection
			);
	}
	else
	{
		// 坂を上る方向へ進んでいる。
		// 急坂ほど強く減速する。
		float brakeRate =
			1.0f -
			slopeRideParameter.slopeUpBrakePower *
			slopeRate;

		if (brakeRate < 0.0f)
		{
			brakeRate = 0.0f;
		}

		m_SlideSpeed *= brakeRate;
	}

	if (state.velocity.y < 0.0f)
	{
		state.velocity.y *= slopeRideParameter.fallVelocityRate;
	}
}

bool PlayerSlopeRideController::GetSlopeDownInfo(
	Vector3 position,
	PlayerMoveParameter& moveParameter,
	PlayerSlopeRideParameter& slopeRideParameter,
	Vector3& outDirection,
	float& outSlopeRate
)
{
	outDirection =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	outSlopeRate = 0.0f;

	Scene* scene =
		Manager::GetScene();

	if (scene == nullptr)
	{
		return false;
	}

	StageBlockManager* stageBlockManager =
		scene->GetGameObject<StageBlockManager>();

	if (stageBlockManager == nullptr)
	{
		return false;
	}

	Vector3 checkPosition =
		position;

	checkPosition.y -= slopeRideParameter.footCheckOffsetY;

	StageBlock* hitBlock =
		stageBlockManager->GetHitStageBlock(
			checkPosition,
			moveParameter.hitSize
		);

	if (hitBlock == nullptr)
	{
		return false;
	}

	Vector3 slopeNormal =
		Vector3(
			0.0f,
			1.0f,
			0.0f
		);

	if (!hitBlock->GetSlopeRideNormal(
		checkPosition,
		moveParameter.hitSize,
		slopeNormal
	))
	{
		return false;
	}

	// normal.yが1に近いほど平地。
	// normal.yが小さいほど急坂。
	float normalY =
		slopeNormal.y;

	if (normalY < 0.0f)
	{
		normalY = 0.0f;
	}

	if (normalY > 1.0f)
	{
		normalY = 1.0f;
	}

	outSlopeRate =
		sqrtf(
			1.0f -
			normalY *
			normalY
		);

	Vector3 gravity =
		Vector3(
			0.0f,
			-1.0f,
			0.0f
		);

	float normalDotGravity =
		gravity.x * slopeNormal.x +
		gravity.y * slopeNormal.y +
		gravity.z * slopeNormal.z;

	Vector3 direction =
		Vector3(
			gravity.x - slopeNormal.x * normalDotGravity,
			gravity.y - slopeNormal.y * normalDotGravity,
			gravity.z - slopeNormal.z * normalDotGravity
		);

	direction.y = 0.0f;

	float length =
		sqrtf(
			direction.x * direction.x +
			direction.z * direction.z
		);

	if (length <= slopeRideParameter.minDirectionLength)
	{
		return false;
	}

	outDirection =
		Vector3(
			direction.x / length,
			0.0f,
			direction.z / length
		);

	return true;
}

// 滑る速度を上限内に収める
void PlayerSlopeRideController::LimitSlideSpeed(
	PlayerSlopeRideParameter& slopeRideParameter
)
{
	if (slopeRideParameter.slideMaxSpeed <= 0.0f)
	{
		return;
	}

	if (m_SlideSpeed <= slopeRideParameter.slideMaxSpeed)
	{
		return;
	}

	m_SlideSpeed = slopeRideParameter.slideMaxSpeed;
}

Vector3 PlayerSlopeRideController::NormalizeXZ(
	Vector3 vector
)
{
	vector.y = 0.0f;

	float length =
		sqrtf(
			vector.x * vector.x +
			vector.z * vector.z
		);

	if (length <= 0.0001f)
	{
		return Vector3(
			0.0f,
			0.0f,
			0.0f
		);
	}

	return Vector3(
		vector.x / length,
		0.0f,
		vector.z / length
	);
}