#pragma once

struct PlayerWallKickParameter
{
	bool enable = true;

	// 壁キックの上方向速度。
	float jumpPower = 0.80f;

	// 壁から離れる横方向の強さ。
	float sidePower = 0.55f;

	// ランゲーム用の前方向速度。
	float forwardPower = 0.50f;

	// 壁から少し離すための補助速度。
	float detachPower = 0.03f;

	float canWallNormalX = 0.15f;  // 壁キックに必要な壁法線のX成分の下限（絶対値）

	int coyoteFrameMax = 8;  // 壁を離れてからキックを受け付けるフレーム数

	// 壁キック成功時に入れる一瞬停止のフレーム数。
	int freezeFrame = 5;
};
