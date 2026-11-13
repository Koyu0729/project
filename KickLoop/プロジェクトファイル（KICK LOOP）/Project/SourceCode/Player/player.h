#pragma once

#include <vector>

#include "gameObject.h"

#include "playerRuntimeState.h"

#include "playerEffectController.h"
#include "playerSoundController.h"

#include "playerMoveController.h"

#include "playerJumpController.h"

#include "playerDiveParameter.h"
#include "playerAirDiveController.h"

#include "playerStateController.h"

#include "playerWallKickParameter.h"
#include "playerWallKickController.h"

#include "playerRunParameter.h"
#include "playerRunController.h"

#include "playerBonusActionParameter.h"
#include "playerBonusActionController.h"

#include "playerActionRequest.h"
#include "playerActionResolver.h"

class AnimationModel;
class Camera;
class StageBlockManager;
class Time;

// プレイヤー本体のGameObject。
// 入力・移動・演出を各Controllerへ渡し、メイン状態に応じて更新する。
class Player : public GameObject
{
private:
	// プレイヤーモデル描画用のシェーダーとアニメーションモデル。
	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

	AnimationModel* m_AnimationModel = nullptr;

	int m_AnimationFrame = 0;

	// 接地、壁接触、よじ登り、ゴール状態など、実行中に変化する状態。
	PlayerRuntimeState m_State;

	// シーンが管理するステージとタイマーを参照する。
	StageBlockManager* m_StageBlockManager = nullptr;
	Time* m_GameTime = nullptr;

	// アクションごとの入力、移動と演出。
	PlayerEffectController m_EffectController;
	PlayerSoundController m_SoundController;

	PlayerMovementParameters m_MovementParameters;
	PlayerMoveController m_MoveController;
	PlayerJumpController m_JumpController;

	PlayerDiveParameter m_DiveParameter;
	PlayerAirDiveController m_AirDiveController;

	PlayerStateController m_StateController;

	PlayerWallKickParameter m_WallKickParameter;
	PlayerWallKickController m_WallKickController;

	PlayerRunParameter m_RunParameter;
	PlayerRunController m_RunController;

	PlayerBonusActionParameter m_BonusActionParameter;
	PlayerBonusActionController m_BonusActionController;

	PlayerActionResolver m_ActionResolver;

	// ジャンプ台の加速や浮遊など、複数フレーム続く外部効果。
	std::vector<PlayerActionEffect> m_ActionEffects;

	bool m_OldLeftClick = false;

	// 落下カメラ開始位置と、死亡扱いにする高さ。
	float m_FallSceneChangeY = -300.0f;
	float m_FallCameraStartY = -10.0f;

	// 進行距離計算用に、開始位置を保持する。
	Vector3 m_StartPosition = Vector3(0.0f, 0.0f, 0.0f);

	// 最後のジャンプ台からゴール床へ飛ばす演出用の状態。
	Vector3 m_GoalLaunchStartPosition = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_GoalLaunchTargetPosition = Vector3(0.0f, 0.0f, 0.0f);

	int m_GoalLaunchFrame = 0;
	int m_GoalLaunchFrameMax = 90;

	float m_GoalLaunchHeight = 35.0f;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void SetSceneReferences(Camera* camera, StageBlockManager* stageBlockManager, Time* time);
	void Draw() override;

	void SetMainState(PlayerMainState state);
	PlayerMainState GetMainState();

	bool IsFallCameraMode();

	void AddActionEffect(PlayerActionEffect effect);

	float GetProgressDistance() const;

	bool IsWallSliding()
	{
		return !m_State.isGround &&
			m_State.isTouchWall &&
			m_State.velocity.y <= 0.0f;
	}

	bool IsCrouchRiding()
	{
		return m_State.isCrouchRide;
	}

	Vector3 GetWallNormal()
	{
		return m_State.wallNormal;
	}

	Vector3 GetHitSize()
	{
		return m_MovementParameters.move.hitSize;
	}

	float GetMoveSpeed()
	{
		return m_MovementParameters.move.moveSpeed;
	}

	int GetCombo()
	{
		return m_RunController.GetCombo();
	}

	bool IsComboActive()
	{
		return m_RunController.IsComboActive();
	}

	float GetBonusGauge()
	{
		return m_RunController.GetBonusGauge();
	}

	float GetBonusGaugeRate()
	{
		return m_RunController.GetBonusGaugeRate(m_RunParameter);
	}

	bool ConsumeBonusGauge(float amount)
	{
		return m_RunController.ConsumeBonusGauge(amount);
	}

	PlayerSoundController& GetSoundController()
	{
		return m_SoundController;
	}

	void SetWallKick(bool isWallKick)
	{
		m_State.isWallKick = isWallKick;
	}

	bool IsWallKick()
	{
		return m_State.isWallKick;
	}

	bool InRange(
		int gx,
		int gz,
		int cx,
		int cz,
		int range
	)
	{
		return gx >= cx - range && gx <= cx + range &&
			gz >= cz - range && gz <= cz + range;
	}

	bool IsGround() const
	{
		return m_State.isGround;
	}

	Vector3 GetVelocity()
	{
		return m_State.velocity;
	}

	void StartGoalLaunch(
		Vector3 targetPosition,
		int frameMax,
		float height
	);

	bool IsGoalLaunching() const;

	bool IsGoalReached() const
	{
		return m_State.mainState == PlayerMainState::Goal;
	}

	bool IsDead() const
	{
		return m_State.mainState == PlayerMainState::Dead;
	}

private:
	void UpdateMainState(bool advanceFrame);
	void OnMainStateChanged(PlayerMainState previousState);
	void UpdateGameplay();
	PlayerMovementResult ResolveMovement(PlayerActionRequest& action);
	void ApplyMovementEffects(const PlayerMovementResult& result);

	void BuildActionEffects(PlayerActionRequest& action);

	void UpdateAnimation();

	void UpdateGoalLaunch();
	float SmoothStep01(float value);

	void CheckGoalTopBlock();

	void StartGoalSequence();
};
