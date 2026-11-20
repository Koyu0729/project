#pragma once

struct PlayerJumpParameter
{
	float jumpPower = 0.80f;  // 通常ジャンプの上向きの初速

	// 通常重力。
	float gravity = 0.030f;

	// 基準地面の高さ。
	float groundY = 1.5f;

	// 壁スライド中の重力と最大落下速度。
	float wallSlideGravity = 0.002f;
	float wallSlideMaxFallSpeed = -0.04f;  // 壁滑り中の落下速度の上限（下向きが負）

	int jumpBufferFrameMax = 8;  // 着地前のジャンプ入力を覚えておくフレーム数
};
