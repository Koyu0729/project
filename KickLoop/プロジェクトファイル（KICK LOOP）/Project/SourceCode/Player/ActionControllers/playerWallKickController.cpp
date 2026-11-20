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
	Vector3 playerPosition,
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
		m_CoyotePlayerPosition = playerPosition;
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

bool PlayerWallKickController::IsAvailableWall(const StageBlock* wallBlock) const
{
	return m_StageBlockManager != nullptr &&
		m_StageBlockManager->HasActiveStageBlock(wallBlock) && wallBlock->CanWallKick();
}

Vector3 PlayerWallKickController::GetWallKickNormal(const PlayerRuntimeState& state) const
{
	if (state.isTouchWall)
	{
		return state.wallNormal;
	}

	return state.wallKickCoyoteNormal;
}

bool PlayerWallKickController::IsSameCoyoteWall(const StageBlock* wallBlock) const
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

	if (!Input::GetKeyTrigger(VK_SPACE))
	{
		return result;
	}

	StageBlock* wallBlock = GetKickableWall(state, wallKickParameter);
	if (wallBlock == nullptr)
	{
		return result;
	}

	Vector3 wallNormal = GetWallKickNormal(state);
	Vector3 wallKickVelocity = BuildWallKickVelocity(wallNormal, wallKickParameter);
	Vector3 footPosition = m_CoyotePlayerPosition - Vector3(0.0f, 0.9f, 0.0f);
	result.contactPosition = wallBlock->GetSurfacePoint(footPosition, wallNormal) + wallNormal * 0.06f;
	result.wallNormal = wallNormal;

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
StageBlock* PlayerWallKickController::GetKickableWall(
	const PlayerRuntimeState& state,
	const PlayerWallKickParameter& wallKickParameter
) const
{
	if (!wallKickParameter.enable || state.isGround || state.isLedgeClimb)
	{
		return nullptr;
	}

	Vector3 wallNormal = GetWallKickNormal(state);
	if (fabsf(wallNormal.x) <= wallKickParameter.canWallNormalX ||
		(fabsf(wallNormal.x) < 0.001f && fabsf(wallNormal.z) < 0.001f))
	{
		return nullptr;
	}

	if (state.isTouchWall)
	{
		if (IsAvailableWall(state.currentWallBlock))
		{
			return state.currentWallBlock;
		}

		return nullptr;
	}

	StageBlock* wallBlock = state.wallKickCoyoteBlock;
	if (state.wallKickCoyoteFrame > 0 && IsAvailableWall(wallBlock) && IsSameCoyoteWall(wallBlock))
	{
		return wallBlock;
	}

	return nullptr;
}

bool PlayerWallKickController::CanWallKickOnNextUpdate(
	const PlayerRuntimeState& state,
	const PlayerWallKickParameter& wallKickParameter
) const
{
	// 次の更新で猶予が切れるフレームには案内を出さない。
	if (!state.isTouchWall && !m_WasTouchingWall && state.wallKickCoyoteFrame <= 1)
	{
		return false;
	}

	return GetKickableWall(state, wallKickParameter) != nullptr;
}

// 壁から離れる速度と前進速度を計算する
Vector3 PlayerWallKickController::BuildWallKickVelocity(
	Vector3 wallNormal,
	PlayerWallKickParameter& wallKickParameter
)
{
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
