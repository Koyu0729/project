#pragma once

#include "UIQuadBase.h"

class Polygon2D : public UIQuadBase
{
private:
	XMFLOAT2 m_Size = XMFLOAT2(100.0f, 100.0f);

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetRect(float x, float y, float w, float h);

	void SetColor(float r, float g, float b, float a);
	void SetAlpha(float alpha);

	XMFLOAT2 GetSize() const { return m_Size; }
	float GetAlpha() const { return m_Color.w; }
};