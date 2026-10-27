#pragma once

struct PlayerSlopeRideParameter
{
	bool enable = true;

	// Ctrlで開始するスライディング用の調整値。
	float slideStartMinSpeed = 0.28f;
	float slideStartBoost = 0.22f;
	float slideFriction = 0.985f;
	float slideSteerPower = 0.035f;
	int slideMaxFrame = 90;
	float slideEndSpeed = 0.12f;
	int slideCooldownFrame = 20;

	// Ctrlを離してキャンセルした時のブレーキ率。
	float slideCancelBrakeRate = 0.80f;

	// 最大スライド速度。
	float slideMaxSpeed = 1.35f;

	// 通常移動の加速が強すぎる時、スライド中だけ速度を少し抑える。
	// 1.0fなら補正なし。
	float slideInputVelocityKeepRate = 0.98f;

	// 足元チェックを少し下げる量。
	float footCheckOffsetY = 0.05f;

	// 斜面方向ベクトルが小さすぎる時は無効にする。
	float minDirectionLength = 0.0001f;

	// 坂下り加速と坂上りブレーキ。
	float slopeDownAccel = 0.035f;
	float slopeUpBrakeRate = 0.94f;

	// 坂上で落下速度を少し抑える。
	float fallVelocityRate = 0.65f;
	float minSlopeSlideRate = 0.04f;
	float slopeStartMinSpeed = 0.04f;
	float slopeStartSpeedByAngle = 0.45f;
	float slopeUpBrakePower = 0.08f;
};
