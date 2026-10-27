#pragma once

#include "main.h"

class StageBlock;

enum class PlayerMainState
{
	Wait,
	Playing,
	Freeze,
	Falling,
	Dead,
};

// プレイヤーの現在状態をまとめて保持する構造体。
// ControllerとResolverから参照・更新する。
struct PlayerRuntimeState
{
	// 基本状態
	PlayerMainState mainState = PlayerMainState::Wait;

	Vector3 velocity = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 wallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);

	bool isGround = false;
	bool isTouchWall = false;
	bool isWallKick = false;

	// スライディング中
	bool isCrouchRide = false;

	Vector3 wallNormal = Vector3(0.0f, 0.0f, 0.0f);

	// 一瞬停止演出や落下カメラ演出用の状態。
	int freezeFrame = 0;

	bool isFallCameraMode = false;

	// 壁から少し離れた直後でも壁キックを許可する。
	int wallKickCoyoteFrame = 0;
	Vector3 wallKickCoyoteNormal = Vector3(0.0f, 0.0f, 0.0f);
	StageBlock* wallKickCoyoteBlock = nullptr;

	// 壁スライド中に壁から不自然に離れないように、
	// 対象ブロック、壁法線、ロック位置を保持する。
	bool isWallSlideLocked = false;

	Vector3 wallSlideLockNormal = Vector3(0.0f, 0.0f, 0.0f);

	float wallSlideLockPositionX = 0.0f;
	float wallSlideLockPositionZ = 0.0f;

	StageBlock* wallSlideLockBlock = nullptr;

	// カメラ方向が壁から離れる方向を向いた時、壁スライドロックを解除するためのしきい値。
	float wallSlideLookReleaseDot = 0.55f;

	// よじ登り中の補間情報。
	// 開始位置から終了位置へ、一定フレームで移動させる。
	bool isLedgeClimb = false;

	int ledgeClimbFrame = 0;

	Vector3 ledgeClimbStartPosition = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 ledgeClimbEndPosition = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 ledgeClimbForward = Vector3(0.0f, 0.0f, 0.0f);

	// 壁アクションでゲージ加算する時に、同じ壁から連続で加算されないように使う。
	StageBlock* currentWallBlock = nullptr;
	StageBlock* oldBonusWallBlock = nullptr;

	// よじ登り候補のブロックを探すための疑似Ray判定結果。
	bool isBlockRayHit = false;
	StageBlock* blockRayHitBlock = nullptr;
	Vector3 blockRayHitPosition = Vector3(0.0f, 0.0f, 0.0f);
};