#pragma once

struct PlayerBonusActionParameter
{
	// ダッシュに必要なゲージ量。
	float dashCost = 1.0f;

	// ダッシュ継続フレーム。
	int dashFrame = 14;

	// 視点方向ダッシュの強さ。
	float dashPower = 1.85f;

	// ダッシュ中の重力倍率。
	// 0.0fなら重力なしとして扱う。
	float dashGravityRate = 0.0f;

	// ダッシュ中の最大落下速度。
	// 0.0fならダッシュ中は落下しない。
	float dashMaxFallSpeed = 0.0f;

	// ダブルジャンプに必要なゲージ量。
	float doubleJumpCost = 1.0f;

	// ダブルジャンプの上方向速度。
	float doubleJumpPower = 0.8f;
};
