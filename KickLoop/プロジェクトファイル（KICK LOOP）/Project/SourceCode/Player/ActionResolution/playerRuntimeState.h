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
	GoalLaunch,
	Goal,
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

	int wallKickCoyoteFrame = 0;  // 壁を離れてもキックできる残りフレーム数
	Vector3 wallKickCoyoteNormal = Vector3(0.0f, 0.0f, 0.0f);  // 入力猶予中に使う、最後に触れた壁の法線
	StageBlock* wallKickCoyoteBlock = nullptr;

	// 壁スライド中に壁から不自然に離れないように、
	// 対象ブロック、壁法線、ロック位置を保持する。
	bool isWallSlideLocked = false;

	Vector3 wallSlideLockNormal = Vector3(0.0f, 0.0f, 0.0f);

	// 壁スライド開始時のプレイヤー座標
	float wallSlideLockPositionX = 0.0f;  // X（滑走中に固定する横位置）
	float wallSlideLockPositionZ = 0.0f;  // Z（開始時の奥行き）

	StageBlock* wallSlideLockBlock = nullptr;

	float wallSlideLookReleaseDot = 0.55f;  // 水平面で視線と壁法線の内積がこの値以上ならロック解除

	// よじ登り中の補間情報。
	// 開始位置から終了位置へ、一定フレームで移動させる。
	bool isLedgeClimb = false;

	int ledgeClimbFrame = 0;

	Vector3 ledgeClimbStartPosition = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 ledgeClimbEndPosition = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 ledgeClimbForward = Vector3(0.0f, 0.0f, 0.0f);

	// 現在接触している壁。
	StageBlock* currentWallBlock = nullptr;
	// 旧ゲージ判定用の壁。現在はリセットとデバッグ出力のみ。
	StageBlock* oldBonusWallBlock = nullptr;

	// よじ登り候補のブロックを探すための疑似Ray判定結果。
	bool isBlockRayHit = false;
	StageBlock* blockRayHitBlock = nullptr;
	Vector3 blockRayHitPosition = Vector3(0.0f, 0.0f, 0.0f);
};