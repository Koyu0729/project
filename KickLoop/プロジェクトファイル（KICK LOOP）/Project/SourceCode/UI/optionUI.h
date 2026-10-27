#pragma once

#include "UIQuadBase.h"
#include "UISlider.h"
#include "UITextureText.h"

class OptionUI : public UIQuadBase
{
private:
	enum class SelectItem
	{
		MasterVolume,
		BgmVolume,
		SeVolume,
		VoiceVolume,
		MouseSensitivity,

		Reset,
		CloseOption,
		ReturnTitle,
		ExitGame,

		Max
	};

private:
	bool m_IsOpen = false;
	int m_SelectIndex = 0;
	bool m_PrevLeftMouseDown = false;


	float m_MinVolume = 0.0f;
	float m_MaxVolume = 1.0f;

	float m_MinMouseSensitivity = 0.1f;
	float m_MaxMouseSensitivity = 10.0f;

	UISlider* m_MasterSlider = nullptr;
	UISlider* m_BgmSlider = nullptr;
	UISlider* m_SeSlider = nullptr;
	UISlider* m_VoiceSlider = nullptr;
	UISlider* m_MouseSensitivitySlider = nullptr;

	UITextureText* m_MasterText = nullptr;
	UITextureText* m_BgmText = nullptr;
	UITextureText* m_SeText = nullptr;
	UITextureText* m_VoiceText = nullptr;
	UITextureText* m_MouseSensitivityText = nullptr;

	UITextureText* m_ResetText = nullptr;
	UITextureText* m_CloseOptionText = nullptr;
	UITextureText* m_ReturnTitleText = nullptr;
	UITextureText* m_ExitGameText = nullptr;

private:
	void CreateParts();
	void DeleteParts();

	void SetupSlider(
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
	);

	void SetupText(
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
	);

	void UpdateInput();
	void UpdateSliderMouseSelect();
	void UpdateParts();
	void UpdateButtonMouseInput();
	void DrawParts();

	void UpdateSelectVisual();
	void ApplyOption();

	bool IsSliderItem() const;
	UISlider* GetSelectedSlider() const;

	void ExecuteSelectedItem();
	void ResetOptionValues();

	bool IsMouseInRect(
		float mouseX,
		float mouseY,
		float x,
		float y,
		float width,
		float height
	) const;

	void DrawRect(
		float x,
		float y,
		float width,
		float height,
		float r,
		float g,
		float b,
		float a
	);

	void DrawButton(
		float x,
		float y,
		float width,
		float height,
		bool selected,
		bool isDanger
	);

	float Clamp(float value, float minValue, float maxValue) const;

	void SetMouseCursorVisible(bool visible);

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void Open();
	void Close();
	void Toggle();

	bool IsOpen() const;
};