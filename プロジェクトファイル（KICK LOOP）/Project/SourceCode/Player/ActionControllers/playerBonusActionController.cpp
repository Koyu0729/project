#include "playerBonusActionController.h"

#include "playerRunController.h"
#include "playerSoundController.h"

#include "input.h"
#include "mouseInput.h"

#include "manager.h"
#include "scene.h"
#include "camera.h"

#include <cmath>

// ボーナスアクションの状態を初期化する。
void PlayerBonusActionController::Init()
{
	m_Mode = PlayerBonusActionMode::None;
	m_Frame = 0;

	m_DashMoveX = 0.0f;
	m_DashForwardMove = 0.0f;
}

// 入力とゲージを調べて追加アクションを要求する
void PlayerBonusActionController::BuildAction(
	PlayerRuntimeState& state,
	PlayerBonusActionParameter& parameter,
	PlayerRunController& runController,
	PlayerSoundController& soundController,
	PlayerActionRequest& action
)
{
	if (m_Mode == PlayerBonusActionMode::None)
	{
		if (Input::GetKeyTrigger('E'))
		{
			StartViewDash(parameter, runController, soundController);
		}

		TryAirJump(state, parameter, runController, soundController, action);
	}

	if (m_Mode == PlayerBonusActionMode::ViewDash)
	{
		UpdateViewDash(parameter, action);
	}
}


// ブリンク中の移動方向と速度を記録する
void PlayerBonusActionController::StartViewDash(
	PlayerBonusActionParameter& parameter,
	PlayerRunController& runController,
	PlayerSoundController& soundController
)
{
	Vector3 cameraForward = GetCameraForwardXZ();
	float forwardLength = sqrtf(cameraForward.x * cameraForward.x + cameraForward.z * cameraForward.z);

	if (forwardLength <= 0.001f)
	{
		return;
	}

	if (!runController.ConsumeBonusGauge(parameter.dashCost))
	{
		return;
	}

	m_Mode = PlayerBonusActionMode::ViewDash;
	m_Frame = parameter.dashFrame;

	m_DashMoveX = cameraForward.x * parameter.dashPower;
	m_DashForwardMove = cameraForward.z * parameter.dashPower;

	soundController.PlayBlink();
}

void PlayerBonusActionController::UpdateViewDash(
	PlayerBonusActionParameter& parameter,
	PlayerActionRequest& action
)
{
	if (m_Frame <= 0)
	{
		End();
		return;
	}

	action.AddMoveVelocity(Vector3(m_DashMoveX, 0.0f, 0.0f));
	action.AddForwardMove(m_DashForwardMove);
	action.SetGravityRate(parameter.dashGravityRate);
	action.SetMaxFallSpeed(parameter.dashMaxFallSpeed);
	action.RequestBoostEffect();

	m_Frame--;

	if (m_Frame <= 0)
	{
		End();
	}
}

// 壁キックと重ならない場合に空中ジャンプする
void PlayerBonusActionController::TryAirJump(
	PlayerRuntimeState& state,
	PlayerBonusActionParameter& parameter,
	PlayerRunController& runController,
	PlayerSoundController& soundController,
	PlayerActionRequest& action
)
{
	if (!Mouse::RightClick())
	{
		return;
	}

	if (state.isGround)
	{
		return;
	}

	if (state.isTouchWall)
	{
		return;
	}

	if (state.wallKickCoyoteFrame > 0)
	{
		return;
	}

	if (!runController.ConsumeBonusGauge(parameter.doubleJumpCost))
	{
		return;
	}

	action.SetVerticalVelocity(parameter.doubleJumpPower);
	action.RequestBoostEffect();

	soundController.PlayJump();
}

// カメラの前方向を水平面に投影する
Vector3 PlayerBonusActionController::GetCameraForwardXZ()
{
	Vector3 result = Vector3(0.0f, 0.0f, 0.0f);
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return result;
	}

	Camera* camera = scene->GetGameObject<Camera>();

	if (camera == nullptr)
	{
		return result;
	}

	result = camera->GetTarget() - camera->GetPosition();
	result.y = 0.0f;

	float length = sqrtf(result.x * result.x + result.z * result.z);

	if (length <= 0.001f)
	{
		return Vector3(0.0f, 0.0f, 0.0f);
	}

	result.x /= length;
	result.z /= length;

	return result;
}

void PlayerBonusActionController::End()
{
	m_Mode = PlayerBonusActionMode::None;
	m_Frame = 0;

	m_DashMoveX = 0.0f;
	m_DashForwardMove = 0.0f;
}
