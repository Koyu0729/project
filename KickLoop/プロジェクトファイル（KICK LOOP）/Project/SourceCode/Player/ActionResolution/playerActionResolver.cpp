#include "playerActionResolver.h"

#include "playerStateController.h"

#include <cmath>
#include <cstdio>

void PlayerActionResolver::SetSceneReferences(Camera* camera, StageBlockManager* stageBlockManager)
{
	m_Collision.SetStageBlockManager(stageBlockManager);
	m_WallSlideController.SetSceneReferences(camera, stageBlockManager);
}

void PlayerActionResolver::Init(PlayerRuntimeState& state)
{
	m_SlopeRideController.Reset(state);
}

void PlayerActionResolver::ResetClimbAndSlide(PlayerRuntimeState& state)
{
	m_ClimbController.End(state);
	m_SlopeRideController.Reset(state);
}

bool PlayerActionResolver::UpdateClimb(
	Vector3& position,
	PlayerRuntimeState& state
)
{
	return m_ClimbController.Update(
		position,
		state
	);
}

// 通常移動の前に接触候補、走行速度、状態を更新する
bool PlayerActionResolver::PrepareMovement(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerActionRequest& action,
	PlayerStateController& stateController
)
{
	m_ClimbController.UpdateBlockRay(
		position,
		state,
		moveParameter
	);

	ApplySpeedRequest(
		moveParameter,
		action
	);

	return ApplyStateRequest(
		state,
		action,
		stateController
	);
}

// 移動の要求を順番に反映する
PlayerMovementResult PlayerActionResolver::Resolve(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMovementParameters& parameters,
	PlayerActionRequest& action,
	PlayerSoundController* soundController
)
{
	PlayerMoveParameter& moveParameter = parameters.move;
	PlayerJumpParameter& jumpParameter = parameters.jump;

	bool wasGround = state.isGround;

	PlayerMovementResult result;
	result.updateEffects = true;

	ApplyVerticalRequest(
		state,
		action
	);

	ApplyAirDiveRequest(
		state,
		action
	);

	ApplyStartWallKickVelocity(
		state,
		action
	);

	UpdateGroundSupport(
		position,
		state,
		moveParameter,
		jumpParameter,
		action
	);

	ApplyHorizontalVelocityRequest(
		state,
		moveParameter,
		action
	);

	m_SlopeRideController.Apply(
		position,
		state,
		moveParameter,
		action,
		soundController
	);

	float forwardMove = ApplyHorizontalMove(position, state, moveParameter, action);

	SnapToSlopeGround(
		position,
		state,
		moveParameter
	);

	if (m_ClimbController.CanStartLedgeClimb(
		position,
		state,
		action
	))
	{
		m_SlopeRideController.Reset(state);
		m_ClimbController.Start(
			position,
			state,
			state.currentWallBlock,
			soundController
		);

		return result;
	}

	ApplyGravity(
		state,
		jumpParameter,
		action
	);

	ApplyVerticalMove(
		position,
		state,
		moveParameter
	);

	UpdateGround(
		position,
		state,
		jumpParameter
	);

	result.forwardMove = forwardMove;
	result.landed = !wasGround && state.isGround;

	return result;
}

// 走行速度の要求を上下限内に収める
void PlayerActionResolver::ApplySpeedRequest(
	PlayerMoveParameter& moveParameter,
	PlayerActionRequest& action
)
{
	if (action.useResolvedMoveSpeed)
	{
		moveParameter.moveSpeed = action.resolvedMoveSpeed;
	}
	else
	{
		moveParameter.moveSpeed = moveParameter.baseMoveSpeed;
	}

	if (moveParameter.moveSpeed > moveParameter.maxMoveSpeed)
	{
		moveParameter.moveSpeed = moveParameter.maxMoveSpeed;
	}

	if (moveParameter.moveSpeed < moveParameter.minMoveSpeed)
	{
		moveParameter.moveSpeed = moveParameter.minMoveSpeed;
	}
}

// 死亡・壁キック・停止の要求を反映する
bool PlayerActionResolver::ApplyStateRequest(
	PlayerRuntimeState& state,
	PlayerActionRequest& action,
	PlayerStateController& stateController
)
{
	if (action.requestGameOver)
	{
		stateController.ChangeState(state, PlayerMainState::Dead);
		return false;
	}

	if (action.requestWallKickSuccess)
	{
		state.isWallKick = true;

		m_WallSlideController.ClearLock(state);
	}

	if (action.requestFreeze)
	{
		stateController.StartFreeze(
			state,
			action.freezeFrame
		);
	}

	return action.requestBoostEffect;
}

// ジャンプの要求を上向きの速度に反映する
void PlayerActionResolver::ApplyVerticalRequest(
	PlayerRuntimeState& state,
	PlayerActionRequest& action
)
{
	if (!action.useVerticalVelocity)
	{
		return;
	}

	state.velocity.y = action.verticalVelocity;
	state.isGround = false;
}

// 浮上や急降下の速度を反映する
void PlayerActionResolver::ApplyAirDiveRequest(
	PlayerRuntimeState& state,
	PlayerActionRequest& action
)
{
	if (action.requestAirFloat)
	{
		state.velocity.y = action.airFloatVelocity;

		state.velocity.x *= action.airDiveHorizontalBrakeRate;
		state.velocity.z *= action.airDiveHorizontalBrakeRate;

		state.wallKickVelocity.x *= action.airDiveHorizontalBrakeRate;
		state.wallKickVelocity.z *= action.airDiveHorizontalBrakeRate;

		state.isGround = false;
	}

	if (action.requestAirDive)
	{
		state.velocity.y = action.airDiveVelocity;

		state.velocity.x *= action.airDiveHorizontalBrakeRate;
		state.velocity.z *= action.airDiveHorizontalBrakeRate;

		state.wallKickVelocity.x *= action.airDiveHorizontalBrakeRate;
		state.wallKickVelocity.z *= action.airDiveHorizontalBrakeRate;

		state.isGround = false;
	}
}

// 入力に応じて加速か摩擦を適用する
void PlayerActionResolver::ApplyHorizontalVelocityRequest(
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerActionRequest& action
)
{
	if (action.useHorizontalTargetVelocity)
	{
		ApplyAcceleration(
			state,
			moveParameter,
			action.horizontalTargetVelocity
		);
	}
	else
	{
		ApplyFriction(
			state,
			moveParameter
		);
	}

	if (fabsf(state.velocity.x) < moveParameter.stopVelocity)
	{
		state.velocity.x = 0.0f;
	}

	if (fabsf(state.velocity.z) < moveParameter.stopVelocity)
	{
		state.velocity.z = 0.0f;
	}
}

// 目標の水平速度に近づける
void PlayerActionResolver::ApplyAcceleration(
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	Vector3 targetVelocity
)
{
	float acceleration = moveParameter.airAcceleration;

	if (state.isGround)
	{
		acceleration = moveParameter.groundAcceleration;
	}

	state.velocity.x += (targetVelocity.x - state.velocity.x) * acceleration;

	state.velocity.z += (targetVelocity.z - state.velocity.z) * acceleration;
}

// 入力のない水平速度を減らす
void PlayerActionResolver::ApplyFriction(
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter
)
{
	float friction = moveParameter.airFriction;

	if (state.isGround)
	{
		friction = moveParameter.groundFriction;
	}

	state.velocity.x *= friction;
	state.velocity.z *= friction;
}

// 壁キック直後の追加速度を設定する
void PlayerActionResolver::ApplyStartWallKickVelocity(
	PlayerRuntimeState& state,
	PlayerActionRequest& action
)
{
	if (!action.useStartWallKickVelocity)
	{
		return;
	}

	state.wallKickVelocity = action.startWallKickVelocity;
}

Vector3 PlayerActionResolver::BuildFinalHorizontalMove(
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerActionRequest& action
)
{
	Vector3 move = Vector3(0.0f, 0.0f, 0.0f);

	move.x += state.velocity.x;
	move.z += state.velocity.z;

	move.x += state.wallKickVelocity.x;
	move.z += state.wallKickVelocity.z;

	move.x += action.moveVelocity.x;
	move.z += action.moveVelocity.z;

	ApplyWallKickDecay(
		state,
		moveParameter
	);

	return move;
}

// 壁キックの追加速度を徐々に減らす
void PlayerActionResolver::ApplyWallKickDecay(
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter
)
{
	state.wallKickVelocity.x *= moveParameter.wallKickDecay;
	state.wallKickVelocity.z *= moveParameter.wallKickDecay;

	if (fabsf(state.wallKickVelocity.x) < 0.001f)
	{
		state.wallKickVelocity.x = 0.0f;
	}

	if (fabsf(state.wallKickVelocity.z) < 0.001f)
	{
		state.wallKickVelocity.z = 0.0f;
	}
}

// 壁との接触を調べて水平移動を補正する
float PlayerActionResolver::ApplyHorizontalMove(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerActionRequest& action
)
{
	state.currentWallBlock = nullptr;

	Vector3 move = BuildFinalHorizontalMove(state, moveParameter, action);

	// 移動の前に固定中の壁を確認し、移動のあとに新しい接触を反映する。
	PlayerWallContact wall;
	m_WallSlideController.Update(position, move, moveParameter.hitSize, state, wall);

	float forwardMove = move.z + action.forwardMove;
	move.z = forwardMove;

	bool ignoreFloorContact = state.isGround && !(state.velocity.y > 0.0f);
	PlayerHorizontalCollisionResult collision = m_Collision.MoveHorizontal(position, move, moveParameter, ignoreFloorContact);
	position = collision.position;

	if (collision.stopX)
	{
		state.velocity.x = 0.0f;
		state.wallKickVelocity.x = 0.0f;
	}
	if (collision.stopZ)
	{
		state.velocity.z = 0.0f;
		state.wallKickVelocity.z = 0.0f;
	}

	wall = collision.wall;
	m_WallSlideController.ApplyContact(state, wall);
	m_WallSlideController.Update(position, move, moveParameter.hitSize, state, wall);

	return forwardMove;
}

// 落下速度を重力で増やす
void PlayerActionResolver::ApplyGravity(
	PlayerRuntimeState& state,
	PlayerJumpParameter& jumpParameter,
	PlayerActionRequest& action
)
{
	if (state.isGround)
	{
		return;
	}

	if (action.useGravityRate)
	{
		float gravity = jumpParameter.gravity * action.gravityRate;

		if (state.velocity.y <= 0.0f)
		{
			gravity *= 0.55f;
		}

		state.velocity.y -= gravity;

		if (action.useMaxFallSpeed)
		{
			if (state.velocity.y < action.maxFallSpeed)
			{
				state.velocity.y = action.maxFallSpeed;
			}
		}

		return;
	}

	if (state.isTouchWall && state.velocity.y <= 0.0f)
	{
		state.velocity.y -= jumpParameter.wallSlideGravity;

		if (state.velocity.y < jumpParameter.wallSlideMaxFallSpeed)
		{
			state.velocity.y = jumpParameter.wallSlideMaxFallSpeed;
		}

		return;
	}

	state.velocity.y -= jumpParameter.gravity;
}

// 足元のブロックが接地を支えているか調べる
void PlayerActionResolver::UpdateGroundSupport(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerJumpParameter& jumpParameter,
	PlayerActionRequest& action
)
{
	if (action.useVerticalVelocity)
	{
		return;
	}

	if (action.requestAirFloat || action.requestAirDive)
	{
		state.isGround = false;
		return;
	}

	if (state.velocity.y > 0.0f)
	{
		state.isGround = false;
		return;
	}

	if (position.y <= jumpParameter.groundY)
	{
		state.isGround = true;
		return;
	}

	if (m_Collision.IsStandingOnStageBlock(position, moveParameter.hitSize))
	{
		state.isGround = true;
		return;
	}

	state.isGround = false;
}

// プレイヤーの高さを斜面に合わせる
void PlayerActionResolver::SnapToSlopeGround(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter
)
{
	if (state.velocity.y > 0.0f)
	{
		return;
	}

	float slopeSurfaceY = 0.0f;

	if (!m_Collision.TryGetSlopeGroundY(position, moveParameter.hitSize, slopeSurfaceY))
	{
		return;
	}

	float playerBottomY = position.y - moveParameter.hitSize.y;

	float distance = playerBottomY - slopeSurfaceY;

	float maxSnapUp = 0.35f;
	float maxSnapDown = 0.75f;

	if (distance > maxSnapUp)
	{
		return;
	}

	if (distance < -maxSnapDown)
	{
		return;
	}

	position.y = slopeSurfaceY + moveParameter.hitSize.y;

	SetGrounded(state);
}

// 上下移動の接触結果をプレイヤー状態へ反映する。
void PlayerActionResolver::ApplyVerticalMove(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter
)
{
	PlayerVerticalCollisionResult collision = m_Collision.MoveVertical(position, state.velocity.y, moveParameter.hitSize);
	position.y = collision.positionY;

	if (collision.contact == PlayerVerticalContact::Floor)
	{
		SetGrounded(state);
	}
	else if (collision.contact == PlayerVerticalContact::Ceiling)
	{
		state.velocity.y = 0.0f;
	}
}

// 基準の地面より下に出たら位置を戻す。
void PlayerActionResolver::UpdateGround(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerJumpParameter& jumpParameter
)
{
	if (position.y > jumpParameter.groundY)
	{
		return;
	}

	position.y = jumpParameter.groundY;
	SetGrounded(state);
}

void PlayerActionResolver::SetGrounded(PlayerRuntimeState& state)
{
	state.velocity.y = 0.0f;
	state.isGround = true;

	state.wallKickVelocity.x = 0.0f;
	state.wallKickVelocity.z = 0.0f;

	state.isTouchWall = false;
	state.wallNormal = Vector3(0.0f, 0.0f, 0.0f);
	state.currentWallBlock = nullptr;
	state.oldBonusWallBlock = nullptr;

	m_WallSlideController.ClearLock(state);
}

// 位置・速度・状態をデバッグ出力する
void PlayerActionResolver::DebugPlayerStatus(
	Vector3& position,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerActionRequest& action
)
{
#ifdef _DEBUG
	printf(
		"Pos(%.2f, %.2f, %.2f) "
		"Vel(%.2f, %.2f, %.2f) "
		"WallKick(%.2f, %.2f, %.2f) "
		"MoveSpeed %.2f "
		"ActionMove(%.2f, %.2f, %.2f) "
		"ForwardMove %.2f "
		"Ground %d Wall %d WallNormal(%.2f, %.2f, %.2f) "
		"CurrentWall %p OldBonusWall %p "
		"AirFloat %d AirDive %d "
		"LedgeClimb %d LedgeFrame %d "
		"WallSlideLock %d LockNormal(%.2f, %.2f, %.2f) LockBlock %p\n",
		position.x,
		position.y,
		position.z,

		state.velocity.x,
		state.velocity.y,
		state.velocity.z,

		state.wallKickVelocity.x,
		state.wallKickVelocity.y,
		state.wallKickVelocity.z,

		moveParameter.moveSpeed,

		action.moveVelocity.x,
		action.moveVelocity.y,
		action.moveVelocity.z,

		action.forwardMove,

		state.isGround ? 1 : 0,
		state.isTouchWall ? 1 : 0,

		state.wallNormal.x,
		state.wallNormal.y,
		state.wallNormal.z,

		state.currentWallBlock,
		state.oldBonusWallBlock,

		action.requestAirFloat ? 1 : 0,
		action.requestAirDive ? 1 : 0,

		state.isLedgeClimb ? 1 : 0,
		state.ledgeClimbFrame,

		state.isWallSlideLocked ? 1 : 0,

		state.wallSlideLockNormal.x,
		state.wallSlideLockNormal.y,
		state.wallSlideLockNormal.z,

		state.wallSlideLockBlock
	);
#endif
}
