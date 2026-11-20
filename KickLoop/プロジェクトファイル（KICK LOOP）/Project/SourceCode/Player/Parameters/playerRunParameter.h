#pragma once

struct PlayerRunParameter
{
	// コンボ最大数。
	int maxCombo = 5;

	// ボーナスゲージ最大値。
	float maxBonusGauge = 3.0f;

	// 壁キック1回目の基本ゲージ増加量。
	float baseGaugeAdd = 0.25f;

	// コンボが1増えるごとに増えるゲージ倍率。
	float comboGaugeAddRate = 0.25f;

	// 1回の壁キックで増えるゲージ量の最大倍率。
	float maxGaugeAddRate = 2.0f;

	// 壁スライド中に1フレームで増えるゲージ量。
	float wallSlideGaugeAddPerFrame = 0.006f;

	// 壁スライドゲージ加算開始までに必要なフレーム数。
	int wallSlideBonusStartFrame = 20;
};
