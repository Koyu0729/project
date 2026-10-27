#pragma once

#include "UIQuadBase.h"

class UISlider : public UIQuadBase
{
private:
	float m_X = 0.0f;
	float m_Y = 0.0f;
	float m_Width = 300.0f;
	float m_Height = 20.0f;

	float m_Value = 1.0f;
	float m_MinValue = 0.0f;
	float m_MaxValue = 1.0f;

	float m_Step = 0.01f;

	bool m_IsSelected = false;
	bool m_IsDragging = false;
	bool m_IsChanged = false;

	float m_ColorR = 1.0f;
	float m_ColorG = 1.0f;
	float m_ColorB = 1.0f;

	float m_SelectedColorR = 0.0f;
	float m_SelectedColorG = 0.8f;
	float m_SelectedColorB = 1.0f;

private:
	void UpdateMouseInput();

	bool IsMouseInRect(float mouseX, float mouseY, float x, float y, float width, float height) const;

	void SetValueByMouseX(float mouseX);

	void DrawRect(float x, float y, float width, float height, float r, float g, float b, float a);

	float Clamp(float value, float minValue, float maxValue) const;
	float GetRate() const;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetRect(float x, float y, float width, float height);
	void SetValueRange(float minValue, float maxValue);
	void SetValue(float value);
	void AddValue(float value);
	void SetStep(float step);
	void SetSelected(bool selected);
	void SetBarColor(float r, float g, float b);
	void SetSelectedColor(float r, float g, float b);

	float GetValue() const;
	float GetStep() const;

	bool IsSelected() const;
	bool IsDragging() const;
	bool IsChanged() const;
};