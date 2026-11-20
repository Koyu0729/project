#pragma once

#include "UIQuadBase.h"

class UISlider : public UIQuadBase
{
private:
	UIRect m_Rect{ 0.0f, 0.0f, 300.0f, 20.0f };
	float m_Value = 1.0f;
	float m_MinValue = 0.0f;
	float m_MaxValue = 1.0f;
	bool m_IsSelected = false;

public:
	void Init() override;
	void Draw() override;

	void SetRect(const UIRect& rect);
	void SetValueRange(float minValue, float maxValue);
	void SetValue(float value);
	void SetValueFromMouse(float mouseX);
	void SetSelected(bool selected);

	float GetValue() const;
	bool HitTest(float mouseX, float mouseY) const;
};
