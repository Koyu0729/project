#include "UISlider.h"

#include "main.h"

#include <Windows.h>

void UISlider::Init()
{
	CreateQuad();

	SetTexture("asset\\texture\\frameBoxR.png");

	m_X = 0.0f;
	m_Y = 0.0f;
	m_Width = 300.0f;
	m_Height = 20.0f;

	m_Value = 1.0f;
	m_MinValue = 0.0f;
	m_MaxValue = 1.0f;

	m_Step = 0.01f;

	m_IsSelected = false;
	m_IsDragging = false;
	m_IsChanged = false;

	m_ColorR = 1.0f;
	m_ColorG = 1.0f;
	m_ColorB = 1.0f;

	m_SelectedColorR = 0.0f;
	m_SelectedColorG = 0.8f;
	m_SelectedColorB = 1.0f;

	SetActive(true);
	SetVisible(true);
}

void UISlider::Uninit()
{
	UIQuadBase::Uninit();
}

void UISlider::Update()
{
	m_IsChanged = false;

	UpdateMouseInput();
}

void UISlider::Draw()
{
	SetDraw2D();

	float rate = GetRate();

	float knobR = 1.0f;
	float knobG = 1.0f;
	float knobB = 1.0f;

	if (m_IsSelected || m_IsDragging)
	{
		knobR = 1.0f;
		knobG = 1.0f;
		knobB = 0.0f;
	}

	// バー本体。常に固定長で白。
	DrawRect(
		m_X,
		m_Y,
		m_Width,
		m_Height,
		1.0f,
		1.0f,
		1.0f,
		0.95f
	);

	float knobX =
		m_X +
		m_Width *
		rate;

	DrawRect(
		knobX - 5.0f,
		m_Y - 8.0f,
		10.0f,
		m_Height + 16.0f,
		knobR,
		knobG,
		knobB,
		1.0f
	);
}

void UISlider::UpdateMouseInput()
{
	POINT mousePosition{};

	GetCursorPos(&mousePosition);
	ScreenToClient(GetWindow(), &mousePosition);

	float mouseX = (float)mousePosition.x;
	float mouseY = (float)mousePosition.y;

	bool isLeftMouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

	if (!isLeftMouseDown)
	{
		m_IsDragging = false;

		return;
	}

	if (!m_IsDragging)
	{
		if (!IsMouseInRect(mouseX, mouseY, m_X, m_Y - 12.0f, m_Width, m_Height + 24.0f))
		{
			return;
		}

		m_IsDragging = true;
	}

	SetValueByMouseX(mouseX);
}

void UISlider::SetValueByMouseX(float mouseX)
{
	float rate = (mouseX - m_X) / m_Width;

	rate = Clamp(rate, 0.0f, 1.0f);

	float oldValue = m_Value;

	m_Value = m_MinValue + (m_MaxValue - m_MinValue) * rate;
	m_Value = Clamp(m_Value, m_MinValue, m_MaxValue);

	if (oldValue != m_Value)
	{
		m_IsChanged = true;
	}
}

bool UISlider::IsMouseInRect(float mouseX, float mouseY, float x, float y, float width, float height) const
{
	if (mouseX < x)
	{
		return false;
	}

	if (mouseX > x + width)
	{
		return false;
	}

	if (mouseY < y)
	{
		return false;
	}

	if (mouseY > y + height)
	{
		return false;
	}

	return true;
}

void UISlider::DrawRect(float x, float y, float width, float height, float r, float g, float b, float a)
{
	SetTexture("asset\\texture\\frameBoxR.png");

	VERTEX_3D vertex[4]{};

	vertex[0].Position = XMFLOAT3(x, y, 0.0f);
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

	vertex[1].Position = XMFLOAT3(x + width, y, 0.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

	vertex[2].Position = XMFLOAT3(x, y + height, 0.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

	vertex[3].Position = XMFLOAT3(x + width, y + height, 0.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	UIQuadBase::SetColor(r, g, b, a);

	UpdateQuad(vertex);

	DrawQuad();
}

void UISlider::SetRect(float x, float y, float width, float height)
{
	m_X = x;
	m_Y = y;
	m_Width = width;
	m_Height = height;
}

void UISlider::SetValueRange(float minValue, float maxValue)
{
	m_MinValue = minValue;
	m_MaxValue = maxValue;

	if (m_MaxValue < m_MinValue)
	{
		float temp = m_MinValue;
		m_MinValue = m_MaxValue;
		m_MaxValue = temp;
	}

	m_Value = Clamp(m_Value, m_MinValue, m_MaxValue);
}

void UISlider::SetValue(float value)
{
	m_Value = Clamp(value, m_MinValue, m_MaxValue);
}

void UISlider::AddValue(float value)
{
	float oldValue = m_Value;

	m_Value += value;
	m_Value = Clamp(m_Value, m_MinValue, m_MaxValue);

	if (oldValue != m_Value)
	{
		m_IsChanged = true;
	}
}

void UISlider::SetStep(float step)
{
	m_Step = step;
}

void UISlider::SetSelected(bool selected)
{
	m_IsSelected = selected;
}

void UISlider::SetBarColor(float r, float g, float b)
{
	m_ColorR = r;
	m_ColorG = g;
	m_ColorB = b;
}

void UISlider::SetSelectedColor(float r, float g, float b)
{
	m_SelectedColorR = r;
	m_SelectedColorG = g;
	m_SelectedColorB = b;
}

float UISlider::GetValue() const
{
	return m_Value;
}

float UISlider::GetStep() const
{
	return m_Step;
}

bool UISlider::IsSelected() const
{
	return m_IsSelected;
}

bool UISlider::IsDragging() const
{
	return m_IsDragging;
}

bool UISlider::IsChanged() const
{
	return m_IsChanged;
}

float UISlider::Clamp(float value, float minValue, float maxValue) const
{
	if (value < minValue)
	{
		return minValue;
	}

	if (value > maxValue)
	{
		return maxValue;
	}

	return value;
}

float UISlider::GetRate() const
{
	float range = m_MaxValue - m_MinValue;

	if (range <= 0.0001f)
	{
		return 0.0f;
	}

	return Clamp((m_Value - m_MinValue) / range, 0.0f, 1.0f);
}