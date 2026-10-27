#pragma once

#include "main.h"

#include "playerRuntimeState.h"
#include "playerMoveParameter.h"
#include "playerJumpParameter.h"
#include "playerClimbParameter.h"
#include "playerActionRequest.h"

class PlayerEffectController;
class PlayerStateController;
class PlayerSoundController;
class PlayerSlopeRideController;

struct PlayerSlopeRideParameter;

class StageBlock;

// 各ControllerがPlayerActionRequestへ集めた移動・状態変更の要求を、
// 実際のプレイヤー座標、速度、接地状態へ反映するクラス。
// 通常移動、壁キック、スライド、ジャンプ台などが同じフレームで重なっても、
// 処理順で挙動が変わらないように、反映順と合成ルールをここで統一する。
class PlayerActionResolver
{
public:
	void Resolve(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerJumpParameter& jumpParameter,
		PlayerClimbParameter& climbParameter,
		PlayerSlopeRideController& slopeRideController,
		PlayerSlopeRideParameter& slopeRideParameter,
		PlayerActionRequest& action,
		PlayerEffectController& effectController,
		PlayerStateController& stateController,
		PlayerSoundController* soundController
	);

private:
	// PlayerActionRequestに入った速度変更、状態変更、特殊移動要求を反映する。
	// 横方向の移動は後で合算し、縦速度やゲーム状態のように競合しやすいものはここで反映する。
	void ApplySpeedRequest(
		PlayerMoveParameter& moveParameter,
		PlayerActionRequest& action
	);

	void ApplyStateRequest(
		PlayerRuntimeState& state,
		PlayerActionRequest& action,
		PlayerEffectController& effectController,
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

	// 通常移動、壁キック反動、スライドやブリンクなどの追加移動を合算し、
	// 最終的な横移動量を作る。
	void ApplyStartWallKickVelocity(
		PlayerRuntimeState& state,
		PlayerActionRequest& action
	);

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

	// 重力、落下、接地判定、足場への補正を行う。
	// 壁スライド中や特殊アクション中は、状態や要求に応じて重力のかかり方を変える。
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

	bool IsStandingOnStageBlock(
		Vector3& position,
		PlayerMoveParameter& moveParameter
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

	// 坂ブロックの接地位置と法線を取得し、必要に応じてプレイヤーを坂の上へ補正する。
	// 通常の平面接地とは別に、坂スライドや坂上での安定した接地に使う。
	bool TryGetSlopeRideGroundY(
		Vector3 position,
		PlayerMoveParameter& moveParameter,
		float& outSurfaceY,
		Vector3& outNormal,
		StageBlock*& outBlock
	);

	void SnapToSlopeGround(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter
	);


	// 移動速度や壁スライド状態に応じて、スピードラインなどの演出を更新する。
	void UpdateSpeedLineEffect(
		float forwardMove,
		PlayerMoveParameter& moveParameter,
		PlayerEffectController& effectController
	);

	void UpdateWallSlideEffect(
		PlayerRuntimeState& state,
		PlayerEffectController& effectController
	);

	// ステージブロックとの横方向接触、壁法線、壁スライド状態を更新する。
	// 横移動はここを通すことで、壁抜けや壁接触状態のズレを抑える。
	void MoveWithWallSlide(
		Vector3& position,
		Vector3 move,
		Vector3 playerHitSize,
		PlayerRuntimeState& state,
		bool& isTouchWall,
		Vector3& wallNormal,
		StageBlock*& hitWallBlock
	);

	bool IsGroundOnlyHorizontalContact(
		Vector3 position,
		Vector3 playerHitSize,
		PlayerRuntimeState& state,
		StageBlock* hitBlock,
		Vector3 hitNormal
	);

	// ランゲームとしてプレイヤーが想定範囲外へ出ないように、X/Z方向の移動範囲を制限する。
	// Z方向は戻り防止と、最終ステージ終端の制限に使う。
	void ClampMoveX(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter
	);

	void ClampMoveZ(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter
	);

	// 壁接触、壁スライドロック、壁キック用の接触情報を更新する。
	// 壁スライド中は壁から不自然に離れないように、対象ブロックと法線を保持する。
	void UpdateTouchWallState(
		PlayerRuntimeState& state,
		bool isTouchWall,
		Vector3 wallNormal,
		StageBlock* hitWallBlock
	);

	void UpdateWallSlideLock(
		Vector3& position,
		Vector3& move,
		Vector3 playerHitSize,
		PlayerRuntimeState& state,
		bool& isTouchWall,
		Vector3& wallNormal,
		StageBlock*& hitWallBlock
	);

	void StartWallSlideLock(
		Vector3& position,
		PlayerRuntimeState& state,
		Vector3 wallNormal,
		StageBlock* hitWallBlock
	);

	void ClearWallSlideLock(
		PlayerRuntimeState& state
	);

	bool UpdateStillTouchingWallSlideLock(
		Vector3 position,
		Vector3 playerHitSize,
		PlayerRuntimeState& state
	);

	bool ShouldReleaseWallSlideLockByCamera(
		PlayerRuntimeState& state
	);

	Vector3 GetCameraForwardXZ();

	// よじ登りは通常移動より優先度が高いため、専用の開始判定と更新処理を持つ。
	// 開始後は一定フレーム、補間でプレイヤーをブロック上へ移動させる。
	bool UpdateLedgeClimb(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerClimbParameter& climbParameter
	);


	bool CanStartLedgeClimb(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerActionRequest& action,
		PlayerClimbParameter& climbParameter
	);

	void StartLedgeClimb(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerClimbParameter& climbParameter,
		PlayerSoundController* soundController
	);

	// よじ登り用に、プレイヤー前方のブロックを疑似Rayで取得する。
	// 通常の当たり判定とは別に、登れる壁の候補を探すために使う。
	void UpdateBlockRay(
		Vector3 position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerClimbParameter& climbParameter
	);

	// 補間やXZ正規化など、このResolver内だけで使う補助処理。
	// デバッグ表示用の補助処理。
	float SmoothStep(float t);

	Vector3 NormalizeXZ(Vector3 vector);

	void DebugPlayerStatus(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerActionRequest& action
	);
};