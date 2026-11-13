#pragma once

#include "main.h"

#include "playerRuntimeState.h"
#include "playerMoveParameter.h"
#include "playerJumpParameter.h"
#include "playerActionRequest.h"
#include "playerClimbController.h"
#include "playerSlopeRideController.h"
#include "playerCollision.h"
#include "playerWallSlideController.h"

class PlayerStateController;
class PlayerSoundController;

class StageBlockManager;
class Camera;

struct PlayerMovementParameters
{
	PlayerMoveParameter move;
	PlayerJumpParameter jump;
};

struct PlayerMovementResult
{
	float forwardMove = 0.0f;
	bool landed = false;
	bool updateEffects = false;
};

// 各ControllerがPlayerActionRequestへ集めた移動・状態変更の要求を、
// 実際のプレイヤー座標、速度、接地状態へ反映するクラス。
// 通常移動、壁キック、スライド、ジャンプ台などが同じフレームで重なっても、
// 反映順と合成ルールをこのクラスで管理する。
class PlayerActionResolver
{
private:
	PlayerCollision m_Collision;
	PlayerWallSlideController m_WallSlideController;

	PlayerClimbController m_ClimbController;
	PlayerSlopeRideController m_SlopeRideController;

public:
	void SetSceneReferences(Camera* camera, StageBlockManager* stageBlockManager);

	void Init(PlayerRuntimeState& state);
	void ResetClimbAndSlide(PlayerRuntimeState& state);

	bool UpdateClimb(
		Vector3& position,
		PlayerRuntimeState& state
	);

	// 通常移動の準備を行い、ブースト演出を出すか返す。
	bool PrepareMovement(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerActionRequest& action,
		PlayerStateController& stateController
	);

	PlayerMovementResult Resolve(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMovementParameters& parameters,
		PlayerActionRequest& action,
		PlayerSoundController* soundController
	);

	void DebugPlayerStatus(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerActionRequest& action
	);

private:
	// 走行速度の要求を適用し、上下限に収める。
	void ApplySpeedRequest(
		PlayerMoveParameter& moveParameter,
		PlayerActionRequest& action
	);

	bool ApplyStateRequest(
		PlayerRuntimeState& state,
		PlayerActionRequest& action,
		PlayerStateController& stateController
	);

	void ApplyVerticalRequest(
		PlayerRuntimeState& state,
		PlayerActionRequest& action
	);

	void ApplyAirDiveRequest(
		PlayerRuntimeState& state,
		PlayerActionRequest& action
	);

	// 入力から作られた目標速度へ近づける。
	// 入力がある時は加速、入力がない時は摩擦で減速させ、通常移動のベース速度を作る。
	void ApplyHorizontalVelocityRequest(
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerActionRequest& action
	);

	void ApplyAcceleration(
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		Vector3 targetVelocity
	);

	void ApplyFriction(
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter
	);

	// 壁キック開始時の反動速度を設定する。
	void ApplyStartWallKickVelocity(
		PlayerRuntimeState& state,
		PlayerActionRequest& action
	);

	// 通常の水平速度、壁キックの反動、アクションの追加移動を合算する。
	Vector3 BuildFinalHorizontalMove(
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerActionRequest& action
	);

	void ApplyWallKickDecay(
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter
	);

	float ApplyHorizontalMove(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerActionRequest& action
	);

	// 空中のY速度に重力を反映し、必要に応じて落下速度を制限する。
	void ApplyGravity(
		PlayerRuntimeState& state,
		PlayerJumpParameter& jumpParameter,
		PlayerActionRequest& action
	);

	void UpdateGroundSupport(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerJumpParameter& jumpParameter,
		PlayerActionRequest& action
	);

	void ApplyVerticalMove(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter
	);

	void UpdateGround(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerJumpParameter& jumpParameter
	);

	// 斜面に合わせて高さを補正し、接地時に必要な状態をそろえる。
	void SnapToSlopeGround(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter
	);

	void SetGrounded(PlayerRuntimeState& state);
};
