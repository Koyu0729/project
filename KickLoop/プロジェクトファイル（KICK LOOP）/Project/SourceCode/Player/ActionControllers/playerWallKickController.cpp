#include "playerWallKickController.h"

#include "stageBlock.h"
#include "input.h"

#include "manager.h"
#include "scene.h"
#include "particleEffectManager.h"

#include "playerSoundController.h"

#include <cmath>

// 壁キックの速度と演出を要求する
PlayerWallKickResult PlayerWallKickController::BuildAction(
	Vector3 playerPosition,
	PlayerRuntimeState& state,
	PlayerWallKickParameter& wallKickParameter,
	PlayerSoundController* soundController,
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

	state.wallKickCoyoteFrame = 0;
	state.wallKickCoyoteNormal = Vector3(0.0f, 0.0f, 0.0f);
	state.wallKickCoyoteBlock = nullptr;

	if (soundController != nullptr)
	{
		soundController->PlayWallKick();
	}

	ParticleEffectManager* particleEffectManager = Manager::GetScene()->GetGameObject<ParticleEffectManager>();

	if (particleEffectManager != nullptr)
	{
		particleEffectManager->EmitWallKickSpark(playerPosition);
	}

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

	if (wallBlock == nullptr)
	{
		return false;
	}

	if (!wallBlock->CanWallKick())
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
	Vector3 wallNormal = state.wallNormal;

	if (fabsf(wallNormal.x) < 0.001f && fabsf(wallNormal.z) < 0.001f)
	{
		wallNormal = state.wallKickCoyoteNormal;
	}

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
