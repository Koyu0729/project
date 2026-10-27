#pragma once

struct PlayerJumpParameter
{
	// 通常ジャンプの上方向速度。
	float jumpPower = 0.80f;

	// 通常重力。
	float gravity = 0.030f;

	// 基準地面の高さ。
	float groundY = 1.5f;

	// 壁スライド中の重力と最大落下速度。
	float wallSlideGravity = 0.002f;
	float wallSlideMaxFallSpeed = -0.04f;

	// 着地直前にSpaceを押した場合、何フレームまでジャンプできるか。
	int jumpBufferFrameMax = 8;
};
