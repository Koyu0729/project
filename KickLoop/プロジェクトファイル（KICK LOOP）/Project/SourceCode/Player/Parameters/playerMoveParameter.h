#pragma once

#include "main.h"

struct PlayerMoveParameter
{
	// プレイヤー当たり判定の半径。
	// x / z は横幅、y は高さ半分として扱う。
	Vector3 hitSize = Vector3(1.0f, 1.5f, 1.0f);

	// 正面や背面の壁接触を少し先まで見るための距離。
	// 現在の移動制限には使わず、壁判定用の補助値として残している。
	float virtualWallCheckDistance = 0.08f;

	// 通常移動の基準速度と、現在フレームで使う移動速度。
	float baseMoveSpeed = 0.60f;
	float moveSpeed = 0.60f;

	// Resolverで速度補正した後の上限と下限。
	float maxMoveSpeed = 1.20f;
	float minMoveSpeed = 0.18f;

	// 壁スライド中の横移動速度倍率。
	float wallSlideMoveSpeedRate = 1.25f;

	// 壁スライド中に自動で前方向入力を入れるか。
	bool wallSlideAutoForward = true;

	// 壁スライド中の自動前入力量。
	// 1.0fならW入力と同じ強さとして扱う。
	float wallSlideAutoForwardInput = 1.0f;

	// 入力方向へ近づける強さ。
	float groundAcceleration = 0.60f;
	float airAcceleration = 0.14f;

	// 入力を離した時の減速率。
	float groundFriction = 0.40f;
	float airFriction = 0.90f;

	// この値未満の速度は0として扱う。
	float stopVelocity = 0.001f;

	// プレイヤーのX移動制限。
	// 歩行可能範囲を取得できない場合に使う。
	float minMoveX = 15.0f;
	float maxMoveX = 35.0f;

	// 壁キック反動速度の減衰率。
	float wallKickDecay = 0.94f;
};
