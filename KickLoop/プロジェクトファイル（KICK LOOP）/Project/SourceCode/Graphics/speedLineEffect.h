#pragma once

#include "UIQuadBase.h"

class SpeedLineEffect : public UIQuadBase
{
private:
	float m_SpeedRate = 0.0f;

	float m_BoostAlpha = 0.0f;
	float m_CurrentAlpha = 0.0f;

	float m_MaxAlpha = 0.25f;
	float m_BoostMaxAlpha = 0.40f;

	// 最終的なアルファ上限
	float m_AlphaLimit = 0.70f;

	float m_AlphaLerpSpeed = 0.08f;
	float m_BoostDecay = 0.06f;

	float m_MinRunAlpha = 0.10f;

	// この速度割合を超えたら通常スピードラインを出す
	float m_AppearSpeedRate = 0.08f;

	bool m_IsActiveEffect = false;

	float m_BaseWidth = 0.0f;
	float m_BaseHeight = 0.0f;

	float m_CurrentScale = 1.0f;
	float m_TargetScale = 1.0f;
	float m_ScaleLerpSpeed = 0.12f;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetSpeedRate(float rate);
	void PlayBoost();

	void SetActiveEffect(bool active);

	void SetAlphaLimit(float alphaLimit);

	// 呼び出し互換用。ここでは速度を変更しない
	void SetWallSlideEffect(
		bool isWallSlide,
		float wallNormalX
	);

private:
	void SetRect(
		float x,
		float y,
		float w,
		float h
	);

	void UpdateScaleRect();

	float ClampAlpha(float alpha);
};