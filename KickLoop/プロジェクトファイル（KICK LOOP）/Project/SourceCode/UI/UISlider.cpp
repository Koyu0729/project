#include "UISlider.h"

#include <algorithm>

namespace
{
	constexpr float KnobHalfWidth = 5.0f;
	constexpr float KnobPaddingY = 8.0f;
	constexpr float HitPaddingY = 12.0f;
}

void UISlider::Init()
{
	CreateQuad();
	SetTexture("asset\\texture\\frameBoxR.png");
}

void UISlider::Draw()
{
	const float range = m_MaxValue - m_MinValue;
	const float rate = range > 0.0f ? (m_Value - m_MinValue) / range : 0.0f;
	const float knobX = m_Rect.x + m_Rect.width * rate;
	const UIRect knobRect{
		knobX - KnobHalfWidth,
		m_Rect.y - KnobPaddingY,
		KnobHalfWidth * 2.0f,
		m_Rect.height + KnobPaddingY * 2.0f
	};

	DrawRect2D(m_Rect, XMFLOAT4(1.0f, 1.0f, 1.0f, 0.95f));
	DrawRect2D(knobRect, XMFLOAT4(1.0f, 1.0f, m_IsSelected ? 0.0f : 1.0f, 1.0f));
}

void UISlider::SetRect(const UIRect& rect)
{
	m_Rect = rect;
}

void UISlider::SetValueRange(float minValue, float maxValue)
{
	m_MinValue = std::min(minValue, maxValue);
	m_MaxValue = std::max(minValue, maxValue);
	SetValue(m_Value);
}

void UISlider::SetValue(float value)
{
	m_Value = std::max(m_MinValue, std::min(m_MaxValue, value));
}

void UISlider::SetValueFromMouse(float mouseX)
{
	if (m_Rect.width <= 0.0f)
	{
		return;
	}

	const float rate = (mouseX - m_Rect.x) / m_Rect.width;
	SetValue(m_MinValue + (m_MaxValue - m_MinValue) * rate);
}

void UISlider::SetSelected(bool selected)
{
	m_IsSelected = selected;
}

float UISlider::GetValue() const
{
	return m_Value;
}

bool UISlider::HitTest(float mouseX, float mouseY) const
{
	const UIRect hitRect{
		m_Rect.x - KnobHalfWidth,
		m_Rect.y - HitPaddingY,
		m_Rect.width + KnobHalfWidth * 2.0f,
		m_Rect.height + HitPaddingY * 2.0f
	};
	return hitRect.Contains(mouseX, mouseY);
}
