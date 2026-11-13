#include "optionUI.h"

#include "gameData.h"
#include "input.h"
#include "manager.h"
#include "title.h"

namespace
{
	constexpr UIRect PanelRect{ 190.0f, 25.0f, 900.0f, 675.0f };
	constexpr UIRect HeaderRect{ PanelRect.x, PanelRect.y, PanelRect.width, 70.0f };
	constexpr UIRect FirstSliderRect{ 420.0f, 165.0f, 430.0f, 22.0f };
	constexpr float SliderIntervalY = 68.0f;
	constexpr float SliderLabelOffsetY = -40.0f;
	constexpr float SliderLabelHeight = 34.0f;
	constexpr float ButtonLabelOffsetY = 2.0f;
	constexpr float ButtonLabelHeight = 30.0f;
	constexpr float ButtonHeight = 36.0f;
	constexpr float RightButtonX = PanelRect.x + PanelRect.width - 260.0f;
	constexpr float RightButtonWidth = 230.0f;
	const XMFLOAT2 LabelTextureSize(653.0f, 1259.0f);

	struct SliderSetting
	{
		const float* value;
		void (*apply)(float);
		float minValue;
		float maxValue;
		float step;
		float labelWidth;
		XMFLOAT4 labelPixels;
	};

	const SliderSetting SliderSettings[] =
	{
		{ &GameData::MasterVolume, GameData::SetMasterVolume,
		  0.0f, 1.0f, 0.01f, 250.0f, { 8.0f, 8.0f, 503.0f, 113.0f } },
		{ &GameData::BgmVolume, GameData::SetBgmVolume,
		  0.0f, 1.0f, 0.01f, 205.0f, { 9.0f, 149.0f, 398.0f, 255.0f } },
		{ &GameData::SeVolume, GameData::SetSeVolume,
		  0.0f, 1.0f, 0.01f, 190.0f, { 9.0f, 292.0f, 363.0f, 397.0f } },
		{ &GameData::VoiceVolume, GameData::SetVoiceVolume,
		  0.0f, 1.0f, 0.01f, 230.0f, { 8.0f, 434.0f, 468.0f, 538.0f } },
		{ &GameData::MouseSensitivity, GameData::SetMouseSensitivity,
		  0.1f, 10.0f, 0.05f, 320.0f, { 8.0f, 575.0f, 644.0f, 681.0f } },
	};

	enum class ButtonAction
	{
		Reset,
		ReturnTitle,
		ExitGame,
	};

	struct ButtonLayout
	{
		ButtonAction action;
		UIRect rect;
		float labelWidth;
		XMFLOAT4 labelPixels;
	};

	const ButtonLayout ButtonLayouts[] =
	{
		{ ButtonAction::Reset, { PanelRect.x + 35.0f, 650.0f, 160.0f, ButtonHeight },
		  115.0f, { 8.0f, 718.0f, 224.0f, 823.0f } },
		{ ButtonAction::ReturnTitle, { RightButtonX, 608.0f, RightButtonWidth, ButtonHeight },
		  225.0f, { 8.0f, 1001.0f, 468.0f, 1106.0f } },
		{ ButtonAction::ExitGame, { RightButtonX, 650.0f, RightButtonWidth, ButtonHeight },
		  185.0f, { 9.0f, 1144.0f, 363.0f, 1250.0f } },
	};

	void SetupLabel(UITextureText& label, const UIRect& rect, const XMFLOAT4& pixels)
	{
		label.Init();
		label.SetTexture("asset\\texture\\option.png");
		label.SetAtlasRect(rect, pixels, LabelTextureSize);
	}

	XMFLOAT4 GetButtonColor(bool selected, bool isExit)
	{
		const float alpha = selected ? 0.95f : 0.70f;
		if (isExit)
		{
			return XMFLOAT4(selected ? 0.95f : 0.55f, 0.15f, 0.15f, alpha);
		}

		if (selected)
		{
			return XMFLOAT4(0.0f, 0.75f, 1.0f, alpha);
		}

		return XMFLOAT4(0.20f, 0.45f, 0.65f, alpha);
	}

	bool ReadMousePosition(HWND window, float& x, float& y)
	{
		POINT position{};
		RECT client{};
		if (!GetCursorPos(&position) || !ScreenToClient(window, &position) ||
			!GetClientRect(window, &client) || client.right <= 0 || client.bottom <= 0)
		{
			return false;
		}

		// UI は固定解像度で描くため、リサイズ後も同じ座標系で判定する。
		x = (float)position.x * SCREEN_WIDTH / (float)client.right;
		y = (float)position.y * SCREEN_HEIGHT / (float)client.bottom;
		return true;
	}

	bool IsLeftMouseDown()
	{
		return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	}
}

void OptionUI::Init()
{
	CreateQuad();
	SetTexture("asset\\texture\\frameBoxR.png");

	m_IsOpen = false;
	m_SelectedIndex = 0;
	m_DraggingSlider = -1;
	m_PrevLeftMouseDown = false;

	CreateParts();
	LoadSliderValues();
	SetActive(true);
	SetVisible(true);
}

void OptionUI::Uninit()
{
	if (m_IsOpen)
	{
		Close();
	}

	for (SliderRow& row : m_Sliders)
	{
		row.slider.Uninit();
		row.label.Uninit();
	}

	for (UITextureText& label : m_ButtonLabels)
	{
		label.Uninit();
	}

	UIQuadBase::Uninit();
}

void OptionUI::CreateParts()
{
	static_assert(sizeof(SliderSettings) / sizeof(SliderSettings[0]) == SliderCount,
		"Every slider needs a setting.");
	static_assert(sizeof(ButtonLayouts) / sizeof(ButtonLayouts[0]) == ButtonCount,
		"Every button needs a layout.");

	for (int i = 0; i < SliderCount; i++)
	{
		const SliderSetting& setting = SliderSettings[i];
		SliderRow& row = m_Sliders[i];
		UIRect rect = FirstSliderRect;
		rect.y += SliderIntervalY * i;

		row.slider.Init();
		row.slider.SetRect(rect);
		row.slider.SetValueRange(setting.minValue, setting.maxValue);

		const UIRect labelRect{
			rect.x, rect.y + SliderLabelOffsetY, setting.labelWidth, SliderLabelHeight
		};
		SetupLabel(row.label, labelRect, setting.labelPixels);
	}

	for (int i = 0; i < ButtonCount; i++)
	{
		const ButtonLayout& button = ButtonLayouts[i];
		const UIRect labelRect{
			button.rect.x, button.rect.y + ButtonLabelOffsetY,
			button.labelWidth, ButtonLabelHeight
		};
		SetupLabel(m_ButtonLabels[i], labelRect, button.labelPixels);
	}
}

void OptionUI::Open()
{
	m_IsOpen = true;
	m_DraggingSlider = -1;
	m_PrevLeftMouseDown = IsLeftMouseDown();

	LoadSliderValues();
	SetCursorLockEnabled(false);
}

void OptionUI::Close()
{
	m_IsOpen = false;
	m_DraggingSlider = -1;
	m_PrevLeftMouseDown = false;

	SetCursorLockEnabled(true);
}

void OptionUI::Toggle()
{
	if (m_IsOpen)
	{
		Close();
	}
	else
	{
		Open();
	}
}

bool OptionUI::IsOpen() const
{
	return m_IsOpen;
}

void OptionUI::Update()
{
	if (Input::GetKeyTrigger(VK_ESCAPE))
	{
		Toggle();
		return;
	}

	if (!m_IsOpen)
	{
		return;
	}

	UpdateKeyboard();
	if (m_IsOpen)
	{
		UpdateMouse();
	}
}

void OptionUI::UpdateKeyboard()
{
	if (m_DraggingSlider >= 0)
	{
		return;
	}

	const int itemCount = SliderCount + ButtonCount;
	const int direction = (int)Input::GetKeyTrigger(VK_DOWN) - (int)Input::GetKeyTrigger(VK_UP);
	m_SelectedIndex = (m_SelectedIndex + direction + itemCount) % itemCount;

	if (m_SelectedIndex < SliderCount)
	{
		const int adjustment = (int)Input::GetKeyPress(VK_RIGHT) - (int)Input::GetKeyPress(VK_LEFT);
		if (adjustment != 0)
		{
			UISlider& slider = m_Sliders[m_SelectedIndex].slider;
			slider.SetValue(slider.GetValue() + SliderSettings[m_SelectedIndex].step * adjustment);
			ApplySliderValue(m_SelectedIndex);
		}
		return;
	}

	if (Input::GetKeyTrigger(VK_RETURN) || Input::GetKeyTrigger(VK_SPACE))
	{
		ExecuteButton(m_SelectedIndex - SliderCount);
	}
}

void OptionUI::UpdateMouse()
{
	const bool leftDown = IsLeftMouseDown();
	const bool leftPressed = leftDown && !m_PrevLeftMouseDown;
	m_PrevLeftMouseDown = leftDown;

	const HWND window = GetWindow();
	if (!leftDown || GetActiveWindow() != window)
	{
		m_DraggingSlider = -1;
		return;
	}

	float mouseX = 0.0f;
	float mouseY = 0.0f;
	if (!ReadMousePosition(window, mouseX, mouseY))
	{
		return;
	}

	if (m_DraggingSlider >= 0)
	{
		m_Sliders[m_DraggingSlider].slider.SetValueFromMouse(mouseX);
		ApplySliderValue(m_DraggingSlider);
		return;
	}

	if (!leftPressed)
	{
		return;
	}

	for (int i = 0; i < SliderCount; i++)
	{
		UISlider& slider = m_Sliders[i].slider;
		if (slider.HitTest(mouseX, mouseY))
		{
			m_SelectedIndex = i;
			m_DraggingSlider = i;
			slider.SetValueFromMouse(mouseX);
			ApplySliderValue(i);
			return;
		}
	}

	for (int i = 0; i < ButtonCount; i++)
	{
		if (ButtonLayouts[i].rect.Contains(mouseX, mouseY))
		{
			m_SelectedIndex = SliderCount + i;
			ExecuteButton(i);
			return;
		}
	}
}

void OptionUI::LoadSliderValues()
{
	for (int i = 0; i < SliderCount; i++)
	{
		m_Sliders[i].slider.SetValue(*SliderSettings[i].value);
	}
}

void OptionUI::ApplySliderValue(int index)
{
	const SliderSetting& setting = SliderSettings[index];
	const float value = m_Sliders[index].slider.GetValue();
	if (value != *setting.value)
	{
		setting.apply(value);
	}
}

void OptionUI::ExecuteButton(int index)
{
	switch (ButtonLayouts[index].action)
	{
	case ButtonAction::Reset:
		GameData::InitOption();
		LoadSliderValues();
		break;

	case ButtonAction::ReturnTitle:
		Close();
		Manager::SetScene<Title>();
		break;

	case ButtonAction::ExitGame:
		Close();
		PostMessage(GetWindow(), WM_CLOSE, 0, 0);
		break;
	}
}

void OptionUI::Draw()
{
	if (!m_IsOpen)
	{
		return;
	}

	DrawRect2D(PanelRect, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.72f));
	DrawRect2D(HeaderRect, XMFLOAT4(0.05f, 0.15f, 0.25f, 0.72f));

	for (int i = 0; i < SliderCount; i++)
	{
		SliderRow& row = m_Sliders[i];
		const bool selected = m_SelectedIndex == i;
		row.label.SetColor(selected ? 0.0f : 1.0f, 1.0f, 1.0f, 1.0f);
		row.label.Draw();
		row.slider.SetSelected(selected);
		row.slider.Draw();
	}

	for (int i = 0; i < ButtonCount; i++)
	{
		const ButtonLayout& button = ButtonLayouts[i];
		const bool selected = m_SelectedIndex == SliderCount + i;
		const bool isExit = button.action == ButtonAction::ExitGame;
		UITextureText& label = m_ButtonLabels[i];

		DrawRect2D(button.rect, GetButtonColor(selected, isExit));
		if (selected && isExit)
		{
			label.SetColor(1.0f, 0.25f, 0.25f, 1.0f);
		}
		else
		{
			label.SetColor(selected ? 0.0f : 1.0f, 1.0f, 1.0f, 1.0f);
		}
		label.Draw();
	}
}
