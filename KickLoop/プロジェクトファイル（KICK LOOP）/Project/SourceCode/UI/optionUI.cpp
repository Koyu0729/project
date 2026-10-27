#include "optionUI.h"

#include "input.h"
#include "GameData.h"

#include "main.h"
#include "manager.h"

#include "title.h"

#include "mouseInput.h"

#include <Windows.h>

void OptionUI::Init()
{
	CreateQuad();

	SetTexture("asset\\texture\\frameBoxR.png");

	m_IsOpen = false;
	m_SelectIndex = 0;
	m_PrevLeftMouseDown = false;

	CreateParts();
	UpdateSelectVisual();

	SetActive(true);
	SetVisible(true);
}

void OptionUI::Uninit()
{
	SetMouseCursorVisible(false);

	DeleteParts();

	UIQuadBase::Uninit();
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

	UpdateInput();
	UpdateSliderMouseSelect();
	UpdateParts();
	UpdateButtonMouseInput();
	UpdateSelectVisual();
	ApplyOption();
}

void OptionUI::Draw()
{
	if (!m_IsOpen)
	{
		return;
	}

	SetDraw2D();

	DrawRect(
		190.0f,
		25.0f,
		900.0f,
		675.0f,
		0.0f,
		0.0f,
		0.0f,
		0.72f
	);

	DrawRect(
		190.0f,
		25.0f,
		900.0f,
		70.0f,
		0.05f,
		0.15f,
		0.25f,
		0.72f
	);

	DrawParts();

	const float backgroundX = 190.0f;
	const float backgroundWidth = 900.0f;

	const float buttonHeight = 36.0f;

	const float resetX = backgroundX + 35.0f;
	const float resetY = 650.0f;
	const float resetWidth = 160.0f;

	const float rightButtonX = backgroundX + backgroundWidth - 260.0f;
	const float rightButtonWidth = 230.0f;

	const float returnY = 608.0f;
	const float exitY = 650.0f;

	DrawButton(
		resetX,
		resetY,
		resetWidth,
		buttonHeight,
		m_SelectIndex == (int)SelectItem::Reset,
		false
	);

	DrawButton(
		rightButtonX,
		returnY,
		rightButtonWidth,
		buttonHeight,
		m_SelectIndex == (int)SelectItem::ReturnTitle,
		false
	);

	DrawButton(
		rightButtonX,
		exitY,
		rightButtonWidth,
		buttonHeight,
		m_SelectIndex == (int)SelectItem::ExitGame,
		true
	);
}

void OptionUI::Open()
{
	m_IsOpen = true;
	m_PrevLeftMouseDown = false;

	SetCursorLockEnabled(false);

	if (m_MasterSlider != nullptr)
	{
		m_MasterSlider->SetValue(GameData::MasterVolume);
	}

	if (m_BgmSlider != nullptr)
	{
		m_BgmSlider->SetValue(GameData::BgmVolume);
	}

	if (m_SeSlider != nullptr)
	{
		m_SeSlider->SetValue(GameData::SeVolume);
	}

	if (m_VoiceSlider != nullptr)
	{
		m_VoiceSlider->SetValue(GameData::VoiceVolume);
	}

	if (m_MouseSensitivitySlider != nullptr)
	{
		m_MouseSensitivitySlider->SetValue(GameData::MouseSensitivity);
	}

	UpdateSelectVisual();
}

void OptionUI::Close()
{
	m_IsOpen = false;
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

void OptionUI::CreateParts()
{
	const float sliderX = 420.0f;
	const float startY = 165.0f;

	const float barWidth = 430.0f;
	const float barHeight = 22.0f;

	const float intervalY = 68.0f;

	const float textX = sliderX;
	const float textYAdjust = -40.0f;
	const float textHeight = 34.0f;

	const float optionTextWidth = 653.0f;
	const float optionTextHeight = 1259.0f;

	struct TextUv
	{
		float left;
		float top;
		float right;
		float bottom;
	};

	struct SliderRow
	{
		UISlider** slider;
		UITextureText** text;

		float value;
		float minValue;
		float maxValue;
		float step;

		float colorR;
		float colorG;
		float colorB;

		float textWidth;
		TextUv uv;
	};

	SliderRow sliderRows[] =
	{
		{
			&m_MasterSlider,
			&m_MasterText,

			GameData::MasterVolume,
			m_MinVolume,
			m_MaxVolume,
			0.01f,

			0.2f,
			0.9f,
			1.0f,

			250.0f,
			{ 8.0f, 8.0f, 503.0f, 113.0f }
		},

		{
			&m_BgmSlider,
			&m_BgmText,

			GameData::BgmVolume,
			m_MinVolume,
			m_MaxVolume,
			0.01f,

			0.2f,
			0.7f,
			1.0f,

			205.0f,
			{ 9.0f, 149.0f, 398.0f, 255.0f }
		},

		{
			&m_SeSlider,
			&m_SeText,

			GameData::SeVolume,
			m_MinVolume,
			m_MaxVolume,
			0.01f,

			0.2f,
			1.0f,
			0.5f,

			190.0f,
			{ 9.0f, 292.0f, 363.0f, 397.0f }
		},

		{
			&m_VoiceSlider,
			&m_VoiceText,

			GameData::VoiceVolume,
			m_MinVolume,
			m_MaxVolume,
			0.01f,

			1.0f,
			0.6f,
			0.9f,

			230.0f,
			{ 8.0f, 434.0f, 468.0f, 538.0f }
		},

		{
			&m_MouseSensitivitySlider,
			&m_MouseSensitivityText,

			GameData::MouseSensitivity,
			m_MinMouseSensitivity,
			m_MaxMouseSensitivity,
			0.05f,

			1.0f,
			0.95f,
			0.2f,

			320.0f,
			{ 8.0f, 575.0f, 644.0f, 681.0f }
		},
	};

	const int sliderRowCount = sizeof(sliderRows) / sizeof(sliderRows[0]);

	for (int i = 0; i < sliderRowCount; i++)
	{
		const float sliderY = startY + intervalY * i;
		const float labelY = sliderY + textYAdjust;

		SetupSlider(
			*sliderRows[i].slider,
			sliderX,
			sliderY,
			barWidth,
			barHeight,
			sliderRows[i].minValue,
			sliderRows[i].maxValue,
			sliderRows[i].value,
			sliderRows[i].step,
			sliderRows[i].colorR,
			sliderRows[i].colorG,
			sliderRows[i].colorB
		);

		SetupText(
			*sliderRows[i].text,
			textX,
			labelY,
			sliderRows[i].textWidth,
			textHeight,
			optionTextWidth,
			optionTextHeight,
			sliderRows[i].uv.left,
			sliderRows[i].uv.top,
			sliderRows[i].uv.right,
			sliderRows[i].uv.bottom
		);
	}

	const float backgroundX = 190.0f;
	const float backgroundWidth = 900.0f;

	const float resetX = backgroundX + 35.0f;
	const float resetY = 650.0f;

	const float rightButtonX = backgroundX + backgroundWidth - 260.0f;
	const float returnY = 608.0f;
	const float exitY = 650.0f;

	struct ButtonTextRow
	{
		UITextureText** text;
		float x;
		float y;
		float width;
		float height;
		TextUv uv;
	};

	ButtonTextRow buttonTextRows[] =
	{
		{
			&m_ResetText,
			resetX,
			resetY + 2.0f,
			115.0f,
			30.0f,
			{ 8.0f, 718.0f, 224.0f, 823.0f }
		},

		{
			&m_ReturnTitleText,
			rightButtonX,
			returnY + 2.0f,
			225.0f,
			30.0f,
			{ 8.0f, 1001.0f, 468.0f, 1106.0f }
		},

		{
			&m_ExitGameText,
			rightButtonX,
			exitY + 2.0f,
			185.0f,
			30.0f,
			{ 9.0f, 1144.0f, 363.0f, 1250.0f }
		},
	};

	const int buttonTextCount = sizeof(buttonTextRows) / sizeof(buttonTextRows[0]);

	for (int i = 0; i < buttonTextCount; i++)
	{
		SetupText(
			*buttonTextRows[i].text,
			buttonTextRows[i].x,
			buttonTextRows[i].y,
			buttonTextRows[i].width,
			buttonTextRows[i].height,
			optionTextWidth,
			optionTextHeight,
			buttonTextRows[i].uv.left,
			buttonTextRows[i].uv.top,
			buttonTextRows[i].uv.right,
			buttonTextRows[i].uv.bottom
		);
	}
}

void OptionUI::DeleteParts()
{
	if (m_MasterSlider != nullptr)
	{
		m_MasterSlider->Uninit();
		delete m_MasterSlider;
		m_MasterSlider = nullptr;
	}

	if (m_BgmSlider != nullptr)
	{
		m_BgmSlider->Uninit();
		delete m_BgmSlider;
		m_BgmSlider = nullptr;
	}

	if (m_SeSlider != nullptr)
	{
		m_SeSlider->Uninit();
		delete m_SeSlider;
		m_SeSlider = nullptr;
	}

	if (m_VoiceSlider != nullptr)
	{
		m_VoiceSlider->Uninit();
		delete m_VoiceSlider;
		m_VoiceSlider = nullptr;
	}

	if (m_MouseSensitivitySlider != nullptr)
	{
		m_MouseSensitivitySlider->Uninit();
		delete m_MouseSensitivitySlider;
		m_MouseSensitivitySlider = nullptr;
	}

	if (m_MasterText != nullptr)
	{
		m_MasterText->Uninit();
		delete m_MasterText;
		m_MasterText = nullptr;
	}

	if (m_BgmText != nullptr)
	{
		m_BgmText->Uninit();
		delete m_BgmText;
		m_BgmText = nullptr;
	}

	if (m_SeText != nullptr)
	{
		m_SeText->Uninit();
		delete m_SeText;
		m_SeText = nullptr;
	}

	if (m_VoiceText != nullptr)
	{
		m_VoiceText->Uninit();
		delete m_VoiceText;
		m_VoiceText = nullptr;
	}

	if (m_MouseSensitivityText != nullptr)
	{
		m_MouseSensitivityText->Uninit();
		delete m_MouseSensitivityText;
		m_MouseSensitivityText = nullptr;
	}

	if (m_ResetText != nullptr)
	{
		m_ResetText->Uninit();
		delete m_ResetText;
		m_ResetText = nullptr;
	}

	if (m_CloseOptionText != nullptr)
	{
		m_CloseOptionText->Uninit();
		delete m_CloseOptionText;
		m_CloseOptionText = nullptr;
	}

	if (m_ReturnTitleText != nullptr)
	{
		m_ReturnTitleText->Uninit();
		delete m_ReturnTitleText;
		m_ReturnTitleText = nullptr;
	}

	if (m_ExitGameText != nullptr)
	{
		m_ExitGameText->Uninit();
		delete m_ExitGameText;
		m_ExitGameText = nullptr;
	}
}

void OptionUI::SetupSlider(
	UISlider*& slider,
	float x,
	float y,
	float width,
	float height,
	float minValue,
	float maxValue,
	float value,
	float step,
	float r,
	float g,
	float b
)
{
	slider = new UISlider();
	slider->Init();
	slider->SetRect(
		x,
		y,
		width,
		height
	);

	slider->SetValueRange(
		minValue,
		maxValue
	);

	slider->SetValue(
		value
	);

	slider->SetStep(
		step
	);

	// 通常時は全部白
	slider->SetBarColor(
		1.0f,
		1.0f,
		1.0f
	);

	// 選択中・ドラッグ中は黄色
	slider->SetSelectedColor(
		1.0f,
		1.0f,
		0.0f
	);
}

void OptionUI::SetupText(
	UITextureText*& text,
	float x,
	float y,
	float width,
	float height,
	float textureWidth,
	float textureHeight,
	float left,
	float top,
	float right,
	float bottom
)
{
	text = new UITextureText();
	text->Init();
	text->SetTexturePath("asset\\texture\\option.png");
	text->SetRect(x, y, width, height);
	text->SetUVPixel(textureWidth, textureHeight, left, top, right, bottom);
	text->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void OptionUI::UpdateInput()
{
	if (Input::GetKeyTrigger(VK_UP))
	{
		m_SelectIndex--;

		if (m_SelectIndex < 0)
		{
			m_SelectIndex = (int)SelectItem::Max - 1;
		}
	}

	if (Input::GetKeyTrigger(VK_DOWN))
	{
		m_SelectIndex++;

		if (m_SelectIndex >= (int)SelectItem::Max)
		{
			m_SelectIndex = 0;
		}
	}

	if (IsSliderItem())
	{
		UISlider* selectedSlider = GetSelectedSlider();

		if (selectedSlider != nullptr)
		{
			if (Input::GetKeyPress(VK_LEFT))
			{
				selectedSlider->AddValue(-selectedSlider->GetStep());
			}

			if (Input::GetKeyPress(VK_RIGHT))
			{
				selectedSlider->AddValue(selectedSlider->GetStep());
			}
		}
	}

	if (Input::GetKeyTrigger(VK_RETURN))
	{
		ExecuteSelectedItem();
	}

	if (Input::GetKeyTrigger(VK_SPACE))
	{
		ExecuteSelectedItem();
	}
}

void OptionUI::UpdateSliderMouseSelect()
{
	if (!Mouse::LeftClick())
	{
		return;
	}

	Vector3 mousePosition =
		Mouse::Pos();

	float mouseX = mousePosition.x;
	float mouseY = mousePosition.z;

	const float sliderX = 420.0f;
	const float startY = 165.0f;
	const float barWidth = 430.0f;
	const float barHeight = 22.0f;
	const float intervalY = 68.0f;

	for (int i = 0; i < 5; i++)
	{
		float sliderY = startY + intervalY * i;

		if (IsMouseInRect(mouseX, mouseY, sliderX, sliderY, barWidth, barHeight))
		{
			m_SelectIndex = i;
			return;
		}
	}
}

void OptionUI::UpdateParts()
{
	UISlider* selectedSlider =
		GetSelectedSlider();

	if (selectedSlider == nullptr)
	{
		return;
	}

	selectedSlider->Update();

	if (m_MasterSlider != nullptr && m_MasterSlider->IsDragging())
	{
		m_SelectIndex = (int)SelectItem::MasterVolume;
	}

	if (m_BgmSlider != nullptr && m_BgmSlider->IsDragging())
	{
		m_SelectIndex = (int)SelectItem::BgmVolume;
	}

	if (m_SeSlider != nullptr && m_SeSlider->IsDragging())
	{
		m_SelectIndex = (int)SelectItem::SeVolume;
	}

	if (m_VoiceSlider != nullptr && m_VoiceSlider->IsDragging())
	{
		m_SelectIndex = (int)SelectItem::VoiceVolume;
	}

	if (m_MouseSensitivitySlider != nullptr && m_MouseSensitivitySlider->IsDragging())
	{
		m_SelectIndex = (int)SelectItem::MouseSensitivity;
	}
}

void OptionUI::UpdateButtonMouseInput()
{
	POINT mousePosition{};

	GetCursorPos(
		&mousePosition
	);

	ScreenToClient(
		GetWindow(),
		&mousePosition
	);

	float mouseX = (float)mousePosition.x;
	float mouseY = (float)mousePosition.y;

	bool isLeftMouseDown =
		(GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

	bool isLeftMouseTrigger =
		isLeftMouseDown &&
		!m_PrevLeftMouseDown;

	m_PrevLeftMouseDown =
		isLeftMouseDown;

	if (!isLeftMouseTrigger)
	{
		return;
	}

	const float backgroundX = 190.0f;
	const float backgroundWidth = 900.0f;

	const float buttonHeight = 36.0f;

	const float resetX = backgroundX + 35.0f;
	const float resetY = 650.0f;
	const float resetWidth = 160.0f;

	const float rightButtonX = backgroundX + backgroundWidth - 260.0f;
	const float rightButtonWidth = 230.0f;

	const float returnY = 608.0f;
	const float exitY = 650.0f;

	if (IsMouseInRect(
		mouseX,
		mouseY,
		resetX,
		resetY,
		resetWidth,
		buttonHeight
	))
	{
		m_SelectIndex =
			(int)SelectItem::Reset;

		ExecuteSelectedItem();

		return;
	}

	if (IsMouseInRect(
		mouseX,
		mouseY,
		rightButtonX,
		returnY,
		rightButtonWidth,
		buttonHeight
	))
	{
		m_SelectIndex =
			(int)SelectItem::ReturnTitle;

		ExecuteSelectedItem();

		return;
	}

	if (IsMouseInRect(
		mouseX,
		mouseY,
		rightButtonX,
		exitY,
		rightButtonWidth,
		buttonHeight
	))
	{
		m_SelectIndex =
			(int)SelectItem::ExitGame;

		ExecuteSelectedItem();

		return;
	}
}

void OptionUI::DrawParts()
{
	if (m_MasterText != nullptr)
	{
		m_MasterText->Draw();
	}

	if (m_BgmText != nullptr)
	{
		m_BgmText->Draw();
	}

	if (m_SeText != nullptr)
	{
		m_SeText->Draw();
	}

	if (m_VoiceText != nullptr)
	{
		m_VoiceText->Draw();
	}

	if (m_MouseSensitivityText != nullptr)
	{
		m_MouseSensitivityText->Draw();
	}

	if (m_MasterSlider != nullptr)
	{
		m_MasterSlider->Draw();
	}

	if (m_BgmSlider != nullptr)
	{
		m_BgmSlider->Draw();
	}

	if (m_SeSlider != nullptr)
	{
		m_SeSlider->Draw();
	}

	if (m_VoiceSlider != nullptr)
	{
		m_VoiceSlider->Draw();
	}

	if (m_MouseSensitivitySlider != nullptr)
	{
		m_MouseSensitivitySlider->Draw();
	}

	if (m_ResetText != nullptr)
	{
		m_ResetText->Draw();
	}

	if (m_CloseOptionText != nullptr)
	{
		m_CloseOptionText->Draw();
	}

	if (m_ReturnTitleText != nullptr)
	{
		m_ReturnTitleText->Draw();
	}

	if (m_ExitGameText != nullptr)
	{
		m_ExitGameText->Draw();
	}
}

void OptionUI::UpdateSelectVisual()
{
	if (m_MasterSlider != nullptr)
	{
		m_MasterSlider->SetSelected(m_SelectIndex == (int)SelectItem::MasterVolume);
	}

	if (m_BgmSlider != nullptr)
	{
		m_BgmSlider->SetSelected(m_SelectIndex == (int)SelectItem::BgmVolume);
	}

	if (m_SeSlider != nullptr)
	{
		m_SeSlider->SetSelected(m_SelectIndex == (int)SelectItem::SeVolume);
	}

	if (m_VoiceSlider != nullptr)
	{
		m_VoiceSlider->SetSelected(m_SelectIndex == (int)SelectItem::VoiceVolume);
	}

	if (m_MouseSensitivitySlider != nullptr)
	{
		m_MouseSensitivitySlider->SetSelected(m_SelectIndex == (int)SelectItem::MouseSensitivity);
	}

	if (m_MasterText != nullptr)
	{
		m_MasterText->SetColor(m_SelectIndex == (int)SelectItem::MasterVolume ? 0.0f : 1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (m_BgmText != nullptr)
	{
		m_BgmText->SetColor(m_SelectIndex == (int)SelectItem::BgmVolume ? 0.0f : 1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (m_SeText != nullptr)
	{
		m_SeText->SetColor(m_SelectIndex == (int)SelectItem::SeVolume ? 0.0f : 1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (m_VoiceText != nullptr)
	{
		m_VoiceText->SetColor(m_SelectIndex == (int)SelectItem::VoiceVolume ? 0.0f : 1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (m_MouseSensitivityText != nullptr)
	{
		m_MouseSensitivityText->SetColor(m_SelectIndex == (int)SelectItem::MouseSensitivity ? 0.0f : 1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (m_ResetText != nullptr)
	{
		m_ResetText->SetColor(m_SelectIndex == (int)SelectItem::Reset ? 0.0f : 1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (m_CloseOptionText != nullptr)
	{
		m_CloseOptionText->SetColor(m_SelectIndex == (int)SelectItem::CloseOption ? 0.0f : 1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (m_ReturnTitleText != nullptr)
	{
		m_ReturnTitleText->SetColor(m_SelectIndex == (int)SelectItem::ReturnTitle ? 0.0f : 1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (m_ExitGameText != nullptr)
	{
		if (m_SelectIndex == (int)SelectItem::ExitGame)
		{
			m_ExitGameText->SetColor(1.0f, 0.25f, 0.25f, 1.0f);
		}
		else
		{
			m_ExitGameText->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
}

void OptionUI::ApplyOption()
{
	if (m_MasterSlider != nullptr)
	{
		GameData::SetMasterVolume(Clamp(m_MasterSlider->GetValue(), m_MinVolume, m_MaxVolume));
	}

	if (m_BgmSlider != nullptr)
	{
		GameData::SetBgmVolume(Clamp(m_BgmSlider->GetValue(), m_MinVolume, m_MaxVolume));
	}

	if (m_SeSlider != nullptr)
	{
		GameData::SetSeVolume(Clamp(m_SeSlider->GetValue(), m_MinVolume, m_MaxVolume));
	}

	if (m_VoiceSlider != nullptr)
	{
		GameData::SetVoiceVolume(Clamp(m_VoiceSlider->GetValue(), m_MinVolume, m_MaxVolume));
	}

	if (m_MouseSensitivitySlider != nullptr)
	{
		GameData::SetMouseSensitivity(Clamp(m_MouseSensitivitySlider->GetValue(), m_MinMouseSensitivity, m_MaxMouseSensitivity));
	}
}

bool OptionUI::IsSliderItem() const
{
	switch ((SelectItem)m_SelectIndex)
	{
	case SelectItem::MasterVolume:
	case SelectItem::BgmVolume:
	case SelectItem::SeVolume:
	case SelectItem::VoiceVolume:
	case SelectItem::MouseSensitivity:
		return true;

	default:
		return false;
	}
}

UISlider* OptionUI::GetSelectedSlider() const
{
	switch ((SelectItem)m_SelectIndex)
	{
	case SelectItem::MasterVolume:
		return m_MasterSlider;

	case SelectItem::BgmVolume:
		return m_BgmSlider;

	case SelectItem::SeVolume:
		return m_SeSlider;

	case SelectItem::VoiceVolume:
		return m_VoiceSlider;

	case SelectItem::MouseSensitivity:
		return m_MouseSensitivitySlider;

	default:
		return nullptr;
	}
}

void OptionUI::ExecuteSelectedItem()
{
	switch ((SelectItem)m_SelectIndex)
	{
	case SelectItem::Reset:
		ResetOptionValues();
		break;

	case SelectItem::CloseOption:
		Close();
		break;

	case SelectItem::ReturnTitle:
		Close();
		Manager::SetScene<Title>();
		break;

	case SelectItem::ExitGame:
		PostMessage(GetWindow(), WM_CLOSE, 0, 0);
		break;

	default:
		break;
	}
}

void OptionUI::ResetOptionValues()
{
	if (m_MasterSlider != nullptr)
	{
		m_MasterSlider->SetValue(1.0f);
	}

	if (m_BgmSlider != nullptr)
	{
		m_BgmSlider->SetValue(0.8f);
	}

	if (m_SeSlider != nullptr)
	{
		m_SeSlider->SetValue(0.80f);
	}

	if (m_VoiceSlider != nullptr)
	{
		m_VoiceSlider->SetValue(0.8f);
	}

	if (m_MouseSensitivitySlider != nullptr)
	{
		m_MouseSensitivitySlider->SetValue(0.8f);
	}

	ApplyOption();
}

bool OptionUI::IsMouseInRect(
	float mouseX,
	float mouseY,
	float x,
	float y,
	float width,
	float height
) const
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

void OptionUI::DrawRect(
	float x,
	float y,
	float width,
	float height,
	float r,
	float g,
	float b,
	float a
)
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

void OptionUI::DrawButton(
	float x,
	float y,
	float width,
	float height,
	bool selected,
	bool isDanger
)
{
	if (isDanger)
	{
		DrawRect(
			x,
			y,
			width,
			height,
			selected ? 0.95f : 0.55f,
			0.15f,
			0.15f,
			selected ? 0.95f : 0.70f
		);
	}
	else
	{
		DrawRect(
			x,
			y,
			width,
			height,
			selected ? 0.0f : 0.20f,
			selected ? 0.75f : 0.45f,
			selected ? 1.0f : 0.65f,
			selected ? 0.95f : 0.70f
		);
	}
}

float OptionUI::Clamp(float value, float minValue, float maxValue) const
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

void OptionUI::SetMouseCursorVisible(bool visible)
{
	if (visible)
	{
		while (ShowCursor(TRUE) < 0)
		{
		}
	}
	else
	{
		while (ShowCursor(FALSE) >= 0)
		{
		}
	}
}