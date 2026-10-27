#pragma once

#include "UIQuadBase.h"

class BonusGaugeUI : public UIQuadBase
{
private:
	float m_MaxGauge = 3.0f;
	float m_CurrentGauge = 0.0f;
	float m_DisplayGauge = 0.0f;

	float m_BaseX = 80.0f;
	float m_BottomY = 520.0f;
	float m_Width = 48.0f;
	float m_Height = 270.0f;

	float m_DisplayLerpRate = 0.20f;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetGauge(float gauge);
	void AddGauge(float gauge);
	bool ConsumeGauge(float gauge);

	float GetGauge() const;
	float GetMaxGauge() const;
	bool IsFull() const;

	void SetBasePosition(float x, float bottomY);
	void SetSize(float width, float height);

private:
	void DrawGauge();

	void DrawRect(
		float x,
		float y,
		float width,
		float height,
		XMFLOAT4 color
	);

	float ClampGauge(float gauge);
};