#pragma once

#include "UIQuadBase.h"

class LoadPercent : public UIQuadBase
{
private:
	float m_Angle = 0.0f;

	float m_X = 0.0f;
	float m_Y = 0.0f;

	float m_Width = 200.0f;
	float m_Height = 200.0f;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

private:
	void UpdateRotationQuad();
};