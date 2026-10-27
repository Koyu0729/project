#pragma once

#include <vector>

#include "gameObject.h"

#include "playerRuntimeState.h"

#include "playerEffectController.h"
#include "playerSoundController.h"

#include "playerMoveParameter.h"
#include "playerMoveController.h"

#include "playerJumpParameter.h"
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

#include "playerClimbParameter.h"
#include "playerClimbController.h"

#include "playerSlopeRideParameter.h"
#include "playerSlopeRideController.h"

#include "playerActionRequest.h"
#include "playerActionResolver.h"

class AnimationModel;

// プレイヤー本体のGameObject。
// 各Controllerで入力や状態からActionRequestを作成し、
// 最終的な速度・座標・接地状態はPlayerActionResolverで反映する。
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

	// 各Controllerは入力や状態からActionRequestを作成し、Resolverが最終反映を行う。
	PlayerEffectController m_EffectController;
	PlayerSoundController m_SoundController;

	PlayerMoveParameter m_MoveParameter;
	PlayerMoveController m_MoveController;

	PlayerJumpParameter m_JumpParameter;
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

	PlayerClimbParameter m_ClimbParameter;
	PlayerClimbController m_ClimbController;

	PlayerSlopeRideParameter m_SlopeRideParameter;
	PlayerSlopeRideController m_SlopeRideController;

	PlayerActionResolver m_ActionResolver;

	// ジャンプ台やレーザーなど、複数フレーム続く外部効果。
	std::vector<PlayerActionEffect> m_ActionEffects;

	bool m_OldLeftClick = false;

	// 落下カメラ開始位置と、死亡扱いにする高さ。
	float m_FallSceneChangeY = -300.0f;
	float m_FallCameraStartY = -10.0f;

	// 進行距離計算用に、開始位置を保持する。
	Vector3 m_StartPosition = Vector3(0.0f, 0.0f, 0.0f);

	// 最後のジャンプ台からゴール床へ飛ばす演出用の状態。
	bool m_IsGoalLaunch = false;

	Vector3 m_GoalLaunchStartPosition = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_GoalLaunchTargetPosition = Vector3(0.0f, 0.0f, 0.0f);

	int m_GoalLaunchFrame = 0;
	int m_GoalLaunchFrameMax = 90;

	float m_GoalLaunchHeight = 35.0f;

	// ゴール到達後は通常移動を止め、ゴール演出だけ更新する。
	bool m_IsGoal = false;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetMainState(PlayerMainState state);
	PlayerMainState GetMainState();

	bool IsFallCameraMode();

	void AddActionEffect(PlayerActionEffect effect);
	void AddOrRefreshActionEffect(PlayerActionEffect effect);

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
		return m_MoveParameter.hitSize;
	}

	float GetMoveSpeed()
	{
		return m_MoveParameter.moveSpeed;
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
		return m_IsGoal;
	}

	bool IsDead() const
	{
		return m_State.mainState == PlayerMainState::Dead;
	}

private:
	void UpdateGameplay();

	void BuildActionEffects(PlayerActionRequest& action);

	void UpdateAnimation();

	void UpdateGoalLaunch();
	float SmoothStep01(float value);

	void CheckGoalTopBlock();

	void StartGoalSequence();
};
