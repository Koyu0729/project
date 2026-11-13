#include "playerWallKickController.h"

#include "stageBlock.h"
#include "input.h"

#include "stageBlockManager.h"

#include <cmath>

// 壁キックの入力猶予を解除する。
void PlayerWallKickController::Reset(PlayerRuntimeState& state)
{
	state.wallKickCoyoteFrame = 0;
	state.wallKickCoyoteNormal = Vector3(0.0f, 0.0f, 0.0f);
	state.wallKickCoyoteBlock = nullptr;
	m_WasTouchingWall = false;
}

// 接触を失った次の入力フレームから猶予を数える。
void PlayerWallKickController::UpdateCoyote(
	PlayerRuntimeState& state,
	const PlayerWallKickParameter& wallKickParameter
)
{
	if (!wallKickParameter.enable || state.isGround || state.isLedgeClimb)
	{
		Reset(state);
		return;
	}

	if (state.isTouchWall)
	{
		StageBlock* wallBlock = state.currentWallBlock;

		if (!IsAvailableWall(wallBlock) ||
			fabsf(state.wallNormal.x) <= wallKickParameter.canWallNormalX)
		{
			Reset(state);
			return;
		}

		state.wallKickCoyoteFrame = wallKickParameter.coyoteFrameMax;
		if (state.wallKickCoyoteFrame < 0)
		{
			state.wallKickCoyoteFrame = 0;
		}

		state.wallKickCoyoteNormal = state.wallNormal;
		state.wallKickCoyoteBlock = wallBlock;
		m_CoyoteWallPosition = wallBlock->GetPosition();
		m_CoyoteWallRotation = wallBlock->GetRotation();
		m_CoyoteWallScale = wallBlock->GetScale();
		m_WasTouchingWall = true;
		return;
	}

	if (!IsAvailableWall(state.wallKickCoyoteBlock) ||
		!IsSameCoyoteWall(state.wallKickCoyoteBlock))
	{
		Reset(state);
		return;
	}

	if (!m_WasTouchingWall && state.wallKickCoyoteFrame > 0)
	{
		state.wallKickCoyoteFrame--;
	}
	m_WasTouchingWall = false;

	if (state.wallKickCoyoteFrame <= 0)
	{
		Reset(state);
	}
}

bool PlayerWallKickController::IsAvailableWall(StageBlock* wallBlock)
{
	return m_StageBlockManager != nullptr &&
		m_StageBlockManager->HasActiveStageBlock(wallBlock) && wallBlock->CanWallKick();
}

bool PlayerWallKickController::IsSameCoyoteWall(StageBlock* wallBlock)
{
	// プールから別の場所へ再配置された壁では猶予を使わない。
	Vector3 position = wallBlock->GetPosition();
	Vector3 rotation = wallBlock->GetRotation();
	Vector3 scale = wallBlock->GetScale();

	return
		position.x == m_CoyoteWallPosition.x &&
		position.y == m_CoyoteWallPosition.y &&
		position.z == m_CoyoteWallPosition.z &&
		rotation.x == m_CoyoteWallRotation.x &&
		rotation.y == m_CoyoteWallRotation.y &&
		rotation.z == m_CoyoteWallRotation.z &&
		scale.x == m_CoyoteWallScale.x &&
		scale.y == m_CoyoteWallScale.y &&
		scale.z == m_CoyoteWallScale.z;
}

// 壁キックの速度と停止フレームを要求する
PlayerWallKickResult PlayerWallKickController::BuildAction(
	PlayerRuntimeState& state,
	PlayerWallKickParameter& wallKickParameter,
	PlayerActionRequest& action
)
{
	PlayerWallKickResult result;

	StageBlock* wallBlock = nullptr;

	if (!CanWallKick(state, wallKickParameter, wallBlock))
	{
		return result;
	}

	Vector3 wallKickVelocity = BuildWallKickVelocity(state, wallKickParameter);

	action.SetStartWallKickVelocity(wallKickVelocity);
	action.SetVerticalVelocity(wallKickParameter.jumpPower);
	action.SetWallKickSuccess();

	if (wallKickParameter.freezeFrame > 0)
	{
		action.RequestFreeze(wallKickParameter.freezeFrame);
	}

	state.isGround = false;
	state.isTouchWall = false;

	Reset(state);

	result.isSuccess = true;
	result.wallBlock = wallBlock;

	return result;
}

// 接触中の壁か入力猶予内の壁を調べる
bool PlayerWallKickController::CanWallKick(
	PlayerRuntimeState& state,
	PlayerWallKickParameter& wallKickParameter,
	StageBlock*& wallBlock
)
{
	wallBlock = nullptr;

	if (!wallKickParameter.enable)
	{
		return false;
	}

	if (state.isGround)
	{
		return false;
	}

	if (state.isLedgeClimb)
	{
		return false;
	}

	if (!Input::GetKeyTrigger(VK_SPACE))
	{
		return false;
	}

	Vector3 wallNormal = Vector3(0.0f, 0.0f, 0.0f);

	if (state.isTouchWall)
	{
		wallNormal = state.wallNormal;
		wallBlock = state.currentWallBlock;
	}
	// 壁を離れた直後は最後に触れた壁で判定する
	else if (state.wallKickCoyoteFrame > 0)
	{
		wallNormal = state.wallKickCoyoteNormal;
		wallBlock = state.wallKickCoyoteBlock;
	}
	else
	{
		return false;
	}

	if (!IsAvailableWall(wallBlock))
	{
		return false;
	}

	if (fabsf(wallNormal.x) < 0.001f && fabsf(wallNormal.z) < 0.001f)
	{
		return false;
	}

	if (fabsf(wallNormal.x) <= wallKickParameter.canWallNormalX)
	{
		return false;
	}

	return true;
}

// 壁から離れる速度と前進速度を計算する
Vector3 PlayerWallKickController::BuildWallKickVelocity(
	PlayerRuntimeState& state,
	PlayerWallKickParameter& wallKickParameter
)
{
	Vector3 wallNormal = state.isTouchWall ? state.wallNormal : state.wallKickCoyoteNormal;

	wallNormal = NormalizeXZ(wallNormal);

	Vector3 velocity = Vector3(0.0f, 0.0f, 0.0f);

	velocity.x += wallNormal.x * wallKickParameter.sidePower;
	velocity.z += wallNormal.z * wallKickParameter.sidePower;

	velocity.z += wallKickParameter.forwardPower;

	velocity.x += wallNormal.x * wallKickParameter.detachPower;
	velocity.z += wallNormal.z * wallKickParameter.detachPower;

	return velocity;
}

// 水平面の方向ベクトルを正規化する
Vector3 PlayerWallKickController::NormalizeXZ(Vector3 vector)
{
	vector.y = 0.0f;

	float length = sqrtf(vector.x * vector.x + vector.z * vector.z);

	if (length <= 0.0001f)
	{
		return Vector3(0.0f, 0.0f, 0.0f);
	}

	vector.x /= length;
	vector.z /= length;

	return vector;
}
