#include "playerJumpController.h"

#include "playerSoundController.h"

#include "input.h"

// ジャンプ入力から移動の要求を作る
void PlayerJumpController::BuildAction(
	PlayerRuntimeState& state,
	PlayerJumpParameter& jumpParameter,
	PlayerSoundController& soundController,
	PlayerActionRequest& action
)
{
	UpdateJumpBuffer(state, jumpParameter);
	BuildJumpInputAction(state, jumpParameter, soundController, action);
}

// 着地前のジャンプ入力を短時間保持する
void PlayerJumpController::UpdateJumpBuffer(
	PlayerRuntimeState& state,
	PlayerJumpParameter& jumpParameter
)
{
	if (Input::GetKeyTrigger(VK_SPACE))
	{
		if (!state.isGround)
		{
			// 壁キックで使う入力を通常ジャンプに残さない
			if (state.isTouchWall)
			{
				m_JumpBufferFrame = 0;
				return;
			}

			// 壁を離れた直後も壁キックの入力として扱う
			if (state.wallKickCoyoteFrame > 0)
			{
				m_JumpBufferFrame = 0;
				return;
			}
		}

		m_JumpBufferFrame = jumpParameter.jumpBufferFrameMax;
		return;
	}

	if (m_JumpBufferFrame > 0)
	{
		m_JumpBufferFrame--;
	}
}

// 保持した入力で着地後にジャンプする
void PlayerJumpController::BuildJumpInputAction(
	PlayerRuntimeState& state,
	PlayerJumpParameter& jumpParameter,
	PlayerSoundController& soundController,
	PlayerActionRequest& action
)
{
	if (!state.isGround)
	{
		return;
	}

	if (m_JumpBufferFrame <= 0)
	{
		return;
	}

	action.SetVerticalVelocity(jumpParameter.jumpPower);
	soundController.PlayJump();

	m_JumpBufferFrame = 0;
}
